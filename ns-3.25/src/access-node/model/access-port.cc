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
 */

#include <sstream>
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/csma-channel.h"
#include "ns3/csma-net-device.h"
#include "ns3/data-rate.h"
#include "ns3/ancp-an-agent.h"
#include "access-net-device.h"
#include "access-port.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AccessPort");

const uint AccessPort::QUERY_INTERVAL = (10U); /* 60 secs */

AccessPort::AccessPort() :
  m_accessNode(nullptr),
  m_port(nullptr),
  m_ttl(0),
  m_showtime(false),
  m_circuitId(""),
  m_usBytes(0),
  m_usPkts(0),
  m_dsBytes(0),
  m_dsPkts(0),
  m_access_prof(nullptr),
  m_mcast_prof(nullptr)
{
  NS_LOG_FUNCTION_NOARGS();
}

AccessPort::~AccessPort()
{
  NS_LOG_FUNCTION_NOARGS();
  m_accessNode = nullptr;
  m_access_prof = nullptr;
  m_mcast_prof = nullptr;
  m_port = nullptr;
}

void AccessPort::SetTtl(Time ttl)
{
  NS_LOG_FUNCTION(this << ttl);
  m_ttl = ttl;
}

Time AccessPort::GetTtl() const
{
  NS_LOG_FUNCTION(this);
  return(m_ttl);
}

void AccessPort::SetShowTime(bool enabled)
{
  NS_LOG_FUNCTION(this << m_circuitId << enabled);
  m_showtime = enabled;
  Ptr<AncpAnAgent> agent = m_accessNode->GetAncpAgent();

  NS_ASSERT(agent != nullptr);

  if (enabled)
    {
      Ptr<CsmaChannel> channel = DynamicCast<CsmaChannel, Channel>(m_port->GetChannel());
      uint32_t bps = channel->GetDataRate().GetBitRate();

      agent->SendPortUp(m_circuitId, 1, bps, bps);
    }
  else
    {
      agent->SendPortDown(m_circuitId);
      if (m_mcast_prof != nullptr)
        {
          agent->SendMCastAdmissionCtl(m_circuitId,
                                       AncpTlvMCastCommand::CMD_DELETE_ALL,
                                       Ipv4Address::GetAny());
        }
    }
}

bool AccessPort::GetShowTime() const
{
  NS_LOG_FUNCTION(this);
  return(m_showtime);
}

void AccessPort::SetAccessPort(Ptr<AccessNetDevice> anDev,
                               Ptr<NetDevice> accessDev,
                               const Mac48Address &source)
{
  NS_LOG_FUNCTION(this << anDev << accessDev);
  m_accessNode = anDev;
  m_port = accessDev;

  std::stringstream os;
  os << source;
  m_circuitId = os.str();

  m_port->AddLinkChangeCallback(MakeCallback(&AccessPort::LinkStateChangedHandler, this));

  if (m_port->IsLinkUp())
    SetShowTime(true); /* Already up */
}

Ptr<NetDevice> AccessPort::GetAccessPort() const
{
  NS_LOG_FUNCTION(this);
  return(m_port);
}

void AccessPort::LinkStateChangedHandler(void)
{
  NS_LOG_FUNCTION(this);

  if (m_port->IsLinkUp())
    SetShowTime(true);
  else
    SetShowTime(false);
}

const std::string AccessPort::GetAccessProfileName() const
{
  if (m_access_prof == nullptr)
    return "(none)";

  return m_access_prof->GetName();
}

const std::string AccessPort::GetMcastProfileName() const
{
  if (m_mcast_prof == nullptr)
    return "(none)";

  return m_mcast_prof->GetName();
}

