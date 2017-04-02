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
#include "ns3/action-tp-port-header.h"

NS_LOG_COMPONENT_DEFINE("ActionTpPortHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionTpPortHeader);

ActionTpPortHeader::ActionTpPortHeader (void) :
  m_tpPort(0)
{
  NS_LOG_FUNCTION(this);
}

ActionTpPortHeader::ActionTpPortHeader (uint16_t tpPort) :
  m_tpPort(tpPort)
{
  NS_LOG_FUNCTION(this);
}

ActionTpPortHeader::~ActionTpPortHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t ActionTpPortHeader::GetTpPort(void) const
{
  return m_tpPort;
}

void ActionTpPortHeader::SetTpPort(uint16_t tpPort)
{
  m_tpPort = tpPort;
}

TypeId ActionTpPortHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionTpPortHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionTpPortHeader> ()
  ;

  return tid;
}

TypeId ActionTpPortHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionTpPortHeader::Print(std::ostream &os) const
{
  os << "Action TCP/UDP Port Header" << std::endl
     << " TCP/UDP Port: " << uint(m_tpPort) << std::endl
     << std::endl;
}

uint32_t ActionTpPortHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 2 (tpPort) + 2 (pad: align 64)
}

void ActionTpPortHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_tpPort);
  start.WriteU8(0, 2);
}

uint32_t ActionTpPortHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_tpPort = start.ReadNtohU16();
  start.Next(2);

  return GetSerializedSize();
}
} // namespace ns3
