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

#include <ns3/log.h>
#include <ns3/ethernet-header.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/arp-header.h>
#include <ns3/arp-l3-protocol.h>
#include <ns3/action-header.h>
#include <ns3/action-utils.h>
#include <ns3/controller.h>
#include <ns3/openflow-header.h>
#include <ns3/packet-in-header.h>
#include <ns3/flow-match-header.h>
#include <ns3/flow-modification-header.h>
#include <ns3/action-output-header.h>
#include <ns3/controller-ofswitch.h>
#include <ns3/vertex-switch.h>
#include <ns3/vertex-host.h>
#include <ns3/adjacency.h>
#include <ns3/of-arp.h>

NS_LOG_COMPONENT_DEFINE("ArpHandler");

namespace ns3 {
TypeId
ArpHandler::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ArpHandler")
                      .SetParent(ControllerApplication::GetTypeId())
                      .AddConstructor<ArpHandler>()
  ;

  return tid;
}

ArpHandler::ArpHandler()
{
  NS_LOG_FUNCTION_NOARGS();
}

ArpHandler::~ArpHandler()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
ArpHandler::InitSwitch(Ptr<OfSwitch>origin)
{
  NS_LOG_FUNCTION(origin);
  uint32_t wildcards = ~(OFPFW_DL_TYPE);
  uint16_t dlType    = ArpL3Protocol::PROT_NUMBER;

  Ptr<FlowMatchHeader> matchHeader =
    m_controller->CreateFlowMatch(2, wildcards, dlType);

  action_utils::ActionsList actions;

  action_utils::CreateOutputAction(&actions, ActionOutputHeader::OFPP_CONTROLLER, 0);

  Ptr<OpenflowHeader> ofHeader =
    m_controller->CreateFlowModification(matchHeader,
                                         0, OFPFC_ADD, OFP_FLOW_PERMANENT, OFP_FLOW_PERMANENT,
                                         OFP_DEFAULT_PRIORITY, -1, OFPP_NONE, OFPFF_SEND_FLOW_REM,
                                         actions);

  m_controller->SendOpenflowMessage(origin, ofHeader);
}

bool
ArpHandler::ReceiveFromSwitch(Ptr<OfSwitch>origin,
                              unsigned portIn, unsigned bufferId,
                              Ptr<Packet>packet)
{
  NS_LOG_FUNCTION(origin << portIn);

  EthernetHeader ethHeader;
  packet->PeekHeader(ethHeader);

  if (ethHeader.GetLengthType() != ArpL3Protocol::PROT_NUMBER)
    return false;

  Ptr<Packet> origPkt = packet->Copy();

  packet->RemoveHeader(ethHeader);
  ArpHeader arpHeader;
  packet->RemoveHeader(arpHeader);

  bool handled;

  if (arpHeader.IsRequest())
    handled = HandleArpRequest(origin, portIn, arpHeader);
  else
    handled = HandleArpReply(origin, portIn, arpHeader);

  if (!handled)
    FloodRequest(origin, portIn, origPkt);

  return true;
}

bool
ArpHandler::HandleArpRequest(Ptr<OfSwitch>origin, unsigned portIn, ArpHeader& arpHeader)
{
  NS_LOG_FUNCTION(arpHeader);

  Mac48Address targetHwAddress = Mac48Address::GetBroadcast();
  Ipv4Address  targetAddr      = arpHeader.GetDestinationIpv4Address();
  Mac48Address sourceHwAddress = Mac48Address::ConvertFrom(arpHeader.GetSourceHardwareAddress());
  Ipv4Address  sourceAddr      = arpHeader.GetSourceIpv4Address();

  Ptr<HostVertex> srcHost = m_controller->LookupHost(sourceHwAddress);

  if (!srcHost) {
    // Learn source host
    srcHost = m_controller->AddHost(sourceHwAddress);

    Ptr<SwitchVertex> sw  = m_controller->LookupSwitch(origin->GetChassisId());
    Ptr<Adjacency>    adj = m_controller->AddEdge(sw, srcHost, portIn);
    m_controller->AddEdge(adj->Invert());
  }

  if (srcHost->HasAddress(sourceAddr) == false)
    srcHost->AddAddress(sourceAddr);

  if (targetAddr == Ipv4Address::GetBroadcast())
    return true; // gratuitous ARP

  // Discover destination host
  Request requester(origin, portIn, sourceHwAddress, sourceAddr);

  if (Ptr<HostVertex>host = m_controller->LookupHost(targetAddr))
    targetHwAddress = host->GetHwAddress();

  if (targetHwAddress != Mac48Address::GetBroadcast())
  {
    SendReply(targetAddr, targetHwAddress, requester);
    return true;
  } else {
    NS_LOG_LOGIC("ARP missing for " << targetAddr);

    // should flood and discover this MAC
    m_pending.emplace(targetAddr, requester);
    return false;
  }
}

