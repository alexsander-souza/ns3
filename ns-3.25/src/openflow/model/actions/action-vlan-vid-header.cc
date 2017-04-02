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
#include "ns3/action-vlan-vid-header.h"

NS_LOG_COMPONENT_DEFINE("ActionVlanVidHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionVlanVidHeader);

ActionVlanVidHeader::ActionVlanVidHeader (void) :
  m_vlanVid(0)
{
  NS_LOG_FUNCTION(this);
}

ActionVlanVidHeader::ActionVlanVidHeader (uint16_t vlanVid) :
  m_vlanVid(vlanVid)
{
  NS_LOG_FUNCTION(this);
}

ActionVlanVidHeader::~ActionVlanVidHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t ActionVlanVidHeader::GetVlanVid(void) const
{
  return m_vlanVid;
}

void ActionVlanVidHeader::SetVlanVid(uint16_t vlanVid)
{
  m_vlanVid = vlanVid;
}

TypeId ActionVlanVidHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionVlanVidHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionVlanVidHeader> ()
  ;

  return tid;
}

TypeId ActionVlanVidHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionVlanVidHeader::Print(std::ostream &os) const
{
  os << "Action VLAN VID Header" << std::endl
     << " VLAN VID: " << uint(m_vlanVid) << std::endl
     << std::endl;
}

uint32_t ActionVlanVidHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 2 (vlanVid) + 2 (pad: align 64)
}

void ActionVlanVidHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_vlanVid);
  start.WriteU8(0, 2);
}

uint32_t ActionVlanVidHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_vlanVid = start.ReadNtohU16();
  start.Next(2);

  return GetSerializedSize();
}
} // namespace ns3
