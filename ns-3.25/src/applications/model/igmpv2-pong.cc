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
#include <arpa/inet.h>
#include "ns3/internet-module.h"

#include "igmpv2-pong.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("Igmpv2Pong");

TypeId Igmpv2Pong::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::Igmpv2Pong")
    .SetParent<Application>()
    .AddConstructor<Igmpv2Pong>()
    .AddAttribute("Local", "The Address on which to Bind the rx socket.",
                  Ipv4AddressValue(),
                  MakeIpv4AddressAccessor(&Igmpv2Pong::m_myAddr),
                  MakeIpv4AddressChecker());

  return tid;
}

Igmpv2Pong::Igmpv2Pong():m_netdev(0), m_sock_pkt(0)
{
  NS_LOG_FUNCTION_NOARGS();
}

Igmpv2Pong::~Igmpv2Pong()
{
  NS_LOG_FUNCTION_NOARGS();
}

void Igmpv2Pong::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void Igmpv2Pong::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  CreateServerSocket();

  if(m_sock_pkt == 0)
    {
      return;
    }
}

void Igmpv2Pong::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  if(m_sock_pkt != 0)
    {
      m_sock_pkt->Close();
      m_sock_pkt = 0;
    }
}

void Igmpv2Pong::CreateServerSocket(void)
{
  NS_LOG_FUNCTION_NOARGS();

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
      m_sock_pkt->SetRecvCallback(MakeCallback(&Igmpv2Pong::NetHandler, this));
    }
}

void Igmpv2Pong::NetHandler(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Address from;
  Ptr<Packet> packet = m_sock_pkt->RecvFrom(from);

  if(packet->GetSize () <= 0)
    {
      return;
    }

  m_srvMacAddr = Mac48Address::ConvertFrom(PacketSocketAddress::ConvertFrom(from).GetPhysicalAddress());

  /* Deserialize packet, and check its content */
  Ipv4Header ip4Header;
  packet->RemoveHeader(ip4Header);

  m_srvAddr = ip4Header.GetSource();

  if ((ip4Header.GetDestination() != Ipv4Address::GetBroadcast()
       && ip4Header.GetDestination() != m_myAddr)
      || ip4Header.GetProtocol() != Igmpv2L4Protocol::PROT_NUMBER)
    {
      return;
    }

  /* Probably a IGMPv2 packet */
  Igmpv2Header igmpv2Header;
  packet->RemoveHeader(igmpv2Header);

  PongPacket(igmpv2Header);
}

void Igmpv2Pong::PongPacket(Igmpv2Header &igmpv2Header)
{
  NS_LOG_FUNCTION(this << igmpv2Header);

  SendMessage(igmpv2Header);
}

void Igmpv2Pong::SendMessage(Igmpv2Header &igmpv2Header)
{
  NS_LOG_FUNCTION(this << igmpv2Header);

  /* Create IPv4 Header */
  Ipv4Header ipHeader;
  if(!m_binded)
    {
      ipHeader.SetSource(Ipv4Address::GetAny());
      ipHeader.SetDestination(m_srvAddr);
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
  dst.SetPhysicalAddress(m_srvMacAddr);

  dst.SetSingleDevice(GetNode()->GetDevice(0)->GetIfIndex());
  dst.SetProtocol(0x0800);

  if(m_sock_pkt->SendTo(packet, 0, dst) < 0)
    NS_LOG_WARN(this << " Failed to send " << packet);
}
}
