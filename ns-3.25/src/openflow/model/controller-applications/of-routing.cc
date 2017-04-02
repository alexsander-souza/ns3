/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2015 UFRGS
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

#ifdef NS3_OPENFLOW

#include <ns3/simulator.h>
#include <ns3/log.h>
#include "ns3/uinteger.h"
#include <ns3/mac48-address.h>
#include <ns3/ethernet-header.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>
#include <ns3/icmpv4-l4-protocol.h>
#include <ns3/tcp-l4-protocol.h>
#include <ns3/udp-l4-protocol.h>
#include <ns3/controller-application.h>
#include <ns3/controller.h>
#include <ns3/openflow-header.h>
#include <ns3/flow-match-header.h>
#include <ns3/flow-modification-header.h>
#include <ns3/action-header.h>
#include <ns3/adjacency.h>
#include <ns3/vertex-host.h>
#include <ns3/vertex-switch.h>
#include <ns3/controller-ofswitch.h>
#include "of-routing.h"

NS_LOG_COMPONENT_DEFINE("OFRouting");

namespace ns3 {
TypeId
OFRouting::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::OFRouting")
                      .SetParent(ControllerApplication::GetTypeId())
                      .AddConstructor<OFRouting>()
                      .AddAttribute("NspGw",
                                    "NSP gateway address",
                                    Ipv4AddressValue(Ipv4Address::GetAny()),
                                    MakeIpv4AddressAccessor(&OFRouting::m_nspGw),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("AccessNetPrefix",
                                    "Access network IP prefix",
                                    Ipv4AddressValue(Ipv4Address::GetAny()),
                                    MakeIpv4AddressAccessor(&OFRouting::m_accessNetPrefix),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("AccessNetMask",
                                    "Access network IP prefix mask",
                                    Ipv4MaskValue(Ipv4Mask::GetZero()),
                                    MakeIpv4MaskAccessor(&OFRouting::m_accessNetMask),
                                    MakeIpv4MaskChecker())
                      .AddAttribute("FlowLifetime",
                                    "OF Flow max idle time (in seconds)",
                                    UintegerValue(OFP_FLOW_PERMANENT),
                                    MakeUintegerAccessor(&OFRouting::m_flowLifetime),
                                    MakeUintegerChecker<uint16_t>())
                      .AddAttribute("FlowPriority",
                                    "OF Flow priority (Higher numbers indicate higher prio)",
                                    UintegerValue((OFP_DEFAULT_PRIORITY / 2U)),
                                    MakeUintegerAccessor(&OFRouting::m_flowPrio),
                                    MakeUintegerChecker<uint16_t>())
  ;

  return tid;
}

OFRouting::OFRouting() :
  m_nspGw(Ipv4Address::GetAny()),
  m_accessNetPrefix(Ipv4Address::GetAny()),
  m_accessNetMask(Ipv4Mask::GetZero())
{
  NS_LOG_FUNCTION_NOARGS();
}

OFRouting::~OFRouting()
{
  NS_LOG_FUNCTION_NOARGS();
}

int
OFRouting::GetPriority() const
{
  return 750; /* this app is a catch-all router */
}

bool
OFRouting::ReceiveFromSwitch(Ptr<OfSwitch>origin,
                             unsigned     portIn,
                             unsigned     bufferId,
                             Ptr<Packet>  packet)
{
  NS_LOG_FUNCTION(origin->GetChassisId() << portIn << bufferId);

  Ptr<Packet> packetOrig = packet->Copy();

  EthernetHeader ethHdr;
  packet->RemoveHeader(ethHdr);

  if (ethHdr.GetLengthType() != Ipv4L3Protocol::PROT_NUMBER)
    return false;

  /* Deserialize packet, and check its content */
  Ipv4Header ip4;
  packet->RemoveHeader(ip4);

  if (ip4.GetDestination() == Ipv4Address::GetBroadcast())
    return false; /* Broadcast protocols need a specific app */

  /* We want to differentiate TCP, UDP and ICMP flows */
  if ((ip4.GetProtocol() != Icmpv4L4Protocol::PROT_NUMBER)
      && (ip4.GetProtocol() != TcpL4Protocol::PROT_NUMBER)
      && (ip4.GetProtocol() != UdpL4Protocol::PROT_NUMBER)) {
    NS_LOG_WARN("OF-ROUTE: unknown IP protocol " << int(ip4.GetProtocol()));
    return false;
  }

  // FIXME check addresses, one end should be in the access network
  Ptr<HostVertex> srcHost = m_controller->LookupHost(ip4.GetSource());
  Ptr<HostVertex> dstHost = m_controller->LookupHost(ip4.GetDestination());

  if (dstHost == nullptr) {
    dstHost = m_controller->LookupHost(m_nspGw); // use default GW
    ///FIXME use GW only when destination prefix doesn't belong to RAN
  }

  if ((srcHost == nullptr) || (dstHost == nullptr)) {
    NS_LOG_WARN("OF-ROUTE: unknown hosts " << srcHost << " --> " << dstHost);
    return false;
  }

  UndirectedGraph::GraphPath path = m_controller->FindShortestPath(srcHost, dstHost);

  if (path.empty()) {
    NS_LOG_WARN("OF-ROUTE: unknown route " << srcHost << " --> " << dstHost);
    return false;
  }

  uint32_t wildcards =
    ~(OFPFW_DL_TYPE | OFPFW_NW_PROTO | OFPFW_NW_SRC_MASK | OFPFW_NW_DST_MASK);

  Ptr<FlowMatchHeader> matchHeader =
    m_controller->CreateFlowMatch(4, wildcards,
                                  Ipv4L3Protocol::PROT_NUMBER,
                                  ip4.GetProtocol(),
                                  ip4.GetSource().Get(),
                                  ip4.GetDestination().Get());

  SetupPath(matchHeader, path);

  if (bufferId != OFP_NO_BUFFER)
    RelayPacket(origin, bufferId, packetOrig);

  return true;
}

void
OFRouting::RelayPacket(Ptr<OfSwitch>origin,
                       unsigned     bufferId,
                       Ptr<Packet>  packet)
{
  int newState = m_controller->SwitchSync();

  NS_LOG_LOGIC("OF-ROUTE: barrier ID " << newState);

  PendingPacketType pp;

  pp.bufferId = bufferId;
  pp.origin   = origin;
  pp.packet   = packet;

  m_pendingPacket[newState] = pp;
}

void
OFRouting::SetupPath(Ptr<FlowMatchHeader>        matchHeader,
                     UndirectedGraph::GraphPath& path)
{
  NS_LOG_FUNCTION_NOARGS();

  for (auto& adj: path) {
    NS_LOG_LOGIC("OF-ROUTE Path leg: " << *adj);

    if (adj->GetOrigin()->IsLeaf())
      continue;

    Ptr<SwitchVertex> sw = DynamicCast<SwitchVertex, Vertex>(adj->GetOrigin());

    action_utils::ActionsList actions;
    action_utils::CreateOutputAction(&actions, adj->GetPortNumber(), 0);

    Ptr<OpenflowHeader> ofHeader =
      m_controller->CreateFlowModification(matchHeader, 0, OFPFC_ADD,
                                           m_flowLifetime, 0, m_flowPrio, -1,
                                           OFPP_NONE, OFPFF_SEND_FLOW_REM,
                                           actions);

    m_controller->SendOpenflowMessage(sw->GetSwitch(), ofHeader);
  }
}

void
OFRouting::TearDownPath(Ptr<FlowMatchHeader>        matchHeader,
                     UndirectedGraph::GraphPath& path)
{
  NS_LOG_FUNCTION_NOARGS();

  for (auto& adj: path) {
    NS_LOG_LOGIC("OF-ROUTE Remove path leg: " << *adj);

    if (adj->GetOrigin()->IsLeaf())
      continue;

    Ptr<SwitchVertex> sw = DynamicCast<SwitchVertex, Vertex>(adj->GetOrigin());

    action_utils::ActionsList actions;
    // action_utils::CreateOutputAction(&actions, adj->GetPortNumber(), 0);

    Ptr<OpenflowHeader> ofHeader =
      m_controller->CreateFlowModification(matchHeader, 0, OFPFC_DELETE,
                                           0, 0, 0, 0,
                                           OFPP_NONE, OFPFF_SEND_FLOW_REM,
                                           actions);

    m_controller->SendOpenflowMessage(sw->GetSwitch(), ofHeader);
  }
}

void
OFRouting::SyncCompleted(int statedId)
{
  NS_LOG_FUNCTION(this << statedId);

  if (m_pendingPacket.count(statedId) == 0)
    return;

  PendingPacketType& pp = m_pendingPacket[statedId];

  /* Output original packet through new path */
  action_utils::ActionsList actionsList;
  action_utils::CreateOutputAction(&actionsList, OFPP_TABLE, 0);

  Ptr<OpenflowHeader> ofHeader =
    m_controller->CreatePacketOut(pp.bufferId, OFPP_NONE, sizeof(ofp_action_output),
                                  actionsList, pp.packet);

  m_controller->SendOpenflowMessage(pp.origin, ofHeader);

  m_pendingPacket.erase(statedId);
}
} // namespace ns3
#endif // NS3_OPENFLOW
