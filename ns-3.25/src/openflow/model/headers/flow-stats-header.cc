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
#include "ns3/flow-stats-header.h"

NS_LOG_COMPONENT_DEFINE("FlowStatsHeader");

namespace ns3
{
using namespace action_utils;

NS_OBJECT_ENSURE_REGISTERED(FlowStatsHeader);

FlowStatsHeader::FlowStatsHeader (void) :
  m_length(0),
  m_tableId(0),
  m_FMatchHeader(nullptr),
  m_durationSec(0),
  m_durationNsec(0),
  m_priority(0),
  m_idleTimeout(0),
  m_hardTimeout(0),
  m_cookie(0),
  m_packetCount(0),
  m_byteCount(0)
{
  NS_LOG_FUNCTION(this);
}

FlowStatsHeader::FlowStatsHeader (uint16_t length, uint8_t tableId, uint32_t durationSec,
                                  uint32_t durationNsec, uint16_t priority, uint16_t idleTimeout,
                                  uint16_t hardTimeout, uint64_t cookie, uint64_t packetCount,
                                  uint64_t byteCount) :
  m_length(length),
  m_tableId(tableId),
  m_FMatchHeader(nullptr),
  m_durationSec(durationSec),
  m_durationNsec(durationNsec),
  m_priority(priority),
  m_idleTimeout(idleTimeout),
  m_hardTimeout(hardTimeout),
  m_cookie(cookie),
  m_packetCount(packetCount),
  m_byteCount(byteCount)
{
  NS_LOG_FUNCTION(this);
}

FlowStatsHeader::~FlowStatsHeader ()
{
  NS_LOG_FUNCTION(this);

  m_actionsList.clear();
}

uint16_t FlowStatsHeader::GetLength() const
{
  return m_length;
}

void FlowStatsHeader::SetLength(uint16_t length)
{
  m_length = length;
}

uint8_t FlowStatsHeader::GetTableId() const
{
  return m_tableId;
}

void FlowStatsHeader::SetTableId(uint8_t tableId)
{
  m_tableId = tableId;
}

Ptr<FlowMatchHeader> FlowStatsHeader::GetFlowMatch() const
{
  return m_FMatchHeader;
}

void FlowStatsHeader::SetFlowMatch(Ptr<FlowMatchHeader> header)
{
  m_FMatchHeader = header;
}

uint32_t FlowStatsHeader::GetDurationSec() const
{
  return m_durationSec;
}

void FlowStatsHeader::SetDurationSec(uint32_t durationSec)
{
  m_durationSec = durationSec;
}

uint32_t FlowStatsHeader::GetDurationNsec() const
{
  return m_durationNsec;
}

void FlowStatsHeader::SetDurationNsec(uint32_t durationNsec)
{
  m_durationNsec = durationNsec;
}

uint16_t FlowStatsHeader::GetPriority() const
{
  return m_priority;
}

void FlowStatsHeader::SetPriority(uint16_t priority)
{
  m_priority = priority;
}

uint16_t FlowStatsHeader::GetIdleTimeout() const
{
  return m_idleTimeout;
}

void FlowStatsHeader::SetIdleTimeout(uint16_t idleTimeout)
{
  m_idleTimeout = idleTimeout;
}

uint16_t FlowStatsHeader::GetHardTimeout() const
{
  return m_hardTimeout;
}

void FlowStatsHeader::SetHardTimeout(uint16_t hardTimeout)
{
  m_hardTimeout = hardTimeout;
}

uint64_t FlowStatsHeader::GetCookie() const
{
  return m_cookie;
}

void FlowStatsHeader::SetCookie(uint64_t cookie)
{
  m_cookie = cookie;
}

uint64_t FlowStatsHeader::GetPacketCount() const
{
  return m_packetCount;
}

void FlowStatsHeader::SetPacketCount(uint64_t packetCount)
{
  m_packetCount = packetCount;
}

uint64_t FlowStatsHeader::GetByteCount() const
{
  return m_byteCount;
}

void FlowStatsHeader::SetByteCount(uint64_t byteCount)
{
  m_byteCount = byteCount;
}

void FlowStatsHeader::AddAction(Ptr<ActionHeader> action)
{
  NS_LOG_FUNCTION(this << *action);
  m_actionsList.push_back(action);
}

void FlowStatsHeader::AddActionsList(ActionsList &actionsList)
{
  NS_LOG_FUNCTION(this);
  m_actionsList.merge(actionsList);
}

const ActionsList& FlowStatsHeader::GetActionsList(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_actionsList;
}

Ptr<ActionHeader> FlowStatsHeader::GetActionByType(uint16_t type) const
{
  NS_LOG_FUNCTION(this << type);

  for (auto &act: m_actionsList)
    {
      if (act->GetType() == type)
        return act;
    }
  NS_LOG_WARN("Action not found");
  return 0;
}

void FlowStatsHeader::PrintActionsList(std::ostream &os) const
{
  for (auto &act: m_actionsList)
    {
      act->Print(os);

      switch (act->GetType())
        {
        case OFPAT_OUTPUT:
          act->GetOutput()->Print(os);
          break;

        case OFPAT_SET_VLAN_VID:
          act->GetVlanVid()->Print(os);
          break;

        case OFPAT_SET_VLAN_PCP:
          act->GetVlanPcp()->Print(os);
          break;

        case OFPAT_SET_DL_SRC:
          act->GetDlSrcAddress()->Print(os);
          break;

        case OFPAT_SET_DL_DST:
          act->GetDlDstAddress()->Print(os);
          break;

        case OFPAT_SET_NW_SRC:
          act->GetNwSrcAddress()->Print(os);
          break;

        case OFPAT_SET_NW_DST:
          act->GetNwDstAddress()->Print(os);
          break;

        case OFPAT_SET_TP_SRC:
          act->GetTpSrc()->Print(os);
          break;

        case OFPAT_SET_TP_DST:
          act->GetTpDst()->Print(os);
          break;

        case OFPAT_VENDOR:
          act->GetVendor()->Print(os);
          break;
        }
    }
}

TypeId FlowStatsHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::FlowStatsHeader")
                      .SetParent<Header> ()
                      .AddConstructor<FlowStatsHeader> ()
  ;

