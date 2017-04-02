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
#include "ns3/action-vlan-pcp-header.h"

NS_LOG_COMPONENT_DEFINE("ActionVlanPcpHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionVlanPcpHeader);

ActionVlanPcpHeader::ActionVlanPcpHeader (void) :
  m_vlanPcp(0)
{
  NS_LOG_FUNCTION(this);
}

ActionVlanPcpHeader::ActionVlanPcpHeader (uint8_t vlanPcp) :
  m_vlanPcp(vlanPcp)
{
  NS_LOG_FUNCTION(this);
}

ActionVlanPcpHeader::~ActionVlanPcpHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint8_t ActionVlanPcpHeader::GetVlanPcp(void) const
{
  return m_vlanPcp;
}

void ActionVlanPcpHeader::SetVlanPcp(uint8_t vlanPcp)
{
  m_vlanPcp = vlanPcp;
}

TypeId ActionVlanPcpHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionVlanPcpHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionVlanPcpHeader> ()
  ;

  return tid;
}

TypeId ActionVlanPcpHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionVlanPcpHeader::Print(std::ostream &os) const
{
  os << "Action VLAN PCP Header" << std::endl
     << " VLAN PCP: " << uint(m_vlanPcp) << std::endl
     << std::endl;
}

uint32_t ActionVlanPcpHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 1 (vlanPcp) + 3 (pad: align 64)
}

void ActionVlanPcpHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteU8(m_vlanPcp);
  start.WriteU8(0, 3);
}

uint32_t ActionVlanPcpHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_vlanPcp = start.ReadU8();
  start.Next(3);

  return GetSerializedSize();
}
} // namespace ns3
