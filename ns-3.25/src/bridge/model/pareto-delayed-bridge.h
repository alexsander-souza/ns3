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

#ifndef PARETO_DELAYED_BRIDGE_H
#define PARETO_DELAYED_BRIDGE_H

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

#include "ns3/delayed-bridge.h"

namespace ns3
{

class Node;

class ParetoDelayedBridge : public DelayedBridge
{
public:
  static TypeId GetTypeId (void);
  ParetoDelayedBridge();
  ~ParetoDelayedBridge();
  void Jitter(void);
  void SetMean(double mean);
  void SetShape(double shape);
  double GetMean(void);
  double GetShape(void);

private:
  double mean;
  double shape;
};

} // namespace ns3

#endif /* PARETO_DELAYED_BRIDGE_H */
