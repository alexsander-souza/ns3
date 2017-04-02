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
#include "ns3/action-vendor-header.h"

NS_LOG_COMPONENT_DEFINE("ActionVendorHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionVendorHeader);

ActionVendorHeader::ActionVendorHeader (void) :
  m_vendor(0)
{
  NS_LOG_FUNCTION(this);
}

ActionVendorHeader::ActionVendorHeader (uint32_t vendor) :
  m_vendor(vendor)
{
  NS_LOG_FUNCTION(this);
}

ActionVendorHeader::~ActionVendorHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint32_t ActionVendorHeader::GetVendor(void) const
{
  return m_vendor;
}

void ActionVendorHeader::SetVendor(uint32_t vendor)
{
  m_vendor = vendor;
}

TypeId ActionVendorHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionVendorHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionVendorHeader> ()
  ;

  return tid;
}

TypeId ActionVendorHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionVendorHeader::Print(std::ostream &os) const
{
  os << "Action Vendor Header" << std::endl
     << " Vendor ID: " << uint(m_vendor) << std::endl
     << std::endl;
}

uint32_t ActionVendorHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4;   // 4 (vendor)
}

void ActionVendorHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU32(m_vendor);
}

uint32_t ActionVendorHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_vendor = start.ReadNtohU32();

  return GetSerializedSize();
}
} // namespace ns3
