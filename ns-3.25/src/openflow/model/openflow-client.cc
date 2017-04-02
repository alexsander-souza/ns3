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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#include <ns3/log.h>
#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/switch-configuration-header.h"
#include "ns3/physical-port-header.h"
#include "ns3/flow-match-header.h"
#include "ns3/action-header.h"
#include "ns3/flow-modification-header.h"
#include "ns3/packet-in-header.h"
#include "ns3/packet-out-header.h"
#include "ns3/openflow-header.h"
#include "ns3/port-modification-header.h"
#include "ns3/stats-request-header.h"
#include "ns3/flow-stats-request-header.h"
#include "ns3/port-stats-request-header.h"
#include "ns3/stats-reply-header.h"
#include "ns3/flow-stats-header.h"
#include "ns3/port-stats-header.h"
#include "ns3/flow-removed-header.h"
#include "ns3/port-status-header.h"
#include "ns3/error-msg-header.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/abort.h"
#include "ns3/openflow-client.h"

NS_LOG_COMPONENT_DEFINE("OpenflowClient");

namespace ns3 {
using namespace action_utils;

NS_OBJECT_ENSURE_REGISTERED(OpenflowClient);

TypeId
OpenflowClient::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::OpenflowClient")
                      .SetParent<Application>()
                      .SetGroupName("OpenFlow")
                      .AddConstructor<OpenflowClient>()
                      .AddAttribute("LocalAddress",
                                    "Local IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&OpenflowClient::m_myAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("ControllerAddress",
                                    "Address to communicate with the controller using openflow.",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&OpenflowClient::m_ctrlAddress),
                                    MakeIpv4AddressChecker())
  ;

  return tid;
}

OpenflowClient::OpenflowClient() :
  m_xid(0),
  m_ctrlAddress(Ipv4Address::GetAny()),
  m_myAddress(Ipv4Address::GetAny()),
  m_fragment(nullptr)
{
  NS_LOG_FUNCTION(this);
}

OpenflowClient::~OpenflowClient()
{
  NS_LOG_FUNCTION(this);
}

void
OpenflowClient::Print(std::ostream& os) const
{
  os << "[OF_SW" << GetNode()->GetId() << "] ";
}

void
OpenflowClient::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void
OpenflowClient::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  NS_ABORT_IF(m_myAddress == Ipv4Address::GetAny());
  NS_ABORT_IF(m_ctrlAddress == Ipv4Address::GetAny());

  NS_LOG_INFO("Switch Node " << GetNode()->GetId() << " IP: " << m_myAddress);

  /* Get netdev */
  Ptr<Ipv4> ipv4    = GetNode()->GetObject<Ipv4>();
  int32_t   ifIndex = ipv4->GetInterfaceForAddress(m_myAddress);

  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to local interface");

  Ptr<NetDevice> ofNetDev = ipv4->GetNetDevice(ifIndex);
  m_myMacAddress = Mac48Address::ConvertFrom(ofNetDev->GetAddress());
  NS_LOG_INFO("Switch Node " << GetNode()->GetId() << " MAC: " << m_myMacAddress);

  Ptr<NetDevice> switchNetDev;

  TypeId tid = OpenFlowSwitchNetDevice::GetTypeId();

  for (uint32_t i = 0; i < GetNode()->GetNDevices(); ++i)
  {
    if (tid == GetNode()->GetDevice(i)->GetInstanceTypeId())
    {
      switchNetDev = GetNode()->GetDevice(i);
      break;
    }
  }

  m_ofSwNetDev = DynamicCast<OpenFlowSwitchNetDevice>(switchNetDev);

  m_ofSwNetDev->SetAddress(ofNetDev->GetAddress());
  m_ofSwNetDev->SetOpenflowClient(this);

  if (m_socket == 0)
  {
    TypeId tid              = TypeId::LookupByName("ns3::TcpSocketFactory");
    InetSocketAddress local = InetSocketAddress(m_myAddress);

    NS_LOG_INFO("Connect to controller " << m_ctrlAddress << ", local is " << m_myAddress);

    m_socket = Socket::CreateSocket(GetNode(), tid);
    m_socket->Bind(local);
    m_socket->Connect(InetSocketAddress(m_ctrlAddress, OFP_TCP_PORT));
  }

  m_socket->SetRecvCallback(MakeCallback(&OpenflowClient::OpenflowHandler, this));
}

