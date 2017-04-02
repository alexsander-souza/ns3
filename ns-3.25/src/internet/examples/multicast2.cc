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

NS_LOG_COMPONENT_DEFINE("Multicast2Topology");

int
main (int argc, char *argv[])
{
  NodeContainer hosts;
  hosts.Create(3);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer hostsDevices;

  hostsDevices = csma.Install(hosts);

  InternetStackHelper stack;
  stack.Install(hosts);

  Ipv4AddressHelper address;
  address.SetBase("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipIface;
  ipIface = address.Assign(hostsDevices);

  MulticastClientHelper clientHelper1("ns3::UdpSocketFactory",
                                      ipIface.GetAddress(1),
                                      Ipv4Address("230.1.1.1"));

  MulticastClientHelper clientHelper2("ns3::UdpSocketFactory",
                                      ipIface.GetAddress(2),
                                      Ipv4Address("230.1.1.1"));

  ApplicationContainer clientApp1 = clientHelper1.Install(hosts.Get(1));
  ApplicationContainer clientApp2 = clientHelper2.Install(hosts.Get(2));

  clientApp1.Start(Seconds(30.0));
  clientApp1.Stop(Seconds(80.0));

  clientApp2.Start(Seconds(40.0));
  clientApp2.Stop(Seconds(50.0));

  MulticastSourcev2Helper sourceHelper(9);
  sourceHelper.SetAttribute("MaxPackets", UintegerValue(100));
  sourceHelper.SetAttribute("Interval", TimeValue(Seconds(2.0)));
  sourceHelper.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer sourceApp = sourceHelper.Install(hosts.Get(0));

  sourceApp.Start(Seconds(0.0));
  sourceApp.Stop(Seconds(80.0));

  csma.EnablePcapAll("multicast", true);

  Simulator::Stop(Seconds(110.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
