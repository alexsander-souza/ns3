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
#include <cmath>
#include <ns3/random-variable-stream.h>
#include <ns3/double.h>

#include "ns3/drop-bridge.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("DropBridge");

NS_OBJECT_ENSURE_REGISTERED(DropBridge);

TypeId DropBridge::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DropBridge")
                      .SetParent<Object>()
                      .SetGroupName("Bridge");

  return tid;
}

DropBridge::DropBridge()
{
  drop = false;
  dropProb = 0;
}

DropBridge::~DropBridge()
{
  NS_LOG_FUNCTION_NOARGS();
}

bool DropBridge::ShouldDrop(void)
{
  Ptr<UniformRandomVariable> randUniform = CreateObject<UniformRandomVariable>();
  randUniform->SetAttribute("Min", DoubleValue(0));
  randUniform->SetAttribute("Max", DoubleValue(1));

  double rand = randUniform->GetValue();

  if(rand <= dropProb)
    return true;
  else
    return false;
}

void DropBridge::SetProb(double prob)
{
  dropProb = prob;
}

} // namespace ns3