void
OpenflowClient::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
OpenflowClient::SendBasicMessage(uint8_t type)
{
  NS_LOG_FUNCTION(this << int(type));

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(type, m_xid);
  SendOpenflowMessage(openflowHeader);
}

void
OpenflowClient::SendConfigurationReply(uint32_t xid, uint16_t flags, uint16_t maxFlowLen)
{
  NS_LOG_FUNCTION(this << xid);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_GET_CONFIG_REPLY,
                                                              xid);
  Ptr<SwitchConfigurationHeader> swConfigHeader = Create<SwitchConfigurationHeader>(flags, maxFlowLen);

  openflowHeader->SetSwitchConfiguration(swConfigHeader);

  SendOpenflowMessage(openflowHeader);
}

void
OpenflowClient::SendFeaturesReply(long unsigned int portsNum, uint32_t xid, uint64_t datapathId,
                                  uint8_t nTables, uint32_t nBuffers,
                                  OpenFlowSwitchNetDevice::Ports_t ports)
{
  NS_LOG_FUNCTION(this << xid);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_FEATURES_REPLY,
                                                              xid);

  Ptr<SwitchFeaturesHeader> swFeatHeader = Create<SwitchFeaturesHeader>(datapathId, nBuffers, nTables,
                                                                        OFP_SUPPORTED_CAPABILITIES,
                                                                        OFP_SUPPORTED_ACTIONS);

  SwitchFeaturesHeader::PhysicalPortsList portsList;

  CreatePortsList(&portsList, ports);

  swFeatHeader->AddPortsList(portsList);
  openflowHeader->SetSwitchFeatures(swFeatHeader);

  SendOpenflowMessage(openflowHeader);
}

void
OpenflowClient::CreatePortsList(SwitchFeaturesHeader::PhysicalPortsList *portsList,
                                OpenFlowSwitchNetDevice::Ports_t         ports)
{
  for (auto& p: ports)
  {
    uint16_t number = (uint16_t)m_ofSwNetDev->GetSwitchPortIndex(p);
    uint8_t  hwAddress[OFP_ETH_ALEN];

    p.netdev->GetAddress().CopyTo(hwAddress);

    std::string portName = "eth" + std::to_string(number);
    uint8_t     nameBuffer[OFP_MAX_PORT_NAME_LEN];

    std::memset(nameBuffer, 0, sizeof(nameBuffer));
    std::memcpy(nameBuffer, portName.c_str(), portName.size());

    // Os campos zerados estavam assim em OpenflowSwitchNetDevice, ver função
    // FillPortDesc() para mais detalhes
    Ptr<PhysicalPortHeader> phyPortHeader = Create<PhysicalPortHeader>(number, hwAddress, nameBuffer,
                                                                       p.config, p.state, 0, 0, 0, 0);
    portsList->push_back(phyPortHeader);
  }
}

void
OpenflowClient::SendPacketIn(ofpbuf *packetInBuffer)
{
  NS_LOG_FUNCTION(this);

  ofp_packet_in *opi = (ofp_packet_in *)ofpbuf_try_pull(packetInBuffer, offsetof(ofp_packet_in, data));

  ofp_header ofh = opi->header;

  uint32_t xid = ntohl(ofh.xid);

  uint32_t bufferId    = ntohl(opi->buffer_id);
  uint16_t totalLength = ntohs(opi->total_len);
  uint16_t inPort      = ntohs(opi->in_port);
  uint8_t  reason      = opi->reason;

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFP_VERSION1, OFPT_PACKET_IN,
                                                              xid);
  Ptr<PacketInHeader> packetInHeader = Create<PacketInHeader>(bufferId, totalLength, inPort,
                                                              reason, opi->data);

  openflowHeader->SetPacketIn(packetInHeader);

  SendOpenflowMessage(openflowHeader);
}

