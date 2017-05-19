/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "delayed-bridge-net-device.h"
#include "delayed-bridge-policy.h"
#include "ns3/node.h"
#include "ns3/channel.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"
#include "ns3/simulator.h"

#include <ns3/random-variable-stream.h>
#include <ns3/double.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DelayedBridgeNetDevice");

NS_OBJECT_ENSURE_REGISTERED(DelayedBridgeNetDevice);

TypeId DelayedBridgeNetDevice::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DelayedBridgeNetDevice")
                      .SetParent<BridgedNetDevice>()
                      .SetGroupName("Bridge")
                      .AddConstructor<DelayedBridgeNetDevice>()
                      .AddAttribute("Mtu", "The MAC-level Maximum Transmission Unit",
                                    UintegerValue(1500),
                                    MakeUintegerAccessor(&DelayedBridgeNetDevice::SetMtu,
                                        &DelayedBridgeNetDevice::GetMtu),
                                    MakeUintegerChecker<uint16_t>());
  return tid;
}


DelayedBridgeNetDevice::DelayedBridgeNetDevice()
  : m_node(0),
    m_ifIndex(0)
{
  NS_LOG_FUNCTION_NOARGS();

  m_north_port = NULL;
  m_south_port = NULL;
  m_hasNorthPort = false;
  m_hasSouthPort = false;

  m_channel = CreateObject<BridgeChannel>();
  m_delayedBridge = CreateObject<ParetoDelayedBridge>();
  m_delayedBridge->SetBridgeNetDevice(this);
}

DelayedBridgeNetDevice::~DelayedBridgeNetDevice()
{
  NS_LOG_FUNCTION_NOARGS();
}

void DelayedBridgeNetDevice::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS();

  m_north_port = NULL;
  m_south_port = NULL;
  m_hasNorthPort = false;
  m_hasSouthPort = false;

  m_channel = 0;
  m_node = 0;

  BridgedNetDevice::DoDispose();
}

void DelayedBridgeNetDevice::ReceiveFromDevice(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
    uint16_t protocol, Address const &src, Address const &dst,
    PacketType packetType)
{
  NS_LOG_FUNCTION_NOARGS();
  NS_LOG_DEBUG("UID is " << packet->GetUid());

  Mac48Address src48 = Mac48Address::ConvertFrom(src);
  Mac48Address dst48 = Mac48Address::ConvertFrom(dst);

  if(!m_promiscRxCallback.IsNull())
    {
      m_promiscRxCallback(this, packet, protocol, src, dst, packetType);
    }

  switch(packetType)
    {
    case PACKET_HOST:
      if(dst48 == m_address)
        {
          m_rxCallback(this, packet, protocol, src);
        }
      break;

    case PACKET_BROADCAST:
    case PACKET_MULTICAST:
    case PACKET_OTHERHOST:
      if(dst48 == m_address)
        {
          m_rxCallback(this, packet, protocol, src);
        }
      else
        {
          Time incomingTime = Simulator::Now();
          PacketTime pt(incomingPort, packet, protocol, src48, dst48, incomingTime);
          m_delayedBridge->ReceivePacket(pt);
        }
      break;
    }
}

void DelayedBridgeNetDevice::ForwardUnicast(void)
{
  NS_LOG_FUNCTION_NOARGS();

  PacketTime nextPacket = m_delayedBridge->NextPacket();

  Ptr<NetDevice> incomingPort = nextPacket.GetIncomingPort();
  Ptr<const Packet> packet = nextPacket.GetPacket();
  Mac48Address src = nextPacket.GetSrc();
  Mac48Address dst = nextPacket.GetDst();
  uint16_t protocol = nextPacket.GetProtocol();

  Ptr<NetDevice> outPort = GetOutport(incomingPort);

  outPort->SendFrom(packet->Copy(), src, dst, protocol);
}

void DelayedBridgeNetDevice::Forward(void)
{
  ForwardUnicast();
}

Ptr<NetDevice> DelayedBridgeNetDevice::GetOutport(Ptr<NetDevice> incomingPort)
{
  NS_LOG_FUNCTION_NOARGS();
  if(incomingPort == m_north_port)
    {
      return m_south_port;
    }
  else
    {
      return m_north_port;
    }
}

uint32_t DelayedBridgeNetDevice::GetNBridgePorts(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return 2;
}

Ptr<NetDevice> DelayedBridgeNetDevice::GetNorthPort() const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_north_port;
}

Ptr<NetDevice> DelayedBridgeNetDevice::GetSouthPort() const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_south_port;
}

Ptr<NetDevice> DelayedBridgeNetDevice::GetBridgePort(uint32_t n) const
{
  if(n == 1)
    {
      Ptr<NetDevice> bridgePort;
      bridgePort = GetNorthPort();
      return bridgePort;
    }
  else if(n == 2)
    {
      Ptr<NetDevice> bridgePort;
      bridgePort = GetSouthPort();
      return bridgePort;
    }
  else
    {
      NS_FATAL_ERROR("Attempting to get beyond the second port.");
    }
}

