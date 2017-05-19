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

#ifndef IGMPV2_PONG_H
#define IGMPV2_PONG_H

#include "ns3/application.h"
#include "ns3/simple-ref-count.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-value.h"

#include "ns3/traced-callback.h"
#include "ns3/address.h"

namespace ns3
{

class Socket;
class NetDevice;

/**
* \class Igmpv2Pong
* \brief An IGMPv2 pong application.
*/
class Igmpv2Pong : public Application
{
public:

  /* ns3::Object method */
  static TypeId GetTypeId(void);

  /**
  * Default constructor
  */
  Igmpv2Pong();

  /**
  * Default destructor
  */
  virtual ~Igmpv2Pong();

  /**
  * \brief Receive packets
  */
  void NetHandler(Ptr<Socket> socket);

protected:

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:

  /**
  * Create server socket.
  */
  void CreateServerSocket(void);

  void PongPacket(Igmpv2Header &igmpv2Header);

  /**
  * Send pong the IGMPv2 packet back to the network
  * \param client client's HW address
  * \param response IGMPv2 header
  */
  void SendMessage(Igmpv2Header &igmpv2Header);

  Ptr<NetDevice>           m_netdev;               /**< Binded netdev */
  Ptr<Socket>              m_sock_pkt;             /**< AF_PACKET-like socket */
  Ptr<Socket>              m_sock_udp;             /**< AF_INET UDP socket */
  uint16_t                 m_port;                 /**< Server UDP port */
  Mac48Address             m_myMacAddr;            /**< Local interface MAC address */
  Ipv4Address              m_myAddr;               /**< Local interface IPv4 address */
  Mac48Address             m_srvMacAddr;           /**< Remote interface MAC address */
  Ipv4Address              m_srvAddr;              /**< Remote interface IPv4 address */
  bool                     m_binded;               /**< Bound to address */
};

} // namespace ns3

#endif /* IGMPV2_PONG_H */