int AccessPort::ApplyProfile(Ptr<AccessProfile>prof)
{
  NS_LOG_FUNCTION(this << *prof);

  m_access_prof = prof;

  if (m_port == nullptr)
    return 0; /* Done for now */

  Ptr<Channel> chan = m_port->GetChannel();

  for (uint32_t i = 0; i < chan->GetNDevices(); ++i)
    {
      Ptr<CsmaNetDevice> dev = DynamicCast<CsmaNetDevice, NetDevice>(chan->GetDevice(i));

      if (dev == m_port)
        dev->SetDataRate(m_access_prof->GetDownstreamRate());
      else
        dev->SetDataRate(m_access_prof->GetUpstreamRate());
    }

  return 0;
}

void AccessPort::AccPacket(enum AccessPort::PacketDir dir, Ptr<const Packet> pkt)
{
  switch (dir)
    {
    case DOWNSTREAM:
      ++m_dsPkts;
      m_dsBytes += pkt->GetSize();
      break;

    case UPSTREAM:
      ++m_usPkts;
      m_usBytes += pkt->GetSize();
      break;
    }
}

int AccessPort::ApplyProfile(Ptr<MCastProfile>prof)
{
  m_mcast_prof = prof;
  return 0;
}

void AccessPort::NasInitiatedMulticastGroup(const Address &mcGroup, bool allow)
{
  NS_LOG_FUNCTION(this << mcGroup << allow);
  if (allow)
    m_mcastGroups[mcGroup] = Time::Max();
  else
    m_mcastGroups.erase(mcGroup);
}

bool AccessPort::EnterMulticastGroup(const Address &mcGroup)
{
  NS_LOG_FUNCTION(this << mcGroup);

  if (m_mcast_prof == nullptr)
    {
      NS_LOG_LOGIC("No multicast profile associated");
      return false;
    }

  if (m_mcastGroups.find(mcGroup) != m_mcastGroups.end())
    {
      /* already authorized */
      if (m_mcastGroups[mcGroup] != Time::Max())
        m_mcastGroups[mcGroup] = Simulator::Now();
      return false;
    }

  switch (m_mcast_prof->GetFlowPolicy(mcGroup))
    {
    case ListClass::WHITELIST:
      m_mcastGroups[mcGroup] = Simulator::Now();
      return true;

    case ListClass::GREYLIST:
      /* Ask NAS */
      m_accessNode->GetAncpAgent()->SendMCastAdmissionCtl(m_circuitId,
                                                          AncpTlvMCastCommand::CMD_ADD,
                                                          mcGroup);
      break;

    case ListClass::BLACKLIST:
      m_mcastGroups.erase(mcGroup);
      break; /* ignore */
    }
  return false;
}

bool AccessPort::LeaveMulticastGroup(const Address &mcGroup)
{
  NS_LOG_FUNCTION(this << mcGroup);

  if (m_mcast_prof == nullptr)
    {
      NS_LOG_LOGIC("No multicast profile associated");
      return false;
    }

  if (m_mcastGroups.find(mcGroup) == m_mcastGroups.end())
    return false;

  if (m_mcast_prof->GetFlowPolicy(mcGroup) == ListClass::GREYLIST)
    {
      /* Notify NAS */
      m_accessNode->GetAncpAgent()->SendMCastAdmissionCtl(m_circuitId,
                                                          AncpTlvMCastCommand::CMD_DELETE,
                                                          mcGroup);
    }

  m_mcastGroups.erase(mcGroup);
  return true;
}

bool AccessPort::IsMember(const Address &mcGroup) const
{
  NS_LOG_FUNCTION(this << mcGroup);

  auto it = m_mcastGroups.find(mcGroup);

  if (it == m_mcastGroups.end())
    return false;

  if ((Simulator::Now() - it->second) > Seconds(QUERY_INTERVAL))
    return false;

  return true;
}

void AccessPort::CleanupMulticastMembership()
{
  NS_LOG_FUNCTION(this);

  auto it = m_mcastGroups.begin();

  while (it != m_mcastGroups.end())
    {
      auto cur = it;
      ++it;

      if ((Simulator::Now() - cur->second) > (2 * QUERY_INTERVAL))
        {
          LeaveMulticastGroup(cur->first);
        }
    }
}
}
