/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 UFRGS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"

#include "ancp-header.h"
#include "ancp-nas-agent.h"

NS_LOG_COMPONENT_DEFINE("AncpNasAgent");
namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(AncpNasAgent);

TypeId
AncpNasAgent::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AncpNasAgent")
                      .SetParent<Application> ()
                      .AddConstructor<AncpNasAgent> ()
                      .AddAttribute("LocalAddress",
                                    "Local agent IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&AncpNasAgent::m_MyAddr),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("PortUpCallback", "Callback invoked on PortUp events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpNasAgent::m_portUpHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("PortDownCallback", "Callback invoked on PortDown events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpNasAgent::m_portDownHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("MCastCacCallback", "Callback invoked on Multicast Admission CAC events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpNasAgent::m_admissionCacHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("NewAdjCallback", "Callback invoked on New ANCP adjacency events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpNasAgent::m_newAdjacencyHandler),
                                    MakeCallbackChecker());

  return tid;
}

AncpNasAgent::AncpNasAgent() :
  m_Sock(nullptr),
  m_MyMac(Mac48Address("00:00:00:00:00:00")),
  m_MyAddr(Ipv4Address::GetAny()),
  m_MyPort(0)
{
  NS_LOG_FUNCTION(this << " ANCP NAS Agent on " << GetNode());

  /* FIXME review Capabilities list */
  m_capList.push_back(AncpCapability(AncpCapability::CAP_DSL_TOPO_DISCOVERY, 0, 0));
  m_capList.push_back(AncpCapability(AncpCapability::CAP_DSL_LINE_CONFIG, 0, 0));
  m_capList.push_back(AncpCapability(AncpCapability::CAP_NAS_MCAST_REPLICATION, 0, 0));
}

AncpNasAgent::~AncpNasAgent()
{
  NS_LOG_FUNCTION(this);
}

void AncpNasAgent::Print(std::ostream& os) const
{
}

void AncpNasAgent::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void AncpNasAgent::StartApplication(void)
{
  NS_ABORT_IF(m_MyAddr == Ipv4Address::GetAny());

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  int32_t ifIndex = ipv4->GetInterfaceForAddress(m_MyAddr);

  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to local interface");

  Ptr<NetDevice> netdev = ipv4->GetNetDevice(ifIndex);

  StartNetwork(netdev);
}

void AncpNasAgent::StartNetwork(Ptr<NetDevice> netdev)
{
  NS_LOG_FUNCTION(this << " ANCP NAS Agent bound to " << m_MyAddr);

  m_MyMac = Mac48Address::ConvertFrom(netdev->GetAddress());
  m_MyPort = netdev->GetIfIndex();

  TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
  InetSocketAddress local = InetSocketAddress(m_MyAddr, ANCP_NAS_PORT);

  m_Sock = Socket::CreateSocket(GetNode(), tid);
  m_Sock->Bind(local);
  m_Sock->Listen();

  m_Sock->SetAcceptCallback(
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback(&AncpNasAgent::AcceptHandler, this));
}

void AncpNasAgent::StopApplication(void)
{
  NS_LOG_FUNCTION(this);

  if (m_Sock != nullptr)
    {
      m_Sock->Close();
      m_Sock = nullptr;
    }

  m_AdjList.clear();
}

void AncpNasAgent::AcceptHandler(Ptr<Socket> socket, const Address& from)
{
  NS_LOG_FUNCTION(this << socket << from);

  Ptr<AncpAdjacency> new_adj = Create<AncpAdjacency>(m_MyMac, m_MyPort,
                                                     socket, true);
  m_AdjList.insert(m_AdjList.begin(), std::make_pair(from, new_adj));

  new_adj->SetCapabilities(m_capList);
  new_adj->Start(MakeCallback(&AncpNasAgent::HandleAncpMessage, this));
}

Ptr<AncpAdjacency> AncpNasAgent::GetAdjacency(const Mac48Address &anName) const
{
  for (AdjacencyList::const_iterator it = m_AdjList.begin(); it != m_AdjList.end(); it++)
    {
      Ptr<AncpAdjacency> adj = it->second;

      if (adj->IsEstablished() && adj->GetAdjacencyName() == anName)
        return adj;
    }

  return(nullptr);
}

int AncpNasAgent::HandleAncpMessage(Mac48Address &adj_name, AncpHeader &msg)
{
  NS_LOG_FUNCTION(this << msg);

  std::string circuit_id;

  if (auto tlv_circuit_id = msg.GetTlvByType(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID))
    circuit_id = static_cast<std::string>(*tlv_circuit_id);

  switch (msg.GetMsgType())
    {
    case AncpHeader::MSG_ADJ_PROTOCOL:
      if (!m_newAdjacencyHandler.IsNull())
        return m_newAdjacencyHandler(adj_name);
      break;

    case AncpHeader::MSG_PORT_UP:
      if (const AncpTlvDslAttr *tlvDsl =
            static_cast<const AncpTlvDslAttr *>(msg.GetTlvByType(AncpTlv::TLV_DSL_LINE_ATTRIBUTES)))
        {
          uint32_t rate_up = tlvDsl->GetUpstreamRate();
          uint32_t rate_down = tlvDsl->GetDownstreamRate();
          uint32_t tag_mode = tlvDsl->GetTagMode();

          if (!m_portUpHandler.IsNull())
            return m_portUpHandler(adj_name, circuit_id, rate_up, rate_down, tag_mode);
        }
      break;

    case AncpHeader::MSG_PORT_DOWN:
      if (!m_portDownHandler.IsNull())
        return m_portDownHandler(adj_name, circuit_id);
      break;

    case AncpHeader::MSG_MCAST_ADMISSION_CONTROL:
      if (const AncpTlvMCastCommand *cmd =
            static_cast<const AncpTlvMCastCommand *>(msg.GetTlvByType(AncpTlv::TLV_COMMAND)))
        {
          switch (cmd->GetCommand())
            {
            case AncpTlvMCastCommand::CMD_DELETE:
            case AncpTlvMCastCommand::CMD_DELETE_ALL:
              if (!m_admissionCacHandler.IsNull())
                return m_admissionCacHandler(adj_name, circuit_id, cmd->GetFlowGroup(), false);
              break;

            case AncpTlvMCastCommand::CMD_ADD:
              if (!m_admissionCacHandler.IsNull())
                return m_admissionCacHandler(adj_name, circuit_id, cmd->GetFlowGroup(), true);
              break;

            default:
              NS_LOG_WARN(this << " Don't known how to handle ANCP MCast Admission message");
              break;
            }
        }
      break;

    default:
      NS_LOG_WARN(this << " Don't known how to handle ANCP message");
    }

  return(0);
}

int AncpNasAgent::SendPortConfigCommand(const Mac48Address &anName,
                                        const std::string &circuitId, const std::string &profName)
{
  NS_LOG_FUNCTION(this << anName << circuitId << profName);

  Ptr<AncpAdjacency> adj = GetAdjacency(anName);

  if (adj == nullptr)
    {
      NS_LOG_WARN("Adjacency not found");
      return(-1);
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_PORT_MANAGEMENT);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuitId.length(),
                                (const uint8_t*)circuitId.c_str());
  message.AddTlv(id_tlv);

  /* Profile */
  AncpTlv *prof_tlv = new AncpTlv(AncpTlv::TLV_SRV_PROF_NAME,
                                  profName.length(),
                                  (const uint8_t*)profName.c_str());
  message.AddTlv(prof_tlv);

  adj->SendControlMessage(message);

  return(0);
}

