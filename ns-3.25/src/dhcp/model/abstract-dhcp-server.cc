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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/config.h"
#include "ns3/ipv4.h"
#include "ns3/udp-header.h"
#include "ns3/udp-l4-protocol.h"
#include "dhcp-header.h"
#include "abstract-dhcp-server.h"
#include <arpa/inet.h>

#include <iostream>

NS_LOG_COMPONENT_DEFINE("AbstractDhcpServer");

namespace ns3 {
DhcpLease::DhcpLease(Mac48Address& hwId, Ipv4Address& ip) :
  m_hwId(hwId),
  m_address(ip),
  m_lease_forever(false)
{
  NS_LOG_FUNCTION(this << hwId << ip);

  m_expirationTime = Simulator::Now() + Seconds(GRANTED_LEASE_TIME);
}

DhcpLease::~DhcpLease()
{
  NS_LOG_FUNCTION(this);
}

void
DhcpLease::SetExpirationTime(Time& expirationTime)
{
  NS_LOG_FUNCTION(this << expirationTime);
  m_expirationTime = expirationTime;
}

void
DhcpLease::SetExpirationMode(bool lease_forever)
{
  NS_LOG_FUNCTION(this << lease_forever);
  m_lease_forever = lease_forever;
}

void
DhcpLease::Renew(uint32_t grant_time)
{
  NS_LOG_FUNCTION(this);
  m_expirationTime = Simulator::Now() + Seconds(grant_time);
}

Ipv4Address
DhcpLease::GetLeasedAddress(void) const
{
  return m_address;
}

bool
DhcpLease::operator<(const Time& cur_time) const
{
  if (m_lease_forever)
    return false;

  return m_expirationTime < cur_time;
}

bool
DhcpLease::operator==(const Ipv4Address& leaseAddr) const
{
  return m_address == leaseAddr;
}

/**************************************************************************/

AbstractDhcpServer::AbstractDhcpServer() :
  m_port(DHCP_BOOTPS_PORT),
  m_leaseTime(GRANTED_LEASE_TIME)
{
  // NS_LOG_FUNCTION_NOARGS();
  m_leaseCb   = MakeNullCallback<int, const Mac48Address&, const Address&>();
  m_releaseCb = MakeNullCallback<int, const Mac48Address&, const Address&>();
}

AbstractDhcpServer::~AbstractDhcpServer()
{
  // NS_LOG_FUNCTION_NOARGS();

  m_leaseDb.clear();
  Simulator::Remove(m_expireEvent);
}

void
AbstractDhcpServer::SetLeaseCallback(DhcpLeaseCb cb)
{
  NS_LOG_FUNCTION(this);
  m_leaseCb = cb;
}

void
AbstractDhcpServer::SetReleaseCallback(DhcpReleaseCb cb)
{
  NS_LOG_FUNCTION(this);
  m_releaseCb = cb;
}

void
AbstractDhcpServer::GetOwnLease(Ptr<NetDevice>netdev, Ptr<Ipv4>ipv4)
{
  m_myAddr = ipv4->SelectSourceAddress(netdev, m_poolAddress, Ipv4InterfaceAddress::GLOBAL);

  GetOwnLease();
}

void
AbstractDhcpServer::GetOwnLease()
{
  Ptr<DhcpLease> lease = Create<DhcpLease>(m_myMacAddr, m_myAddr);
  lease->SetExpirationMode(true);
  m_leaseDb.insert(m_leaseDb.begin(), std::make_pair(m_myMacAddr, lease));

  m_lastLeasedAddress = m_myAddr.GetSubnetDirectedBroadcast(m_poolMask);
}

void
AbstractDhcpServer::ReceivePacket(DhcpHeader& dhcpHeader, Mac48Address& from)
{
  NS_LOG_FUNCTION(this << from);

  if (dhcpHeader.GetOp() != DhcpHeader::BOOT_REQUEST)
    return;

  /* Check the DHCP request */
  const DhcpOption *opt_msg_type =
    dhcpHeader.GetOptionByType(DhcpOption::DHCP_OPT_MESSAGE_TYPE);

  if (opt_msg_type == 0)
  {
    NS_LOG_WARN("Got malformed message from client: " << from);
  }

  switch (uint8_t(*opt_msg_type))
  {
  case DhcpOption::DHCP_TYPE_DISCOVER:

    /* Create an offer */
    HandleDiscovery(from, dhcpHeader);
    break;

  case DhcpOption::DHCP_TYPE_REQUEST:

    /* Confirm/Renew the lease */
    HandleRequest(from, dhcpHeader);
    break;

  case DhcpOption::DHCP_TYPE_RELEASE:

    /* FIXME process RELEASE */
    NS_LOG_WARN("RELEASE from client " << from);
    break;

  default:
    NS_LOG_WARN("Got malformed message from client: " << from);
  }
}

void
AbstractDhcpServer::HandleDiscovery(Mac48Address& client, DhcpHeader& request)
{
  NS_LOG_FUNCTION(this << client << request);

  /*
   * Fetch a lease for this client
   * don't care what the value of DHCP_OPT_REQUESTED_IP_ADDRESS is
   */
  Ptr<DhcpLease> lease = GetLeaseForClient(client);
  lease->Renew(m_leaseTime);

  /* prepare OFFER to client */
  DhcpHeader response;
  response.SetOp(DhcpHeader::BOOT_REPLY);
  response.SetTransactionId(request.GetTransactionId());
  response.SetCHAddr(client);
  response.SetYIAddr(lease->GetLeasedAddress());
  response.SetCIAddr(Ipv4Address::GetAny());
  response.SetSIAddr(m_myAddr);

  /* Create options */
  DhcpHeader::DhcpOptionList options;

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_MESSAGE_TYPE,
                               (uint8_t)DhcpOption::DHCP_TYPE_OFFER));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_SUBNET_MASK,
                               m_poolMask.Get()));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_RENEWAL_TIME_VALUE,
                               ((uint32_t)m_leaseTime / 2)));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_IP_ADDRESS_LEASE_TIME,
                               m_leaseTime));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_DHCP_SERVER_IDENTIFIER,
                               m_myAddr.Get()));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_END));

  response.AddOptionList(options);

  /* Send response to client */
  SendMessage(client, response);
}

