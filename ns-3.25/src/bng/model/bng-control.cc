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

#include <string>
#include <sstream>
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/igmpv2-l4-protocol.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-routing-table-entry.h"
#include "ns3/ancp-nas-agent.h"
#include "ns3/dhcp-server.h"
#include "ns3/radius-client.h"
#include "ns3/bng-control.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("BngControl");

NS_OBJECT_ENSURE_REGISTERED(BngControl);

TypeId
BngControl::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::BngControl")
                      .SetParent<Application>()
                      .AddConstructor<BngControl>()
                      .AddAttribute("SessionIdleTimeout",
                                    "Idle session timeout",
                                    TimeValue(Seconds(15)),
                                    MakeTimeAccessor(&BngControl::m_sessionTimeout),
                                    MakeTimeChecker())

  ;

  return tid;
}

BngControl::BngControl() :
  m_regionalNetPort(0),
  m_accessNetPort(0),
  m_address(Mac48Address("00:00:00:00:00:00")),
  m_agent(0),
  m_radClient(0),
  m_dhcp(0)
{
  NS_LOG_FUNCTION_NOARGS();
}

BngControl::~BngControl()
{
  NS_LOG_FUNCTION_NOARGS();

  /* All clean-up code SHOULD go to DoDispose() method */
}

void BngControl::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  m_agent = 0;
  m_radClient = 0;
  m_dhcp = 0;
  m_regionalNetPort = 0;
  m_accessNetPort = 0;
  m_sessionMap.clear();
  Application::DoDispose();
}

void BngControl::SetRegionalNetPort(Ptr< NetDevice > regionalNetPort)
{
  NS_LOG_FUNCTION(this << regionalNetPort);

  if (!Mac48Address::IsMatchingType(regionalNetPort->GetAddress()) || !regionalNetPort->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Regional port SHOULD be Ethernet-compatible");
    }

  NS_LOG_LOGIC("Regional port " << regionalNetPort->GetAddress());

  m_regionalNetPort = regionalNetPort;
  m_address = Mac48Address::ConvertFrom(m_regionalNetPort->GetAddress());
}

void BngControl::SetAccessNetPort(Ptr< NetDevice > accessNetPort)
{
  NS_LOG_FUNCTION(this << accessNetPort);

  if (!Mac48Address::IsMatchingType(accessNetPort->GetAddress()) || !accessNetPort->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Access Network port SHOULD be Ethernet-compatible");
    }

  NS_LOG_LOGIC("Access Network port " << accessNetPort->GetAddress());

  m_accessNetPort = accessNetPort;
  GetNode()->RegisterProtocolHandler(MakeCallback(&BngControl::ActivityMonitor, this),
                                     0, m_accessNetPort, true);
}

void BngControl::ActivityMonitor(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                 uint16_t protocol, Address const &source,
                                 Address const &destination, NetDevice::PacketType packetType)
{
  NS_LOG_FUNCTION(this << source << destination);

  Mac48Address cpeHwId = Mac48Address::ConvertFrom(source);

  if (cpeHwId == m_address)
    return; /* Ignore own packet */

  auto sessionIT = m_sessionMap.find(cpeHwId);

  if (sessionIT == m_sessionMap.end())
    return;

  SubscriberSession &session = sessionIT->second;
  session.AccPacket(SubscriberSession::UPSTREAM, packet);
}

void BngControl::AddBandwidthProfile(const Mac48Address &cpeHwId, const std::string &profName)
{
  NS_LOG_FUNCTION(this << cpeHwId << profName);

  m_accessProfileMap[cpeHwId] = profName;
}

void BngControl::AddMcastProfile(const Mac48Address &cpeHwId, const std::string &profName)
{
  NS_LOG_FUNCTION(this << cpeHwId << profName);

  m_mcastProfileMap[cpeHwId] = profName;
}

void BngControl::SetupMcastProfile(const struct McastProfile &profile)
{
  NS_LOG_FUNCTION(this << profile.name << " len=" << uint(sizeof(profile))) ;

  m_mcastProfiles[profile.name] = profile;
}

Ptr<AncpNasAgent>BngControl::GetAncpAgent()
{
  NS_LOG_FUNCTION(this);
  return m_agent;
}

void BngControl::SetAncpAgent(Ptr<AncpNasAgent>agent)
{
  NS_LOG_FUNCTION(this);
  m_agent = agent;

  m_agent->SetAttribute("PortUpCallback", CallbackValue(MakeCallback(&BngControl::SubscriberPortUp, this)));
  m_agent->SetAttribute("PortDownCallback", CallbackValue(MakeCallback(&BngControl::SubscriberPortDown, this)));
  m_agent->SetAttribute("MCastCacCallback", CallbackValue(MakeCallback(&BngControl::SubscriberMRepCtlHandler, this)));
  m_agent->SetAttribute("NewAdjCallback", CallbackValue(MakeCallback(&BngControl::DoAnProvisioning, this)));
}

