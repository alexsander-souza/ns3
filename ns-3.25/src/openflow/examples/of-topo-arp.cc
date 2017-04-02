/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2015 UFRGS
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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/openflow-module.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("OfTopoARP");

bool verbose                   = true;
bool use_drop                  = false;
static const uint SIM_DURATION = 15;

bool
SetVerbose(std::string value)
{
  verbose = true;
  return true;
}

int
main(int argc, char *argv[])
{
#ifdef NS3_OPENFLOW

  //
  // Allow the user to override any of the defaults and the above Bind() at
  // run-time, via command-line arguments
  //
  CommandLine cmd;
  cmd.AddValue("v",       "Verbose (turns on logging).", MakeCallback(&SetVerbose));
  cmd.AddValue("verbose", "Verbose (turns on logging).", MakeCallback(&SetVerbose));

  cmd.Parse(argc, argv);

  if (verbose)
  {
    LogComponentEnable("OfTopoARP",               LOG_LEVEL_ALL);
    LogComponentEnable("OpenFlowInterface",       LOG_LEVEL_DEBUG);
    LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_DEBUG);
    LogComponentEnable("OpenFlowSwitchHelper",    LOG_LEVEL_DEBUG);
    LogComponentEnable("Controller",              LOG_LEVEL_DEBUG);
    LogComponentEnable("OpenflowClient",          LOG_LEVEL_DEBUG);
    LogComponentEnable("OfSwitch",                LOG_LEVEL_DEBUG);
    LogComponentEnable("UndirectedGraph",         LOG_LEVEL_ALL);
    LogComponentEnable("ArpHandler",              LOG_LEVEL_ALL);
    LogComponentEnable("OFRouting",               LOG_LEVEL_ALL);
  }

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO("Create nodes");
  NodeContainer terminals1;
  terminals1.Create(1);

  NodeContainer terminals2;
  terminals2.Create(1);

  NodeContainer controllers;
  controllers.Create(1);

  NodeContainer ofSwitches;
  ofSwitches.Create(3);

  NodeContainer allHosts;
  allHosts.Add(terminals1);
  allHosts.Add(terminals2);

  NodeContainer allOFDev;
  allOFDev.Add(controllers);
  allOFDev.Add(ofSwitches);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay",    TimeValue(MilliSeconds(2)));

  NetDeviceContainer sw1_sw2   = csma.Install(NodeContainer(ofSwitches.Get(0), ofSwitches.Get(1)));
  NetDeviceContainer sw1_sw3   = csma.Install(NodeContainer(ofSwitches.Get(0), ofSwitches.Get(2)));
  NetDeviceContainer sw1_hosts = csma.Install(NodeContainer(ofSwitches.Get(0), terminals1));
  NetDeviceContainer sw2_hosts = csma.Install(NodeContainer(ofSwitches.Get(1), terminals2));
  NetDeviceContainer ctrl_sw   = csma.Install(allOFDev);

  // Add internet stack to controller, switches and terminals
  InternetStackHelper internet;
  internet.Install(allHosts);
  internet.Install(allOFDev);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");
  ipv4.Assign(NetDeviceContainer(sw1_hosts.Get(1)));
  ipv4.Assign(NetDeviceContainer(sw2_hosts.Get(1)));

  ipv4.SetBase("20.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer ofNet = ipv4.Assign(ctrl_sw);

  NS_LOG_INFO("Setup OpenFlow Controller");
  ApplicationContainer ofApps;
  OpenFlowControllerHelper controller;
  controller.SetAttribute("LocalAddress", Ipv4AddressValue(ofNet.GetAddress(0)));
  ofApps.Add(controller.Install(controllers.Get(0)));

  Ptr<LldpHandler> lldpApp = CreateObject<LldpHandler>();
  lldpApp->AddApplicationToController(controllers.Get(0));

  Ptr<ArpHandler> arpApp = CreateObject<ArpHandler>();
  arpApp->AddApplicationToController(controllers.Get(0));

  Ptr<OFRouting> routeApp = CreateObject<OFRouting>();
  routeApp->AddApplicationToController(controllers.Get(0));

  // Create the switch netdevice, which will do the packet switching
  OpenFlowSwitchNetDeviceHelper swtch;
  swtch.SetDeviceAttribute("FlowTableMissSendLength", UintegerValue(1024));
  swtch.SetControllerAddress(ofNet.GetAddress(0));

  ofApps.Add(swtch.Install(ofSwitches, ctrl_sw));
  ofApps.Start(Seconds(1.0));

  /* Configure UDP pinger *****************************************************/
  ApplicationContainer udpPinger;

  UdpEchoServerHelper echoServer(10); // server port

  UdpEchoClientHelper echoClient(Ipv4Address("10.0.0.1"), 10);
  echoClient.SetAttribute("MaxPackets", UintegerValue(50));
  echoClient.SetAttribute("Interval",   TimeValue(Seconds(0.5)));
  echoClient.SetAttribute("PacketSize", UintegerValue(256));

  udpPinger.Add(echoServer.Install(terminals1.Get(0)));
  udpPinger.Add(echoClient.Install(terminals2.Get(0)));

  udpPinger.Start(Seconds(10.0));
  udpPinger.Stop(Seconds(SIM_DURATION - 1));

  NS_LOG_INFO("Configure Tracing.");

  csma.EnablePcap("of-ctrl",  controllers);
  csma.EnablePcap("of-sw",    ofSwitches);
  csma.EnablePcap("of-hosts", allHosts);

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO("Run Simulation.");
  Simulator::Stop(Seconds(SIM_DURATION));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
  return 0;

#else // ifdef NS3_OPENFLOW
  NS_LOG_INFO("NS-3 OpenFlow is not enabled. Cannot run simulation.");
#endif // NS3_OPENFLOW
}
