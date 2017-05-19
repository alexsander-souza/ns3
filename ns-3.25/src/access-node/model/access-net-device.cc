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

#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/igmpv2-l4-protocol.h"
#include "ns3/ipv4-raw-socket-factory.h"
#include "ns3/udp-socket.h"
#include "ns3/ancp-an-agent.h"
#include "ns3/access-net-device.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AccessNetDevice");

NS_OBJECT_ENSURE_REGISTERED(AccessNetDevice);


TypeId
AccessNetDevice::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AccessNetDevice")
                      .SetParent<BridgedNetDevice> ()
                      .AddConstructor<AccessNetDevice> ()
                      .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                                    UintegerValue(1500),
                                    MakeUintegerAccessor(&AccessNetDevice::m_mtu),
                                    MakeUintegerChecker<uint16_t> ())
                      .AddAttribute("ExpirationTime",
                                    "Time it takes for access port entry to expire.",
                                    TimeValue(Seconds(30)),
                                    MakeTimeAccessor(&AccessNetDevice::m_portMapTtl),
                                    MakeTimeChecker())
  ;

  return tid;
}

AccessNetDevice::AccessNetDevice() :
  m_portMapTtl(Seconds(30)),
  m_uplink(0),
  m_channel(0),
  m_node(0),
  m_address(Mac48Address("00:00:00:00:00:00")),
  m_ifIndex(0),
  m_mtu(1500),
  m_agent(0)
{
  NS_LOG_FUNCTION_NOARGS();
  m_channel = CreateObject<AccessChannel> ();
}

AccessNetDevice::~AccessNetDevice()
{
  NS_LOG_FUNCTION_NOARGS();
  /* All clean-up code SHOULD go to DoDispose() method */
}

void AccessNetDevice::DoDispose(void)
{
  NS_LOG_FUNCTION_NOARGS();
  m_uplink = 0;
  m_access_ports.clear();
  m_accessProfiles.clear();
  m_mcastProfiles.clear();
  m_portMap.clear();
  m_channel = 0;
  m_node = 0;
  m_agent = 0;
  Simulator::Cancel(m_mcastQueryTimer);
  BridgedNetDevice::DoDispose();
}

void AccessNetDevice::SetUplinkPort(Ptr< NetDevice > uplink)
{
  NS_LOG_FUNCTION(this << uplink);
  NS_ASSERT(uplink != this);

  if (!Mac48Address::IsMatchingType(uplink->GetAddress()) || !uplink->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Uplink port SHOULD be Ethernet-compatible");
    }

  NS_LOG_LOGIC("Uplink port " << uplink->GetAddress());

  m_address = Mac48Address::ConvertFrom(uplink->GetAddress());
  m_uplink = uplink;
  m_node->RegisterProtocolHandler(MakeCallback(&AccessNetDevice::ReceiveFromUplinkPort, this),
                                  0, uplink, true);
  m_channel->AddChannel(uplink->GetChannel());

  /* Start Multicast Snooping */
  HandleMCastQueryTimer();
}

void AccessNetDevice::AddAccessPort(Ptr< NetDevice > port)
{
  NS_LOG_FUNCTION(this << port);
  NS_ASSERT(port != this);

  if (!Mac48Address::IsMatchingType(port->GetAddress()) || !port->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Access Port SHOULD be Ethernet-compatible");
    }

  NS_LOG_LOGIC("New access port " << port->GetAddress());

  /* Shutdown port, we'll enable it latter */
  port->SetLineProtocolStatus(false);

  m_access_ports.push_back(port);
  m_node->RegisterProtocolHandler(MakeCallback(&AccessNetDevice::ReceiveFromAccessPort, this),
                                  0, port, true);
  m_channel->AddChannel(port->GetChannel());
}

uint32_t AccessNetDevice::GetNBridgePorts(void) const
{
  return m_access_ports.size();
}

Ptr<NetDevice> AccessNetDevice::GetBridgePort(uint32_t n) const
{
  return m_access_ports[n];
}