  return tid;
}

TypeId FlowStatsHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void FlowStatsHeader::Print(std::ostream &os) const
{
  os << "Flow Stats Header" << std::endl
     << " Length: " << uint(m_length) << std::endl
     << " Table ID: " << uint(m_tableId) << std::endl
     << " Duration (Seconds): " << uint(m_durationSec) << std::endl
     << " Duration (Nanoseconds): " << uint(m_durationNsec) << std::endl
     << " Priority: " << uint(m_priority) << std::endl
     << " Idle Timeout: " << uint(m_idleTimeout) << std::endl
     << " Hard Timeout: " << uint(m_hardTimeout) << std::endl
     << " Cookie: " << uint(m_cookie) << std::endl
     << " Packet Count: " << uint(m_packetCount) << std::endl
     << " Byte Count: " << uint(m_byteCount) << std::endl
     << std::endl;
}

uint32_t FlowStatsHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  uint32_t msgSize = 88;   // 2 + 1 + 1 (pad) + 40 (FlowMatch) + 4 + 4 + 2 + 2 + 2 +
  // + 6 (pad: align 64) + 8 + 8 + 8 = 88

  for (auto &act: m_actionsList)
    {
      msgSize += act->GetSerializedSize();
    }

  return msgSize;
}

void FlowStatsHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_length);
  start.WriteU8(m_tableId);
  start.WriteU8(0);
  m_FMatchHeader->Serialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  start.WriteHtonU32(m_durationSec);
  start.WriteHtonU32(m_durationNsec);
  start.WriteHtonU16(m_priority);
  start.WriteHtonU16(m_idleTimeout);
  start.WriteHtonU16(m_hardTimeout);
  start.WriteU8(0, 6);
  start.WriteHtonU64(m_cookie);
  start.WriteHtonU64(m_packetCount);
  start.WriteHtonU64(m_byteCount);

  for (auto &act: m_actionsList)
    {
      act->Serialize(start);
      start.Next(act->GetSerializedSize());
    }
}

uint32_t FlowStatsHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_length = start.ReadNtohU16();
  m_tableId = start.ReadU8();
  start.Next();
  m_FMatchHeader = Create<FlowMatchHeader>();
  m_FMatchHeader->Deserialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  m_durationSec = start.ReadNtohU32();
  m_durationNsec = start.ReadNtohU32();
  m_priority = start.ReadNtohU16();
  m_idleTimeout = start.ReadNtohU16();
  m_hardTimeout = start.ReadNtohU16();
  start.Next(6);
  m_cookie = start.ReadNtohU64();
  m_packetCount = start.ReadNtohU64();
  m_byteCount = start.ReadNtohU64();

  /* Read actions */
  while (start.IsEnd() == false)
    {
      Ptr<ActionHeader> action = Create<ActionHeader>();
      start.Next(action->Deserialize(start));
      m_actionsList.push_back(action);
    }

  return GetSerializedSize();
}
} // namespace ns3