void
AbstractDhcpServer::HandleRequest(Mac48Address& client, DhcpHeader& request)
{
  NS_LOG_FUNCTION(this << client << request);

  /* Fetch a lease for this client */
  Ptr<DhcpLease> lease = GetLeaseForClient(client);

  /* Check requested address */
  const DhcpOption *opt_req_addr =
    request.GetOptionByType(DhcpOption::DHCP_OPT_REQUESTED_IP_ADDRESS);

  if (opt_req_addr == 0)
  {
    NS_LOG_WARN("missing REQUESTED_IP_ADDRESS");
    return;
  }

  Ipv4Address req_addr(*opt_req_addr);

  if (lease->GetLeasedAddress() != req_addr)
  {
    NS_LOG_WARN("mismatch in REQUESTED_IP_ADDRESS: our " << lease->GetLeasedAddress() << " theirs " << req_addr);

    // TODO send DHCKNak
    return;
  }

  /* Renew existing lease */
  lease->Renew(m_leaseTime);

  /* Notify external services */

  if (!m_leaseCb.IsNull()) {
    Address leaseAddr = lease->GetLeasedAddress();
    m_leaseCb(client, leaseAddr);
  }

  /* prepare ACK to client */
  DhcpHeader response;
  response.SetOp(DhcpHeader::BOOT_REPLY);
  response.SetTransactionId(request.GetTransactionId());
  response.SetYIAddr(lease->GetLeasedAddress());
  response.SetCHAddr(client);
  response.SetCIAddr(Ipv4Address::GetAny());
  response.SetSIAddr(m_myAddr);

  /* Create options */
  DhcpHeader::DhcpOptionList options;

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_MESSAGE_TYPE,
                               (uint8_t)DhcpOption::DHCP_TYPE_ACK));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_SUBNET_MASK,
                               m_poolMask.Get()));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_RENEWAL_TIME_VALUE,
                               (uint32_t)(m_leaseTime / 2)));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_IP_ADDRESS_LEASE_TIME,
                               m_leaseTime));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_DHCP_SERVER_IDENTIFIER,
                               m_myAddr.Get()));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_END));

  response.AddOptionList(options);

  /* Send response to client */
  SendMessage(client, response);
}

Ptr<DhcpLease>AbstractDhcpServer::GetLeaseForClient(Mac48Address& clientId)
{
  NS_LOG_FUNCTION(this << clientId);

  auto it = m_leaseDb.find(clientId);

  if (it != m_leaseDb.end())
    return it->second;

  /* Not found, select a new address in the pool */
  bool got_new = false;

  // FIXME possible infinite loop here
  Ipv4Address new_addr;

  while (!got_new)
  {
    new_addr = Ipv4Address(m_lastLeasedAddress.Get() - 1);
    got_new  = true;

    for (it = m_leaseDb.begin(); it != m_leaseDb.end(); it++)
    {
      if (*(it->second) == new_addr)
      {
        got_new             = false;
        m_lastLeasedAddress = new_addr;
        break;
      }
    }
  }

  m_lastLeasedAddress = new_addr;
  Ptr<DhcpLease> new_lease = Create<DhcpLease>(clientId, m_lastLeasedAddress);
  m_leaseDb.insert(m_leaseDb.begin(), std::make_pair(clientId, new_lease));

  return new_lease;
}
} // Namespace ns3
