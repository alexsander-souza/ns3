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
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/access-net-device.h"
#include "ns3/ancp-helper.h"
#include "ns3/access-node-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AnNetDevTest");

static const uint SIM_DURATION = 30;
Ptr<AccessProfile> profile1 = Create<AccessProfile>("Internet10Mbps", DataRate(10000000), DataRate(10000000));

NetDeviceContainer cpeDevices;
NetDeviceContainer anDevices;
ApplicationContainer nasApps;

int AncpNasPortUp(const Mac48Address &anName, const std::string &circuitId,
                  uint32_t rateUp, uint32_t rateDown, uint32_t tagMode)
{
  NS_LOG_FUNCTION(anName << circuitId << rateUp << rateDown << tagMode);

  Ptr<AncpNasAgent> nasAgent = DynamicCast<AncpNasAgent, Application>(nasApps.Get(0));
  std::string profName = profile1->GetName();
  nasAgent->SendPortConfigCommand(anName, circuitId, profName);

  return(0);
}

int AncpNasPortDown(const Mac48Address &anName, const std::string &circuitId)
{
  NS_LOG_FUNCTION(anName << circuitId);
  return(0);
}

void ActivateAccessLoops()
{
  for (auto it = anDevices.Begin(); it != anDevices.End(); ++it)
    {
      (*it)->SetLineProtocolStatus(true);
    }

  Ipv4GlobalRoutingHelper::RecomputeRoutingTables();
}

void DeactivateAccessLoops()
{
  for (auto it = anDevices.Begin(); it != anDevices.End(); ++it)
    {
      (*it)->SetLineProtocolStatus(false);
    }

  Ipv4GlobalRoutingHelper::RecomputeRoutingTables();
}

int
main(int argc, char *argv[])
{
  bool verbose = true;
  int nCpes = 2;

  CommandLine cmd;

  cmd.AddValue("verbose", "Tell application to log if true", verbose);

  cmd.Parse(argc, argv);

  LogComponentEnable("AnNetDevTest", LOG_LEVEL_ALL);

  if (verbose)
    {
      LogComponentEnable("AccessPort", LOG_LEVEL_DEBUG);
      LogComponentEnable("AccessNetDevice", LOG_LEVEL_DEBUG);
      // LogComponentEnable ("AccessNodeHelper", LOG_LEVEL_ALL);
      LogComponentEnable("AncpAnAgent", LOG_LEVEL_ALL);
    }

  NS_LOG_INFO("Create nodes");
  NodeContainer cpes;
  cpes.Create(nCpes);

  NodeContainer anNode;
  anNode.Create(1);

  NodeContainer nasNode;
  nasNode.Create(1);

  NS_LOG_INFO("Build Topology");
  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

  /* Create the Ethernet links, from each CPE to the AccessNode */
  for (int i = 0; i < nCpes; i++)
    {
      NetDeviceContainer link = csma.Install(NodeContainer(cpes.Get(i), anNode));
      cpeDevices.Add(link.Get(0));
      anDevices.Add(link.Get(1));
    }

  /* Create link between AN and fake NAS */
  NetDeviceContainer nasLink = csma.Install(NodeContainer(anNode.Get(0), nasNode));

  /* Call AN Helper */
  AccessNodeHelper anHelper;
  NetDeviceContainer anDev;

  /* Setup service profiles */
  anHelper.SetupServiceProfile(profile1);

  anDev = anHelper.CreateAccessNodeDevice(anNode.Get(0), anDevices, nasLink.Get(0));

  /* Add internet stack to the terminals */
  InternetStackHelper internet;
  internet.Install(cpes);
  internet.Install(nasNode);
  internet.Install(anNode);

  /* Give fake NAS an IP, it will act as a simple router */
  NS_LOG_INFO("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer nasInterfaces = ipv4.Assign(NetDeviceContainer(nasLink.Get(1), anDev));
  ipv4.Assign(cpeDevices);

  /* install ANCP agents */
  ApplicationContainer ancpApps = anHelper.InstallAccessNodeControl(anNode.Get(0), nasInterfaces.GetAddress(0));
  ancpApps.Start(Seconds(1.0));
  ancpApps.Stop(Seconds(SIM_DURATION));

  /* create fake NAS agent */
  AncpHelper ancp;
  ancp.SetNasAttribute("PortUpCallback", CallbackValue(MakeCallback(AncpNasPortUp)));
  ancp.SetNasAttribute("PortDownCallback", CallbackValue(MakeCallback(AncpNasPortDown)));

  nasApps = ancp.InstallNas(nasNode.Get(0), nasInterfaces.GetAddress(0));
  nasApps.Start(Seconds(0.0));
  nasApps.Stop(Seconds(SIM_DURATION));

  Simulator::Schedule(Seconds(3.0), &ActivateAccessLoops);
  Simulator::Schedule(Seconds(SIM_DURATION - 2), &DeactivateAccessLoops);

  /* Send UDP traffic from CPE-0 to SRV */
  NS_LOG_INFO("Create Applications.");
  uint16_t port = 10;

  /* Setup UDP pinger */
  UdpEchoServerHelper echoServer(port);  // porta do servidor

  ApplicationContainer serverApps = echoServer.Install(nasNode.Get(0));
  serverApps.Start(Seconds(1.0));

  UdpEchoClientHelper echoClient(nasInterfaces.GetAddress(0), port);
  echoClient.SetAttribute("MaxPackets", UintegerValue(50));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(0.5)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  ApplicationContainer clientApps = echoClient.Install(cpes.Get(0));
  clientApps.Start(Seconds(5.0));
  clientApps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  csma.EnablePcapAll("access-node", true);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
