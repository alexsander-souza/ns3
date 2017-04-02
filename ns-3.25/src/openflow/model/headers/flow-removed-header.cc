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
#include "ns3/flow-match-header.h"
#include "ns3/flow-removed-header.h"

NS_LOG_COMPONENT_DEFINE("FlowRemovedHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(FlowRemovedHeader);

FlowRemovedHeader::FlowRemovedHeader (void) :
  m_FMatchHeader(nullptr),
  m_cookie(0),
  m_priority(0),
  m_reason(0),
  m_durationSec(0),
  m_durationNSec(0),
  m_idleTimeout(0),
  m_packetCount(0),
  m_byteCount(0)
{
  NS_LOG_FUNCTION(this);
}

FlowRemovedHeader::FlowRemovedHeader (uint64_t cookie, uint16_t priority, uint8_t reason,
                                      uint32_t durationSec, uint32_t durationNSec, uint16_t idleTimeout,
                                      uint64_t packetCount, uint64_t byteCount) :
  m_FMatchHeader(nullptr),
  m_cookie(cookie),
  m_priority(priority),
  m_reason(reason),
  m_durationSec(durationSec),
  m_durationNSec(durationNSec),
  m_idleTimeout(idleTimeout),
  m_packetCount(packetCount),
  m_byteCount(byteCount)
{
  NS_LOG_FUNCTION(this);
}

FlowRemovedHeader::~FlowRemovedHeader ()
{
  NS_LOG_FUNCTION(this);
}

Ptr<FlowMatchHeader> FlowRemovedHeader::GetFlowMatch() const
{
  return m_FMatchHeader;
}

void FlowRemovedHeader::SetFlowMatch(Ptr<FlowMatchHeader> FMatchHeader)
{
  m_FMatchHeader = FMatchHeader;
}

uint64_t FlowRemovedHeader::GetCookie() const
{
  return m_cookie;
}

void FlowRemovedHeader::SetCookie(uint64_t cookie)
{
  m_cookie = cookie;
}

uint16_t FlowRemovedHeader::GetPriority() const
{
  return m_priority;
}

void FlowRemovedHeader::SetPriority(uint16_t priority)
{
  m_priority = priority;
}

uint8_t FlowRemovedHeader::GetReason() const
{
  return m_reason;
}

void FlowRemovedHeader::SetReason(uint8_t reason)
{
  m_reason = reason;
}

uint32_t FlowRemovedHeader::GetDurationSec() const
{
  return m_durationSec;
}

void FlowRemovedHeader::SetDurationSec(uint32_t durationSec)
{
  m_durationSec = durationSec;
}

uint32_t FlowRemovedHeader::GetDurationNSec() const
{
  return m_durationNSec;
}

void FlowRemovedHeader::SetDurationNSec(uint32_t durationNSec)
{
  m_durationNSec = durationNSec;
}

uint16_t FlowRemovedHeader::GetIdleTimeout() const
{
  return m_idleTimeout;
}

void FlowRemovedHeader::SetIdleTimeout(uint16_t idleTimeout)
{
  m_idleTimeout = idleTimeout;
}

uint64_t FlowRemovedHeader::GetPacketCount() const
{
  return m_packetCount;
}

void FlowRemovedHeader::SetPacketCount(uint64_t packetCount)
{
  m_packetCount = packetCount;
}

uint64_t FlowRemovedHeader::GetByteCount() const
{
  return m_byteCount;
}

void FlowRemovedHeader::SetByteCount(uint64_t byteCount)
{
  m_byteCount = byteCount;
}

TypeId FlowRemovedHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::FlowRemovedHeader")
                      .SetParent<Header> ()
                      .AddConstructor<FlowRemovedHeader> ()
  ;

  return tid;
}

TypeId FlowRemovedHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void FlowRemovedHeader::Print(std::ostream &os) const
{
  os << "Flow Removed Header" << std::endl
     << " Cookie: " << uint(m_cookie) << std::endl
     << " Priority: " << uint(m_priority) << std::endl
     << " Reason: " << uint(m_reason) << std::endl
     << " Duration (Seconds): " << uint(m_durationSec) << std::endl
     << " Duration (Nanoseconds): " << uint(m_durationNSec) << std::endl
     << " Idle Timeout: " << uint(m_idleTimeout) << std::endl
     << " Packet Count: " << uint(m_packetCount) << std::endl
     << " Byte Count: " << uint(m_byteCount) << std::endl
     << std::endl;
}

uint32_t FlowRemovedHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 80;   // 40 (FlowMatch) + 8 + 2 + 1 + 1 (pad: align 32) + 4 + 4 + 2 + 2 (pad: align 64) + 8 + 8 = 80
}

void FlowRemovedHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader->Serialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  start.WriteHtonU64(m_cookie);
  start.WriteHtonU16(m_priority);
  start.WriteU8(m_reason);
  start.WriteU8(0);
  start.WriteHtonU32(m_durationSec);
  start.WriteHtonU32(m_durationNSec);
  start.WriteHtonU16(m_idleTimeout);
  start.WriteU8(0, 2);
  start.WriteHtonU64(m_packetCount);
  start.WriteHtonU64(m_byteCount);
}

uint32_t FlowRemovedHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader = Create<FlowMatchHeader>();
  m_FMatchHeader->Deserialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  m_cookie = start.ReadNtohU64();
  m_priority = start.ReadNtohU16();
  m_reason = start.ReadU8();
  start.Next();
  m_durationSec = start.ReadNtohU32();
  m_durationNSec = start.ReadNtohU32();
  m_idleTimeout = start.ReadNtohU16();
  start.Next(2);
  m_packetCount = start.ReadNtohU64();
  m_byteCount = start.ReadNtohU64();

  return GetSerializedSize();
}
} // namespace ns3
