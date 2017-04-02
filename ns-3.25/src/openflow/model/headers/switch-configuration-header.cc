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
#include "ns3/switch-configuration-header.h"

NS_LOG_COMPONENT_DEFINE("SwitchConfigurationHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(SwitchConfigurationHeader);

SwitchConfigurationHeader::SwitchConfigurationHeader (void) :
  m_flags(0),
  m_maxFlowLen(0)
{
  NS_LOG_FUNCTION(this);
}

SwitchConfigurationHeader::SwitchConfigurationHeader (uint16_t flags, uint16_t length) :
  m_flags(flags),
  m_maxFlowLen(length)
{
  NS_LOG_FUNCTION(this);
}

SwitchConfigurationHeader::~SwitchConfigurationHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t SwitchConfigurationHeader::GetFlags() const
{
  return m_flags;
}

void SwitchConfigurationHeader::SetFlags(uint16_t flags)
{
  m_flags = flags;
}

uint16_t SwitchConfigurationHeader::GetMaxFlowLen() const
{
  return m_maxFlowLen;
}

void SwitchConfigurationHeader::SetMaxFlowLen(uint16_t length)
{
  m_maxFlowLen = length;
}

TypeId SwitchConfigurationHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::SwitchConfigurationHeader")
                      .SetParent<Header> ()
                      .AddConstructor<SwitchConfigurationHeader> ()
  ;

  return tid;
}

TypeId SwitchConfigurationHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void SwitchConfigurationHeader::Print(std::ostream &os) const
{
  os << "Switch Config Header" << std::endl
     << " Flags: " << uint(m_flags) << std::endl
     << " Max Flow Length: " << uint(m_maxFlowLen) << std::endl
     << std::endl;
}

uint32_t SwitchConfigurationHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 2 (flags) + 2 (maxFlowLen) = 4
}

void SwitchConfigurationHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_flags);
  start.WriteHtonU16(m_maxFlowLen);
}

uint32_t SwitchConfigurationHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_flags = start.ReadNtohU16();
  m_maxFlowLen = start.ReadNtohU16();

  return GetSerializedSize();
}
} // namespace ns3