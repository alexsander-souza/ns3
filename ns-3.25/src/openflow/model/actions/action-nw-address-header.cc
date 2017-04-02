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
#include "ns3/action-nw-address-header.h"

NS_LOG_COMPONENT_DEFINE("ActionNwAddressHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionNwAddressHeader);

ActionNwAddressHeader::ActionNwAddressHeader (void) :
  m_nwAddress(0)
{
  NS_LOG_FUNCTION(this);
}

ActionNwAddressHeader::ActionNwAddressHeader (uint32_t nwAddress) :
  m_nwAddress(nwAddress)
{
  NS_LOG_FUNCTION(this);
}

ActionNwAddressHeader::~ActionNwAddressHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint32_t ActionNwAddressHeader::GetNwAddress(void) const
{
  return m_nwAddress;
}

void ActionNwAddressHeader::SetNwAddress(uint32_t nwAddress)
{
  m_nwAddress = nwAddress;
}

TypeId ActionNwAddressHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionNwAddressHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionNwAddressHeader> ()
  ;

  return tid;
}

TypeId ActionNwAddressHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionNwAddressHeader::Print(std::ostream &os) const
{
  os << "Action NW Address Header" << std::endl
     << " NW Address: " << uint(m_nwAddress) << std::endl
     << std::endl;
}

uint32_t ActionNwAddressHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 4 (nwAddress)
}

void ActionNwAddressHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU32(m_nwAddress);
}

uint32_t ActionNwAddressHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_nwAddress = start.ReadNtohU32();

  return GetSerializedSize();
}
} // namespace ns3
