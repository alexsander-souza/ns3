/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB, 2014 UFRGS
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

#ifndef IGMPV2_PING_H
#define IGMPV2_PING_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"

#include "ns3/traced-callback.h"
#include "ns3/address.h"

namespace ns3
{

class Socket;
class Packet;
class Igmpv2Header;

/**
* \ingroup Igmpv2Ping
* \class Igmpv2Ping
* \brief A host that sends IGMPv2 messages in order to validate the
* protocol header.
*
*/
class Igmpv2Ping : public Application
{
public:

  static TypeId GetTypeId (void);

  Igmpv2Ping();

  virtual ~Igmpv2Ping();

  virtual void Print (std::ostream& os) const;

protected:

  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Create IGMPv2 packet
   */
  void CreateIgmpv2Header(Igmpv2Header &header, uint8_t type,
                          uint8_t maxRespTime, Ipv4Address groupAddress,
                          bool enableChecksum);

  void SendIgmpv2Header(uint8_t type, uint8_t maxRespTime,
                        Ipv4Address groupAddress, bool enableChecksum);

  Ptr<Socket> m_sock_pkt;             /**< AF_PACKET-like socket */
  Mac48Address m_myMacAddr;           /**< Local interface MAC address */
  Ipv4Address m_myAddr;               /**< Local interface IPv4 address */
  Mac48Address m_srvMacAddr;          /**< Remote interface MAC address */
  Ipv4Address m_srvAddr;              /**< Remote interface IPv4 address */
  Ipv4Mask m_myMask;                  /**< Network mask */
  bool m_binded;                      /**< Bound to address */
  Ipv4Address m_groupAddr;            /**< Multicast IPv4 group address */
};

} // namespace ns3

#endif
