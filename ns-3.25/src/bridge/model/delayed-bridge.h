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

#ifndef DELAYED_BRIDGE_H
#define DELAYED_BRIDGE_H

#include "ns3/net-device.h"
#include "ns3/bridged-net-device.h"
#include "ns3/packet-time.h"
#include "ns3/drop-bridge.h"
#include "ns3/mac48-address.h"
#include "ns3/nstime.h"
#include "ns3/bridge-channel.h"
#include <stdint.h>
#include <string>
#include <map>
#include <set>
#include <ns3/nstime.h>

#include <iostream>

namespace ns3
{

class Node;
class DelayedBridgeNetDevice;

class DelayedBridge : public Object
{
public:
  static TypeId GetTypeId (void);

  DelayedBridge();
  ~DelayedBridge();

  void DelayConstant(void);
  void QueuePacket(PacketTime pt);
  PacketTime NextPacket(void);

  //Template Method
  void ReceivePacket(PacketTime pt);
  void SendPacket(void);

  void DropPacket(void);

  virtual void Jitter(void) = 0;

  void SetBridgeNetDevice(DelayedBridgeNetDevice *bridge);
  void SetDropBridge(DropBridge dropBridge);

protected:
  Time DELAY = Seconds(0.5);
  std::multiset<PacketTime, std::less<PacketTime> > m_packetsQueue;
  DelayedBridgeNetDevice *bridgeNetDevice;
  DropBridge dropBridge;
  bool busy;
  bool drop;
};

} // namespace ns3

#endif /* DELAYED_BRIDGE_H */
