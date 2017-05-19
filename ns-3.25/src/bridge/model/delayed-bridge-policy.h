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

#ifndef DELAYED_BRIDGE_POLICY_H
#define DELAYED_BRIDGE_POLICY_H

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

#include <iostream>

namespace ns3
{

class Node;

class DelayedBridgePolicy
{
public:
  //DelayedBridgePolicy();
  void DelayConstant(void);
  void Jitter(void);
  void JitterNormal(double mean, double variance);
  void JitterExponential(double mean, double bound);
  void JitterPareto(double mean, double shape);

  void QueuePacket(PacketTime pt);
  PacketTime NextPacket(void);

  Time DELAY = Seconds(0.5);
  std::multiset<PacketTime, std::less<PacketTime> > m_packetsQueue;
};

} // namespace ns3

#endif /* DELAYED_BRIDGE_POLICY_H */
