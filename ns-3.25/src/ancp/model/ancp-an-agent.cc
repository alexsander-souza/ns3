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
#include "ancp-an-agent.h"

NS_LOG_COMPONENT_DEFINE("AncpAnAgent");
namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(AncpAnAgent);

TypeId
AncpAnAgent::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AncpAnAgent")
                      .SetParent<Application> ()
                      .AddConstructor<AncpAnAgent> ()
                      .AddAttribute("LocalAddress",
                                    "Local agent IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&AncpAnAgent::m_MyAddr),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("NasAddress",
                                    "NAS IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&AncpAnAgent::m_NasAddr),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("LineConfigCallback", "Callback invoked on LineConfig events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpAnAgent::m_lineConfigHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("MCastProfileCallback", "Callback invoked on Multicast Provisioning events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpAnAgent::m_mcastProfileHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("MCastCommandCallback", "Callback invoked on Multicast Command events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpAnAgent::m_mcastCommandHandler),
                                    MakeCallbackChecker())
                      .AddAttribute("MCastLineConfigCallback", "Callback invoked on Multicast LineConfig events.",
                                    CallbackValue(),
                                    MakeCallbackAccessor(&AncpAnAgent::m_mcastLineConfigHandler),
                                    MakeCallbackChecker());


  return tid;
}

AncpAnAgent::AncpAnAgent() :
  m_MyMac(Mac48Address("00:00:00:00:00:00")),
  m_MyAddr(Ipv4Address::GetAny()),
  m_MyPort(0),
  m_NasAddr(Ipv4Address::GetAny())
{
  NS_LOG_FUNCTION(this << " ANCP AN Agent on " << GetNode());

  /* FIXME review Capabilities list */
  m_capList.push_back(AncpCapability(AncpCapability::CAP_DSL_TOPO_DISCOVERY, 0, 0));
  m_capList.push_back(AncpCapability(AncpCapability::CAP_DSL_LINE_CONFIG, 0, 0));
  m_capList.push_back(AncpCapability(AncpCapability::CAP_CAC_GREY, 0, 0));
  m_capList.push_back(AncpCapability(AncpCapability::CAP_CAC_WHITE_BLACK, 0, 0));
}

AncpAnAgent::~AncpAnAgent()
{
  NS_LOG_FUNCTION(this);
}

void AncpAnAgent::Print(std::ostream& os) const
{
}

void AncpAnAgent::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void AncpAnAgent::StartApplication(void)
{
  NS_ABORT_IF(m_MyAddr == Ipv4Address::GetAny());
  NS_ABORT_IF(m_NasAddr == Ipv4Address::GetAny());

  NS_LOG_FUNCTION(this << " ANCP AN Agent bound on " << m_MyAddr << " to NAS " << m_NasAddr);

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  int32_t ifIndex = ipv4->GetInterfaceForAddress(m_MyAddr);

  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to local interface");

  Ptr<NetDevice> netdev = ipv4->GetNetDevice(ifIndex);
  m_MyMac = Mac48Address::ConvertFrom(netdev->GetAddress());
  m_MyPort = netdev->GetIfIndex();
  Address nas = InetSocketAddress(m_NasAddr, ANCP_NAS_PORT);

  CreateAdjacency(nas);
}

void AncpAnAgent::StopApplication(void)
{
  NS_LOG_FUNCTION(this);

  m_AdjList.clear();
}

Ptr<AncpAdjacency> AncpAnAgent::CreateAdjacency(Address &nas)
{
  NS_LOG_FUNCTION(this << nas);

  AdjacencyList::iterator it = m_AdjList.find(nas);

  if (it != m_AdjList.end())
    {
      // Duplicated?
      return it->second;
    }

  /* Not found, Create new adjacency state */
  TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
  InetSocketAddress local = InetSocketAddress(m_MyAddr);

  Ptr<Socket> sock = Socket::CreateSocket(GetNode(), tid);
  sock->Bind(local);
  sock->Connect(nas);

  Ptr<AncpAdjacency> new_adj = Create<AncpAdjacency>(m_MyMac, m_MyPort,
                                                     sock, false);
  m_AdjList.insert(m_AdjList.begin(), std::make_pair(nas, new_adj));

  new_adj->SetCapabilities(m_capList);
  new_adj->Start(MakeCallback(&AncpAnAgent::HandleAncpMessage, this));

  return new_adj;
}

Ptr<AncpAdjacency> AncpAnAgent::GetActiveAdjacency() const
{
  for (AdjacencyList::const_iterator it = m_AdjList.begin(); it != m_AdjList.end(); it++)
    {
      Ptr<AncpAdjacency> adj = it->second;

      if (adj->IsEstablished())
        return adj;
    }

  return(nullptr);
}

bool AncpAnAgent::IsEstablished() const
{
  const Ptr<AncpAdjacency> adj = GetActiveAdjacency();

  return(adj != nullptr);
}

void AncpAnAgent::SendPortUp(const std::string &circuit_id, uint32_t tag_mode,
                             uint32_t up_rate, uint32_t down_rate) const
{
  Ptr<AncpAdjacency> adj = GetActiveAdjacency();

  if (adj == nullptr)
    {
      NS_LOG_WARN("Discarding message, no active adjacency");
      return;
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_PORT_UP);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuit_id.length(),
                                (const uint8_t*)circuit_id.c_str());
  message.AddTlv(id_tlv);

  /* DSL params */
  AncpTlvDslAttr *dsl_attr = new AncpTlvDslAttr();
  dsl_attr->SetLineRate(up_rate, down_rate);
  dsl_attr->SetTagMode(tag_mode);
  dsl_attr->SetShowtime(true);
  message.AddTlv(dsl_attr);

  adj->SendControlMessage(message);
}

