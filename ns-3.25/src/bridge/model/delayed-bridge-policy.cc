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

namespace ns3
{


/*DelayedBridgePolicy::DelayedBridgePolicy()
{

}

DelayedBridgePolicy::~DelayedBridgePolicy()
{
  NS_LOG_FUNCTION_NOARGS();
}

void DelayedBridgePolicy::DoDispose()
{
  m_packetsQueue = NULL;
}*/

void DelayedBridgePolicy::QueuePacket(PacketTime pt)
{
  m_packetsQueue.insert(pt);
}

PacketTime DelayedBridgePolicy::NextPacket(void)
{
  std::multiset<PacketTime, std::less<PacketTime> >::iterator nextPacket;
  nextPacket = m_packetsQueue.begin();
  PacketTime pt(*nextPacket);
  m_packetsQueue.erase(nextPacket);

  return pt;
}

void DelayedBridgePolicy::DelayConstant()
{
  std::cout << "DelayConstant: Schedule" << std::endl;
  //Simulator::Schedule(DelayedBridgePolicy::DELAY, &DelayedBridgePolicy::ForwardUnicast, this);
}

void DelayedBridgePolicy::Jitter()
{
  double mean = (DELAY.GetSeconds())/(2.0);
  //double variance = (DELAY.GetSeconds())/(10.0);
  double shape = 2.0;

  JitterPareto(mean, shape);
}

void DelayedBridgePolicy::JitterNormal(double mean, double variance)
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

  std::cout << "JitterNormal: Schedule to " << jitter.GetSeconds() << std::endl;
  //Simulator::Schedule(jitter, &DelayedBridgePolicy::ForwardUnicast, this);
}

void DelayedBridgePolicy::JitterExponential(double mean, double bound)
{
  Ptr<ExponentialRandomVariable> randExponential = CreateObject<ExponentialRandomVariable>();
  randExponential->SetAttribute("Mean", DoubleValue(mean));
  randExponential->SetAttribute("Bound", DoubleValue(bound));

  double rand = randExponential->GetValue();
  double delay = DELAY.GetSeconds() + rand;

  Time jitter;
  if(delay <= 0)
    jitter = DELAY;
  else
    jitter = Time::FromDouble(delay, Time::Unit::S);

  std::cout << "JitterExponential: Schedule to " << jitter.GetSeconds() << std::endl;
  //Simulator::Schedule(jitter, &DelayedBridgePolicy::ForwardUnicast, this);
}

void DelayedBridgePolicy::JitterPareto(double mean, double shape)
{
  Ptr<ParetoRandomVariable> randPareto = CreateObject<ParetoRandomVariable>();
  randPareto->SetAttribute("Mean", DoubleValue(mean));
  randPareto->SetAttribute("Shape", DoubleValue(shape));

  double rand = randPareto->GetValue();
  double delay = DELAY.GetSeconds() + rand;

  Time jitter;
  if(delay <= 0)
    jitter = DELAY;
  else
    jitter = Time::FromDouble(delay, Time::Unit::S);

  std::cout << "JitterPareto: Schedule to " << jitter.GetSeconds() << std::endl;
  //Simulator::Schedule(jitter, &DelayedBridgePolicy::ForwardUnicast, this);
}

} // namespace ns3
