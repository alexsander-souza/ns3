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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "multicast-sink.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MulticastSink");
NS_OBJECT_ENSURE_REGISTERED(MulticastSink);

TypeId MulticastSink::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::MulticastSink")
                      .SetParent<Application>()
                      .AddConstructor<MulticastSink>()
                      .AddAttribute("Port", "UDP port number",
                                    UintegerValue(9),
                                    MakeUintegerAccessor(&MulticastSink::m_port),
                                    MakeUintegerChecker<uint16_t>(1))
                      .AddAttribute("Group", "The multicast IPv4 group address.",
                                    Ipv4AddressValue(Ipv4Address("224.1.1.1")),
                                    MakeIpv4AddressAccessor(&MulticastSink::m_groupAddr),
                                    MakeIpv4AddressChecker())
                      .AddTraceSource("Rx", "A packet has been received",
                                      MakeTraceSourceAccessor(&MulticastSink::m_rxTrace),
                                      "ns3::Packet::PacketSizeTracedCallback");

  return tid;
}

MulticastSink::MulticastSink() :
  m_sock_udp(nullptr),
  m_sock_igmp(nullptr),
  m_maxRespTime(0x50),
  m_enableChecksum(false),
  m_totalRx(0)
{
  NS_LOG_FUNCTION(this);
}

MulticastSink::~MulticastSink()
{
  NS_LOG_FUNCTION(this);
}

uint32_t MulticastSink::GetTotalRx() const
{
  NS_LOG_FUNCTION(this);

  return m_totalRx;
}

void MulticastSink::DoDispose(void)
{
  NS_LOG_FUNCTION(this);

  if (m_sock_udp)
    m_sock_udp->Close();
  if (m_sock_igmp)
    m_sock_igmp->Close();

  // chain up
  Application::DoDispose();
}

// Application Methods
void MulticastSink::StartApplication()
{
  NS_LOG_FUNCTION(this << m_groupAddr);

  NS_ASSERT(m_groupAddr.IsMulticast());
  NS_ASSERT(m_sock_udp == nullptr);

  /* Get local address */
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  Ipv4Address local = ipv4->SelectSourceAddress(0, m_groupAddr,
                                                Ipv4InterfaceAddress::GLOBAL);

  NS_LOG_LOGIC("Local address " << local);

  /* Create UDP socket */
  m_sock_udp = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
  NS_ASSERT(m_sock_udp != 0);
  m_sock_udp->Bind(InetSocketAddress(local, m_port));
  m_sock_udp->Listen();
  m_sock_udp->ShutdownSend();
  m_sock_udp->SetRecvCallback(MakeCallback(&MulticastSink::HandleReadUdp, this));

  /* Create IGMP socket (RAW socket) */
  m_sock_igmp = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  NS_ASSERT(m_sock_igmp != 0);
  m_sock_igmp->SetAttribute("Protocol", UintegerValue(Igmpv2L4Protocol::PROT_NUMBER));
  m_sock_igmp->SetRecvCallback(MakeCallback(&MulticastSink::HandleReadIgmp, this));
  m_sock_igmp->Bind(InetSocketAddress(Ipv4Address::GetAny(), 0));
  m_sock_igmp->Listen();

  SendIgmpv2(Igmpv2Header::MEMBERSHIP_REPORT_V2);
}

void MulticastSink::StopApplication()
{
  NS_LOG_FUNCTION(this);

  SendIgmpv2(Igmpv2Header::LEAVE_GROUP);

  if (m_sock_udp)
    {
      m_sock_udp->Close();
      m_sock_udp->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
      m_sock_udp = nullptr;
    }

  if (m_sock_igmp)
    {
      m_sock_igmp->Close();
      m_sock_igmp->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> >());
      m_sock_igmp = nullptr;
    }
}

void MulticastSink::SendIgmpv2(uint8_t type)
{
  NS_LOG_FUNCTION(this);

  NS_ASSERT(m_sock_igmp != nullptr);

  /* Create IGMPv2 Header */
  Igmpv2Header hIgmp;
  hIgmp.SetType(type);
  hIgmp.SetMaxRespTime(m_maxRespTime);
  hIgmp.SetGroupAddress(m_groupAddr);
  if (m_enableChecksum)
    hIgmp.EnableChecksum();

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(hIgmp);

  SocketIpTtlTag ttlTag;
  ttlTag.SetTtl(1);
  packet->AddPacketTag(ttlTag);

  if (m_sock_igmp->SendTo(packet, 0, InetSocketAddress(m_groupAddr, 0)) < 0)
    NS_LOG_WARN(this << " Failed to send IGMP");
}

void MulticastSink::HandleReadUdp(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      if (packet->GetSize() == 0)
        break;

      m_totalRx += packet->GetSize();

      if (InetSocketAddress::IsMatchingType(from))
        {
          NS_LOG_INFO("At time " << Simulator::Now().GetSeconds()
                                 << "s packet sink received "
                                 << packet->GetSize() << " bytes from "
                                 << InetSocketAddress::ConvertFrom(from).GetIpv4()
                                 << " port " << InetSocketAddress::ConvertFrom(from).GetPort()
                                 << " total Rx " << m_totalRx << " bytes");
        }
      m_rxTrace(packet, from);
    }
}

void MulticastSink::HandleReadIgmp(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      if (packet->GetSize() == 0)
        break;

      Ipv4Header hIp4;
      packet->RemoveHeader(hIp4);

      NS_ASSERT(hIp4.GetProtocol() == Igmpv2L4Protocol::PROT_NUMBER); // protocol should be IGMP

      Igmpv2Header hIgmp;
      packet->RemoveHeader(hIgmp);

      if (hIgmp.GetType() != Igmpv2Header::MEMBERSHIP_QUERY
          || (hIgmp.GetGroupAddress() != m_groupAddr
              && hIgmp.GetGroupAddress() != Ipv4Address::GetZero()))
        break;

      SendIgmpv2(Igmpv2Header::MEMBERSHIP_REPORT_V2);
    }
}
} // Namespace ns3