void
OpenflowClient::SendOpenflowMessage(Ptr<OpenflowHeader>openflowHeader)
{
  NS_LOG_FUNCTION(this);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(*openflowHeader);

  if (m_socket->Send(packet, 0) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);
}

void
OpenflowClient::SendHello(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);

  if (m_xid == xid) {
    NS_LOG_INFO("Hello's handshake finished on client");
    return;
  }

  m_xid = xid;
  SendBasicMessage(OFPT_HELLO);
}

void
OpenflowClient::SendBarrier(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);

  m_xid = xid;
  SendBasicMessage(OFPT_BARRIER_REPLY);
}

void
OpenflowClient::HandleSetConfiguration(ofp_header *ofh, Ptr<SwitchConfigurationHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  ofp_switch_config ofsc;

  ofsc.header        = *ofh;
  ofsc.flags         = header->GetFlags();
  ofsc.miss_send_len = header->GetMaxFlowLen();

  m_ofSwNetDev->ForwardControlInput(&ofsc, ofh->length);
}

void
OpenflowClient::HandleFlowModification(ofp_header *ofh, Ptr<FlowModificationHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  uint16_t actionsLength = ofh->length - sizeof(ofp_flow_mod);
  ofp_flow_mod *ofm      = (ofp_flow_mod *)std::malloc(sizeof(ofp_flow_mod) + actionsLength);

  ofm->header = *ofh;

  ofm->header.version = OFP_VERSION;
  ofm->header.type    = OFPT_FLOW_MOD;
  ofm->header.length  = htons(ofm->header.length);

  Ptr<FlowMatchHeader> matchHeader = header->GetFlowMatch();
  FlowMatchHeader::ConvertToStruct(ofm->match, matchHeader);

  ofm->match.wildcards = htonl(ofm->match.wildcards);
  ofm->match.in_port   = htons(ofm->match.in_port);
  ofm->match.dl_vlan   = htons(ofm->match.dl_vlan);
  ofm->match.dl_type   = htons(ofm->match.dl_type);
  ofm->match.nw_src    = htonl(ofm->match.nw_src);
  ofm->match.nw_dst    = htonl(ofm->match.nw_dst);
  ofm->match.tp_src    = htons(ofm->match.tp_src);
  ofm->match.tp_dst    = htons(ofm->match.tp_dst);
  ofm->command         = htons(header->GetCommand());
  ofm->idle_timeout    = htons(header->GetIdleTimeout());
  ofm->hard_timeout    = htons(header->GetHardTimeout());
  ofm->priority        = header->GetPriority();
  ofm->buffer_id       = htonl(header->GetBufferId());
  ofm->out_port        = header->GetOutPort();

  // cookie not implemented
  // flags not implemented

  Ptr<ActionHeader> actionOutputHeader = header->GetActionByType(OFPAT_OUTPUT);

  ofp_action_output action;
  ActionOutputHeader::ConvertToStruct(action, actionOutputHeader);
  action.type = htons(action.type);
  action.len  = htons(sizeof(ofp_action_output));
  memcpy(ofm->actions, &action, sizeof(ofp_action_output));

  m_ofSwNetDev->ForwardControlInput(ofm, ofm->header.length);

  std::free(ofm);
}

void
OpenflowClient::HandlePacketOut(ofp_header *ofh, Ptr<PacketOutHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  uint16_t actionsLength = ofh->length - sizeof(ofp_packet_out);
  uint16_t dataLength    = header->GetDataLength();

  ofp_packet_out *opo = (ofp_packet_out *)std::malloc(sizeof(ofp_packet_out) + actionsLength + dataLength);

  opo->header = *ofh;

  opo->header.version = OFP_VERSION;
  opo->header.type    = OFPT_PACKET_OUT;
  opo->header.length  = htons(opo->header.length);
  opo->buffer_id      = htonl(header->GetBufferId());
  opo->in_port        = htons(header->GetInPort());
  opo->actions_len    = htons(header->GetActionsLen());

  action_utils::ActionsList actionsList = header->GetActionsList();
  Ptr<ActionHeader> actionOutputHeader  = header->GetActionByType(OFPAT_OUTPUT);

  ofp_action_output action;
  ActionOutputHeader::ConvertToStruct(action, actionOutputHeader);
  action.type = htons(action.type);
  action.len  = htons(sizeof(ofp_action_output));
  memcpy(opo->actions, &action, sizeof(ofp_action_output));

  header->GetData((uint8_t *)opo->actions + ntohs(opo->actions_len), dataLength);

  m_ofSwNetDev->ForwardControlInput(opo, opo->header.length);

  std::free(opo);
}

