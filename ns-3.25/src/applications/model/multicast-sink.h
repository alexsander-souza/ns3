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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#ifndef MULTICAST_SINK_H
#define MULTICAST_SINK_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/ipv4-address.h"
#include "ns3/address.h"
#include "ns3/ipv4.h"
#include "ns3/internet-module.h"

namespace ns3
{
class Address;
class Socket;
class Packet;

class MulticastSink : public Application
{
public:

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);

  MulticastSink();
  virtual ~MulticastSink();

  /**
   * \return the total bytes received in this sink app
   */
  uint32_t GetTotalRx() const;

protected:

  virtual void DoDispose(void);

private:

  // inherited from Application base class.
  virtual void StartApplication(void);    // Called at time specified by Start
  virtual void StopApplication(void);     // Called at time specified by Stop

  /**
   * \brief Send an IGMPv2 message
   * \param type IGMPv2 message type
   */
  void SendIgmpv2(uint8_t type);

  /**
   * \brief Handle a UDP packet received by the application
   * \param socket the receiving socket
   */
  void HandleReadUdp(Ptr < Socket > socket);

  /**
   * \brief Handle a IGMP packet received by the application
   * \param socket the receiving socket
   */
  void HandleReadIgmp(Ptr < Socket > socket);

  Ptr < Socket > m_sock_udp;      //!< UDP socket
  Ptr < Socket > m_sock_igmp;     //!< IGMP socket
  uint8_t m_maxRespTime;          //!< Multicast max response time
  bool m_enableChecksum;          //!< enable the IGMPv2 checksum calculation
  Ipv4Address m_groupAddr;        //!< Multicast group IPv4 address
  uint16_t m_port;                //!< Protocol port
  uint32_t m_totalRx;             //!< Total bytes received

  /// Traced Callback: received packets, source address.
  TracedCallback < Ptr < const Packet >, const Address & > m_rxTrace;
};
} // namespace ns3
#endif /* MULTICAST_SINK_H */