void AccessNetDevice::ReceiveFromUplinkPort(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                            uint16_t protocol, Address const &source,
                                            Address const &destination, PacketType packetType)
{
  NS_LOG_FUNCTION(this << packet << source << destination);

  Mac48Address src48 = Mac48Address::ConvertFrom(source);
  Mac48Address dst48 = Mac48Address::ConvertFrom(destination);

  if (src48 == m_address)
    return; /* Drop own packet */

  if (!m_promiscRxCallback.IsNull())
    {
      m_promiscRxCallback(this, packet, protocol, source, destination, packetType);
    }

  switch (packetType)
    {
    case PACKET_HOST:
      if (dst48 == m_address)
        {
          /* Local delivery */
          m_rxCallback(this, packet, protocol, source);
        }
      break;

    case PACKET_BROADCAST:
      /* Local delivery */
      m_rxCallback(this, packet, protocol, source);

      /* XXX Not sure how broadcast should work downstream */
      ForwardBroadcast(packet, protocol, src48, dst48);
      break;

    case PACKET_MULTICAST:
      /* Local delivery */
      m_rxCallback(this, packet, protocol, source);

      /* Check Port Multicast group membership */
      ForwardMulticast(packet, protocol, src48, dst48);
      break;

    case PACKET_OTHERHOST:
      if (dst48 == m_address)
        {
          /* Local delivery */
          m_rxCallback(this, packet, protocol, source);
        }
      else
        {
          /* Forward to an access port */
          ForwardUnicast(packet, protocol, src48, dst48);
        }
      break;
    }
}

void AccessNetDevice::ReceiveFromAccessPort(Ptr<NetDevice> device, Ptr<const Packet> packet,
                                            uint16_t protocol, Address const &source,
                                            Address const &destination, PacketType packetType)
{
  NS_LOG_FUNCTION(this << packet << source << destination << uint(protocol));

  Mac48Address src48 = Mac48Address::ConvertFrom(source);
  Mac48Address dst48 = Mac48Address::ConvertFrom(destination);

  if (!m_promiscRxCallback.IsNull())
    {
      m_promiscRxCallback(this, packet, protocol, source, destination, packetType);
    }

  if (!device->IsLinkUp())
    {
      NS_LOG_FUNCTION("Dropped, link is not ready");
      return;
    }

  LearnAccessPort(src48, device, packet);

  switch (packetType)
    {
    case PACKET_HOST:
      if (dst48 == m_address)
        {
          /* Local delivery */
          m_rxCallback(this, packet, protocol, source);
        }
      break;

    case PACKET_BROADCAST:
      /* Local delivery */
      m_rxCallback(this, packet, protocol, source);

      /* Forward to upstream port */
      ForwardUpstream(packet, protocol, src48, dst48);
      break;

    case PACKET_MULTICAST:
      /* Local delivery */
      m_rxCallback(this, packet, protocol, source);

      /* IGMP/MLD Snooping */
      DoMulticastSnooping(packet, protocol, src48);
      break;

    case PACKET_OTHERHOST:
      if (dst48 == m_address)
        {
          m_rxCallback(this, packet, protocol, source);
        }
      else
        {
          /* Forward to upstream port */
          ForwardUpstream(packet, protocol, src48, dst48);
        }
      break;
    }
}

void AccessNetDevice::ForwardUnicast(Ptr<const Packet> packet, uint16_t protocol,
                                     Mac48Address &src, Mac48Address &dst)
{
  NS_LOG_FUNCTION(this << packet);

  Ptr<NetDevice> outPort = SelectAccessPortForMac(dst, packet);

  if (outPort != NULL && outPort->IsLinkUp())
    {
      outPort->SendFrom(packet->Copy(), src, dst, protocol);
    }
  else
    {
      NS_LOG_LOGIC("Unable to forward packet, dropping");
    }
}