void
OpenflowClient::HandlePortModification(Ptr<PortModificationHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  header->Print(std::cout);
}

void
OpenflowClient::HandleStatsRequest(uint32_t xid, Ptr<StatsRequestHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  m_xid = xid;
  header->Print(std::cout);

  switch (header->GetType())
  {
  case OFPST_FLOW:
    HandleFlowStatsRequest(header->GetFlowStatsRequest());
    break;

  case OFPST_PORT:
    HandlePortStatsRequest(header->GetPortStatsRequest());
    break;
  }
}

void
OpenflowClient::HandleFlowStatsRequest(Ptr<FlowStatsRequestHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  Ptr<FlowMatchHeader> flowMatchHeader = header->GetFlowMatch();

  // FIXME get actual counters from fabric
  flowMatchHeader->Print(std::cout);
  header->Print(std::cout);
}

void
OpenflowClient::HandlePortStatsRequest(Ptr<PortStatsRequestHeader>header)
{
  NS_LOG_FUNCTION(this << header);

  // FIXME get actual counters from fabric
  header->Print(std::cout);
}

void
OpenflowClient::OpenflowHandler(Ptr<Socket>socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = nullptr;
  Address     from;

  while ((packet = socket->RecvFrom(from)))
  {
    while (packet->GetSize() > 0)
    {
      if (m_fragment != nullptr)
      {
        m_fragment->AddAtEnd(packet);
        packet     = m_fragment;
        m_fragment = nullptr;
      }

      OpenflowHeader openflowHeader;

      if (packet->RemoveHeader(openflowHeader) == 0)
      {
        m_fragment = packet;
        break;
      }

      ofp_header *ofh = (ofp_header *)std::malloc(sizeof(ofp_header));
      OpenflowHeader::ConvertToStruct(*ofh, &openflowHeader);

      switch (openflowHeader.GetType())
      {
      case OFPT_HELLO:
        SendHello(openflowHeader.GetXId());
        break;

      case OFPT_BARRIER_REQUEST:
        SendBarrier(openflowHeader.GetXId());
        break;

      case OFPT_FEATURES_REQUEST:
      case OFPT_GET_CONFIG_REQUEST:
        m_ofSwNetDev->ForwardControlInput(ofh, ofh->length);
        break;

      case OFPT_SET_CONFIG:
        HandleSetConfiguration(ofh, openflowHeader.GetSwitchConfiguration());
        break;

      case OFPT_PACKET_OUT:
        HandlePacketOut(ofh, openflowHeader.GetPacketOut());
        break;

      case OFPT_FLOW_MOD:
        HandleFlowModification(ofh, openflowHeader.GetFlowModification());
        break;

      case OFPT_PORT_MOD:
        HandlePortModification(openflowHeader.GetPortModification());
        break;

      case OFPT_STATS_REQUEST:
        HandleStatsRequest(openflowHeader.GetXId(), openflowHeader.GetStatsRequest());
        break;

      default:
        NS_LOG_WARN(this << " Unsupported OF message received, discarding");
      }

      std::free(ofh);
    }
  }
}

void
OpenflowClient::ReceiveFromNetDevice(ofpbuf *buffer)
{
  uint8_t type = ofi::GetPacketType(buffer);

  NS_LOG_FUNCTION(this << int(type));

  switch (type)
  {
  case OFPT_PACKET_IN:
    SendPacketIn(buffer);
    break;
  }
}

std::ostream&
operator<<(std::ostream& os, const OpenflowClient& client)
{
  client.Print(os);
  return os;
}
} // Namespace ns3
