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
#include "ns3/action-dl-address-header.h"

NS_LOG_COMPONENT_DEFINE("ActionDlAddressHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionDlAddressHeader);

ActionDlAddressHeader::ActionDlAddressHeader (void)
{
  NS_LOG_FUNCTION(this);

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_dlAddress[i] = 0;
}

ActionDlAddressHeader::ActionDlAddressHeader (uint8_t dlAddress[])
{
  NS_LOG_FUNCTION(this);

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_dlAddress[i] = dlAddress[i];
}

ActionDlAddressHeader::~ActionDlAddressHeader ()
{
  NS_LOG_FUNCTION(this);
}

const uint8_t* ActionDlAddressHeader::GetDlAddress(void) const
{
  return m_dlAddress;
}

void ActionDlAddressHeader::SetDlAddress(uint8_t* dlAddress)
{
  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_dlAddress[i] = dlAddress[i];
}

TypeId ActionDlAddressHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionDlAddressHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionDlAddressHeader> ()
  ;

  return tid;
}

TypeId ActionDlAddressHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionDlAddressHeader::Print(std::ostream &os) const
{
  uint8_t dlAddress[OFP_ETH_ALEN + 1];

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    dlAddress[i] = m_dlAddress[i];

  dlAddress[OFP_ETH_ALEN] = '\0';

  os << "Action DL Address Header" << std::endl
     << " DL Address: " << dlAddress << std::endl
     << std::endl;
}

uint32_t ActionDlAddressHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 12;   // 6 (dlAddress) + 6 (pad: align 64)
}

void ActionDlAddressHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.Write(m_dlAddress, OFP_ETH_ALEN);
  start.WriteU8(0, 6);
}

uint32_t ActionDlAddressHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  start.Read(m_dlAddress, OFP_ETH_ALEN);
  start.Next(6);

  return GetSerializedSize();
}
} // namespace ns3