void AccessNetDevice::ForwardUpstream(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst)
{
  NS_LOG_FUNCTION(this << packet);

  /* TODO create DHCP option 82 hook */

  m_uplink->SendFrom(packet->Copy(), src, dst, protocol);
}

void AccessNetDevice::ForwardBroadcast(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst)
{
  NS_LOG_FUNCTION(this << packet);

  for (Ptr<NetDevice> &port :m_access_ports)
    {
      if (port->IsLinkUp())
        {
          port->SendFrom(packet->Copy(), src, dst, protocol);
        }
    }
}

void AccessNetDevice::ForwardMulticast(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst)
{
  NS_LOG_FUNCTION(this << packet);

  Address group;

  if (protocol == Ipv4L3Protocol::PROT_NUMBER)
    {
      Ipv4Header ipv4Header;
      packet->PeekHeader(ipv4Header);
      group = ipv4Header.GetDestination();
    }
  else if (protocol == Ipv6L3Protocol::PROT_NUMBER)
    {
      Ipv6Header ipv6Header;
      packet->PeekHeader(ipv6Header);
      group = ipv6Header.GetDestinationAddress();
    }
  else
    {
      NS_LOG_LOGIC("Don't known what do do");
      return;
    }

  /* FIXME Handle IEEE Slow Protocols (01:80:C2:xx:xx:xx) */
  /* FIXME flood 01:00:5E:00:00:01 to 01:00:5E:00:00:FF */
  /* FIXME Handle Link-local IPv4 (224.0.0.0/24) */
  /* FIXME Handle Link-local IPv6 (FF02::1) */

  /* Use membership to forward */
  for (auto &iter :m_portMap)
    {
      const AccessPort& access = iter.second;

      if (access.IsMember(group))
        {
          Ptr<NetDevice> port = access.GetAccessPort();
          if (port->IsLinkUp())
            {
              port->SendFrom(packet->Copy(), src, dst, protocol);
            }
        }
    }
}

bool AccessNetDevice::SendFrom(Ptr<Packet> packet, const Address& source,
                               const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION(this << source << dest);

  Mac48Address dst = Mac48Address::ConvertFrom(dest);

  // use the learned state if destination is unicast
  if (!dst.IsGroup())
    {
      Ptr<NetDevice> outPort = SelectAccessPortForMac(dst, packet);
      if (outPort != NULL)
        {
          outPort->SendFrom(packet, source, dest, protocolNumber);
          return true;
        }
    }

  // not unicast or no state has been setup for that mac,
  // send through uplink port
  m_uplink->SendFrom(packet, source, dest, protocolNumber);

  return true;
}

void AccessNetDevice::LearnAccessPort(Mac48Address &source, Ptr<NetDevice> port, Ptr<const Packet> pkt)
{
  NS_LOG_FUNCTION(this << source << port);

  AccessPort &entry = m_portMap[source];

  if (entry.GetAccessPort() == 0)
    {
      entry.SetAccessPort(this, port, source);
    }

  entry.SetTtl(Simulator::Now() + m_portMapTtl);
  entry.AccPacket(AccessPort::UPSTREAM, pkt);
}

Ptr<NetDevice> AccessNetDevice::SelectAccessPortForMac(Mac48Address &source, Ptr<const Packet> pkt)
{
  NS_LOG_FUNCTION(this << source);

  auto iter = m_portMap.find(source);

  if (iter != m_portMap.end())
    {
      Time now = Simulator::Now();
      AccessPort &entry = iter->second;

      if (entry.GetTtl() < now)
        {
          m_portMap.erase(iter);
        }
      else if (entry.GetShowTime())
        {
          entry.AccPacket(AccessPort::DOWNSTREAM, pkt);
          return entry.GetAccessPort();
        }
    }

  return nullptr;
}

bool AccessNetDevice::Send(Ptr<Packet> packet, const Address& dest,
                           uint16_t protocolNumber)
{
  NS_LOG_FUNCTION(this);
  return SendFrom(packet, m_address, dest, protocolNumber);
}

