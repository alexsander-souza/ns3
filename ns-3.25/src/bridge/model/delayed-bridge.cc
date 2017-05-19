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

#include "ns3/delayed-bridge.h"
#include "ns3/delayed-bridge-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DelayedBridge");

NS_OBJECT_ENSURE_REGISTERED(DelayedBridge);

TypeId DelayedBridge::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DelayedBridge")
                      .SetParent<Object>()
                      .SetGroupName("Bridge");

  return tid;
}

DelayedBridge::DelayedBridge()
{
  busy = false;
  drop = true;
  dropBridge.SetProb(0.20);
}

DelayedBridge::~DelayedBridge()
{
  NS_LOG_FUNCTION_NOARGS();
}

void DelayedBridge::QueuePacket(PacketTime pt)
{
  m_packetsQueue.insert(pt);
}

PacketTime DelayedBridge::NextPacket(void)
{
  std::multiset<PacketTime, std::less<PacketTime> >::iterator nextPacket;
  nextPacket = m_packetsQueue.begin();
  PacketTime pt(*nextPacket);
  m_packetsQueue.erase(nextPacket);

  return pt;
}

void DelayedBridge::DelayConstant()
{
  Simulator::Schedule(DelayedBridge::DELAY, &DelayedBridgeNetDevice::Forward, bridgeNetDevice);
}

void DelayedBridge::ReceivePacket(PacketTime pt)
{
  QueuePacket(pt);

  if(!busy)
    {
      busy = true;
      Jitter();
    }
}

void DelayedBridge::SendPacket(void)
{
  if(!drop)
    {
      bridgeNetDevice->Forward();
    }
  else
    {
      if(dropBridge.ShouldDrop())
        {
          DropPacket();
        }
      else
        {
          bridgeNetDevice->Forward();
        }
    }

  if(m_packetsQueue.empty())
    {
      busy = false;
    }
  else
    {
      Jitter();
    }
}

void DelayedBridge::DropPacket(void)
{
  std::multiset<PacketTime, std::less<PacketTime> >::iterator nextPacket;
  nextPacket = m_packetsQueue.begin();
  m_packetsQueue.erase(nextPacket);
}

void DelayedBridge::SetBridgeNetDevice(DelayedBridgeNetDevice *bridge)
{
  bridgeNetDevice = bridge;
}

void DelayedBridge::SetDropBridge(DropBridge bridge)
{
  dropBridge = bridge;
}

} // namespace ns3
