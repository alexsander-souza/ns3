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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 * Contributor(s):
 *  Alexsander de Souza <asouza@inf.ufrgs.br>
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *
 */

#ifndef DHCP_CLIENT_H
#define DHCP_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"

namespace ns3
{

class Socket;
class Packet;
class DhcpHeader;

/**
 * \ingroup dhcpclientserver
 * \class DhcpClient
 * \brief A Dhcp client. It learns DHCP server and IP gw addresses from IP header.
 * In other words DHCP server must be located on the network gw node.
 *
 */
class DhcpClient : public Application
{
public:
  static TypeId GetTypeId (void);

  DhcpClient();

  virtual ~DhcpClient();

  virtual void Print (std::ostream& os) const;

protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Receive packets
   */
  void NetHandler(Ptr<Socket> socket);

  /**
   * \brief Monitor NetDev status
   */
  void LinkStateChangedHandler(void);

  /**
   * \brief Request retransmission handler
   */
  void RequestTimeoutHandler(void);

  /**
   * \brief Renew lease handler
   */
  void RenewTimeoutHandler(void);

  /**
   * \brief Configure local interface
   */
  void SetupIp(DhcpHeader &offer);

  /**
   * \brief Unconfigure local interface
   */
  void TearDownIp(void);

  /**
   * \brief Create DHCP request packet
   */
  void CreateDhcpHeader(DhcpHeader &header, uint8_t type);

  /**
   * \brief Send DHCP client request
   */
  void DoDhcpRequest(void);

  Ptr<Socket>     m_sock_pkt;             /**< AF_PACKET-like socket */
  Ptr<Socket>     m_sock_udp;             /**< AF_INET UDP socket */
  uint32_t        m_xid;                  /**< Current transaction ID */
  uint32_t        m_leaseTime;            /**< Lease duration (seconds) */
  EventId         m_leaseTimeoutEvent;    /**< Lease timeout event */
  uint32_t        m_requestTimeout;       /**< Request timeout (seconds) */
  EventId         m_requestTimeoutEvent;  /**< Request timeout event */
  Ipv4Address     m_srvAddr;              /**< DHCP server IP address */
  Mac48Address    m_srvMacAddr;           /**< DHCP server MAC address */
  Ipv4Address     m_myAddr;               /**< IP address given to client */
  Ipv4Mask        m_myMask;               /**< Network mask */
  Mac48Address    m_myMacAddr;            /**< local interface MAC address */
  bool            m_binded;               /**< Client bound to address */
};

} // namespace ns3

#endif /* DHCP_CLIENT_H */