int BngControl::DoAnProvisioning(const Mac48Address &anId)
{
  NS_LOG_FUNCTION(this << anId);
  NS_LOG_INFO("BNG new AN: " << anId);

  for (auto &pair: m_mcastProfiles)
    {
      const struct McastProfile &prof = pair.second;
      m_agent->SendMCastServiceProfile(anId, pair.first, prof.whitelist,
                                       prof.greylist, prof.blacklist,
                                       false, false);
    }

  return 0;
}

int BngControl::SubscriberPortUp(const Mac48Address &anId, const std::string &circuit_id,
                                 uint32_t rate_up, uint32_t rate_down, uint32_t tag)
{
  NS_LOG_FUNCTION(this << anId << circuit_id);
  NS_LOG_INFO("BNG port up: " << anId << "--" << circuit_id);

  Mac48Address cpeHwId = Mac48Address(circuit_id.c_str());

  auto sessionP = m_sessionMap.emplace(std::make_pair(cpeHwId, SubscriberSession(anId, circuit_id, m_radClient)));
  SubscriberSession &session = sessionP.first->second;

  session.UpdateLineStatus(true, rate_up, rate_down);

  auto prof = m_accessProfileMap.find(cpeHwId);
  if (prof != m_accessProfileMap.end())
    m_agent->SendPortConfigCommand(anId, circuit_id, prof->second);

  auto profMcast = m_mcastProfileMap.find(cpeHwId);
  if (profMcast != m_mcastProfileMap.end())
    m_agent->SendMCastPortConfigCommand(anId, circuit_id, profMcast->second);

  return 0;
}

int BngControl::SubscriberPortDown(const Mac48Address &anId, const std::string &circuit_id)
{
  NS_LOG_FUNCTION(this << anId << circuit_id);
  NS_LOG_INFO("BNG port down: " << anId << "--" << circuit_id);

  Mac48Address cpeHwId = Mac48Address(circuit_id.c_str());

  auto sessionIT = m_sessionMap.find(cpeHwId);

  if (sessionIT == m_sessionMap.end())
    return -1;

  SubscriberSession &session = sessionIT->second;
  session.UpdateLineStatus(false, 0, 0);

  return 0;
}

int BngControl::SubscriberMRepCtlHandler(const Mac48Address &anId, const std::string &circuit_id,
                                         const Address& group, bool join)
{
  NS_LOG_FUNCTION(this << anId << circuit_id);

  if (group == Ipv4Address::GetAny())
    return 0;

  NS_LOG_DEBUG("BNG MCast request from " << circuit_id << " to " << group << (join?" JOIN":" LEAVE"));

  // Mac48Address cpeHwId = Mac48Address(circuit_id.c_str());

  if (join)
    {
      NS_LOG_LOGIC("Joining group " << group);
      //TODO Ask to Policy Controller

      m_agent->SendMCastCommand(anId, circuit_id, AncpTlvMCastCommand::CMD_ADD, group);
    }
  else
    {
      NS_LOG_LOGIC("Leaving group " << group);
      m_agent->SendMCastCommand(anId, circuit_id, AncpTlvMCastCommand::CMD_DELETE, group);
    }

  return 0;
}

int BngControl::SubscriberIpSessionSetup(const Mac48Address &cpeHwId, const Address &ip)
{
  NS_LOG_FUNCTION(this << cpeHwId << ip);
  auto sessionIT = m_sessionMap.find(cpeHwId);

  if (sessionIT == m_sessionMap.end())
    return -1;

  SubscriberSession &session = sessionIT->second;

  NS_LOG_INFO("BNG IP session setup: " << cpeHwId << " IP " << ip << " LEN=" << sizeof(session));

  session.SetAddressIp(ip);

  if (Ipv4Address::IsMatchingType(ip))
    {
      Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();
      int32_t ifIndex = ipv4->GetInterfaceForDevice(m_accessNetPort);

      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting(ipv4);
      Ipv4Address ip4 = Ipv4Address::ConvertFrom(ip);
      staticRouting->AddHostRouteTo(ip4, ifIndex);

      Ipv4GlobalRoutingHelper::RecomputeRoutingTables();

      /* TODO create static ARP entry */
    }
  else if (Ipv6Address::IsMatchingType(ip))
    {
      Ptr<Ipv6> ipv6 = GetNode()->GetObject<Ipv6> ();
      int32_t ifIndex = ipv6->GetInterfaceForDevice(m_accessNetPort);

      Ipv6StaticRoutingHelper ipv6RoutingHelper;
      Ptr<Ipv6StaticRouting> staticRouting = ipv6RoutingHelper.GetStaticRouting(ipv6);
      Ipv6Address ip6 = Ipv6Address::ConvertFrom(ip);
      staticRouting->AddHostRouteTo(ip6, ifIndex);

      // FIXME How IPv6 routing works on NS3?
      // Ipv6GlobalRoutingHelper::RecomputeRoutingTables();

      /* TODO create static ND entry */
    }

  return 0;
}

