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
#include "ns3/openflow-lib.h"
#include "ns3/flow-stats-request-header.h"
#include "ns3/port-stats-request-header.h"
#include "ns3/stats-request-header.h"

NS_LOG_COMPONENT_DEFINE("StatsRequestHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(StatsRequestHeader);

StatsRequestHeader::StatsRequestHeader (void) :
  m_type(0),
  m_flags(0),
  m_FSReqHeader(nullptr),
  m_PSReqHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

StatsRequestHeader::StatsRequestHeader (uint16_t type, uint16_t flags) :
  m_type(type),
  m_flags(flags),
  m_FSReqHeader(nullptr),
  m_PSReqHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

StatsRequestHeader::~StatsRequestHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t StatsRequestHeader::GetType() const
{
  return m_type;
}

void StatsRequestHeader::SetType(uint16_t type)
{
  m_type = type;
}

uint16_t StatsRequestHeader::GetFlags() const
{
  return m_flags;
}

void StatsRequestHeader::SetFlags(uint16_t flags)
{
  m_flags = flags;
}

Ptr<FlowStatsRequestHeader> StatsRequestHeader::GetFlowStatsRequest() const
{
  return m_FSReqHeader;
}

void StatsRequestHeader::SetFlowStatsRequest(Ptr<FlowStatsRequestHeader> header)
{
  m_FSReqHeader = header;
}

Ptr<PortStatsRequestHeader> StatsRequestHeader::GetPortStatsRequest() const
{
  return m_PSReqHeader;
}

void StatsRequestHeader::SetPortStatsRequest(Ptr<PortStatsRequestHeader> header)
{
  m_PSReqHeader = header;
}

TypeId StatsRequestHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::StatsRequestHeader")
                      .SetParent<Header> ()
                      .AddConstructor<StatsRequestHeader> ()
  ;

  return tid;
}

TypeId StatsRequestHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void StatsRequestHeader::Print(std::ostream &os) const
{
  os << "Stats Request Header" << std::endl
     << " Type: " << uint(m_type) << std::endl
     << " Flags: " << uint(m_flags) << std::endl
     << std::endl;
}

uint32_t StatsRequestHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  uint32_t size = 0;

  switch (m_type)
    {
    case OFPST_FLOW:
      size = m_FSReqHeader->GetSerializedSize();
      break;

    case OFPST_PORT:
      size = m_PSReqHeader->GetSerializedSize();
      break;
    }

  size += 4;

  return size;   // 2 + 2 + (body)
}

void StatsRequestHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_type);
  start.WriteHtonU16(m_flags);

  switch (m_type)
    {
    case OFPST_FLOW:
      m_FSReqHeader->Serialize(start);
      break;

    case OFPST_PORT:
      m_PSReqHeader->Serialize(start);
      break;
    }
}

uint32_t StatsRequestHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_type = start.ReadNtohU16();
  m_flags = start.ReadNtohU16();

  switch (m_type)
    {
    case OFPST_FLOW:
      m_FSReqHeader = Create<FlowStatsRequestHeader>();
      m_FSReqHeader->Deserialize(start);
      break;

    case OFPST_PORT:
      m_PSReqHeader = Create<PortStatsRequestHeader>();
      m_PSReqHeader->Deserialize(start);
      break;
    }

  return GetSerializedSize();
}
} // namespace ns3
