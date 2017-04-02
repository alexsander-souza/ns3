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

#ifndef PORT_STATS_HEADER_H
#define PORT_STATS_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define PORT_STATS_REPLY_LENGTH 116

namespace ns3
{
/**
 * \class PortStatsHeader
 * \brief Openflow protocol port stats reply packet
 */
class PortStatsHeader : public Header,
                        public SimpleRefCount<PortStatsHeader>

{
public:
  /**
   * Default constructor
   */
  PortStatsHeader (void);

  /**
   * Constructor
   * \param portNumber
   * \param rxPackets      Number of received packets
   * \param txPackets      Number of transmitted packets
   * \param rxBytes        Number of received bytes
   * \param txBytes        Number of transmitted bytes
   * \param rxDropped      Number of packets dropped by RX
   * \param txDropped      Number of packets dropped by TX
   * \param rxErrors       Number of receive errors. This is a super-set of more specific receive
                          errors and should be greater than or equal to the sum of all rx_*_err
                          values
   * \param txErrors       Number of transmit errors. This is a super-set of more specific transmit
                          errors and should be greater than or equal to the sum of all tx_*_err
                          values (none currently defined.)
   * \param rxFrameErr     Number of frame alignment errors
   * \param rxOverErr      Number of packets with RX overrun
   * \param rxCrcErr       Number of CRC errors
   * \param collisions     Number of collisions
   */
  PortStatsHeader (uint16_t portNumber, uint64_t rxPackets, uint64_t txPackets,
                   uint64_t rxBytes, uint64_t txBytes, uint64_t rxDropped,
                   uint64_t txDropped, uint64_t rxErrors, uint64_t txErrors,
                   uint64_t rxFrameErr, uint64_t rxOverErr, uint64_t rxCrcErr,
                   uint64_t collisions);

  /**
   * Default destructor
   */
  virtual ~PortStatsHeader ();

  /* Getters and Setters*/
  uint16_t GetPortNumber() const;
  void SetPortNumber(uint16_t portNumber);

  uint64_t GetRxPackets() const;
  void SetRxPackets(uint64_t rxPackets);

  uint64_t GetTxPackets() const;
  void SetTxPackets(uint64_t txPackets);

  uint64_t GetRxBytes() const;
  void SetRxBytes(uint64_t rxBytes);

  uint64_t GetTxBytes() const;
  void SetTxBytes(uint64_t txBytes);

  uint64_t GetRxDropped() const;
  void SetRxDropped(uint64_t rxDropped);

  uint64_t GetTxDropped() const;
  void SetTxDropped(uint64_t txDropped);

  uint64_t GetRxErrors() const;
  void SetRxErrors(uint64_t rxErrors);

  uint64_t GetTxErrors() const;
  void SetTxErrors(uint64_t txErrors);

  uint64_t GetRxFrameErr() const;
  void SetRxFrameErr(uint64_t rxFrameErr);

  uint64_t GetRxOverErr() const;
  void SetRxOverErr(uint64_t rxOverErr);

  uint64_t GetRxCrcErr() const;
  void SetRxCrcErr(uint64_t rxCrcErr);

  uint64_t GetCollisions() const;
  void SetCollisions(uint64_t collisions);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_portNumber;
  uint64_t m_rxPackets;     /* Number of received packets. */
  uint64_t m_txPackets;     /* Number of transmitted packets. */
  uint64_t m_rxBytes;       /* Number of received bytes. */
  uint64_t m_txBytes;       /* Number of transmitted bytes. */
  uint64_t m_rxDropped;     /* Number of packets dropped by RX. */
  uint64_t m_txDropped;     /* Number of packets dropped by TX. */
  uint64_t m_rxErrors;      /* Number of receive errors. This is a super-set
                               of more specific receive errors and should be
                               greater than or equal to the sum of all
                               rx_*_err values. */
  uint64_t m_txErrors;      /* Number of transmit errors. This is a super-set
                               of more specific transmit errors and should be
                               greater than or equal to the sum of all
                               tx_*_err values (none currently defined.) */
  uint64_t m_rxFrameErr;    /* Number of frame alignment errors. */
  uint64_t m_rxOverErr;     /* Number of packets with RX overrun. */
  uint64_t m_rxCrcErr;      /* Number of CRC errors. */
  uint64_t m_collisions;    /* Number of collisions. */
};
} // namespace ns3
#endif  /* PORT_STATS_HEADER_H */