int AncpNasAgent::SendMCastPortConfigCommand(const Mac48Address &anName,
                                             const std::string &circuitId,
                                             const std::string &profName)
{
  NS_LOG_FUNCTION(this << anName << circuitId << profName);
  Ptr<AncpAdjacency> adj = GetAdjacency(anName);

  if (adj == nullptr)
    {
      NS_LOG_WARN("Adjacency not found");
      return(-1);
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_PORT_MANAGEMENT);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuitId.length(),
                                (const uint8_t*)circuitId.c_str());
  message.AddTlv(id_tlv);

  /* Profile */
  AncpTlv *prof_tlv = new AncpTlv(AncpTlv::TLV_MCAST_SRV_PROF_NAME,
                                  profName.length(),
                                  (const uint8_t*)profName.c_str());
  message.AddTlv(prof_tlv);

  return adj->SendControlMessage(message);
}

int AncpNasAgent::SendMCastServiceProfile(const Mac48Address &anName,
                                          const std::string &profName,
                                          const std::list<Address> &whitelist,
                                          const std::list<Address> &greylist,
                                          const std::list<Address> &blacklist,
                                          bool doWhitelistCac,
                                          bool doMRepCtlCac)
{
  NS_LOG_FUNCTION(this << anName << profName);
  Ptr<AncpAdjacency> adj = GetAdjacency(anName);

  if (adj == nullptr)
    {
      NS_LOG_WARN("Adjacency not found");
      return(-1);
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_PROVISIONING);
  message.SetPartitionId(0);

  /* Profile */
  AncpTlvMCastServiceProfile *mcast_tlv = new AncpTlvMCastServiceProfile();

  mcast_tlv->GetFlowName().SetSrvProfileName(profName);

  /* WhiteList */
  if (!whitelist.empty())
    {
      AncpTlvMCastListAction &act = mcast_tlv->GetAction(AncpTlvMCastListAction::WHITELIST);
      act.SetOperation(AncpTlvMCastListAction::ADD);
      act.SetListType(AncpTlvMCastListAction::IPv4); /* FIXME get actual value */
      for (auto &ad: whitelist)
        act.AddFlow(ad);
    }
  /* BlackList */
  if (!blacklist.empty())
    {
      AncpTlvMCastListAction &act = mcast_tlv->GetAction(AncpTlvMCastListAction::BLACKLIST);
      act.SetOperation(AncpTlvMCastListAction::ADD);
      act.SetListType(AncpTlvMCastListAction::IPv4); /* FIXME get actual value */
      for (auto &ad: blacklist)
        act.AddFlow(ad);
    }
  /* GreyList */
  if (!greylist.empty())
    {
      AncpTlvMCastListAction &act = mcast_tlv->GetAction(AncpTlvMCastListAction::GREYLIST);
      act.SetOperation(AncpTlvMCastListAction::ADD);
      act.SetListType(AncpTlvMCastListAction::IPv4); /* FIXME get actual value */
      for (auto &ad: greylist)
        act.AddFlow(ad);
    }

  message.AddTlv(mcast_tlv);

  /* Wants Whitelist CAC ? */
  if (doWhitelistCac)
    {
      AncpTlv *cac_tlv = new AncpTlv(AncpTlv::TLV_WHITE_LIST_CAC);
      message.AddTlv(cac_tlv);
    }

  /* Wants Greylist CAC ? */
  if (doMRepCtlCac)
    {
      AncpTlv *cac_tlv = new AncpTlv(AncpTlv::TLV_MREPCTL_CAC);
      message.AddTlv(cac_tlv);
    }


  return adj->SendControlMessage(message);
}

int AncpNasAgent::SendMCastCommand(const Mac48Address &anName,
                                   const std::string &circuitId,
                                   int command,
                                   const Address &group)
{
  NS_LOG_FUNCTION(this << anName << circuitId << group);
  Ptr<AncpAdjacency> adj = GetAdjacency(anName);

  if (adj == nullptr)
    {
      NS_LOG_WARN("Adjacency not found");
      return(-1);
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_MCAST_REPLICATION_CONTROL);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuitId.length(),
                                (const uint8_t*)circuitId.c_str());
  message.AddTlv(id_tlv);

  AncpTlvMCastCommand* cmd = new AncpTlvMCastCommand();
  cmd->SetCommand(command);
  cmd->SetFlow(group);

  message.AddTlv(cmd);

  return adj->SendControlMessage(message);
}
} // Namespace ns3
