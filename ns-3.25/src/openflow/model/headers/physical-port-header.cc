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
#include "ns3/physical-port-header.h"

NS_LOG_COMPONENT_DEFINE("PhysicalPortHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PhysicalPortHeader);

PhysicalPortHeader::PhysicalPortHeader (void) :
  m_portNumber(0),
  m_config(0),
  m_state(0),
  m_curr(0),
  m_advertised(0),
  m_supported(0),
  m_peer(0)
{
  NS_LOG_FUNCTION(this);

  std::memset(m_hwAddr, 0, sizeof(m_hwAddr));
  std::memset(m_name, 0, sizeof(m_name));
}

PhysicalPortHeader::PhysicalPortHeader (uint16_t portNumber, uint8_t hwAddr[], uint8_t name[],
                                        uint32_t config, uint32_t state, uint32_t curr,
                                        uint32_t advertised, uint32_t supported, uint32_t peer) :
  m_portNumber(portNumber),
  m_config(config),
  m_state(state),
  m_curr(curr),
  m_advertised(advertised),
  m_supported(supported),
  m_peer(peer)
{
  NS_LOG_FUNCTION(this);

  for (int i = 0; i < OFP_ETH_ALEN; i++)
    m_hwAddr[i] = hwAddr[i];

  for (int i = 0; i < OFP_MAX_PORT_NAME_LEN; i++)
    m_name[i] = name[i];
}

PhysicalPortHeader::~PhysicalPortHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t PhysicalPortHeader::GetPortNumber() const
{
  return m_portNumber;
}

void PhysicalPortHeader::SetPortNumber(uint16_t portNumber)
{
  m_portNumber = portNumber;
}

const uint8_t* PhysicalPortHeader::GetHwAddr() const
{
  return m_hwAddr;
}

void PhysicalPortHeader::SetHwAddr(uint8_t* hwAddr)
{
  std::memcpy(m_hwAddr, hwAddr, OFP_ETH_ALEN);
}

const uint8_t* PhysicalPortHeader::GetName() const
{
  return m_name;
}

void PhysicalPortHeader::SetName(uint8_t* name)
{
  for (int i = 0; i < OFP_MAX_PORT_NAME_LEN; i++)
    m_name[i] = name[i];
}

uint32_t PhysicalPortHeader::GetConfig() const
{
  return m_config;
}

void PhysicalPortHeader::SetConfig(uint32_t config)
{
  m_config = config;
}

uint32_t PhysicalPortHeader::GetState() const
{
  return m_state;
}

void PhysicalPortHeader::SetState(uint32_t state)
{
  m_state = state;
}

uint32_t PhysicalPortHeader::GetCurr() const
{
  return m_curr;
}

void PhysicalPortHeader::SetCurr(uint32_t curr)
{
  m_curr = curr;
}

uint32_t PhysicalPortHeader::GetAdvertised() const
{
  return m_advertised;
}

void PhysicalPortHeader::SetAdvertised(uint32_t advertised)
{
  m_advertised = advertised;
}

uint32_t PhysicalPortHeader::GetSupported() const
{
  return m_supported;
}

void PhysicalPortHeader::SetSupported(uint32_t supported)
{
  m_supported = supported;
}

uint32_t PhysicalPortHeader::GetPeer() const
{
  return m_peer;
}

void PhysicalPortHeader::SetPeer(uint32_t peer)
{
  m_peer = peer;
}

bool PhysicalPortHeader::operator<(const PhysicalPortHeader &other) const
{
  return(int(m_portNumber) - int(other.m_portNumber) < 0);
}

TypeId PhysicalPortHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PhysicalPortHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PhysicalPortHeader> ()
  ;

  return tid;
}

TypeId PhysicalPortHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PhysicalPortHeader::Print(std::ostream &os) const
{
  Mac48Address mac;
  mac.CopyFrom (m_hwAddr);
  os << "Physical Port Header" << std::endl
     << " Port Number: " << uint(m_portNumber) << std::endl
     << " Hardware Address: " << mac << std::endl
     << " Port Name: " << m_name << std::endl
     << " Config Flags: " << uint(m_config) << std::endl
     << " State Flags: " << uint(m_state) << std::endl
     << " Current Flags: " << uint(m_curr) << std::endl
     << " Advertised Flags: " << uint(m_advertised) << std::endl
     << " Supported Flags: " << uint(m_supported) << std::endl
     << " Peer Flags: " << uint(m_peer) << std::endl
     << std::endl;
}

uint32_t PhysicalPortHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 48;   // 2 + 6 (OFP_ETH_ALEN) + 16 (OFP_MAX_PORT_NAME_LEN) + 4 + 4 + 4 + 4 + 4 + 4 = 48
}

void PhysicalPortHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_portNumber);
  start.Write(m_hwAddr, OFP_ETH_ALEN);
  start.Write(m_name, OFP_MAX_PORT_NAME_LEN);
  start.WriteHtonU32(m_config);
  start.WriteHtonU32(m_state);
  start.WriteHtonU32(m_curr);
  start.WriteHtonU32(m_advertised);
  start.WriteHtonU32(m_supported);
  start.WriteHtonU32(m_peer);
}

uint32_t PhysicalPortHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_portNumber = start.ReadNtohU16();
  start.Read(m_hwAddr, OFP_ETH_ALEN);
  start.Read(m_name, OFP_MAX_PORT_NAME_LEN);
  m_config = start.ReadNtohU32();
  m_state = start.ReadNtohU32();
  m_curr = start.ReadNtohU32();
  m_advertised = start.ReadNtohU32();
  m_supported = start.ReadNtohU32();
  m_peer = start.ReadNtohU32();

  return GetSerializedSize();
}
} // namespace ns3
