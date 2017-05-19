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
 *  Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#ifndef ABSTRACT_DHCP_SERVER_H
#define ABSTRACT_DHCP_SERVER_H

#include <map>
#include "ns3/application.h"
#include "ns3/simple-ref-count.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-value.h"

#define GRANTED_LEASE_TIME      4

namespace ns3 {
class Packet;
class NetDevice;
class DhcpHeader;

/**
 * \ingroup dhcpclientserver
 * \class DhcpLease
 * \brief Holds lease state
 */
class DhcpLease : public SimpleRefCount<DhcpLease>{
public:

  /**
   * Lease entry constructor
   * \param hwId       Client HW address
   * \param ip         Leased IPv4
   */
  DhcpLease(Mac48Address& hwId,
            Ipv4Address & ip);

  /**
   * Default destructor
   */
  virtual
  ~DhcpLease();

  /**
   * Set lease expiration timer
   * \param expirationTime         Expiration time
   */
  void SetExpirationTime(Time& expirationTime);

  /**
   * Define a static lease
   * \param lease_forever          Set lease as static
   */
  void SetExpirationMode(bool lease_forever);

  /**
   * Renew lease
   * \param grant_time             Lease grant time (seconds)
   */
  void Renew(uint32_t grant_time);

  /**
   * Get Leased Address
   */
  Ipv4Address GetLeasedAddress(void) const;

  /**
   * Check if this lease has expired
   * \param cur_time      Current Time
   */
  bool operator<(const Time& cur_time) const;

  /**
   * Compare lease address
   * \param leaseAddr      IPv4 Leased address
   */
  bool operator==(const Ipv4Address& leaseAddr) const;

private:

  Mac48Address m_hwId;    /**< Client hardware address */
  Ipv4Address  m_address; /**< Client designated IP address */
  Time m_expirationTime;  /**< Lease expiration time */
  bool m_lease_forever;   /**< Static lease */
};

class AbstractDhcpServer {
public:

  typedef Callback<int, const Mac48Address&, const Address&>DhcpLeaseCb;
  typedef Callback<int, const Mac48Address&, const Address&>DhcpReleaseCb;

  /**
   * Default constructor
   */
  AbstractDhcpServer();

  /**
   * Default destructor
   */
  virtual
  ~AbstractDhcpServer();

  /**
   * \brief Receive packets
   */
  void ReceivePacket(DhcpHeader  & dhcpHeader,
                     Mac48Address& from);

  void SetLeaseCallback(DhcpLeaseCb cb);

  void SetReleaseCallback(DhcpReleaseCb cb);

protected:

  void GetOwnLease();

  void GetOwnLease(Ptr<NetDevice>netdev,
                   Ptr<Ipv4>     ipv4);

  uint16_t m_port;            /**< Server UDP port */
  Mac48Address m_myMacAddr;   /**< local interface MAC address */
  Ipv4Address  m_myAddr;      /**< local interface IPv4 address */
  Ipv4Address  m_poolAddress; /**< IPv4 lease pool */
  Ipv4Mask     m_poolMask;    /**< IPv4 network mask */
  uint32_t     m_leaseTime;   /**< Lease time (sec) */

private:

  typedef std::map<Mac48Address, Ptr<DhcpLease> >DhcpLeaseDatabase;

  Ptr<DhcpLease>GetLeaseForClient(Mac48Address& clientId);

  void HandleDiscovery(Mac48Address& client,
                       DhcpHeader  & request);
  void HandleRequest(Mac48Address& client,
                     DhcpHeader  & request);

  virtual void SendMessage(Mac48Address& clientId,
                           DhcpHeader  & response) = 0;

  Ipv4Address m_lastLeasedAddress; /**< Last IPv4 lease address */
  DhcpLeaseDatabase m_leaseDb;     /**< Lease database */
  EventId m_expireEvent;           /**< Expired leases GC */
  DhcpLeaseCb   m_leaseCb;
  DhcpReleaseCb m_releaseCb;
};
} // namespace ns3

#endif /* ABSTRACT_DHCP_SERVER_H */
