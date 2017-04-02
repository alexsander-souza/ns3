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
#include "ns3/physical-port-header.h"
#include "ns3/port-status-header.h"

NS_LOG_COMPONENT_DEFINE("PortStatusHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PortStatusHeader);

PortStatusHeader::PortStatusHeader (void) :
  m_reason(0),
  m_PhyPortHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

PortStatusHeader::PortStatusHeader (uint8_t reason) :
  m_reason(reason),
  m_PhyPortHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

PortStatusHeader::~PortStatusHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint8_t PortStatusHeader::GetReason() const
{
  return m_reason;
}

void PortStatusHeader::SetReason(uint8_t reason)
{
  m_reason = reason;
}

Ptr<PhysicalPortHeader> PortStatusHeader::GetPhysicalPort() const
{
  return m_PhyPortHeader;
}

void PortStatusHeader::SetPhysicalPort(Ptr<PhysicalPortHeader> PhyPortHeader)
{
  m_PhyPortHeader = PhyPortHeader;
}

TypeId PortStatusHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PortStatusHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PortStatusHeader> ()
  ;

  return tid;
}

TypeId PortStatusHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PortStatusHeader::Print(std::ostream &os) const
{
  os << "Port Status Header" << std::endl
     << " Reason: " << uint(m_reason) << std::endl
     << std::endl;
}

uint32_t PortStatusHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 56;   // 1 + 7 (pad: align 64) +  48 (PhyPort) = 56
}

void PortStatusHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteU8(m_reason);
  start.WriteU8(0, 7);
  m_PhyPortHeader->Serialize(start);
  start.Next(m_PhyPortHeader->GetSerializedSize());
}

uint32_t PortStatusHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_reason = start.ReadU8();
  start.Next(7);
  m_PhyPortHeader = Create<PhysicalPortHeader>();
  m_PhyPortHeader->Deserialize(start);
  start.Next(m_PhyPortHeader->GetSerializedSize());

  return GetSerializedSize();
}
} // namespace ns3
