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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/internet-module.h"
#include "ns3/packet-socket-address.h"
#include "ns3/ethernet-header.h"
#include "multicast-source.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("MulticastSource");
NS_OBJECT_ENSURE_REGISTERED(MulticastSource);

TypeId MulticastSource::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::MulticastSource")
                      .SetParent<Application>()
                      .AddConstructor<MulticastSource>()
                      .AddAttribute("Group", "The multicast IPv4 group address.",
                                    Ipv4AddressValue(Ipv4Address("224.1.1.1")),
                                    MakeIpv4AddressAccessor(&MulticastSource::m_mcastGroup),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("Interval",
                                    "The time to wait between packets",
                                    TimeValue(Seconds(1.0)),
                                    MakeTimeAccessor(&MulticastSource::m_interval),
                                    MakeTimeChecker())
                      .AddAttribute("Port",
                                    "The destination port of the outbound packets",
                                    UintegerValue(9),
                                    MakeUintegerAccessor(&MulticastSource::m_port),
                                    MakeUintegerChecker<uint16_t>())
                      .AddAttribute("PacketSize", "Size of echo data in outbound packets",
                                    UintegerValue(100),
                                    MakeUintegerAccessor(&MulticastSource::SetDataSize,
                                                         &MulticastSource::GetDataSize),
                                    MakeUintegerChecker<uint32_t>())
                      .AddTraceSource("Tx", "A new packet is created and is sent",
                                      MakeTraceSourceAccessor(&MulticastSource::m_txTrace),
                                      "ns3::Packet::PacketSizeTracedCallback");

  return tid;
}

MulticastSource::MulticastSource() :
  m_size(0),
  m_dataSize(0),
  m_data(nullptr),
  m_sent(0),
  m_sock_udp(nullptr),
  m_sock_igmp(nullptr)
{
  NS_LOG_FUNCTION(this);
}

MulticastSource::~MulticastSource()
{
  NS_LOG_FUNCTION(this);
}

void MulticastSource::DoDispose(void)
{
  NS_LOG_FUNCTION(this);

  Simulator::Cancel(m_sendEvent);

  if (m_sock_udp)
    m_sock_udp->Close();

  if (m_sock_igmp)
    m_sock_igmp->Close();

  if (m_data)
    {
      delete [] m_data;
      m_data = nullptr;
      m_dataSize = 0;
    }

  Application::DoDispose();
}

void MulticastSource::StartApplication(void)
{
  NS_LOG_FUNCTION(this << m_mcastGroup);

  NS_ASSERT(m_mcastGroup.IsMulticast());
  NS_ASSERT(m_sock_udp == nullptr);

  /* Get local address */
  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
  Ipv4Address local = ipv4->SelectSourceAddress(0, m_mcastGroup,
                                                Ipv4InterfaceAddress::GLOBAL);

  NS_LOG_LOGIC("Local address " << local);

  /* Create UDP socket */
  m_sock_udp = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::UdpSocketFactory"));
  NS_ASSERT(m_sock_udp != 0);
  m_sock_udp->Bind(InetSocketAddress(local, 0));
  m_sock_udp->ShutdownRecv();
  m_sock_udp->Connect(InetSocketAddress(m_mcastGroup, m_port));

  /* Create IGMP socket (RAW socket) */
  m_sock_igmp = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  NS_ASSERT(m_sock_igmp != 0);
  m_sock_igmp->SetAttribute("Protocol", UintegerValue(Igmpv2L4Protocol::PROT_NUMBER));
  m_sock_igmp->SetRecvCallback(MakeCallback(&MulticastSource::HandleReadIgmp, this));
  m_sock_igmp->Bind(InetSocketAddress(Ipv4Address::GetAny(), 0));
  m_sock_igmp->Listen();
}

void MulticastSource::StopApplication()
{
  NS_LOG_FUNCTION(this);

  Simulator::Cancel(m_sendEvent);

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

void MulticastSource::SetDataSize(uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << dataSize);

  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so
  // neither will we.
  //
  delete [] m_data;
  m_data = nullptr;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t MulticastSource::GetDataSize(void) const
{
  NS_LOG_FUNCTION(this);
  return m_size;
}

void MulticastSource::SetFill(std::string fill)
{
  NS_LOG_FUNCTION(this << fill);

  uint32_t dataSize = fill.size() + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy(m_data, fill.c_str(), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void MulticastSource::SetFill(uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset(m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void MulticastSource::SetFill(uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION(this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy(m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;

  while (filled + fillSize < dataSize)
    {
      memcpy(&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy(&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void MulticastSource::ScheduleTransmit()
{
  NS_LOG_FUNCTION(this << m_interval);
  m_sendEvent = Simulator::Schedule(m_interval, &MulticastSource::Send, this);
}

void MulticastSource::Send(void)
{
  NS_LOG_FUNCTION(this);

  if (clientsAddr.empty())
    return;

  NS_ASSERT(m_sendEvent.IsExpired());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG(m_dataSize == m_size, "MulticastSource::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG(m_data, "MulticastSource::Send(): m_dataSize but no m_data");
      p = Create<Packet>(m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the(zero) m_dataSize.
      //
      p = Create<Packet>(m_size);
    }

  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  m_txTrace(p);

  if (m_sock_udp->Send(p, 0) < 0)
    NS_LOG_WARN("Failed to send Multicast flow");

  ScheduleTransmit();
}

void MulticastSource::HandleReadIgmp(Ptr<Socket> socket)
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

      if (hIgmp.GetGroupAddress() != m_mcastGroup)
        break;

      if (hIgmp.GetType() == Igmpv2Header::MEMBERSHIP_REPORT_V2)
        {
          NS_LOG_INFO(m_mcastGroup << " Got REPORT from " << from);
          if (clientsAddr.empty())
            ScheduleTransmit();

          clientsAddr.insert(hIp4.GetSource());
        }
      else if (hIgmp.GetType() == Igmpv2Header::LEAVE_GROUP)
        {
          NS_LOG_INFO(m_mcastGroup << " Got LEAVE from " << from);
          clientsAddr.erase(hIp4.GetSource());

          if (clientsAddr.empty())
            Simulator::Cancel(m_sendEvent);
        }
    }
}
} // Namespace ns3
