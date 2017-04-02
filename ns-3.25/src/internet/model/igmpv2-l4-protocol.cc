/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 */

#include "igmpv2-l4-protocol.h"
#include "ipv4-raw-socket-factory-impl.h"
#include "ipv4-interface.h"
#include "ipv4-l3-protocol.h"
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/boolean.h"
#include "ns3/ipv4-route.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("Igmpv2L4Protocol");

NS_OBJECT_ENSURE_REGISTERED(Igmpv2L4Protocol);

// see rfc 2236
const uint8_t Igmpv2L4Protocol::PROT_NUMBER = 2;

TypeId Igmpv2L4Protocol::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::Igmpv2L4Protocol").SetParent<IpL4Protocol>().AddConstructor<Igmpv2L4Protocol>();
  return tid;
}

Igmpv2L4Protocol::Igmpv2L4Protocol() : m_node(0)
{
  NS_LOG_FUNCTION(this);
}

Igmpv2L4Protocol::~Igmpv2L4Protocol()
{
  NS_LOG_FUNCTION(this);
  NS_ASSERT(m_node == 0);
}

void Igmpv2L4Protocol::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << node);
  m_node = node;
}

/*
 * This method is called by AddAgregate and completes the aggregation
 * by setting the node in the IGMPv2 stack and adding IGMPv2 factory to
 * IPv4 stack connected to the node
 */
void Igmpv2L4Protocol::NotifyNewAggregate()
{
  NS_LOG_FUNCTION(this);
  if(m_node == 0)
    {
      Ptr<Node> node = this->GetObject<Node>();
      if (node != 0)
        {
          Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();
          if(ipv4 != 0 && m_downTarget.IsNull())
            {
              this->SetNode(node);
              ipv4->Insert(this);
              Ptr<Ipv4RawSocketFactoryImpl> rawFactory = CreateObject<Ipv4RawSocketFactoryImpl>();
              ipv4->AggregateObject(rawFactory);
              this->SetDownTarget (MakeCallback(&Ipv4::Send, ipv4));
            }
        }
    }
  Object::NotifyNewAggregate ();
}

uint16_t Igmpv2L4Protocol::GetStaticProtocolNumber(void)
{
  NS_LOG_FUNCTION_NOARGS();
  return Igmpv2L4Protocol::PROT_NUMBER;
}

int Igmpv2L4Protocol::GetProtocolNumber(void) const
{
  NS_LOG_FUNCTION(this);
  return Igmpv2L4Protocol::PROT_NUMBER;
}

void Igmpv2L4Protocol::SendMessage(Ptr<Packet> packet, Ipv4Address dest,
                                   uint8_t type, uint8_t maxRespTime,
                                   uint16_t checksum, Ipv4Address groupAddress)
{
  NS_LOG_FUNCTION(this);
  Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
  NS_ASSERT (ipv4 != 0 && ipv4->GetRoutingProtocol() != 0);
  Ipv4Header header;
  header.SetDestination(dest);
  header.SetProtocol(PROT_NUMBER);
  Socket::SocketErrno errno_;
  Ptr<Ipv4Route> route;
  Ptr<NetDevice> oif(0); //specify non-zero if bound to a source address
  route = ipv4->GetRoutingProtocol()->RouteOutput(packet, header, oif, errno_);
  if (route != 0)
    {
      NS_LOG_LOGIC ("Route exists");
      Ipv4Address source = route->GetSource();
      SendMessage(packet, source, dest, type, maxRespTime, groupAddress, route);
    }
  else
    {
      NS_LOG_WARN ("drop IGMPv2 message");
    }
}

void Igmpv2L4Protocol::SendMessage(Ptr<Packet> packet, Ipv4Address source,
                                   Ipv4Address dest, uint8_t type, uint8_t maxRespTime,
                                   Ipv4Address groupAddress, Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this << packet << source << dest << static_cast<uint32_t> (type) << route);
  Igmpv2Header igmp;
  igmp.SetType(type);
  igmp.SetMaxRespTime(maxRespTime);
  if(Node::ChecksumEnabled())
    {
      igmp.EnableChecksum();
    }
  packet->AddHeader(igmp);

  m_downTarget(packet, source, dest, PROT_NUMBER, route);
}