void AncpAnAgent::SendPortDown(const std::string &circuit_id) const
{
  Ptr<AncpAdjacency> adj = GetActiveAdjacency();

  if (adj == nullptr)
    {
      NS_LOG_WARN("Discarding message, no active adjacency");
      return;
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_PORT_DOWN);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuit_id.length(),
                                (const uint8_t*)circuit_id.c_str());
  message.AddTlv(id_tlv);

  adj->SendControlMessage(message);
}

void AncpAnAgent::SendMCastAdmissionCtl(const std::string&circuit_id, int command, const Address &group)
{
  NS_LOG_FUNCTION(this << circuit_id << group << command);
  Ptr<AncpAdjacency> adj = GetActiveAdjacency();

  if (adj == nullptr)
    {
      NS_LOG_WARN("Discarding message, no active adjacency");
      return;
    }

  AncpHeader message;
  message.SetMsgType(AncpHeader::MSG_MCAST_ADMISSION_CONTROL);
  message.SetPartitionId(0);

  /* Circuit ID */
  AncpTlv *id_tlv = new AncpTlv(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID,
                                circuit_id.length(),
                                (const uint8_t*)circuit_id.c_str());
  message.AddTlv(id_tlv);

  AncpTlvMCastCommand *cmd = new AncpTlvMCastCommand();
  cmd->SetCommand(command);
  cmd->SetFlow(group);

  message.AddTlv(cmd);

  adj->SendControlMessage(message);
}

int AncpAnAgent::HandleAncpMessage(Mac48Address &adj_name, AncpHeader &msg)
{
  NS_LOG_FUNCTION(this << msg);

  std::string circuit_id;

  switch (msg.GetMsgType())
    {
    case AncpHeader::MSG_ADJ_PROTOCOL:
      /*ignore */
      break;

    case AncpHeader::MSG_PORT_MANAGEMENT:

      if (auto tlv_circuit_id = msg.GetTlvByType(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID))
        circuit_id = static_cast<std::string>(*tlv_circuit_id);
      else
        return(-1);


      if (auto tlv_prof_name = msg.GetTlvByType(AncpTlv::TLV_SRV_PROF_NAME))
        {
          std::string prof_name = static_cast<std::string>(*tlv_prof_name);
          if (!m_lineConfigHandler.IsNull())
            return m_lineConfigHandler(circuit_id, prof_name);
        }
      else if (auto tlv_prof_mcast = msg.GetTlvByType(AncpTlv::TLV_MCAST_SRV_PROF_NAME))
        {
          std::string prof_name = static_cast<std::string>(*tlv_prof_mcast);
          if (!m_mcastLineConfigHandler.IsNull())
            return m_mcastLineConfigHandler(circuit_id, prof_name);
        }
      break;

    case AncpHeader::MSG_PROVISIONING:
      if (const AncpTlvMCastServiceProfile *mcast_tlv =
            static_cast<const AncpTlvMCastServiceProfile *>(msg.GetTlvByType(AncpTlv::TLV_MCAST_SRV_PROF)))
        {
          std::string prof_name = mcast_tlv->GetFlowName().GetSrvProfileName();

          const AncpTlvMCastListAction &actW = mcast_tlv->GetAction(AncpTlvMCastListAction::WHITELIST);
          std::list<Address> whitelist;
          for (int i = 0; i < actW.GetNFlows(); ++i)
            whitelist.push_back(actW.GetFlow(i));

          const AncpTlvMCastListAction &actG = mcast_tlv->GetAction(AncpTlvMCastListAction::GREYLIST);
          std::list<Address> greylist;
          for (int i = 0; i < actG.GetNFlows(); ++i)
            greylist.push_back(actG.GetFlow(i));

          const AncpTlvMCastListAction &actB = mcast_tlv->GetAction(AncpTlvMCastListAction::BLACKLIST);
          std::list<Address> blacklist;
          for (int i = 0; i < actB.GetNFlows(); ++i)
            blacklist.push_back(actB.GetFlow(i));

          bool doWhitelistCac = (msg.GetTlvByType(AncpTlv::TLV_WHITE_LIST_CAC) != nullptr);
          bool doMRepCtlCac = (msg.GetTlvByType(AncpTlv::TLV_MREPCTL_CAC) != nullptr);

          if (!m_mcastProfileHandler.IsNull())
            return m_mcastProfileHandler(prof_name, whitelist, greylist, blacklist, doWhitelistCac, doMRepCtlCac);
        }
      break;

    case AncpHeader::MSG_MCAST_REPLICATION_CONTROL:
      if (auto tlv_circuit_id = msg.GetTlvByType(AncpTlv::TLV_ACCESS_LOOP_CIRCUIT_ID))
        circuit_id = static_cast<std::string>(*tlv_circuit_id);
      else
        return(-1);

      if (const AncpTlvMCastCommand *cmd =
            static_cast<const AncpTlvMCastCommand *>(msg.GetTlvByType(AncpTlv::TLV_COMMAND)))
        {
          if (!m_mcastCommandHandler.IsNull())
            return m_mcastCommandHandler(circuit_id, cmd->GetCommand(), cmd->GetFlowGroup());
        }
      break;

    default:
      NS_LOG_WARN(this << " Don't known how to handle ANCP message");
    }

  return(0);
}
} // Namespace ns3