void DelayedBridgeNetDevice::AddNorthPort(Ptr<NetDevice> bridgePort)
{
  NS_LOG_FUNCTION_NOARGS();
  NS_ASSERT(bridgePort != this);
  if(!Mac48Address::IsMatchingType(bridgePort->GetAddress()))
    {
      NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to bridge.");
    }
  if(!bridgePort->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to bridge.");
    }
  if(m_address == Mac48Address())
    {
      m_address = Mac48Address::ConvertFrom(bridgePort->GetAddress());
    }

  NS_LOG_DEBUG("RegisterProtocolHandler for " << bridgePort->GetInstanceTypeId().GetName());
  m_node->RegisterProtocolHandler(MakeCallback(&DelayedBridgeNetDevice::ReceiveFromDevice, this),
                                  0, bridgePort, true);
  m_north_port = bridgePort;
  m_channel->AddChannel(bridgePort->GetChannel());
}

void DelayedBridgeNetDevice::AddSouthPort(Ptr<NetDevice> bridgePort)
{
  NS_LOG_FUNCTION_NOARGS();
  NS_ASSERT(bridgePort != this);
  if(!Mac48Address::IsMatchingType(bridgePort->GetAddress()))
    {
      NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to bridge.");
    }
  if(!bridgePort->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to bridge.");
    }
  if(m_address == Mac48Address())
    {
      m_address = Mac48Address::ConvertFrom(bridgePort->GetAddress());
    }

  NS_LOG_DEBUG("RegisterProtocolHandler for " << bridgePort->GetInstanceTypeId().GetName());
  m_node->RegisterProtocolHandler(MakeCallback(&DelayedBridgeNetDevice::ReceiveFromDevice, this),
                                  0, bridgePort, true);
  m_south_port = bridgePort;
  m_channel->AddChannel(bridgePort->GetChannel());
}

void DelayedBridgeNetDevice::AddBridgePort(Ptr<NetDevice> bridgePort)
{
  NS_LOG_FUNCTION_NOARGS();
  NS_ASSERT(bridgePort != this);
  if(!Mac48Address::IsMatchingType(bridgePort->GetAddress()))
    {
      NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to bridge.");
    }
  if(!bridgePort->SupportsSendFrom())
    {
      NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to bridge.");
    }
  if(m_address == Mac48Address())
    {
      m_address = Mac48Address::ConvertFrom(bridgePort->GetAddress());
    }

  if(!m_hasNorthPort)
    {
      AddNorthPort(bridgePort);
      m_hasNorthPort = true;
    }
  else if(!m_hasSouthPort)
    {
      AddSouthPort(bridgePort);
      m_hasSouthPort = true;
    }
  else
    {
      NS_FATAL_ERROR("Attempting to add more than two ports.");
    }
}

void DelayedBridgeNetDevice::SetIfIndex(const uint32_t index)
{
  NS_LOG_FUNCTION_NOARGS();
  m_ifIndex = index;
}

uint32_t DelayedBridgeNetDevice::GetIfIndex(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_ifIndex;
}

Ptr<Channel> DelayedBridgeNetDevice::GetChannel(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_channel;
}

void DelayedBridgeNetDevice::SetAddress(Address address)
{
  NS_LOG_FUNCTION_NOARGS();
  m_address = Mac48Address::ConvertFrom(address);
}

Address DelayedBridgeNetDevice::GetAddress(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_address;
}

bool DelayedBridgeNetDevice::SetMtu(const uint16_t mtu)
{
  NS_LOG_FUNCTION_NOARGS();
  m_mtu = mtu;
  return true;
}

uint16_t DelayedBridgeNetDevice::GetMtu(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_mtu;
}

bool DelayedBridgeNetDevice::IsLinkUp(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

void DelayedBridgeNetDevice::AddLinkChangeCallback(Callback<void> callback)
{}

bool DelayedBridgeNetDevice::IsBroadcast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address DelayedBridgeNetDevice::GetBroadcast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool DelayedBridgeNetDevice::IsMulticast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address DelayedBridgeNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION(this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast(multicastGroup);
  return multicast;
}

bool DelayedBridgeNetDevice::IsPointToPoint(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return false;
}

bool DelayedBridgeNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS();
  return SendFrom(packet, m_address, dest, protocolNumber);
}

bool DelayedBridgeNetDevice::SendFrom(Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS();

  m_north_port->SendFrom(packet, src, dest, protocolNumber);
  m_south_port->SendFrom(packet, src, dest, protocolNumber);

  return true;
}

Ptr<Node> DelayedBridgeNetDevice::GetNode(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

void DelayedBridgeNetDevice::SetNode(Ptr<Node> node)
{
  NS_LOG_FUNCTION_NOARGS();
  m_node = node;
}

bool DelayedBridgeNetDevice::NeedsArp(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

void DelayedBridgeNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS();
  m_rxCallback = cb;
}

void DelayedBridgeNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS();
  m_promiscRxCallback = cb;
}

bool DelayedBridgeNetDevice::SupportsSendFrom() const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address DelayedBridgeNetDevice::GetMulticast(Ipv6Address addr) const
{
  NS_LOG_FUNCTION(this << addr);
  return Mac48Address::GetMulticast(addr);
}

} // namespace ns3
