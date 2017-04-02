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
#include "ns3/packet-in-header.h"
#include "ns3/openflow-header.h"

NS_LOG_COMPONENT_DEFINE("PacketInHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PacketInHeader);

void PacketInHeader::ConvertToStruct(ofp_packet_in &ofi, Ptr<OpenflowHeader> header)
{
  Ptr<PacketInHeader> packetInHeader = header->GetPacketIn();

  ofi.header.length = header->GetLength();
  ofi.header.type = header->GetType();
  ofi.header.version = header->GetVersion();
  ofi.header.xid = header->GetXId();
  ofi.buffer_id = packetInHeader->GetBufferId();
  ofi.total_len = packetInHeader->GetTotalLen();
  ofi.in_port = packetInHeader->GetInPort();
  ofi.reason = packetInHeader->GetReason();
  packetInHeader->GetData(ofi.data, ofi.total_len);
}

void PacketInHeader::ConvertToStruct(ofp_packet_in &ofi, Ptr<PacketInHeader> header)
{
  ofi.buffer_id = header->GetBufferId();
  ofi.total_len = header->GetTotalLen();
  ofi.in_port = header->GetInPort();
  ofi.reason = header->GetReason();
  header->GetData(ofi.data, ofi.total_len);
}

PacketInHeader::PacketInHeader (void) :
  m_bufferId(0),
  m_totalLen(0),
  m_inPort(0),
  m_reason(0),
  m_data(nullptr)
{
  NS_LOG_FUNCTION(this);
}

PacketInHeader::PacketInHeader (uint32_t bufferId, uint16_t inPort, uint8_t reason) :
  m_bufferId(bufferId),
  m_totalLen(0),
  m_inPort(inPort),
  m_reason(reason),
  m_data(nullptr)
{
  NS_LOG_FUNCTION(this);
}

PacketInHeader::PacketInHeader (uint32_t bufferId, uint16_t totalLen, uint16_t inPort,
                                uint8_t reason, uint8_t* data) :
  m_bufferId(bufferId),
  m_totalLen(totalLen),
  m_inPort(inPort),
  m_reason(reason),
  m_data(nullptr)
{
  NS_LOG_FUNCTION(this);

  if (m_totalLen > 0)
    {
      m_data = new u_int8_t[m_totalLen];
      memcpy(m_data, data, m_totalLen);
    }
}

PacketInHeader::~PacketInHeader ()
{
  NS_LOG_FUNCTION(this);
  if (m_data != nullptr)
    delete [] m_data;
}

uint32_t PacketInHeader::GetBufferId() const
{
  return m_bufferId;
}

void PacketInHeader::SetBufferId(uint32_t bufferId)
{
  m_bufferId = bufferId;
}

uint16_t PacketInHeader::GetTotalLen() const
{
  return m_totalLen;
}

uint16_t PacketInHeader::GetInPort() const
{
  return m_inPort;
}

void PacketInHeader::SetInPort(uint16_t inPort)
{
  m_inPort = inPort;
}

uint8_t PacketInHeader::GetReason() const
{
  return m_reason;
}

void PacketInHeader::SetReason(uint8_t reason)
{
  m_reason = reason;
}

void PacketInHeader::GetData(uint8_t *data, uint16_t dataLength) const
{
  NS_ASSERT(dataLength >= m_totalLen);
  memcpy(data, m_data, m_totalLen);
}

TypeId PacketInHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PacketInHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PacketInHeader> ()
  ;

  return tid;
}

TypeId PacketInHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PacketInHeader::Print(std::ostream &os) const
{
  os << "Packet In Header" << std::endl
     << " Buffer ID: " << uint(m_bufferId) << std::endl
     << " Total Length: " << uint(m_totalLen) << std::endl
     << " In Port: " << uint(m_inPort) << std::endl
     << " Reason: " << uint(m_reason) << std::endl
     << std::endl;
}

uint32_t PacketInHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 10 + m_totalLen;   // 4 + 2 + 2 + 1 + 1 (pad)) = 10
}

void PacketInHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU32(m_bufferId);
  start.WriteHtonU16(m_totalLen);
  start.WriteHtonU16(m_inPort);
  start.WriteU8(m_reason);
  start.WriteU8(uint8_t(0));

  if (m_totalLen > 0)
    start.Write(m_data, m_totalLen);
}

uint32_t PacketInHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  if (m_data != nullptr)
    {
      delete [] m_data;
      m_data = nullptr;
    }

  m_bufferId = start.ReadNtohU32();
  m_totalLen = start.ReadNtohU16();
  m_inPort = start.ReadNtohU16();
  m_reason = start.ReadU8();
  start.Next();

  if (m_totalLen > 0)
    {
      m_data = new u_int8_t[m_totalLen];
      start.Read(m_data, m_totalLen);
    }

  return GetSerializedSize();
}
} // namespace ns3