void AccessNetDevice::SetIfIndex(const uint32_t index)
{
  NS_LOG_FUNCTION(this << index);
  m_ifIndex = index;
}

uint32_t AccessNetDevice::GetIfIndex(void) const
{
  NS_LOG_FUNCTION(this);
  return m_ifIndex;
}

Ptr<Channel> AccessNetDevice::GetChannel(void) const
{
  NS_LOG_FUNCTION(this);
  return m_channel;
}

void AccessNetDevice::SetAddress(Address address)
{
  NS_LOG_FUNCTION(this << address);
  m_address = Mac48Address::ConvertFrom(address);
}

Address AccessNetDevice::GetAddress(void) const
{
  NS_LOG_FUNCTION(this << m_address);
  return m_address;
}

bool AccessNetDevice::SetMtu(const uint16_t mtu)
{
  NS_LOG_FUNCTION(this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t AccessNetDevice::GetMtu(void) const
{
  NS_LOG_FUNCTION(this);
  return m_mtu;
}

bool AccessNetDevice::IsLinkUp(void) const
{
  NS_LOG_FUNCTION(this);
  return true;
}

void AccessNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
  NS_LOG_FUNCTION(this);
}

bool AccessNetDevice::IsBroadcast(void) const
{
  NS_LOG_FUNCTION(this);
  return true;
}

Address AccessNetDevice::GetBroadcast(void) const
{
  NS_LOG_FUNCTION(this);
  return Mac48Address("FF:FF:FF:FF:FF:FF");
}

bool AccessNetDevice::IsMulticast(void) const
{
  NS_LOG_FUNCTION(this);
  return true;
}

Address AccessNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION(this);
  Mac48Address multicast = Mac48Address::GetMulticast(multicastGroup);
  return multicast;
}

Address AccessNetDevice::GetMulticast(Ipv6Address addr) const
{
  NS_LOG_FUNCTION(this);
  Mac48Address multicast = Mac48Address::GetMulticast(addr);
  return multicast;
}

bool AccessNetDevice::IsPointToPoint(void) const
{
  NS_LOG_FUNCTION(this);
  return false;
}

Ptr<Node> AccessNetDevice::GetNode(void) const
{
  NS_LOG_FUNCTION(this << m_node);
  return m_node;
}

void AccessNetDevice::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION(this << node);
  m_node = node;
}

bool AccessNetDevice::NeedsArp(void) const
{
  NS_LOG_FUNCTION(this);
  return true;
}

void AccessNetDevice::SetReceiveCallback(ReceiveCallback cb)
{
  NS_LOG_FUNCTION(this);
  m_rxCallback = cb;
}

void AccessNetDevice::SetPromiscReceiveCallback(PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION(this);
  m_promiscRxCallback = cb;
}

bool AccessNetDevice::SupportsSendFrom(void) const
{
  NS_LOG_FUNCTION(this);
  return true;
}

Ptr<AncpAnAgent> AccessNetDevice::GetAncpAgent()
{
  NS_LOG_FUNCTION(this);
  return m_agent;
}

void AccessNetDevice::SetAncpAgent(Ptr<AncpAnAgent> agent)
{
  NS_LOG_FUNCTION(this);
  m_agent = agent;

  /* Glue together */
  agent->SetAttribute("LineConfigCallback", CallbackValue(MakeCallback(&AccessNetDevice::HandleLineConfig, this)));
  agent->SetAttribute("MCastLineConfigCallback", CallbackValue(MakeCallback(&AccessNetDevice::HandleMCastConfig, this)));
  agent->SetAttribute("MCastProfileCallback", CallbackValue(MakeCallback(&AccessNetDevice::HandleMCastProfile, this)));
  agent->SetAttribute("MCastCommandCallback", CallbackValue(MakeCallback(&AccessNetDevice::HandleMCastCommand, this)));
}

