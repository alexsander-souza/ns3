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

#include <string>
#include <sstream>
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/radius-client.h"
#include "ns3/bng-session.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("BngSession");

SubscriberSession::SubscriberSession(const Mac48Address &anId,
                                     const std::string& circuitId,
                                     Ptr<RadiusClient> radClient) :
  m_anId(anId),
  m_address4(Ipv4Address::GetAny()),
  m_address6(Ipv6Address::GetAny()),
  m_active(false),
  m_circuitId(circuitId),
  m_upRate(0),
  m_downRate(0),
  m_startTime(Simulator::Now()),
  m_lastSeen(Simulator::Now()),
  m_radiusClient(radClient)
{
  NS_LOG_FUNCTION(this << anId << circuitId);

  m_sessionId = SubscriberSession::GenerateId(m_anId, m_circuitId) + "-"
                + std::to_string(Simulator::Now().GetInteger());
}

SubscriberSession::~SubscriberSession()
{
}

void SubscriberSession::SetAddressIp(const Address& ip)
{
  NS_LOG_FUNCTION(this << m_circuitId << ip);

  if (Ipv4Address::IsMatchingType(ip))
    {
      m_address4 = Ipv4Address::ConvertFrom(ip);
    }
  else if (Ipv6Address::IsMatchingType(ip))
    {
      m_address6 = Ipv6Address::ConvertFrom(ip);
    }

  if (m_radiusClient)
    {
      m_radiusClient->DoStartAccounting(RadiusAVP::RAD_ACCT_UPDATE,
                                        m_sessionId, m_circuitId);
      /* FIXME create a new method and add more AVPs */
    }
}

Ipv4Address SubscriberSession::GetIpv4Address() const
{
  return m_address4;
}

Ipv6Address SubscriberSession::GetIpv6Address() const
{
  return m_address6;
}

Mac48Address SubscriberSession::GetAnAddress() const
{
  return m_anId;
}

bool SubscriberSession::operator==(const Ipv4Address &dst4) const
{
  return m_address4 == dst4;
}

bool SubscriberSession::operator==(const Ipv6Address &dst6) const
{
  return m_address6 == dst6;
}
bool SubscriberSession::IsActive() const
{
  return m_active;
}

void SubscriberSession::UpdateLineStatus(bool active, uint32_t upRate, uint32_t downRate)
{
  NS_LOG_FUNCTION(this << m_sessionId << active);

  m_active = active;

  if (active)
    {
      m_upRate = upRate;
      m_downRate = downRate;
      m_lastSeen = m_startTime = Simulator::Now();
      if (m_radiusClient)
        {
          m_radiusClient->DoStartAccounting(RadiusAVP::RAD_ACCT_START,
                                            m_sessionId, m_circuitId);
        }
    }
  else
    {
      m_upRate = m_downRate = 0;
      m_address4 = Ipv4Address::GetAny();
      m_address6 = Ipv6Address::GetAny();

      if (m_radiusClient)
        {
          m_radiusClient->DoStopAccounting(RadiusAVP::RAD_ACCT_STOP,
                                           m_sessionId, GetUptime().GetSeconds(),
                                           RadiusAVP::RAD_TERM_CAUSE_USER_REQUEST);
          /* FIXME add circuit_id AVP */
        }
    }
}

Time SubscriberSession::GetUptime() const
{
  return Simulator::Now() - m_startTime;
}

Time SubscriberSession::GetIdletime() const
{
  return Simulator::Now() - m_lastSeen;
}

const std::string &SubscriberSession::GetSessionId() const
{
  return m_sessionId;
}

const std::string &SubscriberSession::GetCircuitId() const
{
  return m_circuitId;
}

void SubscriberSession::AccPacket(enum PacketDir dir, Ptr<const Packet> pkt)
{
  NS_LOG_FUNCTION(this << m_sessionId);
  m_lastSeen = Simulator::Now();
}

std::string SubscriberSession::GenerateId(const Mac48Address &anId,
                                          const std::string &circuitId)
{
  std::ostringstream oss;

  oss << anId << "-" << circuitId;
  return oss.str();
}
} // ns3