int BngControl::SubscriberIpSessionTearDown(const Mac48Address &cpeHwId, const Address &ip)
{
  NS_LOG_FUNCTION(this << cpeHwId << " IP " << ip);
  auto sessionIT = m_sessionMap.find(cpeHwId);

  if (sessionIT == m_sessionMap.end())
    return -1;

  NS_LOG_INFO("BNG IP session teardown: " << cpeHwId << " IP " << ip);

  SubscriberSession &session = sessionIT->second;

  if (Ipv4Address::IsMatchingType(ip))
    {
      Ipv4Address oldIp = session.GetIpv4Address();
      session.SetAddressIp(Ipv4Address::GetZero());

      Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();

      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting(ipv4);
      uint32_t routeCnt = staticRouting->GetNRoutes();

      for (uint32_t i = 0; i < routeCnt; ++i)
        {
          const Ipv4RoutingTableEntry &entry = staticRouting->GetRoute(i);
          if (entry.GetDest() == oldIp)
            {
              staticRouting->RemoveRoute(i);
              break;
            }
        }
      Ipv4GlobalRoutingHelper::RecomputeRoutingTables();

      /* TODO remove ARP entry */
    }
  else if (Ipv6Address::IsMatchingType(ip))
    {
      Ipv6Address oldIp = session.GetIpv6Address();
      session.SetAddressIp(Ipv6Address::GetZero());

      Ptr<Ipv6> ipv6 = GetNode()->GetObject<Ipv6> ();

      Ipv6StaticRoutingHelper ipv6RoutingHelper;
      Ptr<Ipv6StaticRouting> staticRouting = ipv6RoutingHelper.GetStaticRouting(ipv6);
      uint32_t routeCnt = staticRouting->GetNRoutes();

      for (uint32_t i = 0; i < routeCnt; ++i)
        {
          const Ipv6RoutingTableEntry &entry = staticRouting->GetRoute(i);
          if (entry.GetDest() == oldIp)
            {
              staticRouting->RemoveRoute(i);
              break;
            }
        }
      // Ipv6GlobalRoutingHelper::RecomputeRoutingTables();

      /* TODO remove ND entry */
    }

  return 0;
}

void BngControl::SetRadiusClient(Ptr<RadiusClient>radClient)
{
  NS_LOG_FUNCTION(this);
  m_radClient = radClient;
}

void BngControl::SetDhcpServer(Ptr<DhcpServer>dhcpSrv)
{
  NS_LOG_FUNCTION(this);
  m_dhcp = dhcpSrv;

  m_dhcp->SetLeaseCallback(MakeCallback(&BngControl::SubscriberIpSessionSetup,
                                        this));
  m_dhcp->SetReleaseCallback(MakeCallback(&BngControl::SubscriberIpSessionTearDown,
                                          this));
}

void BngControl::StartApplication(void)
{
  NS_LOG_FUNCTION(this);

  m_sessionSweepEvent = Simulator::Schedule(m_sessionTimeout / 2, &BngControl::IdleSessionSweep, this);

  /* Create IGMP socket (Downstream) */
  m_sock_igmp_down = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  NS_ASSERT(m_sock_igmp_down != 0);
  NS_LOG_WARN("BNG Downstream " << m_sock_igmp_down);
  m_sock_igmp_down->SetAttribute("Protocol", UintegerValue(Igmpv2L4Protocol::PROT_NUMBER));
  m_sock_igmp_down->SetRecvCallback(MakeCallback(&BngControl::HandleReadIgmp, this));
  m_sock_igmp_down->Bind(InetSocketAddress(Ipv4Address::GetAny(), 0));
  m_sock_igmp_down->Listen();

  /* Create IGMP socket (upstream) */
  m_sock_igmp_up = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  NS_ASSERT(m_sock_igmp_up != 0);
  NS_LOG_WARN("BNG upstream " << m_sock_igmp_up);
  m_sock_igmp_up->SetAttribute("Protocol", UintegerValue(Igmpv2L4Protocol::PROT_NUMBER));
  m_sock_igmp_up->Bind(InetSocketAddress(discoverLocalAddress(m_regionalNetPort), 0));
  m_sock_igmp_up->ShutdownRecv();
}