void AccessNetDevice::StartIpServices()
{
  m_sock_igmp = Socket::CreateSocket(GetNode(), TypeId::LookupByName("ns3::Ipv4RawSocketFactory"));
  NS_ASSERT(m_sock_igmp != 0);
  m_sock_igmp->SetAttribute("Protocol", UintegerValue(Igmpv2L4Protocol::PROT_NUMBER));
  m_sock_igmp->Bind(InetSocketAddress(Ipv4Address::GetAny(), 0));
  m_sock_igmp->ShutdownRecv();
}

void AccessNetDevice::AddAccessProfile(Ptr<AccessProfile>profile)
{
  NS_LOG_INFO(this << *profile << " LEN=" << sizeof(profile)+profile->GetName().size());

  m_accessProfiles[profile->GetName()] = profile;
}

int AccessNetDevice::HandleLineConfig(const std::string &circuitId, const std::string &profileName)
{
  NS_LOG_FUNCTION(this << circuitId << profileName);

  auto profIt = m_accessProfiles.find(profileName);

  if (profIt == m_accessProfiles.end())
    return -1;

  Mac48Address circuit(circuitId.c_str());

  m_portMap[circuit].ApplyProfile(profIt->second);

  return 0;
}

int AccessNetDevice::HandleMCastConfig(const std::string &circuitId, const std::string &profileName)
{
  NS_LOG_FUNCTION(this << circuitId << profileName);
  auto profIt = m_mcastProfiles.find(profileName);

  if (profIt == m_mcastProfiles.end())
    return -1;

  Mac48Address circuit(circuitId.c_str());

  m_portMap[circuit].ApplyProfile(profIt->second);
  return 0;
}

int AccessNetDevice::HandleMCastCommand(const std::string&circuitId, int oper, const Address &group)
{
  Mac48Address circuit(circuitId.c_str());

  switch (oper)
    {
    case AncpTlvMCastCommand::CMD_ADD:
      m_portMap[circuit].NasInitiatedMulticastGroup(group, true);
      SendIgmpJoinUpstream(group, circuit);
      break;

    case AncpTlvMCastCommand::CMD_DELETE:
      m_portMap[circuit].NasInitiatedMulticastGroup(group, false);
      SendIgmpLeaveUpstream(group, circuit);
      break;

    default:
      NS_LOG_LOGIC("Unsupported operation");
      return -1;
    }

  return 0;
}

int AccessNetDevice::HandleMCastProfile(const std::string&prof_name, const std::list<Address>&whitelist,
                                        const std::list<Address>&greylist, const std::list<Address>&blacklist,
                                        bool doWhitelistCac, bool doMRepCtlCac)
{
  if (doWhitelistCac || doMRepCtlCac)
    NS_LOG_LOGIC("Multicast Conditional Access Control is not supported");

  auto profIt = m_mcastProfiles.emplace(prof_name, Create<MCastProfile>(prof_name));
  Ptr<MCastProfile> prof = profIt.first->second;

  if (profIt.second)
    {
      prof->FlushFlowList(ListClass::WHITELIST);
      prof->FlushFlowList(ListClass::GREYLIST);
      prof->FlushFlowList(ListClass::BLACKLIST);
    }

  for (auto &addr: whitelist)
    prof->UpdateFlowList(ListClass::WHITELIST, true, addr);

  for (auto &addr: greylist)
    prof->UpdateFlowList(ListClass::GREYLIST, true, addr);

  for (auto &addr: blacklist)
    prof->UpdateFlowList(ListClass::BLACKLIST, true, addr);


  //FIXME if any port is using this profile, it must be refreshed
  return 0;
}

