/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 */

#include "igmpv2.h"
#include "ns3/packet.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("Igmpv2Header");

namespace ns3
{

/********************************************************
 *        Igmpv2Header
 ********************************************************/

NS_OBJECT_ENSURE_REGISTERED(Igmpv2Header);

TypeId Igmpv2Header::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::Igmpv2Header").SetParent<Header>().AddConstructor<Igmpv2Header>();
  return tid;
}

Igmpv2Header::Igmpv2Header() : m_type(0), m_maxRespTime(0), m_calcChecksum(false)
{
  NS_LOG_FUNCTION(this);
}

Igmpv2Header::~Igmpv2Header()
{
  NS_LOG_FUNCTION(this);
}

void Igmpv2Header::EnableChecksum(void)
{
  NS_LOG_FUNCTION(this);
  m_calcChecksum = true;
}

TypeId Igmpv2Header::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

uint32_t Igmpv2Header::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 8;
}

void Igmpv2Header::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);
  Buffer::Iterator i = start;

  i.WriteU8(m_type);
  i.WriteU8(m_maxRespTime);
  if(m_calcChecksum)
    {
      i = start;
      uint16_t checksum = i.CalculateIpChecksum(i.GetSize());
      i = start;
      i.Next(2);
      i.WriteU16(checksum);
    }
  else
    {
      i.WriteHtonU16(0);
    }
  WriteTo(i, m_groupAddress);
}

uint32_t Igmpv2Header::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);
  m_type = start.ReadU8();
  m_maxRespTime = start.ReadU8();
  start.Next(2); // uint16_t checksum = start.ReadNtohU16 ();
  ReadFrom(start, m_groupAddress);
  return 8;
}

void Igmpv2Header::Print(std::ostream &os) const
{
  NS_LOG_FUNCTION(this << &os);
  os << "type=" << (uint32_t)m_type;
}

void Igmpv2Header::SetType(uint8_t type)
{
  NS_LOG_FUNCTION(this << static_cast<uint32_t>(type));
  m_type = type;
}

void Igmpv2Header::SetMaxRespTime(uint8_t maxRespTime)
{
  NS_LOG_FUNCTION(this << static_cast<uint32_t>(maxRespTime));
  m_maxRespTime = maxRespTime;
}

void Igmpv2Header::SetGroupAddress(Ipv4Address groupAddress)
{
  NS_LOG_FUNCTION(this << static_cast<Ipv4Address>(groupAddress));
  m_groupAddress = groupAddress;
}

uint8_t Igmpv2Header::GetType(void) const
{
  NS_LOG_FUNCTION(this);
  return m_type;
}

uint8_t Igmpv2Header::GetMaxRespTime(void) const
{
  NS_LOG_FUNCTION(this);
  return m_maxRespTime;
}

Ipv4Address Igmpv2Header::GetGroupAddress(void) const
{
  NS_LOG_FUNCTION(this);
  return m_groupAddress;
}

} // namespace ns3
