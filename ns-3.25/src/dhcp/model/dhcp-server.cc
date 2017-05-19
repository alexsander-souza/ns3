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
#include "dhcp-server.h"
#include <arpa/inet.h>

NS_LOG_COMPONENT_DEFINE("DhcpServer");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(DhcpServer);

TypeId
DhcpServer::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DhcpServer")
                      .SetParent<Application>()
                      .AddConstructor<DhcpServer>()
                      .AddAttribute("Port",
                                    "Port on which we listen for incoming packets.",
                                    UintegerValue(DHCP_BOOTPS_PORT),
                                    MakeUintegerAccessor(&DhcpServer::m_port),
                                    MakeUintegerChecker<uint16_t>())
                      .AddAttribute("PoolAddresses",
                                    "Pool of addresses to provide on request.",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&DhcpServer::m_poolAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("PoolMask",
                                    "Mask of the pool of addresses.",
                                    Ipv4MaskValue(),
                                    MakeIpv4MaskAccessor(&DhcpServer::m_poolMask),
                                    MakeIpv4MaskChecker())
                      .AddAttribute("LeaseTime",
                                    "Time until the address leased to client expires.",
                                    UintegerValue(GRANTED_LEASE_TIME),
                                    MakeUintegerAccessor(&DhcpServer::m_leaseTime),
                                    MakeUintegerChecker<uint32_t>())
  ;

  return tid;
}

DhcpServer::DhcpServer() : m_sock_pkt(0)
{
  // NS_LOG_FUNCTION_NOARGS();
}

DhcpServer::~DhcpServer()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
DhcpServer::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void
DhcpServer::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  CreateServerSocket();

  if (m_sock_pkt == 0)
    return;

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();

  GetOwnLease(m_netdev, ipv4);
}

void
DhcpServer::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  if (m_sock_pkt != 0)
  {
    m_sock_pkt->Close();
    m_sock_pkt = 0;
  }
}

void
DhcpServer::CreateServerSocket(void)
{
  NS_LOG_FUNCTION(this);

  if (m_sock_pkt != 0)
  {
    m_sock_pkt->Close();
    m_sock_pkt = 0;
  }

  if (m_poolAddress == Ipv4Address::GetAny())
  {
    NS_LOG_WARN(this << "Leased pool undefined, unable to create socket");
    return;
  }

  /* Get netdev for pool */
  Ptr<Ipv4> ipv4    = GetNode()->GetObject<Ipv4>();
  int32_t   ifIndex = ipv4->GetInterfaceForPrefix(m_poolAddress, m_poolMask);

  if (ifIndex < 0)
  {
    NS_LOG_WARN(this << "Unable to bind socket to a interface");
    return;
  }

  m_netdev    = ipv4->GetNetDevice(ifIndex);
  m_myMacAddr = Mac48Address::ConvertFrom(m_netdev->GetAddress());

  /* RAW socket */
  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  m_sock_pkt = Socket::CreateSocket(GetNode(), tid);

  PacketSocketAddress sock_addr;
  sock_addr.SetSingleDevice(m_netdev->GetIfIndex());
  sock_addr.SetPhysicalAddress(m_netdev->GetAddress());
  sock_addr.SetProtocol(0x0800);

  m_sock_pkt->Bind(sock_addr);
  m_sock_pkt->SetRecvCallback(MakeCallback(&DhcpServer::NetHandler, this));

  /* UDP socket */
  TypeId tid_udp = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_sock_udp = Socket::CreateSocket(GetNode(), tid_udp);
  InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), DHCP_BOOTPS_PORT);
  m_sock_udp->SetAllowBroadcast(true);
  m_sock_udp->Bind(local);

  NS_LOG_INFO(this << "DHCP server binded at " << m_netdev->GetAddress());
}

void
DhcpServer::NetHandler(Ptr<Socket>socket)
{
  Address from;

  Ptr<Packet> packet = m_sock_pkt->RecvFrom(from);

  Mac48Address client_hwaddr =
    Mac48Address::ConvertFrom(PacketSocketAddress::ConvertFrom(from).GetPhysicalAddress());

  /* Deserialize packet, and check its content */
  Ipv4Header ip4Header;
  packet->RemoveHeader(ip4Header);

  if (((ip4Header.GetDestination() != Ipv4Address::GetBroadcast())
       && (ip4Header.GetDestination() != m_myAddr))
      || (ip4Header.GetProtocol() != UdpL4Protocol::PROT_NUMBER))
    return;

  UdpHeader udpHeader;
  packet->RemoveHeader(udpHeader);

  if ((udpHeader.GetDestinationPort() != DHCP_BOOTPS_PORT)
      && (udpHeader.GetSourcePort() != DHCP_BOOTPC_PORT))
    return;

  /* Probably a DHCP packet */
  DhcpHeader dhcpHeader;
  packet->RemoveHeader(dhcpHeader);

  ReceivePacket(dhcpHeader, client_hwaddr);
}

void
DhcpServer::SendMessage(Mac48Address& clientId, DhcpHeader& response)
{
  NS_LOG_FUNCTION(this << clientId);

  /* Create UDP Header */
  UdpHeader udp4_header;
  udp4_header.SetDestinationPort(DHCP_BOOTPC_PORT);
  udp4_header.SetSourcePort(DHCP_BOOTPS_PORT);

  /* Create IPv4 Header */
  Ipv4Header ipHeader;
  ipHeader.SetSource(m_myAddr);
  ipHeader.SetDestination(response.GetYIAddr());
  ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
  ipHeader.SetTtl(64);
  ipHeader.SetPayloadSize(udp4_header.GetSerializedSize() + response.GetSerializedSize());

  /* Final assembly */
  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(response);
  packet->AddHeader(udp4_header);
  packet->AddHeader(ipHeader);

  /* Send response to client */
  PacketSocketAddress dst;
  dst.SetPhysicalAddress(clientId); /* Always Unicast */
  dst.SetSingleDevice(m_netdev->GetIfIndex());
  dst.SetProtocol(0x0800);

  if (m_sock_pkt->SendTo(packet, 0, dst) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);
}
} // Namespace ns3
