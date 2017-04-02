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
#include "ns3/flow-stats-header.h"
#include "ns3/port-stats-header.h"
#include "ns3/stats-reply-header.h"

NS_LOG_COMPONENT_DEFINE("StatsReplyHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(StatsReplyHeader);

StatsReplyHeader::StatsReplyHeader (void) :
  m_type(0),
  m_flags(0),
  m_FSReplyHeader(nullptr),
  m_PSReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

StatsReplyHeader::StatsReplyHeader (uint16_t type, uint16_t flags) :
  m_type(type),
  m_flags(flags),
  m_FSReplyHeader(nullptr),
  m_PSReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

StatsReplyHeader::~StatsReplyHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t StatsReplyHeader::GetType() const
{
  return m_type;
}

void StatsReplyHeader::SetType(uint16_t type)
{
  m_type = type;
}

uint16_t StatsReplyHeader::GetFlags() const
{
  return m_flags;
}

void StatsReplyHeader::SetFlags(uint16_t flags)
{
  m_flags = flags;
}

Ptr<FlowStatsHeader> StatsReplyHeader::GetFlowStatsReply() const
{
  return m_FSReplyHeader;
}

void StatsReplyHeader::SetFlowStatsReply(Ptr<FlowStatsHeader> header)
{
  m_FSReplyHeader = header;
}

Ptr<PortStatsHeader> StatsReplyHeader::GetPortStatsReply() const
{
  return m_PSReplyHeader;
}

void StatsReplyHeader::SetPortStatsReply(Ptr<PortStatsHeader> header)
{
  m_PSReplyHeader = header;
}

TypeId StatsReplyHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::StatsReplyHeader")
                      .SetParent<Header> ()
                      .AddConstructor<StatsReplyHeader> ()
  ;

  return tid;
}

TypeId StatsReplyHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void StatsReplyHeader::Print(std::ostream &os) const
{
  os << "Stats Reply Header" << std::endl
     << " Type: " << uint(m_type) << std::endl
     << " Flags: " << uint(m_flags) << std::endl
     << std::endl;
}

uint32_t StatsReplyHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  uint32_t size = 0;

  switch (m_type)
    {
    case OFPST_FLOW:
      size = m_FSReplyHeader->GetSerializedSize();
      break;

    case OFPST_PORT:
      size = m_PSReplyHeader->GetSerializedSize();
      break;
    }

  size += 4;

  return size;   // 2 + 2 + (body)
}

void StatsReplyHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_type);
  start.WriteHtonU16(m_flags);

  switch (m_type)
    {
    case OFPST_FLOW:
      m_FSReplyHeader->Serialize(start);
      break;

    case OFPST_PORT:
      m_PSReplyHeader->Serialize(start);
      break;
    }
}

uint32_t StatsReplyHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_type = start.ReadNtohU16();
  m_flags = start.ReadNtohU16();

  switch (m_type)
    {
    case OFPST_FLOW:
      m_FSReplyHeader = Create<FlowStatsHeader>();
      m_FSReplyHeader->Deserialize(start);
      break;

    case OFPST_PORT:
      m_PSReplyHeader = Create<PortStatsHeader>();
      m_PSReplyHeader->Deserialize(start);
      break;
    }

  return GetSerializedSize();
}
} // namespace ns3
