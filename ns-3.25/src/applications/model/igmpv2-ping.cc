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

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ethernet-header.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include <arpa/inet.h>

#include "igmpv2-ping.h"
#include "ns3/internet-module.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Igmpv2Ping");

TypeId Igmpv2Ping::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::Igmpv2Ping")
    .SetParent<Application>()
    .AddConstructor<Igmpv2Ping>()
    .AddAttribute("Local", "The Address on which to Bind the rx socket.",
                  Ipv4AddressValue(),
                  MakeIpv4AddressAccessor(&Igmpv2Ping::m_myAddr),
                  MakeIpv4AddressChecker())
    .AddAttribute("Group", "The multicast IPv4 group address.",
                  Ipv4AddressValue(Ipv4Address("224.1.1.1")),
                  MakeIpv4AddressAccessor(&Igmpv2Ping::m_groupAddr),
                  MakeIpv4AddressChecker());

  return tid;
}

Igmpv2Ping::Igmpv2Ping():
  m_sock_pkt(0),
  m_myMacAddr(Mac48Address::GetBroadcast()),
  m_srvMacAddr(Mac48Address::GetBroadcast()),
  m_srvAddr(Ipv4Address::GetBroadcast()),
  m_myMask(Ipv4Mask::GetZero()),
  m_binded(0)
{
  NS_LOG_FUNCTION_NOARGS();
}

Igmpv2Ping::~Igmpv2Ping()
{
  NS_LOG_FUNCTION_NOARGS();
}

void Igmpv2Ping::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void Igmpv2Ping::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();
  NS_LOG_INFO("aqui StartApplication Ping");
  if(m_sock_pkt > 0)
    return;

  Ptr<NetDevice> netdev = GetNode()->GetDevice(0);

  m_myMacAddr = Mac48Address::ConvertFrom(netdev->GetAddress());

  /* RAW socket */
  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  m_sock_pkt = Socket::CreateSocket(GetNode(), tid);

  PacketSocketAddress sock_addr;
  sock_addr.SetSingleDevice(netdev->GetIfIndex());
  sock_addr.SetPhysicalAddress(netdev->GetAddress());
  sock_addr.SetProtocol(0x0800);

  if(!(m_sock_pkt->Bind(sock_addr)))
    {
      m_binded = true;
    }

  if(netdev->IsLinkUp())
    {
      SendIgmpv2Header(Igmpv2Header::MEMBERSHIP_QUERY, 0x50, m_groupAddr, true);
      SendIgmpv2Header(Igmpv2Header::MEMBERSHIP_REPORT_V2, 0x60, m_groupAddr, false);
      SendIgmpv2Header(Igmpv2Header::LEAVE_GROUP, 0x64, m_groupAddr, true);
    }
}

void Igmpv2Ping::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  m_sock_pkt->Close();
  m_sock_pkt = 0;
}

void Igmpv2Ping::CreateIgmpv2Header(Igmpv2Header &header, uint8_t type,
                                    uint8_t maxRespTime, Ipv4Address groupAddress, bool enableChecksum)
{
  NS_LOG_FUNCTION(this);

  header.SetType(type);
  header.SetMaxRespTime(maxRespTime);
  header.SetGroupAddress(groupAddress);
  if(enableChecksum)
    header.EnableChecksum();
}

void Igmpv2Ping::SendIgmpv2Header(uint8_t type, uint8_t maxRespTime,
                                  Ipv4Address groupAddress, bool enableChecksum)
{
  NS_LOG_FUNCTION(this);

  /* Create IGMPv2 Header */
  Igmpv2Header igmpv2Header;

  CreateIgmpv2Header(igmpv2Header, type, maxRespTime, groupAddress, enableChecksum);

  /* Create IPv4 Header */
  Ipv4Header ipHeader;

  if(!m_binded)
    {
      ipHeader.SetSource(Ipv4Address::GetAny());
      ipHeader.SetDestination(Ipv4Address::GetBroadcast());
    }
  else
    {
      ipHeader.SetSource(m_myAddr);
      ipHeader.SetDestination(m_srvAddr);
    }

  ipHeader.SetProtocol(Igmpv2L4Protocol::PROT_NUMBER);
  ipHeader.SetTtl(64);
  ipHeader.SetPayloadSize(igmpv2Header.GetSerializedSize());

  /* Final assembly */
  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(igmpv2Header);
  packet->AddHeader(ipHeader);

  PacketSocketAddress dst;
  if(m_binded)
    dst.SetPhysicalAddress(m_srvMacAddr);
  else
    dst.SetPhysicalAddress(Mac48Address::GetBroadcast());

  dst.SetSingleDevice(GetNode()->GetDevice(0)->GetIfIndex());
  dst.SetProtocol(0x0800);

  if(m_sock_pkt->SendTo(packet, 0, dst) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);
}

void Igmpv2Ping::Print(std::ostream& os) const
{
  os << "[Node " << GetNode()->GetId() << "] ";
}

std::ostream & operator << (std::ostream &os, const Igmpv2Ping &client)
{
  client.Print (os);
  return os;
}

} // Namespace ns3
