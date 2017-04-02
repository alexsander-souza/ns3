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
 * Author: Andrey Blazejuk  <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#include <ns3/log.h>
#include <ns3/socket.h>
#include <ns3/packet.h>
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
#include "controller-sbi.h"
#include "controller-ofswitch.h"

NS_LOG_COMPONENT_DEFINE("OfSwitch");

namespace ns3 {
OfSwitch::OfSwitch(ControllerSbi *control,
                   Ptr<Socket>    socket,
                   const Address& address) :
  m_socket(socket),
  m_address(address),
  m_chassisId(Mac48Address::GetBroadcast()),
  m_xid(-1),
  m_control(control)
{
  NS_LOG_FUNCTION(this);

  socket->SetRecvCallback(MakeCallback(&OfSwitch::OpenflowHandler, this));
}

OfSwitch::~OfSwitch()
{
  NS_LOG_FUNCTION(this);

  if (m_socket != 0)
    m_socket->Close();
}

void
OfSwitch::Initialize()
{
  HandleHello(rand());
}

void
OfSwitch::SendMessage(Ptr<Packet>packet)
{
  if (m_socket->Send(packet, 0) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);
}

void
OfSwitch::UpdateXid(uint32_t xid)
{
  m_xid = xid;
}

uint32_t
OfSwitch::GetLastXid() const
{
  return m_xid;
}

int
OfSwitch::GetNPorts() const
{
  return m_port_map.size();
}

const Mac48Address&
OfSwitch::GetPortAddress(int port) const
{
  NS_ASSERT(port < GetNPorts());
  return m_port_map.at(port);
}

bool
OfSwitch::IsFloodPort(int port) const
{
  NS_ASSERT(port < GetNPorts());

  auto it = m_flood_map.find(port);

  if (it != m_flood_map.end())
    return it->second;

  return true;
}

void
OfSwitch::SetFloodPort(int port, bool enabled)
{
  NS_LOG_FUNCTION(this << port << enabled);
  NS_ASSERT(port < GetNPorts());
  m_flood_map[port] = enabled;
}

const Mac48Address&
OfSwitch::GetChassisId() const
{
  return m_chassisId;
}

void
OfSwitch::SyncSwitch(int stateId)
{
  NS_LOG_FUNCTION(this << stateId);

  Ptr<OpenflowHeader> barrier = m_control->CreateDefault(OFPT_BARRIER_REQUEST);
  barrier->SetXId(rand());

  m_barrier_map[barrier->GetXId()] = stateId;

  m_control->SendOpenflowMessage(Ptr<OfSwitch>(this), barrier, true);
}

void
OfSwitch::HandleHello(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);
  NS_LOG_INFO("Controller handling hello message");

  if (m_xid == xid)
  {
    NS_LOG_INFO("Hello's handshake finished on controller");

    Ptr<OpenflowHeader> featuresMsg = m_control->CreateDefault(OFPT_FEATURES_REQUEST);
    featuresMsg->SetXId(rand());
    m_control->SendOpenflowMessage(Ptr<OfSwitch>(this), featuresMsg, true);

    return;
  }

  Ptr<OpenflowHeader> helloMsg = m_control->CreateDefault(OFPT_HELLO);
  helloMsg->SetXId(xid);
  m_control->SendOpenflowMessage(Ptr<OfSwitch>(this), helloMsg, true);
}

void
OfSwitch::HandleBarrier(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);

  auto pair = m_barrier_map.find(xid);

  if (pair != m_barrier_map.end()) {
    m_barrier_map.erase(pair);
    m_control->SwitchSyncCompleted(Ptr<OfSwitch>(this), pair->second);
  }
}

void
OfSwitch::HandleErrorMsg(Ptr<ErrorMsgHeader>header)
{
  header->Print(std::cout);
}

void
OfSwitch::HandleFeaturesReply(uint32_t xid, Ptr<SwitchFeaturesHeader>header)
{
  NS_LOG_FUNCTION(this << xid << header);

  if (m_xid != xid)
  {
    NS_LOG_WARN("Switch features wasn't requested");
    return;
  }

  if (m_chassisId == Mac48Address::GetBroadcast())
  {
    m_chassisId = header->GetDatapathMac();

    for (auto& p :header->GetPortsList())
    {
      Mac48Address ifAddr;
      ifAddr.CopyFrom(p->GetHwAddr());
      m_port_map.emplace(p->GetPortNumber(), ifAddr);
    }

    m_control->NewSwitch(Ptr<OfSwitch>(this));
  }
}

