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

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/udp-header.h"
#include "ns3/ethernet-header.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include <arpa/inet.h>
#include "dhcp-client.h"
#include "dhcp-header.h"

#define DHCP_DFT_LEASE_TIME    (10)
#define DHCP_DFT_RETRY_TIME    (5)

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("DhcpClient");
NS_OBJECT_ENSURE_REGISTERED(DhcpClient);

TypeId DhcpClient::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DhcpClient")
                      .SetParent<Application> ()
                      .AddConstructor<DhcpClient> ()
                      .AddAttribute("LeaseTime", "Renew the address leased after n seconds",
                                    UintegerValue(DHCP_DFT_LEASE_TIME),
                                    MakeUintegerAccessor(&DhcpClient::m_leaseTime),
                                    MakeUintegerChecker<uint32_t> ())
                      .AddAttribute("RetransmissionTime", "DHCP requests transmission timeout",
                                    UintegerValue(DHCP_DFT_RETRY_TIME),
                                    MakeUintegerAccessor(&DhcpClient::m_requestTimeout),
                                    MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}

DhcpClient::DhcpClient() :
  m_sock_pkt(0),
  m_srvAddr(Ipv4Address::GetBroadcast()),
  m_srvMacAddr(Mac48Address::GetBroadcast()),
  m_myAddr(Ipv4Address::GetAny()),
  m_myMask(Ipv4Mask::GetZero()),
  m_binded(0)
{
  NS_LOG_FUNCTION_NOARGS();
}

DhcpClient::~DhcpClient()
{
  NS_LOG_FUNCTION_NOARGS();
}

void DhcpClient::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void DhcpClient::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  if (m_sock_pkt > 0)
    return;

  Ptr<NetDevice> netdev = GetNode()->GetDevice(0);
  netdev->AddLinkChangeCallback(
    MakeCallback(&DhcpClient::LinkStateChangedHandler, this));

  m_myMacAddr = Mac48Address::ConvertFrom(netdev->GetAddress());

  /* RAW socket */
  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  m_sock_pkt = Socket::CreateSocket(GetNode(), tid);

  PacketSocketAddress sock_addr;
  sock_addr.SetSingleDevice(netdev->GetIfIndex());
  sock_addr.SetPhysicalAddress(netdev->GetAddress());
  sock_addr.SetProtocol(0x0800);

  m_sock_pkt->Bind(sock_addr);
  m_sock_pkt->SetRecvCallback(MakeCallback(&DhcpClient::NetHandler, this));

  /* UDP socket */
  TypeId tid_udp = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_sock_udp = Socket::CreateSocket(GetNode(), tid_udp);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), DHCP_BOOTPC_PORT);
  m_sock_udp->SetAllowBroadcast(true);
  m_sock_udp->Bind(local);

  if (netdev->IsLinkUp())
    DoDhcpRequest();
}

void DhcpClient::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  Simulator::Cancel(m_leaseTimeoutEvent);
  Simulator::Cancel(m_requestTimeoutEvent);

  m_sock_pkt->Close();
  m_sock_pkt = 0;
}

void DhcpClient::LinkStateChangedHandler(void)
{
  NS_LOG_FUNCTION(this);

  Ptr<NetDevice> netdev = GetNode()->GetDevice(0);

  if (netdev->IsLinkUp())
    {
      NS_LOG_INFO(this << netdev << " link is up");

      DoDhcpRequest();
    }
  else
    {
      NS_LOG_INFO(this << netdev << " link down");

      /* Stop timers */
      Simulator::Cancel(m_leaseTimeoutEvent);
      Simulator::Cancel(m_requestTimeoutEvent);

      /* remove old IP */
      TearDownIp();
    }
}

