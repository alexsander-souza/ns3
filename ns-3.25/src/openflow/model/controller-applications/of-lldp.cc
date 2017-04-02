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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#ifdef NS3_OPENFLOW

#include <ns3/log.h>
#include <ns3/ethernet-header.h>
#include <ns3/action-header.h>
#include <ns3/action-utils.h>
#include <ns3/controller.h>
#include <ns3/lldp-header.h>
#include <ns3/openflow-header.h>
#include <ns3/packet-in-header.h>
#include <ns3/flow-match-header.h>
#include <ns3/flow-modification-header.h>
#include <ns3/action-output-header.h>
#include <ns3/controller-ofswitch.h>
#include <ns3/vertex-switch.h>
#include <ns3/adjacency.h>
#include <ns3/of-lldp.h>

NS_LOG_COMPONENT_DEFINE("LldpHandler");

namespace ns3 {
TypeId
LldpHandler::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::LldpHandler")
                      .SetParent(ControllerApplication::GetTypeId())
                      .AddConstructor<LldpHandler>()
                      .AddAttribute("Address",
                                    "LLDP destination MAC address",
                                    Mac48AddressValue(Mac48Address("01:80:c2:00:00:0e")),
                                    MakeMac48AddressAccessor(&LldpHandler::m_address),
                                    MakeMac48AddressChecker())
                      .AddAttribute("TopologyRefreshInterval",
                                    "LLDP refresh interval",
                                    TimeValue(Seconds(5)),
                                    MakeTimeAccessor(&LldpHandler::m_refreshInterval),
                                    MakeTimeChecker())
  ;

  return tid;
}

LldpHandler::LldpHandler()
{
  NS_LOG_FUNCTION_NOARGS();
}

LldpHandler::~LldpHandler()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
LldpHandler::InitMe()
{
  NS_LOG_FUNCTION_NOARGS();
  RefreshTopology();
}

void
LldpHandler::InitSwitch(Ptr<OfSwitch>origin)
{
  NS_LOG_FUNCTION(this << origin);

  uint32_t wildcards = ~(OFPFW_DL_TYPE | OFPFW_DL_DST);
  uint16_t dlType    = BasicLldpHeader::PROT_NUMBER;

  Ptr<FlowMatchHeader> matchHeader =
    m_controller->CreateFlowMatch(3, wildcards, m_address, dlType);

  action_utils::ActionsList actions;

  action_utils::CreateOutputAction(&actions, ActionOutputHeader::OFPP_CONTROLLER, 0);

  Ptr<OpenflowHeader> ofHeader =
    m_controller->CreateFlowModification(matchHeader,
                                         0, OFPFC_ADD, OFP_FLOW_PERMANENT, OFP_FLOW_PERMANENT,
                                         OFP_DEFAULT_PRIORITY, -1, OFPP_NONE, OFPFF_SEND_FLOW_REM,
                                         actions);

  m_controller->SendOpenflowMessage(origin, ofHeader);
}

void
LldpHandler::RefreshTopology()
{
  NS_LOG_FUNCTION(this);

  for (auto& dev: m_controller->GetSwitchList())
  {
    for (int i = 0; i < dev->GetNPorts(); ++i)
    {
      BasicLldpHeader lldpHeader(dev->GetChassisId(), i, m_refreshInterval.GetSeconds());

      EthernetHeader ethHeader;
      ethHeader.SetDestination(m_address);
      ethHeader.SetLengthType(BasicLldpHeader::PROT_NUMBER);
      ethHeader.SetSource(dev->GetPortAddress(i));

      Ptr<Packet> packet = Create<Packet>();
      packet->AddHeader(lldpHeader);
      packet->AddHeader(ethHeader);

      action_utils::ActionsList actionsList;
      action_utils::CreateOutputAction(&actionsList, i, 0);

      Ptr<OpenflowHeader> ofHeader =
        m_controller->CreatePacketOut(OFP_NO_BUFFER, OFPP_NONE, sizeof(ofp_action_output),
                                      actionsList, packet);

      m_controller->SendOpenflowMessage(dev, ofHeader);
    }
  }


  m_topoRefreshEvent = Simulator::Schedule(m_refreshInterval,
                                           &LldpHandler::RefreshTopology, this);
}

bool
LldpHandler::ReceiveFromSwitch(Ptr<OfSwitch>origin,
                               unsigned portIn, unsigned bufferId,
                               Ptr<Packet>packet)
{
  NS_LOG_FUNCTION(origin << portIn);

  EthernetHeader ethHeader;
  packet->RemoveHeader(ethHeader);

  if (ethHeader.GetLengthType() != BasicLldpHeader::PROT_NUMBER)
    return false;

  BasicLldpHeader lldp;
  packet->RemoveHeader(lldp);

  Ptr<Vertex> source = m_controller->LookupSwitch(lldp.GetChassisId());
  Ptr<Vertex> dest   = m_controller->LookupSwitch(origin->GetChassisId());

  if (source && dest)
  {
    // We known that source-->dest works, so we could infer that the reverse
    // path also works, but let's be conservative
    m_controller->AddEdge(source, dest, lldp.GetPortId());

    // Disable flooding in this port
    if (Ptr<OfSwitch>swSrc = m_controller->GetSwitch(lldp.GetChassisId()))
      swSrc->SetFloodPort(lldp.GetPortId(), false);
  }
  return true;
}

int
LldpHandler::GetPriority() const
{
  // TODO create an enum of usual prios
  return 250; /* Specific protocol */
}
} // namespace ns3
#endif // NS3_OPENFLOW
