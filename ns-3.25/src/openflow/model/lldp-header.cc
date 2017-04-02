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
 * Author: Alexsander de Souza  <asouza@inf.ufrgs.br>
 *
 */

#include <array>
#include <string>
#include <ns3/assert.h>
#include <ns3/log.h>
#include <ns3/header.h>
#include <ns3/address-utils.h>
#include "lldp-header.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("BasicLldpHeader");

NS_OBJECT_ENSURE_REGISTERED(BasicLldpHeader);

const uint16_t BasicLldpHeader::PROT_NUMBER = 0x88cc;

BasicLldpHeader::BasicLldpHeader() :
  m_chassis_id(Mac48Address::GetBroadcast()),
  m_port(-1),
  m_ttl(120)
{
  NS_LOG_FUNCTION(this);
}

BasicLldpHeader::BasicLldpHeader(const Mac48Address& chassis_id, int port, int ttl) :
  m_chassis_id(chassis_id),
  m_port(port),
  m_ttl(ttl)
{
  NS_LOG_FUNCTION(this);
}

const Mac48Address&
BasicLldpHeader::GetChassisId() const
{
  return m_chassis_id;
}

int
BasicLldpHeader::GetPortId() const
{
  return m_port;
}

TypeId
BasicLldpHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::BasicLldpHeader")
                      .SetParent<Header>()
                      .AddConstructor<BasicLldpHeader>()
  ;

  return tid;
}

TypeId
BasicLldpHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void
BasicLldpHeader::Print(std::ostream& os) const
{
  os << "LLPD Chassis " << m_chassis_id << " port " << std::to_string(m_port);
}

uint32_t
BasicLldpHeader::GetSerializedSize(void) const
{
  const std::string& port = std::to_string(m_port);

  return 18 + port.size();
}

void
BasicLldpHeader::Serialize(Buffer::Iterator start) const
{
  // TLV Chassis
  start.WriteU8(0x02);
  start.WriteU8(0x07);
  start.WriteU8(0x04);
  WriteTo(start, m_chassis_id);

  // TLV Port
  const std::string& port = std::to_string(m_port);
  start.WriteU8(0x04);
  start.WriteU8(port.size() + 1);
  start.WriteU8(0x05);
  start.Write(reinterpret_cast<const uint8_t *>(port.c_str()), port.size());

  // TLV TTL
  start.WriteU8(0x06);
  start.WriteU8(0x02);
  start.WriteHtonU16(m_ttl);

  // END
  start.WriteU16(0x00);
}

uint32_t
BasicLldpHeader::Deserialize(Buffer::Iterator start)
{
  // We are really optimistic, so no sanity check

  // TLV Chassis
  start.Next(3);
  ReadFrom(start, m_chassis_id);

  // TLV Port
  start.Next(1); // type
  std::string port;
  int portLen = start.ReadU8() - 1;
  start.Next(1); // sub-type

  for (int i = 0; i < portLen; ++i) port.push_back(start.ReadU8());

  m_port = std::stoi(port);

  // TLV TTL
  start.Next(2);
  m_ttl = start.ReadNtohU16();

  return GetSerializedSize();
}
}