void BngControl::StopApplication(void)
{
  NS_LOG_FUNCTION(this);
}

Ipv4Address BngControl::discoverLocalAddress(Ptr<NetDevice> device)
{
  NS_LOG_FUNCTION(device);

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4> ();

  unsigned int i = ipv4->GetInterfaceForDevice(device);

  for (unsigned int j = 0; j < ipv4->GetNAddresses(i); ++j)
    {
      const Ipv4InterfaceAddress &iaddr = ipv4->GetAddress(i, j);
      if (iaddr.GetLocal() != Ipv4Address::GetLoopback())
        {
          return iaddr.GetLocal();
        }
    }

  return Ipv4Address::GetAny();
}

void BngControl::IdleSessionSweep()
{
  NS_LOG_FUNCTION(this);
  for (auto &sessionE: m_sessionMap)
    {
      SubscriberSession &ss = sessionE.second;

      if (ss.IsActive() && ss.GetIdletime() > m_sessionTimeout)
        {
          NS_LOG_INFO(this << " Session Timeout: " << ss.GetSessionId() << " Idle time " << ss.GetIdletime().GetSeconds());
          if (ss.GetIpv4Address() != Ipv4Address::GetAny())
            SubscriberIpSessionTearDown(sessionE.first, ss.GetIpv4Address());
          if (ss.GetIpv6Address() != Ipv6Address::GetAny())
            SubscriberIpSessionTearDown(sessionE.first, ss.GetIpv6Address());

          SubscriberPortDown(ss.GetAnAddress(), ss.GetCircuitId());
        }
    }

  m_sessionSweepEvent = Simulator::Schedule(m_sessionTimeout / 2, &BngControl::IdleSessionSweep, this);
}

void BngControl::HandleReadIgmp(Ptr<Socket> socket)
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

      Address mcastGrp = hIgmp.GetGroupAddress();

      if (hIgmp.GetType() == Igmpv2Header::MEMBERSHIP_REPORT_V2)
        {
          SendIgmpJoinUpstream(mcastGrp, hIp4.GetSource());
        }
      else if (hIgmp.GetType() == Igmpv2Header::LEAVE_GROUP)
        {
          SendIgmpLeaveUpstream(mcastGrp, hIp4.GetSource());
        }
    }
}

void BngControl::SendIgmpJoinUpstream(const Address& group, const Address &src)
{
  NS_LOG_FUNCTION(this << group);

  auto pair = m_mcastActiveGroups[group].insert(src);

  if (pair.second == false)
    {
      NS_LOG_LOGIC(this << group << " already member");
      return;
    }

  const Ipv4Address &multicastGroup = Ipv4Address::ConvertFrom(group);

  Igmpv2Header hIgmp;
  hIgmp.SetType(Igmpv2Header::MEMBERSHIP_REPORT_V2);
  hIgmp.SetMaxRespTime(0x50);
  hIgmp.SetGroupAddress(multicastGroup);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(hIgmp);

  SocketIpTtlTag ttlTag;
  ttlTag.SetTtl(1);
  packet->AddPacketTag(ttlTag);

  if (m_sock_igmp_up->SendTo(packet, 0, InetSocketAddress(multicastGroup, 0)) < 0)
    NS_LOG_WARN(this << " Failed to send IGMP REPORT for " << multicastGroup);
}

void BngControl::SendIgmpLeaveUpstream(const Address& group, const Address &src)
{
  NS_LOG_FUNCTION(this << group);
  m_mcastActiveGroups[group].erase(src);

  if (!m_mcastActiveGroups[group].empty())
    {
      // NS_LOG_LOGIC(this << group << " already member");
      return;
    }

  const Ipv4Address &multicastGroup = Ipv4Address::ConvertFrom(group);

  Igmpv2Header hIgmp;
  hIgmp.SetType(Igmpv2Header::LEAVE_GROUP);
  hIgmp.SetMaxRespTime(0x50);
  hIgmp.SetGroupAddress(multicastGroup);

  Ptr<Packet> packet = Create<Packet>();
  packet->AddHeader(hIgmp);

  SocketIpTtlTag ttlTag;
  ttlTag.SetTtl(1);
  packet->AddPacketTag(ttlTag);

  if (m_sock_igmp_up->SendTo(packet, 0, InetSocketAddress(multicastGroup, 0)) < 0)
    NS_LOG_WARN(this << " Failed to send IGMP Leave for " << multicastGroup);

  m_mcastActiveGroups.erase(group);
}
} // ns3
