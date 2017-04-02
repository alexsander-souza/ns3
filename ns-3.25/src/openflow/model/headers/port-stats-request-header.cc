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
 *
 */

#include "ns3/log.h"
#include "ns3/port-stats-request-header.h"

NS_LOG_COMPONENT_DEFINE("PortStatsRequestHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PortStatsRequestHeader);

PortStatsRequestHeader::PortStatsRequestHeader (void) :
  m_portNumber(0)
{
  NS_LOG_FUNCTION(this);
}

PortStatsRequestHeader::PortStatsRequestHeader (uint16_t portNumber) :
  m_portNumber(portNumber)
{
  NS_LOG_FUNCTION(this);
}

PortStatsRequestHeader::~PortStatsRequestHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t PortStatsRequestHeader::GetPortNumber() const
{
  return m_portNumber;
}

void PortStatsRequestHeader::SetPortNumber(uint16_t portNumber)
{
  m_portNumber = portNumber;
}

TypeId PortStatsRequestHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PortStatsRequestHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PortStatsRequestHeader> ()
  ;

  return tid;
}

TypeId PortStatsRequestHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PortStatsRequestHeader::Print(std::ostream &os) const
{
  os << "Port Stats Request Header" << std::endl
     << " Port Number: " << uint(m_portNumber) << std::endl
     << std::endl;
}

uint32_t PortStatsRequestHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 8;   // 2 + 6 (pad: align 64) = 8
}

void PortStatsRequestHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_portNumber);
  start.WriteU8(0, 6);
}

uint32_t PortStatsRequestHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_portNumber = start.ReadNtohU16();
  start.Next(6);

  return GetSerializedSize();
}
} // namespace ns3