void
OfSwitch::HandleConfigurationReply(uint32_t xid, Ptr<SwitchConfigurationHeader>header)
{
  NS_LOG_FUNCTION(this << xid << header);

  if (m_xid != xid)
  {
    NS_LOG_WARN("Switch configuration wasn't requested");
    return;
  }

  header->Print(std::cout);
}

void
OfSwitch::HandleFlowRemoved(Ptr<FlowRemovedHeader>header)
{
  NS_LOG_FUNCTION(this << header);
  Ptr<FlowMatchHeader> flowMatchHeader = header->GetFlowMatch();

  header->Print(std::cout);
  flowMatchHeader->Print(std::cout);
}

void
OfSwitch::HandlePortStatus(Ptr<PortStatusHeader>header)
{
  NS_LOG_FUNCTION(this << header);
  Ptr<PhysicalPortHeader> phyPortHeader = header->GetPhysicalPort();

  header->Print(std::cout);
  phyPortHeader->Print(std::cout);
}

void
OfSwitch::HandleStatsReply(uint32_t xid, Ptr<StatsReplyHeader>header)
{
  NS_LOG_FUNCTION(this << xid << header);

  if (m_xid != xid)
  {
    NS_LOG_WARN("Stats wasn't requested");
    return;
  }

  header->Print(std::cout);

  switch (header->GetType())
  {
  case OFPST_FLOW:
    HandleFlowStatsReply(header->GetFlowStatsReply());
    break;

  case OFPST_PORT:
    HandlePortStatsReply(header->GetPortStatsReply());
    break;
  }
}

void
OfSwitch::HandleFlowStatsReply(Ptr<FlowStatsHeader>header)
{
  NS_LOG_FUNCTION(this << header);
  Ptr<FlowMatchHeader> flowMatchHeader = header->GetFlowMatch();

  header->Print(std::cout);
  flowMatchHeader->Print(std::cout);

  for (auto& act : header->GetActionsList())
  {
    act->Print(std::cout);
  }
}

void
OfSwitch::HandlePortStatsReply(Ptr<PortStatsHeader>header)
{
  NS_LOG_FUNCTION(this << header);
  header->Print(std::cout);
}

void
OfSwitch::HandlePacketIn(Ptr<PacketInHeader>header)
{
  NS_LOG_FUNCTION(this << header);
  uint16_t dataLength = header->GetTotalLen();
  uint8_t  data[dataLength];
  uint16_t portIn   = header->GetInPort();
  uint32_t bufferId = header->GetBufferId();

  header->GetData(data, dataLength);

  Ptr<Packet> packet = Create<Packet>(data, dataLength);
  m_control->ForwardToApps(Ptr<OfSwitch>(this), portIn, bufferId, packet);
}

void
OfSwitch::OpenflowHandler(Ptr<Socket>socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet;
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

      switch (openflowHeader.GetType())
      {
      case OFPT_HELLO:
        HandleHello(openflowHeader.GetXId());
        break;

      case OFPT_BARRIER_REPLY:
        HandleBarrier(openflowHeader.GetXId());
        break;

      case OFPT_ERROR:
        HandleErrorMsg(openflowHeader.GetErrorMsg());
        break;

      case OFPT_FEATURES_REPLY:
        HandleFeaturesReply(openflowHeader.GetXId(), openflowHeader.GetSwitchFeatures());
        break;

      case OFPT_GET_CONFIG_REPLY:
        HandleConfigurationReply(openflowHeader.GetXId(), openflowHeader.GetSwitchConfiguration());
        break;

      case OFPT_PACKET_IN:
        HandlePacketIn(openflowHeader.GetPacketIn());
        break;

      case OFPT_FLOW_EXPIRED:
        HandleFlowRemoved(openflowHeader.GetFlowRemoved());
        break;

      case OFPT_PORT_STATUS:
        HandlePortStatus(openflowHeader.GetPortStatus());
        break;

      case OFPT_STATS_REPLY:
        HandleStatsReply(openflowHeader.GetXId(), openflowHeader.GetStatsReply());
        break;
      }
    }
  }
}

bool
operator==(const OfSwitch& ofswitch, const Ipv4Address& addr)
{
  return addr == InetSocketAddress::ConvertFrom(ofswitch.m_address).GetIpv4();
}
} // namespace ns3
