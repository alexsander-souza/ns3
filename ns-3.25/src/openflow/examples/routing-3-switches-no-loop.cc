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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *
 */

/*
   Network topology

 *
              --------------------
              | Controller (n10) |
              |    (10.0.0.1)    |
              --------------------
                         |
                         |
     Server             ||\      Client
   (20.0.0.1)          | | \   (20.0.0.7)
        |             |  |  \      |
      n0    n1     |     |   \     n6    n7
      |     |     |      |    \    |     |
   ----------------|     |     \-----------------
   | Switch1 (n11) |    |       | Switch3 (n13) |
   |  (10.0.0.2)   |    |       |  (10.0.0.4)   |
   -----------------    |       -----------------
      |     |    \     |      /  |     |
      n2    n3    \    |     /   n8    n9
                   \   |    /
                  -----------------
                  | Switch2 (n12) |
                  |  (10.0.0.3)   |
                  -----------------
                    |      |
                    n4     n5

   - CBR/UDP flows from n0 to n1, from n3 to n0 and from n4 to n5
   - DropTail queues
   - If order of adding nodes and netdevices is kept:
      n0 = 00:00:00:00:00:01, n1 = 00:00:00:00:00:03, n2 = 00:00:00:00:00:05, n3 = 00:00:00:00:00:07,
      n4 = 00:00:00:00:00:09, n5 = 00:00:00:00:00:0b, n6 = 00:00:00:00:00:0d, n7 = 00:00:00:00:00:0f,
      n8 = 00:00:00:00:00:11, n9 = 00:00:00:00:00:13,
      Controller = 00:00:00:00:00:19, Switch1 = 00:00:00:00:00:1a, Switch2 = 00:00:00:00:00:1b,
      Switch3 = 00:00:00:00:00:1c
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

NS_LOG_COMPONENT_DEFINE("TopologiaOpenFlow");

bool verbose = false;
bool use_drop = false;
ns3::Time timeout = ns3::Seconds(0);

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
  cmd.AddValue("v", "Verbose (turns on logging).", MakeCallback(&SetVerbose));
  cmd.AddValue("verbose", "Verbose (turns on logging).", MakeCallback(&SetVerbose));

  cmd.Parse(argc, argv);

  if (verbose)
    {
      LogComponentEnable("ArpHandler", LOG_LEVEL_ALL);
      LogComponentEnable("TopologiaOpenFlow", LOG_LEVEL_INFO);
      // LogComponentEnable("OpenFlowInterface", LOG_LEVEL_INFO);
      LogComponentEnable("OpenFlowSwitchNetDevice", LOG_LEVEL_INFO);
      LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
      LogComponentEnable("Controller", LOG_LEVEL_ALL);
      LogComponentEnable("OpenflowClient", LOG_LEVEL_INFO);
      LogComponentEnable("OFRouting", LOG_LEVEL_ALL);
    }

  //
  // Explicitly create the nodes required by the topology (shown above).
  //
  NS_LOG_INFO("Create nodes.");
  NodeContainer terminals;
  terminals.Create(10);

  NodeContainer controllerContainer;
  controllerContainer.Create(1);

  NodeContainer csmaSwitch1;
  csmaSwitch1.Create(1);

  NodeContainer csmaSwitch2;
  csmaSwitch2.Create(1);

  NodeContainer csmaSwitch3;
  csmaSwitch3.Create(1);

  NodeContainer terminal1Nodes;

  for (uint32_t i = 0; i < 4; i++)
    terminal1Nodes.Add(terminals.Get(i));

  NodeContainer terminal2Nodes;

  for (uint32_t i = 4; i < 6; i++)
    terminal2Nodes.Add(terminals.Get(i));

  NodeContainer terminal3Nodes;

  for (uint32_t i = 6; i < 10; i++)
    terminal3Nodes.Add(terminals.Get(i));

  NodeContainer openflowNodes;
  openflowNodes.Add(controllerContainer);
  openflowNodes.Add(csmaSwitch1);
  openflowNodes.Add(csmaSwitch2);
  openflowNodes.Add(csmaSwitch3);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  // Create the csma links, from each terminal to the corresponding switch
  NetDeviceContainer terminal1Devices;
  NetDeviceContainer switch1Devices;

  for (uint32_t i = 0; i < terminal1Nodes.GetN(); i++)
    {
      NetDeviceContainer link = csma.Install(NodeContainer(terminal1Nodes.Get(i), csmaSwitch1));
      terminal1Devices.Add(link.Get(0));
      switch1Devices.Add(link.Get(1));
    }

  NetDeviceContainer terminal2Devices;
  NetDeviceContainer switch2Devices;

  for (uint32_t i = 0; i < terminal2Nodes.GetN(); i++)
    {
      NetDeviceContainer link = csma.Install(NodeContainer(terminal2Nodes.Get(i), csmaSwitch2));
      terminal2Devices.Add(link.Get(0));
      switch2Devices.Add(link.Get(1));
    }

  NetDeviceContainer terminal3Devices;
  NetDeviceContainer switch3Devices;

  for (uint32_t i = 0; i < terminal3Nodes.GetN(); i++)
    {
      NetDeviceContainer link = csma.Install(NodeContainer(terminal3Nodes.Get(i), csmaSwitch3));
      terminal3Devices.Add(link.Get(0));
      switch3Devices.Add(link.Get(1));
    }

  // Create a link between Switch1 and Switch2
  NetDeviceContainer link = csma.Install(NodeContainer(csmaSwitch1, csmaSwitch2));
  switch1Devices.Add(link.Get(0));
  switch2Devices.Add(link.Get(1));

  // Create a link between Switch2 and Switch3
  link = csma.Install(NodeContainer(csmaSwitch2, csmaSwitch3));
  switch2Devices.Add(link.Get(0));
  switch3Devices.Add(link.Get(1));

  // Create connections between openflow nodes (all switches and controller)
  NetDeviceContainer openflowDevices = csma.Install(openflowNodes);

  Ptr<Node> controllerNode = controllerContainer.Get(0);
  Ptr<Node> switch1Node = csmaSwitch1.Get(0);
  Ptr<Node> switch2Node = csmaSwitch2.Get(0);
  Ptr<Node> switch3Node = csmaSwitch3.Get(0);

  // Add internet stack to controller, switches and terminals
  InternetStackHelper internet;
  internet.Install(terminals);
  internet.Install(controllerNode);
  internet.Install(switch1Node);
  internet.Install(switch2Node);
  internet.Install(switch3Node);

  // We've got the "hardware" in place.  Now we need to add IP addresses.
  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer openflowInterfaces;
  openflowInterfaces = ipv4.Assign(openflowDevices);

  ipv4.SetBase("20.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer terminal1Interfaces;
  terminal1Interfaces = ipv4.Assign(terminal1Devices);

  Ipv4InterfaceContainer terminal2Interfaces;
  terminal2Interfaces = ipv4.Assign(terminal2Devices);

  Ipv4InterfaceContainer terminal3Interfaces;
  terminal3Interfaces = ipv4.Assign(terminal3Devices);

  // Installs the controller application on controller node
  OpenFlowControllerHelper controller;

  ApplicationContainer controllerApps = controller.Install(controllerNode, openflowInterfaces.GetAddress(0));
  controllerApps.Start(Seconds(1.0));
  controllerApps.Stop(Seconds(15.0));

  NodeContainer topologyNodes;
  topologyNodes.Add(terminals);
  topologyNodes.Add(openflowNodes);

  // Create object to represent this topology in a graph
  UndirectedGraph graph;

  // Nodes
  for (uint32_t i = 0; i < topologyNodes.GetN(); i++)
    {
      Vertex nodeVertex(topologyNodes.Get(i));
      graph.AddVertex(nodeVertex);
    }

  // Controller links
  Vertex controllerVertex(controllerNode);

  NodeContainer ofSwitchesNodes;
  ofSwitchesNodes.Add(switch1Node);
  ofSwitchesNodes.Add(switch2Node);
  ofSwitchesNodes.Add(switch3Node);

  for (uint32_t i = 0; i < ofSwitchesNodes.GetN(); i++)
    {
      Adjacency switchAdjacency(ofSwitchesNodes.Get(i)->GetId(), 0);
      graph.AddEdge(controllerVertex, switchAdjacency);
    }

  // Switch 1 links
  Vertex switch1Vertex(switch1Node);
  uint16_t switch1Port = 0;

  for (uint32_t i = 0; i < terminal1Nodes.GetN(); i++)
    {
      Adjacency hostAdjacency(terminal1Nodes.Get(i)->GetId(), switch1Port++);
      graph.AddEdge(switch1Vertex, hostAdjacency);

      Vertex hostVertex(terminal1Nodes.Get(i));

      Adjacency switchAdjacency(switch1Node->GetId(), 0);
      graph.AddEdge(hostVertex, switchAdjacency);
    }

  Adjacency switch2Adjacency(switch2Node->GetId(), switch1Port++);
  graph.AddEdge(switch1Vertex, switch2Adjacency);

  Adjacency controllerAdjacency(controllerNode->GetId(), switch1Devices.GetN(), 10);
  graph.AddEdge(switch1Vertex, controllerAdjacency);

  // Switch 2 links
  Vertex switch2Vertex(switch2Node);
  uint16_t switch2Port = 0;

  for (uint32_t i = 0; i < terminal2Nodes.GetN(); i++)
    {
      Adjacency hostAdjacency(terminal2Nodes.Get(i)->GetId(), switch2Port++);
      graph.AddEdge(switch2Vertex, hostAdjacency);

      Vertex hostVertex(terminal2Nodes.Get(i)->GetId());

      Adjacency switchAdjacency(switch2Node->GetId(), 0);
      graph.AddEdge(hostVertex, switchAdjacency);
    }

  Adjacency switch1Adjacency(switch1Node->GetId(), switch2Port++);
  graph.AddEdge(switch2Vertex, switch1Adjacency);

  Adjacency switch3Adjacency(switch3Node->GetId(), switch2Port++);
  graph.AddEdge(switch2Vertex, switch3Adjacency);

  controllerAdjacency.SetPortNumber(switch2Devices.GetN());
  graph.AddEdge(switch2Vertex, controllerAdjacency);

  // Switch 3 links
  Vertex switch3Vertex(switch3Node);
  uint16_t switch3Port = 0;

  for (uint32_t i = 0; i < terminal3Nodes.GetN(); i++)
    {
      Adjacency host(terminal3Nodes.Get(i)->GetId(), switch3Port++);
      graph.AddEdge(switch3Vertex, host);

      Vertex hostVertex(terminal3Nodes.Get(i)->GetId());

      Adjacency switchAdjacency(switch3Node->GetId(), 0);
      graph.AddEdge(hostVertex, switchAdjacency);
    }

  switch2Adjacency.SetPortNumber(switch3Port++);
  graph.AddEdge(switch3Vertex, switch2Adjacency);

  controllerAdjacency.SetPortNumber(switch3Devices.GetN());
  graph.AddEdge(switch3Vertex, controllerAdjacency);

  // Create object to store ports information
  OFRouting::MacInfoMap infoMap;

  // Terminals entries
  for (uint32_t i = 0; i < terminal1Nodes.GetN(); i++)
    {
      OFRouting::PortInfo portInfo;
      portInfo.nodeId = terminal1Nodes.Get(i)->GetId();
      portInfo.portNumber = 0;
      portInfo.ipsList.push_back(terminal1Interfaces.GetAddress(i));

      Mac48Address terminalMac = Mac48Address::ConvertFrom(terminal1Devices.Get(i)->GetAddress());

      infoMap.emplace(terminalMac, portInfo);
    }

  for (uint32_t i = 0; i < terminal2Nodes.GetN(); i++)
    {
      OFRouting::PortInfo portInfo;
      portInfo.nodeId = terminal2Nodes.Get(i)->GetId();
      portInfo.portNumber = 0;
      portInfo.ipsList.push_back(terminal2Interfaces.GetAddress(i));

      Mac48Address terminalMac = Mac48Address::ConvertFrom(terminal2Devices.Get(i)->GetAddress());

      infoMap.emplace(terminalMac, portInfo);
    }

  for (uint32_t i = 0; i < terminal3Nodes.GetN(); i++)
    {
      OFRouting::PortInfo portInfo;
      portInfo.nodeId = terminal3Nodes.Get(i)->GetId();
      portInfo.portNumber = 0;
      portInfo.ipsList.push_back(terminal3Interfaces.GetAddress(i));

      Mac48Address terminalMac = Mac48Address::ConvertFrom(terminal3Devices.Get(i)->GetAddress());

      infoMap.emplace(terminalMac, portInfo);
    }

  // Controller entry
  OFRouting::PortInfo portInfo;
  portInfo.nodeId = controllerNode->GetId();
  portInfo.portNumber = 0;
  portInfo.ipsList.push_back(openflowInterfaces.GetAddress(0));
  Mac48Address openflowDeviceMac = Mac48Address::ConvertFrom(openflowDevices.Get(0)->GetAddress());

  infoMap.emplace(openflowDeviceMac, portInfo);

  // Switch 1 entry
  portInfo.nodeId = switch1Node->GetId();
  portInfo.portNumber = switch1Devices.GetN();
  portInfo.ipsList.clear();
  portInfo.ipsList.push_back(openflowInterfaces.GetAddress(1));
  openflowDeviceMac = Mac48Address::ConvertFrom(openflowDevices.Get(1)->GetAddress());

  infoMap.emplace(openflowDeviceMac, portInfo);

  // Switch 2 entry
  portInfo.nodeId = switch2Node->GetId();
  portInfo.portNumber = switch2Devices.GetN();
  portInfo.ipsList.clear();
  portInfo.ipsList.push_back(openflowInterfaces.GetAddress(2));
  openflowDeviceMac = Mac48Address::ConvertFrom(openflowDevices.Get(2)->GetAddress());

  infoMap.emplace(openflowDeviceMac, portInfo);

  // Switch 3 entry
  portInfo.nodeId = switch3Node->GetId();
  portInfo.portNumber = switch3Devices.GetN();
  portInfo.ipsList.clear();
  portInfo.ipsList.push_back(openflowInterfaces.GetAddress(3));
  openflowDeviceMac = Mac48Address::ConvertFrom(openflowDevices.Get(3)->GetAddress());

  infoMap.emplace(openflowDeviceMac, portInfo);

  // Adds an routing application to controller's list of applications
  Ptr<OFRouting> routingApp = CreateObject<OFRouting>();
  routingApp->SetTopology(graph);
  routingApp->SetMacInfoMap(infoMap);
  routingApp->SetOFNetworkAddress("10.0.0.0");
  routingApp->SetOFNetworkMask("255.255.255.0");
  routingApp->AddApplicationToController(controllerNode);

  // Adds an dummy application to controller's list of applications
  // Ptr<Dummy> dummyApp = CreateObject<Dummy>();
  // dummyApp->AddApplicationToController(controllerNode);

  // Adds arp handler application to controller's list of applications
  Ptr<ArpHandler> arpApp = CreateObject<ArpHandler>();
  arpApp->AddApplicationToController(controllerNode);

  // Create the switch netdevice, which will do the packet switching
  OpenFlowSwitchNetDeviceHelper swtch;
  swtch.SetDeviceAttribute("FlowTableMissSendLength", UintegerValue(1024));

  swtch.Install(switch1Node, switch1Devices);
  swtch.Install(switch2Node, switch2Devices);
  swtch.Install(switch3Node, switch3Devices);

  // Configure openflow clients on switches
  OpenFlowSwitchClientHelper openflowClient;

  ApplicationContainer switch1Apps = openflowClient.Install(switch1Node,
                                                            openflowInterfaces.GetAddress(0),
                                                            openflowInterfaces.GetAddress(1));
  switch1Apps.Start(Seconds(1.0));
  switch1Apps.Stop(Seconds(15.0));

  ApplicationContainer switch2Apps = openflowClient.Install(switch2Node,
                                                            openflowInterfaces.GetAddress(0),
                                                            openflowInterfaces.GetAddress(2));
  switch2Apps.Start(Seconds(1.0));
  switch2Apps.Stop(Seconds(15.0));

  ApplicationContainer switch3Apps = openflowClient.Install(switch3Node,
                                                            openflowInterfaces.GetAddress(0),
                                                            openflowInterfaces.GetAddress(3));
  switch3Apps.Start(Seconds(1.0));
  switch3Apps.Stop(Seconds(15.0));

  /* Configure UDP pinger *****************************************************/

  UdpEchoServerHelper echoServer(10);    // server port
  ApplicationContainer serverApps = echoServer.Install(terminals.Get(0));    // n0
  serverApps.Start(Seconds(2.0));
  serverApps.Stop(Seconds(15.0));

  UdpEchoClientHelper echoClient(Ipv4Address("20.0.0.1"), 10);

  echoClient.SetAttribute("MaxPackets", UintegerValue(50));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(0.15)));
  echoClient.SetAttribute("PacketSize", UintegerValue(256));

  ApplicationContainer clientApps = echoClient.Install(terminals.Get(6));    // n6
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(15.0));

  NS_LOG_INFO("Configure Tracing.");

  csma.EnablePcap("ctrl", controllerContainer);
  csma.EnablePcap("sw1", csmaSwitch1);
  csma.EnablePcap("sw2", csmaSwitch2);
  csma.EnablePcap("sw3", csmaSwitch3);
  csma.EnablePcap("n0-sw1", terminal1Devices.Get(0));
  csma.EnablePcap("n6-sw3", terminal3Devices.Get(0));

  //
  // Now, do the actual simulation.
  //
  NS_LOG_INFO("Run Simulation.");
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
  return 0;

#else
  NS_LOG_INFO("NS-3 OpenFlow is not enabled. Cannot run simulation.");
#endif // NS3_OPENFLOW
}
