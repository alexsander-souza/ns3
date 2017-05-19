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


#include <ns3/abort.h>
#include "ns3/nstime.h"
#include "ns3/ipv4.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/socket-factory.h"
#include "ns3/packet-socket-address.h"
#include "ns3/arp-header.h"
#include "ns3/string.h"
#include "arping.h"

NS_LOG_COMPONENT_DEFINE("Arping");
namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(Arping);

TypeId
Arping::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Arping")
                      .SetParent<Application>()
                      .AddConstructor<Arping>()
                      .AddAttribute("AdvAddress",
                                    "Advertisement IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&Arping::m_advAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("ArpInterval",
                                    "Gratuitous ARP interval",
                                    TimeValue(Seconds(5)),
                                    MakeTimeAccessor(&Arping::m_arpInterval),
                                    MakeTimeChecker())
  ;

  return tid;
}

Arping::Arping() :
  m_netdev(nullptr),
  m_sock_pkt(nullptr)
{
  NS_LOG_FUNCTION_NOARGS();
}

Arping::~Arping()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
Arping::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void
Arping::StartApplication(void)
{
  NS_LOG_FUNCTION_NOARGS();

  NS_ABORT_IF(m_advAddress == Ipv4Address::GetAny());

  Ptr<Ipv4> ipv4    = GetNode()->GetObject<Ipv4>();
  int32_t   ifIndex = ipv4->GetInterfaceForAddress(m_advAddress);

  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to interface");
  m_netdev = ipv4->GetNetDevice(ifIndex);

  TypeId tid = TypeId::LookupByName("ns3::PacketSocketFactory");
  m_sock_pkt = Socket::CreateSocket(GetNode(), tid);

  m_arpEvent = Simulator::Schedule(m_arpInterval / 2, &Arping::GratuitousArpSender, this);
}

void
Arping::StopApplication(void)
{
  if (m_sock_pkt != nullptr)
    m_sock_pkt->Close();

  Simulator::Remove(m_arpEvent);
}

void
Arping::GratuitousArpSender()
{
  ArpHeader arp;

  arp.SetReply(m_netdev->GetAddress(), m_advAddress, Mac48Address::GetBroadcast(), m_advAddress);

  Ptr<Packet> pkt = Create<Packet>();
  pkt->AddHeader(arp);

  PacketSocketAddress dst;

  dst.SetPhysicalAddress(Mac48Address::GetBroadcast());
  dst.SetSingleDevice(m_netdev->GetIfIndex());
  dst.SetProtocol(0x0806);

  if (m_sock_pkt->SendTo(pkt, 0, dst) < 0)
    NS_LOG_WARN(this << " Failed to send " << pkt);

  m_arpEvent = Simulator::Schedule(m_arpInterval, &Arping::GratuitousArpSender, this);
}
}