void AccessNetDevice::HandleMCastQueryTimer(void)
{
  NS_LOG_FUNCTION(this);

  /* Prepare IGMP query message */
  Ipv4Address multicastGroup("224.0.0.1"); /* all-systems group */

  Igmpv2Header hIgmp;
  hIgmp.SetType(Igmpv2Header::MEMBERSHIP_QUERY);
  hIgmp.SetMaxRespTime(0x50);
  hIgmp.SetGroupAddress(Ipv4Address::GetAny()); /* General Query */

  Ipv4Header hIp;
  hIp.SetSource(Ipv4Address::GetAny());
  hIp.SetDestination(multicastGroup);
  hIp.SetProtocol(Igmpv2L4Protocol::PROT_NUMBER);
  hIp.SetTtl(1);
  hIp.SetPayloadSize(hIgmp.GetSerializedSize());

  Ptr<Packet> pktIp4 = Create<Packet>();
  pktIp4->AddHeader(hIgmp);
  pktIp4->AddHeader(hIp);
  Mac48Address dst = Mac48Address::GetMulticast(multicastGroup);

  /* FIXME Prepare MLD query message */

  /* Purge stale entries and query existing groups */
  for (auto &iter : m_portMap)
    {
      AccessPort& access = iter.second;
      access.CleanupMulticastMembership();
      Ptr<NetDevice> port = access.GetAccessPort();
      if (port && port->IsLinkUp())
        {
          port->SendFrom(pktIp4->Copy(), m_address, dst, Ipv4L3Protocol::PROT_NUMBER);
        }
    }

  m_mcastQueryTimer = Simulator::Schedule(Seconds(AccessPort::QUERY_INTERVAL),
                                          &AccessNetDevice::HandleMCastQueryTimer, this);
}

void AccessNetDevice::SendIgmpJoinUpstream(const Address& group, const Mac48Address &src)
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

  if (m_sock_igmp->SendTo(packet, 0, InetSocketAddress(multicastGroup, 0)) < 0)
    NS_LOG_WARN(this << " Failed to send IGMP REPORT for " << multicastGroup);
}

void AccessNetDevice::SendIgmpLeaveUpstream(const Address& group, const Mac48Address &src)
{
  NS_LOG_FUNCTION(this << group);
  if (m_mcastActiveGroups[group].empty())
    return;

  m_mcastActiveGroups[group].erase(src);

  if (m_mcastActiveGroups[group].empty())
    SendIgmpLeaveUpstream(group);
}

void AccessNetDevice::SendIgmpLeaveUpstream(const Address& group)
{
  NS_LOG_FUNCTION(this << group);

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

  if (m_sock_igmp->SendTo(packet, 0, InetSocketAddress(multicastGroup, 0)) < 0)
    NS_LOG_WARN(this << " Failed to send IGMP Leave for " << multicastGroup);

  m_mcastActiveGroups.erase(group);
}

void AccessNetDevice::DoMulticastSnooping(Ptr<const Packet> packet, uint16_t protocol,
                                          Mac48Address &src)
{
  NS_LOG_FUNCTION(this);

  if (protocol == Ipv4L3Protocol::PROT_NUMBER)
    {
      Ipv4Header hIp4;
      packet->PeekHeader(hIp4);

      if (hIp4.GetProtocol() != Igmpv2L4Protocol::PROT_NUMBER)
        return;

      /* Is IGMP, copy packet and analise header */
      Ptr<Packet> pktMcast = packet->Copy();
      Igmpv2Header hIgmp;

      pktMcast->RemoveHeader(hIp4);
      pktMcast->RemoveHeader(hIgmp);

      AccessPort &entry = m_portMap[src];

      if (hIgmp.GetType() == Igmpv2Header::MEMBERSHIP_REPORT_V2)
        {
          if (entry.EnterMulticastGroup(hIgmp.GetGroupAddress()))
            SendIgmpJoinUpstream(hIgmp.GetGroupAddress(), src);
        }
      else if (hIgmp.GetType() == Igmpv2Header::LEAVE_GROUP)
        {
          if (entry.LeaveMulticastGroup(hIgmp.GetGroupAddress()))
            SendIgmpLeaveUpstream(hIgmp.GetGroupAddress(), src);
        }
    }
  else if (protocol == Ipv6L3Protocol::PROT_NUMBER)
    {
      NS_LOG_WARN("MLD snooping not implemented");
    }
}
} // ns3
