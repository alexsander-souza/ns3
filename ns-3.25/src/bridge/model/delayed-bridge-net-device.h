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

#ifndef DELAYED_BRIDGE_NET_DEVICE_H
#define DELAYED_BRIDGE_NET_DEVICE_H

#include "ns3/net-device.h"
#include "ns3/bridged-net-device.h"
#include "ns3/packet-time.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/bridge-channel.h"
#include <stdint.h>
#include <string>
#include <map>
#include <set>
#include <ns3/nstime.h>
#include <ns3/delayed-bridge-policy.h>
#include <ns3/normal-delayed-bridge.h>
#include <ns3/exponential-delayed-bridge.h>
#include <ns3/pareto-delayed-bridge.h>
#include <iostream>

namespace ns3
{

class Node;

class DelayedBridgeNetDevice : public BridgedNetDevice
{
public:
  static TypeId GetTypeId(void);
  DelayedBridgeNetDevice();
  virtual ~DelayedBridgeNetDevice();

  void AddNorthPort(Ptr<NetDevice> bridgePort);
  void AddSouthPort(Ptr<NetDevice> bridgePort);
  void AddBridgePort(Ptr<NetDevice> bridgePort);

  Ptr<NetDevice> GetNorthPort() const;
  Ptr<NetDevice> GetSouthPort() const;

  void Forward(void);

  // inherited from BridgedNetDevice base class
  virtual uint32_t GetNBridgePorts(void) const; //Returns constant 2
  virtual Ptr<NetDevice> GetBridgePort(uint32_t n) const;

  // inherited from NetDevice base class.
  virtual void SetIfIndex(const uint32_t index);
  virtual uint32_t GetIfIndex(void) const;
  virtual Ptr<Channel> GetChannel(void) const;
  virtual void SetAddress(Address address);
  virtual Address GetAddress(void) const;
  virtual bool SetMtu(const uint16_t mtu);
  virtual uint16_t GetMtu(void) const;
  virtual bool IsLinkUp(void) const;
  virtual void AddLinkChangeCallback(Callback<void> callback);
  virtual bool IsBroadcast(void) const;
  virtual Address GetBroadcast(void) const;
  virtual bool IsMulticast(void) const;
  virtual Address GetMulticast(Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint(void) const;
  virtual bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom(Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode(void) const;
  virtual void SetNode(Ptr<Node> node);
  virtual bool NeedsArp(void) const;
  virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom() const;
  virtual Address GetMulticast(Ipv6Address addr) const;

protected:
  virtual void DoDispose(void);

  void ReceiveFromDevice(Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                         Address const &source, Address const &destination, PacketType packetType);
  void ForwardUnicast(void);
  Ptr<NetDevice> GetOutport(Ptr<NetDevice> incomingPort);

private:
  DelayedBridgeNetDevice(const DelayedBridgeNetDevice &);
  DelayedBridgeNetDevice &operator =(const DelayedBridgeNetDevice &);

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  Mac48Address m_address;
  Ptr<Node> m_node;
  Ptr<BridgeChannel> m_channel;
  Ptr<NetDevice> m_north_port;
  Ptr<NetDevice> m_south_port;
  uint32_t m_ifIndex;
  uint16_t m_mtu;
  bool m_hasNorthPort;
  bool m_hasSouthPort;

  DelayedBridgePolicy m_policy;
  Ptr<DelayedBridge> m_delayedBridge;
};

} // namespace ns3

#endif /* DELAYED_BRIDGE_NET_DEVICE_H */
