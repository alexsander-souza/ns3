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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/udp-header.h"
#include "ns3/ethernet-header.h"
#include "ns3/udp-l4-protocol.h"
#include "bfd-echo.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("BfdEchoAgent");
NS_OBJECT_ENSURE_REGISTERED(BfdEchoAgent);

TypeId
BfdEchoAgent::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::BfdEchoAgent")
                      .SetParent<Application> ()
                      .AddConstructor<BfdEchoAgent> ()
                      .AddAttribute("AgentAddr",
                                    "Agent IP address",
                                    AddressValue(),
                                    MakeIpv4AddressAccessor(&BfdEchoAgent::m_myAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("GatewayAddr",
                                    "Gateway MAC48 address",
                                    Mac48AddressValue(),
                                    MakeMac48AddressAccessor(&BfdEchoAgent::m_gwHwAddress),
                                    MakeMac48AddressChecker())
                      .AddAttribute("PingInterval",
                                    "Ping interval",
                                    TimeValue(Seconds(5)),
                                    MakeTimeAccessor(&BfdEchoAgent::m_pingInterval),
                                    MakeTimeChecker())
  ;

  return tid;
}

BfdEchoAgent::BfdEchoAgent() :
  m_netdev(nullptr),
  m_sock_pkt(nullptr),
  m_sock_udp(nullptr),
  m_myAddress(Ipv4Address::GetAny()),
  m_echo_prepared(false)
{
  NS_LOG_FUNCTION_NOARGS();
}

BfdEchoAgent::~BfdEchoAgent ()
{
  NS_LOG_FUNCTION_NOARGS();
}

void BfdEchoAgent::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void BfdEchoAgent::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  m_pingEvent = Simulator::Schedule(m_pingInterval, &BfdEchoAgent::SendEcho, this);
}

void BfdEchoAgent::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  if (m_sock_pkt != nullptr)
    m_sock_pkt->Close();

  if (m_sock_udp != nullptr)
    m_sock_udp->Close();

  m_sock_udp = nullptr;
  m_sock_pkt = nullptr;
  m_echo_prepared = false;

  Simulator::Remove(m_pingEvent);
}

void BfdEchoAgent::CreateServerSocket(void)
{
  if (m_sock_pkt != nullptr || !discoverLocalAddress())
    return;

  NS_LOG_FUNCTION_NOARGS();

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  int32_t ifIndex = ipv4->GetInterfaceForAddress(m_myAddress);

  if (ifIndex < 0)
    {
      NS_LOG_LOGIC(this << "Unable to bind to local interface for address " << m_myAddress);
      return;
    }

  m_netdev = ipv4->GetNetDevice(ifIndex);
  m_netdev->AddLinkChangeCallback(
    MakeCallback(&BfdEchoAgent::LinkStateChangedHandler, this));

  /* RAW socket */
  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  m_sock_pkt = Socket::CreateSocket(GetNode(), tid);

  PacketSocketAddress sock_addr;
  sock_addr.SetSingleDevice(m_netdev->GetIfIndex());
  sock_addr.SetPhysicalAddress(m_netdev->GetAddress());
  sock_addr.SetProtocol(0x0800);

  m_sock_pkt->Bind(sock_addr);
  m_sock_pkt->ShutdownRecv();

  /* UDP socket */
  TypeId tid_udp = TypeId::LookupByName("ns3::UdpSocketFactory");
  m_sock_udp = Socket::CreateSocket(GetNode(), tid_udp);
  InetSocketAddress local = InetSocketAddress(m_myAddress, BFD_ECHO_PORT);
  m_sock_udp->Bind(local);
  m_sock_udp->SetRecvCallback(MakeCallback(&BfdEchoAgent::NetHandler, this));
}

bool BfdEchoAgent::discoverLocalAddress()
{
  if (m_myAddress != Ipv4Address::GetAny())
    return true;

  NS_LOG_FUNCTION_NOARGS();

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();

  for (unsigned int i = 0; i < ipv4->GetNInterfaces(); ++i)
    {
      for (unsigned int j = 0; j < ipv4->GetNAddresses(i); ++j)
        {
          const Ipv4InterfaceAddress &iaddr = ipv4->GetAddress(i, j);
          if (iaddr.GetLocal() != Ipv4Address::GetLoopback())
            {
              m_myAddress = iaddr.GetLocal();
              NS_LOG_DEBUG(this << " new local address " << m_myAddress);
              return true;
            }
        }
    }
  return false;
}

void BfdEchoAgent::BuildEchoPacket(void)
{
  if (m_echo_prepared || m_myAddress == Ipv4Address::GetAny())
    return;

  NS_LOG_FUNCTION_NOARGS();

  /* Create UDP Header */
  UdpHeader udp4_header;

  udp4_header.SetDestinationPort(BFD_ECHO_PORT);
  udp4_header.SetSourcePort(49152); /* See RFC */

  /* Create IPv4 Header */
  Ipv4Header ipHeader;
  ipHeader.SetSource(m_myAddress);
  ipHeader.SetDestination(m_myAddress);

  ipHeader.SetProtocol(UdpL4Protocol::PROT_NUMBER);
  ipHeader.SetTtl(64);
  ipHeader.SetPayloadSize(udp4_header.GetSerializedSize());

  /* Final assembly */
  m_echoPacket = Create<Packet>();
  m_echoPacket->AddHeader(udp4_header);
  m_echoPacket->AddHeader(ipHeader);

  m_echo_prepared = true;
}

void BfdEchoAgent::LinkStateChangedHandler(void)
{
  NS_LOG_FUNCTION(this);

  if (m_sock_pkt != nullptr)
    m_sock_pkt->Close();

  if (m_sock_udp != nullptr)
    m_sock_udp->Close();

  m_sock_udp = nullptr;
  m_sock_pkt = nullptr;

  m_myAddress = Ipv4Address::GetAny();
  m_echo_prepared = false;

  Simulator::Remove(m_pingEvent);
  SendEcho();
}

void BfdEchoAgent::SendEcho()
{
  NS_LOG_FUNCTION(this);

  CreateServerSocket();
  BuildEchoPacket();

  if (m_sock_pkt != nullptr)
    {
      PacketSocketAddress dst;

      dst.SetPhysicalAddress(m_gwHwAddress);
      dst.SetSingleDevice(m_netdev->GetIfIndex());
      dst.SetProtocol(0x0800);

      if (m_sock_pkt->SendTo(m_echoPacket->Copy(), 0, dst) < 0)
        NS_LOG_WARN(this << " Failed to send " << m_echoPacket);
    }

  m_pingEvent = Simulator::Schedule(m_pingInterval, &BfdEchoAgent::SendEcho, this);
}

void BfdEchoAgent::NetHandler(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = 0;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      NS_LOG_LOGIC(this << packet);
    }
}

void BfdEchoAgent::Print(std::ostream& os) const
{
  os << "[BFD_ECHO: " << GetNode()->GetId() << "] ";
}

std::ostream & operator <<(std::ostream &os, const BfdEchoAgent &client)
{
  client.Print(os);
  return os;
}
}