void Igmpv2L4Protocol::HandleMembershipQuery(Ptr<Packet> p, Igmpv2Header header,
    Ipv4Address source, Ipv4Address destination)
{
  NS_LOG_FUNCTION(this << p << header << source << destination);

  /*
  Ptr<Packet> reply = Create<Packet> ();
  Igmpv2Echo echo;
  p->RemoveHeader (echo);
  reply->AddHeader (echo);
  SendMessage (reply, destination, source, Igmpv2Header::ECHO_REPLY, 0, 0);
  */
}

void Igmpv2L4Protocol::HandleMembershipReportv2(Ptr<Packet> p, Igmpv2Header igmp,
    Ipv4Address source, Ipv4Address destination)
{
  NS_LOG_FUNCTION(this << p << igmp << source << destination);

  /*
  Igmpv2DestinationUnreachable unreach;
  p->PeekHeader (unreach);
  uint8_t payload[8];
  unreach.GetData (payload);
  Ipv4Header ipHeader = unreach.GetHeader ();
  Forward (source, igmp, unreach.GetNextHopMtu (), ipHeader, payload);
  */
}

void Igmpv2L4Protocol::HandleLeaveGroup(Ptr<Packet> p, Igmpv2Header igmp,
                                        Ipv4Address source, Ipv4Address destination)
{
  NS_LOG_FUNCTION(this << p << igmp << source << destination);

  /*
  Igmpv2TimeExceeded time;
  p->PeekHeader (time);
  uint8_t payload[8];
  time.GetData (payload);
  Ipv4Header ipHeader = time.GetHeader ();
  // info field is zero for TimeExceeded on linux
  Forward (source, igmp, 0, ipHeader, payload);
  */
}

enum IpL4Protocol::RxStatus Igmpv2L4Protocol::Receive(Ptr<Packet> p,
    Ipv4Header const &header,
    Ptr<Ipv4Interface> incomingInterface)
{
  NS_LOG_FUNCTION(this << p << header << incomingInterface);

  Igmpv2Header igmp;
  p->RemoveHeader(igmp);
  switch (igmp.GetType())
    {
    case Igmpv2Header::MEMBERSHIP_QUERY:
      HandleMembershipQuery(p, igmp, header.GetSource(), header.GetDestination());
      break;

    case Igmpv2Header::MEMBERSHIP_REPORT_V2:
      HandleMembershipReportv2(p, igmp, header.GetSource(), header.GetDestination());
      break;

    case Igmpv2Header::LEAVE_GROUP:
      HandleLeaveGroup(p, igmp, header.GetSource(), header.GetDestination());
      break;

    default:
      NS_LOG_DEBUG (igmp << " " << *p);
      break;
    }

  return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus Igmpv2L4Protocol::Receive(Ptr<Packet> p,
    Ipv6Header const &header,
    Ptr<Ipv6Interface> incomingInterface)
{
  NS_LOG_FUNCTION(this << p << header.GetSourceAddress () << header.GetDestinationAddress () << incomingInterface);
  return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void Igmpv2L4Protocol::DoDispose(void)
{
  NS_LOG_FUNCTION(this);
  m_node = 0;
  m_downTarget.Nullify();
  IpL4Protocol::DoDispose();
}

void Igmpv2L4Protocol::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
  NS_LOG_FUNCTION(this << &callback);
  m_downTarget = callback;
}

void Igmpv2L4Protocol::SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback)
{
  NS_LOG_FUNCTION(this << &callback);
}

IpL4Protocol::DownTargetCallback Igmpv2L4Protocol::GetDownTarget(void) const
{
  NS_LOG_FUNCTION(this);
  return m_downTarget;
}

IpL4Protocol::DownTargetCallback6 Igmpv2L4Protocol::GetDownTarget6(void) const
{
  NS_LOG_FUNCTION(this);
  return (IpL4Protocol::DownTargetCallback6)NULL;
}

} // namespace ns3
