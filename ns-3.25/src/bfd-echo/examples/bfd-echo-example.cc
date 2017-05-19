/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/bfd-echo-helper.h"

using namespace ns3;


int
main(int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;

  cmd.AddValue("verbose", "Tell application to log if true", verbose);

  cmd.Parse(argc, argv);

  /* Create simple Ethernet topology */
  NodeContainer csmaNodes;
  csmaNodes.Create(2);

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install(csmaNodes);

  InternetStackHelper stack;
  stack.Install(csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign(csmaDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  /* BFD Echo service */
  BfdEchoHelper helper;

  helper.SetAttribute("AgentAddr", Ipv4AddressValue(csmaInterfaces.GetAddress(0)));
  helper.SetAttribute("GatewayAddr", Mac48AddressValue(Mac48Address::ConvertFrom(csmaDevices.Get(1)->GetAddress())));
  helper.SetAttribute("PingInterval", TimeValue(Seconds(3)));

  ApplicationContainer apps = helper.Install(csmaNodes.Get(0));
  apps.Start(Seconds(2.0));
  apps.Stop(Seconds(20.0));

  csma.EnablePcap ("bfd-echo", csmaDevices.Get (0), true);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