void DhcpClient::SetupIp(DhcpHeader &offer)
{
  NS_LOG_FUNCTION(this);
  const DhcpOption *opt;

  m_myMask = Ipv4Mask("/24");
  m_srvAddr = offer.GetSIAddr();
  uint32_t lease_time = DHCP_DFT_LEASE_TIME;

  /* Have netmask? */
  if ((opt = offer.GetOptionByType(DhcpOption::DHCP_OPT_SUBNET_MASK)) > 0)
    {
      m_myMask = Ipv4Mask(*opt);
    }

  /* Have lease time? */
  if ((opt = offer.GetOptionByType(DhcpOption::DHCP_OPT_IP_ADDRESS_LEASE_TIME)) > 0)
    {
      lease_time = *opt;
    }

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  Ptr<NetDevice> netdev = GetNode()->GetDevice(0);
  int32_t ifIndex = ipv4->GetInterfaceForDevice(netdev);
  NS_LOG_LOGIC("DhcpClient:: ifIndex " << ifIndex);

  /* Set interface */
  if (ipv4->GetNAddresses(ifIndex) > 0)
    ipv4->RemoveAddress(ifIndex, 0); /* remove old address */

  m_myAddr = offer.GetYIAddr();
  ipv4->AddAddress(ifIndex, Ipv4InterfaceAddress(m_myAddr, m_myMask));
  ipv4->SetUp(ifIndex);

  /* Set default route */
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting(ipv4);
  staticRouting->SetDefaultRoute(m_srvAddr, ifIndex, 0);

  // Ipv4GlobalRoutingHelper::RecomputeRoutingTables();

  /* Start lease timer */
  m_leaseTime = lease_time;
  m_leaseTimeoutEvent = Simulator::Schedule(Seconds(m_leaseTime / 2),
                                            &DhcpClient::RenewTimeoutHandler, this);

  /* Done */
  m_binded = true;
  NS_LOG_INFO(this << " DHCP leased addr = " << m_myAddr
                   << " Server = " << m_srvAddr << " for " << m_leaseTime);
}

void DhcpClient::TearDownIp(void)
{
  NS_LOG_FUNCTION(this);

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();

  /* routes are removed by the Ipv4StaticRouting module */
  /* remove IP */
  int32_t ifIndex = ipv4->GetInterfaceForDevice(GetNode()->GetDevice(0));

  if (ipv4->GetNAddresses(ifIndex) > 0)
    ipv4->RemoveAddress(ifIndex, 0);

  ipv4->SetDown(ifIndex);
}

void DhcpClient::CreateDhcpHeader(DhcpHeader &header, uint8_t type)
{
  NS_LOG_FUNCTION(this);

  m_xid = rand();

  header.SetOp(DhcpHeader::BOOT_REQUEST);
  header.SetTransactionId(m_xid);
  header.SetCHAddr(m_myMacAddr);
  if (m_binded)
    header.SetCIAddr(m_myAddr);

  /* Create options */
  DhcpHeader::DhcpOptionList options;

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_MESSAGE_TYPE, type));

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_REQUESTED_IP_ADDRESS,
                               m_myAddr.Get()));

  if (type == DhcpOption::DHCP_TYPE_REQUEST)
    {
      options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_DHCP_SERVER_IDENTIFIER,
                                   m_srvAddr.Get()));
    }

  options.push_back(DhcpOption((uint8_t)DhcpOption::DHCP_OPT_END));

  header.AddOptionList(options);

  NS_LOG_LOGIC(this << header);
}

void DhcpClient::DoDhcpRequest(void)
{
  NS_LOG_FUNCTION(this);

  /* Create DHCP request */
  DhcpHeader message;

  /* If we don't known a server, send a DISCOVER */
  if (m_srvMacAddr == Mac48Address::GetBroadcast())
    {
      CreateDhcpHeader(message, DhcpOption::DHCP_TYPE_DISCOVER);
    }
  else     /* otherwise, send a REQUEST */
    {
      CreateDhcpHeader(message, DhcpOption::DHCP_TYPE_REQUEST);
    }

  /* Create UDP Header */
  UdpHeader udp4_header;
  udp4_header.SetDestinationPort(DHCP_BOOTPS_PORT);
  udp4_header.SetSourcePort(DHCP_BOOTPC_PORT);

  /* Create IPv4 Header */
  Ipv4Header ipHeader;

  if (!m_binded)
    {
      ipHeader.SetSource(Ipv4Address::GetAny());
      ipHeader.SetDestination(Ipv4Address::GetBroadcast());
    }
  else
    {
      ipHeader.SetSource(m_myAddr);
      ipHeader.SetDestination(m_srvAddr);
    }

  ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
  ipHeader.SetTtl(64);
  ipHeader.SetPayloadSize(udp4_header.GetSerializedSize() + message.GetSerializedSize());

  /* Final assembly */
  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(message);
  packet->AddHeader(udp4_header);
  packet->AddHeader(ipHeader);

  PacketSocketAddress dst;
  if (m_binded)
    dst.SetPhysicalAddress(m_srvMacAddr);
  else
    dst.SetPhysicalAddress(Mac48Address::GetBroadcast());
  dst.SetSingleDevice(GetNode()->GetDevice(0)->GetIfIndex());
  dst.SetProtocol(0x0800);

  if (m_sock_pkt->SendTo(packet, 0, dst) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);

  m_requestTimeoutEvent = Simulator::Schedule(Seconds(m_requestTimeout),
                                              &DhcpClient::RequestTimeoutHandler, this);
}