bool
ArpHandler::HandleArpReply(Ptr<OfSwitch>origin, unsigned portIn, ArpHeader& arpHeader)
{
  NS_LOG_FUNCTION(arpHeader);
  Ptr<SwitchVertex> sw = m_controller->LookupSwitch(origin->GetChassisId());

  if (sw == nullptr)
    return true; // Too soon, we don't known this Switch yet

  Mac48Address targetHwAddress = Mac48Address::ConvertFrom(arpHeader.GetSourceHardwareAddress());
  Ipv4Address  sourceAddr      = arpHeader.GetSourceIpv4Address();
  Ipv4Address  targetAddr      = arpHeader.GetSourceIpv4Address();

  Ptr<HostVertex> dstHost = m_controller->LookupHost(targetHwAddress);

  if (dstHost == nullptr)
    dstHost = m_controller->AddHost(targetHwAddress); // Learn host

  if (dstHost->GetEdge(sw) == nullptr) {
    Ptr<Adjacency> adj = m_controller->AddEdge(sw, dstHost, portIn);
    m_controller->AddEdge(adj->Invert());
  }

  if (!dstHost->HasAddress(targetAddr))
    dstHost->AddAddress(targetAddr);

  if (sourceAddr == targetAddr)
    return true; // Gratuitous ARP

  NS_LOG_LOGIC("ARP REPLY " << targetAddr << " at " << targetHwAddress);

  auto reqIter = m_pending.equal_range(targetAddr);

  for (auto& it = reqIter.first; it != reqIter.second; ++it) {
    SendReply(targetAddr, targetHwAddress, it->second);
  }

  m_pending.erase(targetAddr);

  return true;
}

void
ArpHandler::FloodRequest(Ptr<OfSwitch>origin,
                         unsigned     portIn,
                         Ptr<Packet>  packet)
{
  NS_LOG_FUNCTION(origin << portIn);

  for (auto& dev: m_controller->GetSwitchList()) {
    for (int i = 0; i < dev->GetNPorts(); ++i) {
      if ((!dev->IsFloodPort(i)) || ((dev == origin) && (i == int(portIn))))
        continue;

      action_utils::ActionsList actionsList;
      action_utils::CreateOutputAction(&actionsList, i, 0);

      Ptr<OpenflowHeader> ofHeader =
        m_controller->CreatePacketOut(OFP_NO_BUFFER, OFPP_NONE, sizeof(ofp_action_output),
                                      actionsList, packet->Copy());

      m_controller->SendOpenflowMessage(dev, ofHeader);
    }
  }
}

void
ArpHandler::SendReply(const Ipv4Address & targetAddr,
                      const Mac48Address& targetHwAddress,
                      const Request     & request)
{
  NS_LOG_FUNCTION(targetAddr << request.m_senderHwAddr << request.m_portIn);
  ArpHeader arpHeader;

  arpHeader.SetReply(targetHwAddress, targetAddr,
                     request.m_senderHwAddr, request.m_senderAddr);

  EthernetHeader ethHeader;
  ethHeader.SetDestination(request.m_senderHwAddr);
  ethHeader.SetSource(targetHwAddress);
  ethHeader.SetLengthType(ArpL3Protocol::PROT_NUMBER);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(arpHeader);
  packet->AddHeader(ethHeader);

  action_utils::ActionsList actionsList;
  action_utils::CreateOutputAction(&actionsList, request.m_portIn, 0);

  Ptr<OpenflowHeader> ofHeader =
    m_controller->CreatePacketOut(OFP_NO_BUFFER, OFPP_NONE, sizeof(ofp_action_output),
                                  actionsList, packet);

  m_controller->SendOpenflowMessage(request.m_origin, ofHeader);
}

int
ArpHandler::GetPriority() const
{
  // TODO create an enum of usual prios
  return 250; /* Specific protocol */
}

/******************************************************************************/
ArpHandler::Request::Request(Ptr<OfSwitch>       origin,
                             unsigned            portIn,
                             const Mac48Address& senderHwAddr,
                             const Ipv4Address & senderAddr) :
  m_origin(origin), m_portIn(portIn),
  m_startTime(Simulator::Now()),
  m_senderHwAddr(senderHwAddr), m_senderAddr(senderAddr)
{}

bool
operator==(const ArpHandler::Request& req1,
           const ArpHandler::Request& req2)
{
  return req1.m_senderHwAddr == req2.m_senderHwAddr;
}

bool
operator<(const ArpHandler::Request& req1,
          const ArpHandler::Request& req2)
{
  return req1.m_senderHwAddr < req2.m_senderHwAddr;
}
} // namespace ns3
#endif // NS3_OPENFLOW
