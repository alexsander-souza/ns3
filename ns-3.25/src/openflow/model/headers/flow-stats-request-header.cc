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
#include "ns3/flow-stats-request-header.h"

NS_LOG_COMPONENT_DEFINE("FlowStatsRequestHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(FlowStatsRequestHeader);

FlowStatsRequestHeader::FlowStatsRequestHeader (void) :
  m_FMatchHeader(nullptr),
  m_tableId(0),
  m_outPort(0)
{
  NS_LOG_FUNCTION(this);
}

FlowStatsRequestHeader::FlowStatsRequestHeader (uint8_t tableId, uint16_t outPort) :
  m_FMatchHeader(nullptr),
  m_tableId(tableId),
  m_outPort(outPort)
{
  NS_LOG_FUNCTION(this);
}

FlowStatsRequestHeader::~FlowStatsRequestHeader ()
{
  NS_LOG_FUNCTION(this);
}

Ptr<FlowMatchHeader> FlowStatsRequestHeader::GetFlowMatch() const
{
  return m_FMatchHeader;
}

void FlowStatsRequestHeader::SetFlowMatch(Ptr<FlowMatchHeader> header)
{
  m_FMatchHeader = header;
}

uint8_t FlowStatsRequestHeader::GetTableId() const
{
  return m_tableId;
}

void FlowStatsRequestHeader::SetTableId(uint8_t tableId)
{
  m_tableId = tableId;
}

uint16_t FlowStatsRequestHeader::GetOutPort() const
{
  return m_outPort;
}

void FlowStatsRequestHeader::SetOutPort(uint16_t outPort)
{
  m_outPort = outPort;
}

TypeId FlowStatsRequestHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::FlowStatsRequestHeader")
                      .SetParent<Header> ()
                      .AddConstructor<FlowStatsRequestHeader> ()
  ;

  return tid;
}

TypeId FlowStatsRequestHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void FlowStatsRequestHeader::Print(std::ostream &os) const
{
  os << "Flow Stats Request Header" << std::endl
     << " Table Id: " << uint(m_tableId) << std::endl
     << " Out Port: " << uint(m_outPort) << std::endl
     << std::endl;
}

uint32_t FlowStatsRequestHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 44;   // 40 (FlowMatch) + 1 + 1 (pad: align 32) + 2 = 44
}

void FlowStatsRequestHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader->Serialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  start.WriteU8(m_tableId);
  start.WriteU8(0);
  start.WriteHtonU16(m_outPort);
}

uint32_t FlowStatsRequestHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader = Create<FlowMatchHeader>();
  m_FMatchHeader->Deserialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  m_tableId = start.ReadU8();
  start.Next();
  m_outPort = start.ReadNtohU16();

  return GetSerializedSize();
}
} // namespace ns3
