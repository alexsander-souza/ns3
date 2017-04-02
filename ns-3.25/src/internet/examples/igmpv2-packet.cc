/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB, 2014 UFRGS
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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Topology1");

int
main (int argc, char *argv[])
{
  NodeContainer hosts;
  hosts.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  NetDeviceContainer hostsDevices;
  hostsDevices = pointToPoint.Install(hosts);

  InternetStackHelper stack;
  stack.Install(hosts);

  Ipv4AddressHelper address;
  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipIface;
  ipIface = address.Assign(hostsDevices);

  Igmpv2Helper igmpv2PingHelper(true, ipIface.GetAddress(0), Ipv4Address("240.0.0.1"));

  ApplicationContainer igmpv2PingApp = igmpv2PingHelper.InstallPing(hosts.Get(0));
  igmpv2PingApp.Start(Seconds(2.0));
  igmpv2PingApp.Stop(Seconds(10.0));

  Igmpv2Helper igmpv2PongHelper(false, ipIface.GetAddress(1));

  ApplicationContainer igmpv2PongApp = igmpv2PongHelper.InstallPong(hosts.Get(1));
  igmpv2PongApp.Start(Seconds(1.0));
  igmpv2PongApp.Stop(Seconds(10.0));

  pointToPoint.EnablePcapAll("igmpv2", false);

  Simulator::Stop(Seconds (15.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
