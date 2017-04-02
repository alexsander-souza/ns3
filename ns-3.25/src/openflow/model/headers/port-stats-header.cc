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
#include "ns3/port-stats-header.h"

NS_LOG_COMPONENT_DEFINE("PortStatsHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(PortStatsHeader);

PortStatsHeader::PortStatsHeader (void) :
  m_portNumber(0),
  m_rxPackets(0),
  m_txPackets(0),
  m_rxBytes(0),
  m_txBytes(0),
  m_rxDropped(0),
  m_txDropped(0),
  m_rxErrors(0),
  m_txErrors(0),
  m_rxFrameErr(0),
  m_rxOverErr(0),
  m_rxCrcErr(0),
  m_collisions(0)
{
  NS_LOG_FUNCTION(this);
}

PortStatsHeader::PortStatsHeader (uint16_t portNumber, uint64_t rxPackets, uint64_t txPackets,
                                  uint64_t rxBytes, uint64_t txBytes, uint64_t rxDropped,
                                  uint64_t txDropped, uint64_t rxErrors, uint64_t txErrors,
                                  uint64_t rxFrameErr, uint64_t rxOverErr, uint64_t rxCrcErr,
                                  uint64_t collisions) :
  m_portNumber(portNumber),
  m_rxPackets(rxPackets),
  m_txPackets(txPackets),
  m_rxBytes(rxBytes),
  m_txBytes(txBytes),
  m_rxDropped(rxDropped),
  m_txDropped(txDropped),
  m_rxErrors(rxErrors),
  m_txErrors(txErrors),
  m_rxFrameErr(rxFrameErr),
  m_rxOverErr(rxOverErr),
  m_rxCrcErr(rxCrcErr),
  m_collisions(collisions)
{
  NS_LOG_FUNCTION(this);
}

PortStatsHeader::~PortStatsHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t PortStatsHeader::GetPortNumber() const
{
  return m_portNumber;
}

void PortStatsHeader::SetPortNumber(uint16_t portNumber)
{
  m_portNumber = portNumber;
}

uint64_t PortStatsHeader::GetRxPackets() const
{
  return m_rxPackets;
}

void PortStatsHeader::SetRxPackets(uint64_t rxPackets)
{
  m_rxPackets = rxPackets;
}

uint64_t PortStatsHeader::GetTxPackets() const
{
  return m_txPackets;
}

void PortStatsHeader::SetTxPackets(uint64_t txPackets)
{
  m_txPackets = txPackets;
}

uint64_t PortStatsHeader::GetRxBytes() const
{
  return m_rxBytes;
}

void PortStatsHeader::SetRxBytes(uint64_t rxBytes)
{
  m_rxBytes = rxBytes;
}

uint64_t PortStatsHeader::GetTxBytes() const
{
  return m_txBytes;
}

void PortStatsHeader::SetTxBytes(uint64_t txBytes)
{
  m_txBytes = txBytes;
}

uint64_t PortStatsHeader::GetRxDropped() const
{
  return m_rxDropped;
}

void PortStatsHeader::SetRxDropped(uint64_t rxDropped)
{
  m_rxDropped = rxDropped;
}

uint64_t PortStatsHeader::GetTxDropped() const
{
  return m_txDropped;
}

void PortStatsHeader::SetTxDropped(uint64_t txDropped)
{
  m_txDropped = txDropped;
}

uint64_t PortStatsHeader::GetRxErrors() const
{
  return m_rxErrors;
}

void PortStatsHeader::SetRxErrors(uint64_t rxErrors)
{
  m_rxErrors = rxErrors;
}

uint64_t PortStatsHeader::GetTxErrors() const
{
  return m_txErrors;
}

void PortStatsHeader::SetTxErrors(uint64_t txErrors)
{
  m_txErrors = txErrors;
}

uint64_t PortStatsHeader::GetRxFrameErr() const
{
  return m_rxFrameErr;
}

void PortStatsHeader::SetRxFrameErr(uint64_t rxFrameErr)
{
  m_rxFrameErr = rxFrameErr;
}

uint64_t PortStatsHeader::GetRxOverErr() const
{
  return m_rxOverErr;
}

void PortStatsHeader::SetRxOverErr(uint64_t rxOverErr)
{
  m_rxOverErr = rxOverErr;
}

uint64_t PortStatsHeader::GetRxCrcErr() const
{
  return m_rxCrcErr;
}

void PortStatsHeader::SetRxCrcErr(uint64_t rxCrcErr)
{
  m_rxCrcErr = rxCrcErr;
}

uint64_t PortStatsHeader::GetCollisions() const
{
  return m_collisions;
}

void PortStatsHeader::SetCollisions(uint64_t collisions)
{
  m_collisions = collisions;
}

TypeId PortStatsHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PortStatsHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PortStatsHeader> ()
  ;

  return tid;
}

TypeId PortStatsHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PortStatsHeader::Print(std::ostream &os) const
{
  os << "Port Stats Header" << std::endl
     << " Port Number: " << uint(m_portNumber) << std::endl
     << " RX Packets: " << uint(m_rxPackets) << std::endl
     << " TX Packets: " << uint(m_txPackets) << std::endl
     << " RX Bytes: " << uint(m_rxBytes) << std::endl
     << " TX Bytes: " << uint(m_txBytes) << std::endl
     << " RX Dropped: " << uint(m_rxDropped) << std::endl
     << " TX Dropped: " << uint(m_txDropped) << std::endl
     << " RX Errors: " << uint(m_rxErrors) << std::endl
     << " TX Errors: " << uint(m_txErrors) << std::endl
     << " RX Frame Errors: " << uint(m_rxFrameErr) << std::endl
     << " RX Overrun Errors: " << uint(m_rxOverErr) << std::endl
     << " RX CRC Errors: " << uint(m_rxCrcErr) << std::endl
     << " Collisions: " << uint(m_collisions) << std::endl
     << std::endl;
}

uint32_t PortStatsHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  return 104;     // 2 + 6 (pad: align 64) + 8 + 8 + 8 + 8 + 8 + 8 +
  // + 8 + 8 + 8 + 8 + 8 + 8 = 104
}

void PortStatsHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_portNumber);
  start.WriteU8(0, 6);
  start.WriteHtonU64(m_rxPackets);
  start.WriteHtonU64(m_txPackets);
  start.WriteHtonU64(m_rxBytes);
  start.WriteHtonU64(m_txBytes);
  start.WriteHtonU64(m_rxDropped);
  start.WriteHtonU64(m_txDropped);
  start.WriteHtonU64(m_rxErrors);
  start.WriteHtonU64(m_txErrors);
  start.WriteHtonU64(m_rxFrameErr);
  start.WriteHtonU64(m_rxOverErr);
  start.WriteHtonU64(m_rxCrcErr);
  start.WriteHtonU64(m_collisions);
}

uint32_t PortStatsHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_portNumber = start.ReadNtohU16();
  start.Next(6);
  m_rxPackets = start.ReadNtohU64();
  m_txPackets = start.ReadNtohU64();
  m_rxBytes = start.ReadNtohU64();
  m_txBytes = start.ReadNtohU64();
  m_rxDropped = start.ReadNtohU64();
  m_txDropped = start.ReadNtohU64();
  m_rxErrors = start.ReadNtohU64();
  m_txErrors = start.ReadNtohU64();
  m_rxFrameErr = start.ReadNtohU64();
  m_rxOverErr = start.ReadNtohU64();
  m_rxCrcErr = start.ReadNtohU64();
  m_collisions = start.ReadNtohU64();

  return GetSerializedSize();
}
} // namespace ns3
