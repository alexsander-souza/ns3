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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 * Contributor(s):
 *  Alexsander de Souza <asouza@inf.ufrgs.br>
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
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
#include "ns3/dhcp-module.h"

/*
// Default Network Topology
//
//    (server)
//       n0                          n2
//        \   (p2p1)                 /
//         \ 30.0.0.0               / 10.0.0.0/24
//          \         (p2p4)       /    (csma1)
//           \       20.0.0.0     /
//            r0 --------------- r2
//           / \                /|\   (p2p6)
// 30.1.0.0 /   \     50.0.0.0 / | \ 10.1.0.0
//  (p2p2) /     \     (p2p5) /  |  \
//        /       \          /   |   \
//       /  (p2p3) \        /    |    n3
//      /  40.0.0.0 \      /     |
//     n1            \    /      |
//                    \  /       |
//                     r1        |
//                               ||-- n4 (10.2.0.?)
//                   10.2.0.0/24 ||-- n5 (10.2.0.?)
//                    (csma2)    ||-- n6 (10.2.0.?)
//                               ||-- n7 (10.2.0.?) (client)
//
*/

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Topology1");

int
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("DhcpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("DhcpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("DhcpHeader", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  /* Monta topologia **********************************************************/
  NodeContainer p2p1Nodes;
  p2p1Nodes.Create (2); //[0,1]

  NodeContainer p2p2Nodes;
  p2p2Nodes.Add (p2p1Nodes.Get (0)); // add r0
  p2p2Nodes.Create (1); // create n1 [2]

  NodeContainer p2p3Nodes;
  p2p3Nodes.Add (p2p1Nodes.Get (0));
  p2p3Nodes.Create (1); // create r1 [3]

  NodeContainer p2p4Nodes;
  p2p4Nodes.Add (p2p3Nodes.Get (0));
  p2p4Nodes.Create (1); // create r2 [4]

  NodeContainer p2p5Nodes;
  p2p5Nodes.Add (p2p3Nodes.Get(1)); // add r1
  p2p5Nodes.Add (p2p4Nodes.Get(1)); // add r2

  NodeContainer p2p6Nodes;
  p2p6Nodes.Add (p2p4Nodes.Get(1));
  p2p6Nodes.Create (1); //[5]

  NodeContainer csma1Nodes;
  csma1Nodes.Add (p2p4Nodes.Get (1));
  csma1Nodes.Create (1); // create n2 [6]

  NodeContainer csma2Nodes;
  csma2Nodes.Add (p2p4Nodes.Get (1)); // add r2
  csma2Nodes.Create (4); // [7, 8, 9, 10]

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2p1Devices;
  p2p1Devices = pointToPoint.Install (p2p1Nodes);

  NetDeviceContainer p2p2Devices;
  p2p2Devices = pointToPoint.Install (p2p2Nodes);

  NetDeviceContainer p2p3Devices;
  p2p3Devices = pointToPoint.Install (p2p3Nodes);

  NetDeviceContainer p2p4Devices;
  p2p4Devices = pointToPoint.Install (p2p4Nodes);

  NetDeviceContainer p2p5Devices;
  p2p5Devices = pointToPoint.Install (p2p5Nodes);

  NetDeviceContainer p2p6Devices;
  p2p6Devices = pointToPoint.Install (p2p6Nodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csma1Devices;
  csma1Devices = csma.Install (csma1Nodes);

  NetDeviceContainer csma2Devices;
  csma2Devices = csma.Install (csma2Nodes);

  InternetStackHelper stack;
  stack.Install (p2p1Nodes);
  stack.Install (p2p2Nodes.Get (1));
  stack.Install (p2p3Nodes.Get (1));
  stack.Install (p2p6Nodes.Get (1));
  stack.Install (csma1Nodes.Get (1));
  stack.Install (csma2Nodes);

  /* Configura DHCP Server Router *********************************************/
  Ipv4InterfaceContainer csma2Interfaces;

  //r2 configuration: i/f create + setup
  Ptr<Ipv4> ipv4Router = csma2Nodes.Get(0)->GetObject<Ipv4> ();
  uint32_t ifIndex = ipv4Router->AddInterface (csma2Devices.Get(0));
  ipv4Router->AddAddress (ifIndex, Ipv4InterfaceAddress (Ipv4Address ("10.2.0.1"), Ipv4Mask ("/24")));
  ipv4Router->SetForwarding(ifIndex, true);
  ipv4Router->SetUp (ifIndex);
  csma2Interfaces.Add(ipv4Router, ifIndex);

  /* Configura Nodos DHCP *****************************************************/
  //n4 configuration: i/f create + setup
  Ptr<Ipv4> ipv4n4 = csma2Nodes.Get(1)->GetObject<Ipv4> ();
  ifIndex = ipv4n4->AddInterface (csma2Devices.Get(1));
  ipv4n4->SetUp (ifIndex);

  //n5 configuration: i/f create + setup
  Ptr<Ipv4> ipv4n5 = csma2Nodes.Get(2)->GetObject<Ipv4> ();
  ifIndex = ipv4n5->AddInterface (csma2Devices.Get(2));
  ipv4n5->SetUp (ifIndex);

  //n6 configuration: i/f create + setup
  Ptr<Ipv4> ipv4n6 = csma2Nodes.Get(3)->GetObject<Ipv4> ();
  ifIndex = ipv4n6->AddInterface (csma2Devices.Get(3));
  ipv4n6->SetUp (ifIndex);

  //n7 configuration: i/f create + setup
  Ptr<Ipv4> ipv4n7 = csma2Nodes.Get(4)->GetObject<Ipv4> ();
  ifIndex = ipv4n7->AddInterface (csma2Devices.Get(4));
  ipv4n7->SetUp (ifIndex);

  /* Configura demais roteadores **********************************************/

  Ipv4AddressHelper address;
  address.SetBase ("30.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p1Interfaces;
  p2p1Interfaces = address.Assign (p2p1Devices);

  address.SetBase ("30.1.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p2Interfaces;
  p2p2Interfaces = address.Assign (p2p2Devices);

  address.SetBase ("40.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p3Interfaces;
  p2p3Interfaces = address.Assign (p2p3Devices);

  address.SetBase ("20.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p4Interfaces;
  p2p4Interfaces = address.Assign (p2p4Devices);

  address.SetBase ("50.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p5Interfaces;
  p2p5Interfaces = address.Assign (p2p5Devices);

  address.SetBase ("10.1.0.0", "255.255.255.0");
  Ipv4InterfaceContainer p2p6Interfaces;
  p2p6Interfaces = address.Assign (p2p6Devices);

  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer csma1Interfaces;
  csma1Interfaces = address.Assign (csma1Devices);

  /* Configura Aplicação DHCP server ******************************************/
  DhcpServerHelper dhcp_server(Ipv4Address("10.2.0.0"), Ipv4Mask("/24"));
  ApplicationContainer ap_dhcp_server = dhcp_server.Install(csma2Nodes.Get (0)); // escolhe r2
  ap_dhcp_server.Start (Seconds (1.0));
  ap_dhcp_server.Stop (Seconds (10.0));

  /* Configura Aplicação DHCP client ******************************************/
  DhcpClientHelper dhcp_client;

  ApplicationContainer ap_dhcp_client1 = dhcp_client.Install(csma2Nodes.Get(1));
  ap_dhcp_client1.Start (Seconds (1.0));
  ap_dhcp_client1.Stop (Seconds (10.0));

  ApplicationContainer ap_dhcp_client2 = dhcp_client.Install(csma2Nodes.Get(2));
  ap_dhcp_client2.Start (Seconds (1.0));
  ap_dhcp_client2.Stop (Seconds (10.0));

  ApplicationContainer ap_dhcp_client3 = dhcp_client.Install(csma2Nodes.Get(3));
  ap_dhcp_client3.Start (Seconds (1.0));
  ap_dhcp_client3.Stop (Seconds (10.0));

  ApplicationContainer ap_dhcp_client4 = dhcp_client.Install(csma2Nodes.Get(4));
  ap_dhcp_client4.Start (Seconds (1.0));
  ap_dhcp_client4.Stop (Seconds (10.0));

  /* Configura UDP pinger *****************************************************/
  UdpEchoServerHelper echoServer (10); // porta do servidor
  ApplicationContainer serverApps = echoServer.Install (p2p1Nodes.Get(1)); // n0
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (p2p1Interfaces.GetAddress(1), 10);

  echoClient.SetAttribute ("MaxPackets", UintegerValue (50));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.05)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (csma2Nodes.Get(4)); // n7
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  /* Executa simulação ********************************************************/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcap ("udp_server", p2p1Devices.Get(1), true);
  csma.EnablePcap ("udp_client", csma2Devices.Get(4), true);
  csma.EnablePcap ("topologia1", csma1Devices.Get (1), true);
  csma.EnablePcap ("topologia1", csma2Devices.Get (1), true);

  // Gravar em arquivo todas as tabelas de roteamento
  Ipv4GlobalRoutingHelper g;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("topologia1-dynamic-global-routing.routes", std::ios::out);
  g.PrintRoutingTableAllAt (Seconds (10), routingStream);

  Simulator::Stop (Seconds (15.0));

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
