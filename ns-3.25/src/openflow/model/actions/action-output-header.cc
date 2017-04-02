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
#include "ns3/action-output-header.h"
#include "ns3/action-header.h"

NS_LOG_COMPONENT_DEFINE("ActionOutputHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionOutputHeader);

void ActionOutputHeader::ConvertToStruct(ofp_action_output &ofout, Ptr<ActionHeader> header)
{
  ofout.type = header->GetType();
  ofout.len = header->GetLength();
  ofout.port = header->GetOutput()->GetPort();
  ofout.max_len = header->GetOutput()->GetMaxLength();
}

ActionOutputHeader::ActionOutputHeader (void) :
  m_port(0),
  m_maxLen(0)
{
  NS_LOG_FUNCTION(this);
}

ActionOutputHeader::ActionOutputHeader (ofp_action_output* header) :
  m_port(header->port),
  m_maxLen(header->max_len)
{
  NS_LOG_FUNCTION(this);
}

ActionOutputHeader::ActionOutputHeader (uint16_t port, uint16_t maxLen) :
  m_port(port),
  m_maxLen(maxLen)
{
  NS_LOG_FUNCTION(this);
}

ActionOutputHeader::~ActionOutputHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t ActionOutputHeader::GetPort(void) const
{
  return m_port;
}

void ActionOutputHeader::SetPort(uint16_t port)
{
  m_port = port;
}

uint16_t ActionOutputHeader::GetMaxLength(void) const
{
  return m_maxLen;
}

void ActionOutputHeader::SetMaxLength(uint16_t maxLen)
{
  m_maxLen = maxLen;
}

TypeId ActionOutputHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionOutputHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionOutputHeader> ()
  ;

  return tid;
}

TypeId ActionOutputHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionOutputHeader::Print(std::ostream &os) const
{
  os << "Action Output Header" << std::endl
     << " Port: " << uint(m_port) << std::endl
     << " Max Length: " << uint(m_maxLen) << std::endl
     << std::endl;
}

uint32_t ActionOutputHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 2 (port) + 2 (maxLen)
}

void ActionOutputHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_port);
  start.WriteHtonU16(m_maxLen);
}

uint32_t ActionOutputHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_port = start.ReadNtohU16();
  m_maxLen = start.ReadNtohU16();

  return GetSerializedSize();
}
} // namespace ns3
