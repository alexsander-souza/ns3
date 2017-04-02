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
 * Author: Blake Hurd  <naimorai@gmail.com>
 * Contributor: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */
#ifdef NS3_OPENFLOW

#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/socket.h>
#include "actions/action-header.h"
#include "headers/error-msg-header.h"
#include "headers/flow-match-header.h"
#include "headers/flow-modification-header.h"
#include "headers/flow-removed-header.h"
#include "headers/flow-stats-header.h"
#include "headers/flow-stats-request-header.h"
#include "headers/openflow-header.h"
#include "headers/packet-in-header.h"
#include "headers/packet-out-header.h"
#include "headers/physical-port-header.h"
#include "headers/port-modification-header.h"
#include "headers/port-stats-request-header.h"
#include "headers/port-stats-header.h"
#include "headers/port-status-header.h"
#include "headers/stats-reply-header.h"
#include "headers/stats-request-header.h"
#include "headers/switch-configuration-header.h"
#include "headers/switch-features-header.h"
#include "controller-ofswitch.h"

// #include "openflow-switch-net-device.h"
#include "controller-sbi.h"

NS_LOG_COMPONENT_DEFINE("ControllerSbi");

namespace ns3 {
ControllerSbi::ControllerSbi()
{
  NS_LOG_FUNCTION_NOARGS();
}

ControllerSbi::~ControllerSbi()
{
  NS_LOG_FUNCTION_NOARGS();
}

Ptr<OpenflowHeader>
ControllerSbi::CreateDefault(uint8_t type)
{
  NS_LOG_INFO("Controller creating default message");
  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(type, rand());

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateSetConfiguration(uint16_t flags, uint16_t length)
{
  NS_LOG_INFO("Controller creating switch set configuration message");

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_SET_CONFIG,
                                                              rand());
  Ptr<SwitchConfigurationHeader> swConfigHeader = Create<SwitchConfigurationHeader>(flags, length);

  openflowHeader->SetSwitchConfiguration(swConfigHeader);

  return openflowHeader;
}

Ptr<FlowMatchHeader>
ControllerSbi::CreateFlowMatch(int argsNumber, ...)
{
  Ptr<FlowMatchHeader> header = Create<FlowMatchHeader>();
  va_list varArgsList;

  va_start(varArgsList, argsNumber);

  uint32_t wildcards = va_arg(varArgsList, uint32_t);
  header->SetWildcards(wildcards);

  if ((wildcards & OFPFW_IN_PORT) == 0)
    {
      uint16_t inPort = va_arg(varArgsList, int);
      header->SetInPort(inPort);
    }

  if ((wildcards & OFPFW_DL_VLAN) == 0)
    {
      uint16_t dlVlan = va_arg(varArgsList, int);
      header->SetDlVlan(dlVlan);
    }

  if ((wildcards & OFPFW_DL_SRC) == 0)
    {
      Mac48Address dlSrc = va_arg(varArgsList, Mac48Address);
      header->SetDlSrc(dlSrc);
    }

  if ((wildcards & OFPFW_DL_DST) == 0)
    {
      Mac48Address dlDst = va_arg(varArgsList, Mac48Address);
      header->SetDlDst(dlDst);
    }

  if ((wildcards & OFPFW_DL_TYPE) == 0)
    {
      uint16_t dlType = va_arg(varArgsList, int);
      header->SetDlType(dlType);
    }

  if ((wildcards & OFPFW_NW_TOS) == 0)
    {
      uint8_t nwTos = va_arg(varArgsList, int);
      header->SetNwTos(nwTos);
    }

  if ((wildcards & OFPFW_NW_PROTO) == 0)
    {
      uint8_t nwProto = va_arg(varArgsList, int);
      header->SetNwProto(nwProto);
    }

  if ((wildcards & OFPFW_NW_SRC_ALL) == 0)
    {
      uint32_t nwSrc = va_arg(varArgsList, uint32_t);
      header->SetNwSrc(nwSrc);
    }

  if ((wildcards & OFPFW_NW_DST_ALL) == 0)
    {
      uint32_t nwDst = va_arg(varArgsList, uint32_t);
      header->SetNwDst(nwDst);
    }

  if ((wildcards & OFPFW_TP_SRC) == 0)
    {
      uint16_t tpSrc = va_arg(varArgsList, int);
      header->SetTpSrc(tpSrc);
    }

  if ((wildcards & OFPFW_TP_DST) == 0)
    {
      uint16_t tpDst = va_arg(varArgsList, int);
      header->SetTpDst(tpDst);
    }

  va_end(varArgsList);

  return header;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateFlowModification(ofp_flow_mod *msg)
{
  msg->header.version = OFP_VERSION1;
  msg->header.type = OFPT_FLOW_MOD;
  msg->header.length = ntohs(msg->header.length) - sizeof(ofp_action_output);
  msg->header.xid = 0;                                                    // field
                                                                          // not
                                                                          // considered
                                                                          // in
                                                                          // learning
                                                                          // controller

  msg->command = ntohs(msg->command);
  msg->idle_timeout = ntohs(msg->idle_timeout);
  msg->hard_timeout = ntohs(msg->hard_timeout);
  msg->buffer_id = ntohl(msg->buffer_id);

  msg->match.in_port = ntohs(msg->match.in_port);
  msg->match.dl_vlan = ntohs(msg->match.dl_vlan);
  msg->match.dl_type = ntohs(msg->match.dl_type);
  msg->match.nw_src = ntohl(msg->match.nw_src);
  msg->match.nw_dst = ntohl(msg->match.nw_dst);
  msg->match.tp_src = ntohs(msg->match.tp_src);
  msg->match.tp_dst = ntohs(msg->match.tp_dst);

  // only one action of type output action is currently supported

  ofp_action_header *action = msg->actions;

  action->len = sizeof(ofp_action_output);
  action->type = ntohs(action->type);

  ofp_action_output *outputAction = (ofp_action_output*)action;

  outputAction->port = outputAction->port;
  outputAction->max_len = outputAction->max_len;

  Ptr<OpenflowHeader> ofHeader = Create<OpenflowHeader>((ofp_header*)msg);
  Ptr<FlowModificationHeader> fModHeader = Create<FlowModificationHeader>(msg);

  ofp_match matchStruct = msg->match;
  Ptr<FlowMatchHeader> matchHeader = Create<FlowMatchHeader>(&matchStruct);

  ///FIXME se não alocamos isso, não devemos desalocar aqui!!!!
  delete msg;

  Ptr<ActionOutputHeader> actOutputHeader = Create<ActionOutputHeader>(outputAction);
  Ptr<ActionHeader> actHeader = Create<ActionHeader>(action);

  actHeader->SetOutput(actOutputHeader);
  fModHeader->AddAction(actHeader);
  fModHeader->SetFlowMatch(matchHeader);
  ofHeader->SetFlowModification(fModHeader);

  return ofHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateFlowModification(Ptr<FlowMatchHeader>matchHeader, uint64_t cookie, uint16_t command,
                                      uint16_t idleTimeout, uint16_t hardTimeout, uint16_t priority,
                                      uint32_t bufferId, uint16_t outPort, uint16_t flags,
                                      const action_utils::ActionsList& actionsList)
{
  NS_LOG_INFO("Controller creating flow modification message");

  Ptr<FlowModificationHeader> flowModHeader = Create<FlowModificationHeader>(cookie, command, idleTimeout,
                                                                             hardTimeout, priority, bufferId,
                                                                             outPort, flags);
  flowModHeader->SetFlowMatch(matchHeader);
  flowModHeader->AddActionsList(actionsList);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_FLOW_MOD, rand());
  openflowHeader->SetFlowModification(flowModHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateFlowModification(uint32_t wildcards, uint16_t inPort, const Mac48Address& dlSrc,
                                      const Mac48Address& dlDst, uint16_t dlVlan, uint8_t dlVlanPcp,
                                      uint16_t dlType, uint8_t nwTos, uint8_t nwProto,
                                      uint32_t nwSrc, uint32_t nwDst, uint16_t tpSrc,
                                      uint16_t tpDst, uint64_t cookie, uint16_t command,
                                      uint16_t idleTimeout, uint16_t hardTimeout, uint16_t priority,
                                      uint32_t bufferId, uint16_t outPort, uint16_t flags,
                                      const action_utils::ActionsList& actionsList)
{
  NS_LOG_INFO("Controller creating flow modification message");

  Ptr<FlowMatchHeader> flowMatchHeader = Create<FlowMatchHeader>(wildcards, inPort, dlSrc,
                                                                 dlDst, dlVlan, dlVlanPcp,
                                                                 dlType, nwTos, nwProto,
                                                                 nwSrc, nwDst, tpSrc, tpDst);

  Ptr<FlowModificationHeader> flowModHeader = Create<FlowModificationHeader>(cookie, command, idleTimeout,
                                                                             hardTimeout, priority, bufferId,
                                                                             outPort, flags);
  flowModHeader->SetFlowMatch(flowMatchHeader);
  flowModHeader->AddActionsList(actionsList);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_FLOW_MOD, rand());
  openflowHeader->SetFlowModification(flowModHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreatePacketOut(uint32_t bufferId, uint16_t inPort, uint16_t actionsLen,
                               const action_utils::ActionsList& actionsList, Ptr<Packet>packet)
{
  NS_LOG_INFO("Controller creating packet out message");

  Ptr<PacketOutHeader> packetOutHeader = Create<PacketOutHeader>(bufferId, inPort, actionsLen, packet);
  packetOutHeader->AddActionsList(actionsList);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_PACKET_OUT, rand());

  openflowHeader->SetPacketOut(packetOutHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreatePortModification(uint16_t portNumber, uint8_t hwAddr[], uint32_t config,
                                      uint32_t mask, uint32_t advertise)
{
  NS_LOG_INFO("Controller creating port modification message");

  Ptr<PortModificationHeader> portModHeader = Create<PortModificationHeader>(portNumber, hwAddr, config,
                                                                             mask, advertise);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_PORT_MOD, rand());
  openflowHeader->SetPortModification(portModHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateFlowStatsRequest(uint16_t flags, Ptr<FlowMatchHeader>matchHeader,
                                      uint8_t tableId, uint16_t outPort)
{
  NS_LOG_INFO("Controller creating flow stats request message");

  Ptr<FlowStatsRequestHeader> flowSReqHeader = Create<FlowStatsRequestHeader>(tableId, outPort);
  flowSReqHeader->SetFlowMatch(matchHeader);

  Ptr<StatsRequestHeader> statsReqHeader = Create<StatsRequestHeader>(OFPST_FLOW, flags);
  statsReqHeader->SetFlowStatsRequest(flowSReqHeader);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_STATS_REQUEST, rand());
  openflowHeader->SetStatsRequest(statsReqHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreateFlowStatsRequest(uint16_t flags, uint32_t wildcards,
                                      uint16_t inPort, const Mac48Address& dlSrc,
                                      const Mac48Address& dlDst, uint16_t dlVlan,
                                      uint8_t dlVlanPcp, uint16_t dlType,
                                      uint8_t nwTos, uint8_t nwProto,
                                      uint32_t nwSrc, uint32_t nwDst,
                                      uint16_t tpSrc, uint16_t tpDst,
                                      uint8_t tableId, uint16_t outPort)
{
  NS_LOG_INFO("Controller creating flow stats request message");

  Ptr<FlowMatchHeader> flowMatchHeader = Create<FlowMatchHeader>(wildcards, inPort, dlSrc, dlDst, dlVlan,
                                                                 dlVlanPcp, dlType, nwTos, nwProto,
                                                                 nwSrc, nwDst, tpSrc, tpDst);
  Ptr<FlowStatsRequestHeader> flowSReqHeader = Create<FlowStatsRequestHeader>(tableId, outPort);
  flowSReqHeader->SetFlowMatch(flowMatchHeader);

  Ptr<StatsRequestHeader> statsReqHeader = Create<StatsRequestHeader>(OFPST_FLOW, flags);
  statsReqHeader->SetFlowStatsRequest(flowSReqHeader);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_STATS_REQUEST, rand());
  openflowHeader->SetStatsRequest(statsReqHeader);

  return openflowHeader;
}

Ptr<OpenflowHeader>
ControllerSbi::CreatePortStatsRequest(uint16_t flags, uint16_t portNumber)
{
  NS_LOG_INFO("Controller creating port stats request message");

  Ptr<PortStatsRequestHeader> portSReqHeader = Create<PortStatsRequestHeader>(portNumber);

  Ptr<StatsRequestHeader> statsReqHeader = Create<StatsRequestHeader>(OFPST_PORT, flags);
  statsReqHeader->SetPortStatsRequest(portSReqHeader);

  Ptr<OpenflowHeader> openflowHeader = Create<OpenflowHeader>(OFPT_STATS_REQUEST, rand());
  openflowHeader->SetStatsRequest(statsReqHeader);

  return openflowHeader;
}

void
ControllerSbi::SendOpenflowMessage(Ptr<OfSwitch>      target,
                                   Ptr<OpenflowHeader>openflowHeader,
                                   bool updateXid)
{
  NS_LOG_FUNCTION(this);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(*openflowHeader);

  if (updateXid)
    target->UpdateXid(openflowHeader->GetXId());

  target->SendMessage(packet);
}

void
ControllerSbi::AcceptHandler(Ptr<Socket>socket, const Address& from)
{
  NS_LOG_FUNCTION(this << socket << from);

  NS_LOG_INFO("Accept handler controller");

  Ptr<OfSwitch> newSwitch = Create<OfSwitch>(this, socket, from);
  m_swList.push_front(newSwitch);

  newSwitch->Initialize();
}

void
ControllerSbi::InitializeNetwork(Ptr<Node>node, const Ipv4Address& myAddress)
{
  if (m_socketListener == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::TcpSocketFactory");
      InetSocketAddress local = InetSocketAddress(myAddress, OFP_TCP_PORT);

      m_socketListener = Socket::CreateSocket(node, tid);
      m_socketListener->Bind(local);
      m_socketListener->Listen();
    }

  m_socketListener->SetAcceptCallback(
    MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
    MakeCallback(&ControllerSbi::AcceptHandler, this));
}

ControllerSbi::SwitchList
ControllerSbi::GetSwitchList()
{
  return m_swList;
}

Ptr<OfSwitch>
ControllerSbi::GetSwitch(const Mac48Address &chassisId)
{
  for (auto& dev: m_swList)
    {
      if (dev->GetChassisId() == chassisId)
        return dev;
    }

  return nullptr;
}
} // namespace ns3
#endif // NS3_OPENFLOW