void DhcpClient::RequestTimeoutHandler(void)
{
  NS_LOG_FUNCTION(this << " Timeout for XID " << m_xid);

  /* forget known server, let's start from the beginning */
  m_srvMacAddr = Mac48Address::GetBroadcast();
  m_srvAddr = Ipv4Address::GetBroadcast();
  m_binded = false;

  Simulator::Cancel(m_leaseTimeoutEvent);

  DoDhcpRequest();
}

void DhcpClient::RenewTimeoutHandler(void)
{
  NS_LOG_FUNCTION(this << " Renewing IP " << m_myAddr << " from " << m_srvAddr);

  /* TODO is the lease expired? if so, call TearDownIp() now */

  DoDhcpRequest();
}

void DhcpClient::NetHandler(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Address from;
  Ptr<Packet> packet = m_sock_pkt->RecvFrom(from);


  if (packet->GetSize() <= 0)
    return;

  Mac48Address offer_src =
    Mac48Address::ConvertFrom(PacketSocketAddress::ConvertFrom(from).GetPhysicalAddress());

  /* Deserialize packet, and check its content */
  Ipv4Header ip4Header;
  packet->RemoveHeader(ip4Header);
  if (ip4Header.GetProtocol() != UdpL4Protocol::PROT_NUMBER)
    return;

  /* FIXME: check IP dst address, SHOULD be broadcast, or our local address
            when bounded. */

  UdpHeader udpHeader;
  packet->RemoveHeader(udpHeader);
  if (udpHeader.GetDestinationPort() != DHCP_BOOTPC_PORT
      && udpHeader.GetSourcePort() != DHCP_BOOTPS_PORT)
    return;

  /* Probably a DHCP packet */
  DhcpHeader dhcpHeader;
  packet->RemoveHeader(dhcpHeader);

  /* Is it destinated to us ? */
  if (dhcpHeader.GetCHAddr() != m_myMacAddr)
    return;

  NS_LOG_INFO(this << " RCV response from " << offer_src);

  /* Check the DHCP response */
  const DhcpOption *opt_msg_type =
    dhcpHeader.GetOptionByType(DhcpOption::DHCP_OPT_MESSAGE_TYPE);

  if (dhcpHeader.GetOp() != DhcpHeader::BOOT_REPLY
      || dhcpHeader.GetTransactionId() != m_xid
      || opt_msg_type == 0)
    {
      NS_LOG_WARN("Got malformed message from server: " << offer_src);
      return;
    }

  if (m_binded && offer_src != m_srvMacAddr)
    {
      /* ignore new servers while we are binded to one */
      NS_LOG_INFO("Ignoring offer from " << offer_src);
      return;
    }

  NS_LOG_LOGIC(this << dhcpHeader);

  /* Seems legit, parse response */
  switch (uint8_t(*opt_msg_type))
    {
    case DhcpOption::DHCP_TYPE_OFFER:
      /* Do we like this offer? For now we are easily pleased, and anything goes */
      Simulator::Cancel(m_requestTimeoutEvent);
      m_srvMacAddr = offer_src;
      m_srvAddr = dhcpHeader.GetSIAddr();
      m_myAddr = dhcpHeader.GetYIAddr();
      DoDhcpRequest();
      break;

    case DhcpOption::DHCP_TYPE_ACK:
      /* Everything is ready, setup the interface */
      Simulator::Cancel(m_requestTimeoutEvent);
      SetupIp(dhcpHeader);
      break;

    default:
      NS_LOG_INFO("Got malformed message from server: " << offer_src);
    }
}

void DhcpClient::Print(std::ostream& os) const
{
  os << "[Node " << GetNode()->GetId() << "] ";
}

std::ostream & operator <<(std::ostream &os, const DhcpClient &client)
{
  client.Print(os);
  return os;
}
} // Namespace ns3
