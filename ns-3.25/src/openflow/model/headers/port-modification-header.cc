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
#include "ns3/port-modification-header.h"

NS_LOG_COMPONENT_DEFINE("PortModificationHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PortModificationHeader);

PortModificationHeader::PortModificationHeader (void) :
  m_portNumber(0),
  m_config(0),
  m_mask(0),
  m_advertise(0)
{
  NS_LOG_FUNCTION(this);

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_hwAddr[i] = 0;
}

PortModificationHeader::PortModificationHeader (uint16_t portNumber, uint8_t hwAddr[], uint32_t config,
                                                uint32_t mask, uint32_t advertise) :
  m_portNumber(portNumber),
  m_config(config),
  m_mask(mask),
  m_advertise(advertise)
{
  NS_LOG_FUNCTION(this);

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_hwAddr[i] = hwAddr[i];
}

PortModificationHeader::~PortModificationHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t PortModificationHeader::GetPortNumber() const
{
  return m_portNumber;
}

void PortModificationHeader::SetPortNumber(uint16_t portNumber)
{
  m_portNumber = portNumber;
}

const uint8_t* PortModificationHeader::GetHwAddr() const
{
  return m_hwAddr;
}

void PortModificationHeader::SetHwAddr(uint8_t* hwAddr)
{
  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_hwAddr[i] = hwAddr[i];
}

uint32_t PortModificationHeader::GetConfig() const
{
  return m_config;
}

void PortModificationHeader::SetConfig(uint32_t config)
{
  m_config = config;
}

uint32_t PortModificationHeader::GetMask() const
{
  return m_mask;
}

void PortModificationHeader::SetMask(uint32_t mask)
{
  m_mask = mask;
}

uint32_t PortModificationHeader::GetAdvertise() const
{
  return m_advertise;
}

void PortModificationHeader::SetAdvertise(uint32_t advertise)
{
  m_advertise = advertise;
}

TypeId PortModificationHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PortModificationHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PortModificationHeader> ()
  ;

  return tid;
}

TypeId PortModificationHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PortModificationHeader::Print(std::ostream &os) const
{
  uint8_t hwAddr[OFP_ETH_ALEN + 1];

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    hwAddr[i] = m_hwAddr[i];

  hwAddr[OFP_ETH_ALEN] = '\0';

  os << "Port Modification Header" << std::endl
     << " Port Number: " << uint(m_portNumber) << std::endl
     << " Hardware Address: " << hwAddr << std::endl
     << " Configuration Flags: " << uint(m_config) << std::endl
     << " Mask Flags: " << uint(m_mask) << std::endl
     << " Advertise: " << uint(m_advertise) << std::endl
     << std::endl;
}

uint32_t PortModificationHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 24;   // 2 + 6 + 4 + 4 + 4 + 4 (pad: align 64) = 24
}

void PortModificationHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_portNumber);
  start.Write(m_hwAddr, OFP_ETH_ALEN);
  start.WriteHtonU32(m_config);
  start.WriteHtonU32(m_mask);
  start.WriteHtonU32(m_advertise);
  start.WriteU8(0, 4);
}

uint32_t PortModificationHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_portNumber = start.ReadNtohU16();
  start.Read(m_hwAddr, OFP_ETH_ALEN);
  m_config = start.ReadNtohU32();
  m_mask = start.ReadNtohU32();
  m_advertise = start.ReadNtohU32();
  start.Next(4);

  return GetSerializedSize();
}
} // namespace ns3