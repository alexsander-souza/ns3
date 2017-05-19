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

#include "ns3/normal-delayed-bridge.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("NormalDelayedBridge");

NS_OBJECT_ENSURE_REGISTERED(NormalDelayedBridge);

TypeId NormalDelayedBridge::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::NormalDelayedBridge")
                      .SetParent<DelayedBridge>()
                      .SetGroupName("Bridge");

  return tid;
}

NormalDelayedBridge::NormalDelayedBridge()
{
  mean = (DELAY.GetSeconds())/(2.0);
  variance = (DELAY.GetSeconds())/(10.0);
}

NormalDelayedBridge::~NormalDelayedBridge()
{
  NS_LOG_FUNCTION_NOARGS();
}

void NormalDelayedBridge::Jitter()
{
  Ptr<NormalRandomVariable> randNormal = CreateObject<NormalRandomVariable>();
  randNormal->SetAttribute("Mean", DoubleValue(mean));
  randNormal->SetAttribute("Variance", DoubleValue(variance));

  double rand = randNormal->GetValue();
  double delay = DELAY.GetSeconds() + rand;

  Time jitter;
  if(delay <= 0)
    jitter = DELAY;
  else
    jitter = Time::FromDouble(delay, Time::Unit::S);

  Simulator::Schedule(jitter, &DelayedBridge::SendPacket, this);
}

void NormalDelayedBridge::SetMean(double mean)
{
  this->mean = mean;
}

void NormalDelayedBridge::SetVariance(double variance)
{
  this->variance = variance;
}

double NormalDelayedBridge::GetMean(void)
{
  return mean;
}

double NormalDelayedBridge::GetVariance(void)
{
  return variance;
}

} // namespace ns3
