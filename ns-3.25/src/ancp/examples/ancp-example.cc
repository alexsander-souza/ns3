/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 UFRGS
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
#include "ns3/ancp-module.h"
#include "ns3/ancp-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AncpAgentTest");

NodeContainer csmaNodes;

Mac48Address anMacAddr;

std::ostream & operator <<(std::ostream & os, const std::list<Address> & lst)
{
  for (auto &addr: lst)
    os << addr << ", ";
  return os;
}

void SendPortUp()
{
  Ptr<AncpAnAgent> anAgent = csmaNodes.Get(1)->GetApplication(0)->GetObject<AncpAnAgent>();

  std::string circuit = "port0/0/1";

  anAgent->SendPortUp(circuit, 1, 10000, 10000);
}

void SendPortDown()
{
  Ptr<AncpAnAgent> anAgent = csmaNodes.Get(1)->GetApplication(0)->GetObject<AncpAnAgent>();

  std::string circuit = "port0/0/1";

  anAgent->SendPortDown(circuit);
}

void SendLineConfig(const Mac48Address &anName)
{
  NS_LOG_FUNCTION(anName);
  Ptr<AncpNasAgent> nasAgent = csmaNodes.Get(0)->GetApplication(0)->GetObject<AncpNasAgent>();

  std::string circuit = "port0/0/1";
  std::string profName = "Internet10Mbps";

  nasAgent->SendPortConfigCommand(anName, circuit, profName);
}

void SendMcastLineConfig(const Mac48Address &anName)
{
  NS_LOG_FUNCTION(anName);
  Ptr<AncpNasAgent> nasAgent = csmaNodes.Get(0)->GetApplication(0)->GetObject<AncpNasAgent>();

  std::string circuit = "port0/0/1";
  std::string profName = "TopHD";

  nasAgent->SendMCastPortConfigCommand(anName, circuit, profName);
}

void SendMCastProvisioning()
{
  NS_LOG_FUNCTION(anMacAddr);
  Ptr<AncpNasAgent> nasAgent = csmaNodes.Get(0)->GetApplication(0)->GetObject<AncpNasAgent>();

  std::string profName = "TopHD";
  std::list<Address> whitelist;
  whitelist.push_back(Ipv4Address("224.0.0.1"));
  std::list<Address> greylist;
  greylist.push_back(Ipv4Address("224.0.0.2"));
  std::list<Address> blacklist;
  blacklist.push_back(Ipv4Address("224.0.0.3"));

  nasAgent->SendMCastServiceProfile(anMacAddr, profName, whitelist, greylist, blacklist, true, false);
}

void SendMCastOnJoin()
{
  NS_LOG_FUNCTION_NOARGS();
  Ptr<AncpAnAgent> anAgent = csmaNodes.Get(1)->GetApplication(0)->GetObject<AncpAnAgent>();

  std::string circuit = "port0/0/1";

  anAgent->SendMCastAdmissionCtl(circuit, AncpTlvMCastCommand::CMD_ADD, Ipv4Address("224.0.0.1"));
}

void SendMCastOnLeave()
{
  NS_LOG_FUNCTION_NOARGS();
  Ptr<AncpAnAgent> anAgent = csmaNodes.Get(1)->GetApplication(0)->GetObject<AncpAnAgent>();

  std::string circuit = "port0/0/1";

  anAgent->SendMCastAdmissionCtl(circuit, AncpTlvMCastCommand::CMD_DELETE, Ipv4Address("224.0.0.1"));
}

int AncpNasPortUp(const Mac48Address &anName, const std::string &circuitId,
                  uint32_t rateUp, uint32_t rateDown, uint32_t tagMode)
{
  NS_LOG_FUNCTION(anName << circuitId << rateUp << rateDown << tagMode);
  SendLineConfig(anName);
  SendMcastLineConfig(anName);
  return(0);
}

int AncpNasPortDown(const Mac48Address &anName, const std::string &circuitId)
{
  NS_LOG_FUNCTION(anName << circuitId);
  return(0);
}

int AncpNasMCastAdmission(const Mac48Address&anName, const std::string&circuitId, const Address&group, bool join)
{
  NS_LOG_FUNCTION(anName << circuitId << group << join);

  if (join)
    {
      Ptr<AncpNasAgent> nasAgent = csmaNodes.Get(0)->GetApplication(0)->GetObject<AncpNasAgent>();
      /* Authorize */
      nasAgent->SendMCastCommand(anName, circuitId, AncpTlvMCastCommand::CMD_ADD, group);
    }

  return(0);
}

int AncpAnLineConfig(const std::string &circuitId, const std::string &profName)
{
  NS_LOG_FUNCTION(circuitId << profName);
  return(0);
}

int AncpAnMCastLineConfig(const std::string&circuitId, const std::string&profName)
{
  NS_LOG_FUNCTION(circuitId << profName);
  return(0);
}

int AncpAnMCastProfile(const std::string &profName,
                       const std::list<Address> &whitelist,
                       const std::list<Address> &greylist,
                       const std::list<Address> &blacklist,
                       bool doWhitelistCac,
                       bool doMRepCtlCac)
{
  NS_LOG_FUNCTION(profName);

  NS_LOG_LOGIC("  + Whitelist " << whitelist);
  NS_LOG_LOGIC("  + GreyList " << greylist);
  NS_LOG_LOGIC("  + BlackList " << blacklist);

  return(0);
}

int AncpAnMCastCommand(const std::string &circuitId, int command, const Address &group)
{
  NS_LOG_FUNCTION(circuitId << command << group);
  return(0);
}

int
main(int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;

  cmd.AddValue("verbose", "Tell application to log if true", verbose);

  cmd.Parse(argc, argv);

  LogComponentEnable("AncpAgentTest", LOG_LEVEL_ALL);
  if (verbose)
    {
      LogComponentEnable("AncpAnAgent", LOG_LEVEL_ALL);
      LogComponentEnable("AncpNasAgent", LOG_LEVEL_ALL);
      LogComponentEnable("AncpHeader", LOG_LEVEL_INFO);
      LogComponentEnable("AncpAdjacency", LOG_LEVEL_INFO);
    }

  /* Create simple Ethernet topology */
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

  /* Setup ANCP agents */
  AncpHelper ancp;
  ancp.SetNasAttribute("PortUpCallback", CallbackValue(MakeCallback(AncpNasPortUp)));
  ancp.SetNasAttribute("PortDownCallback", CallbackValue(MakeCallback(AncpNasPortDown)));
  ancp.SetNasAttribute("MCastCacCallback", CallbackValue(MakeCallback(AncpNasMCastAdmission)));

  ancp.SetAnAttribute("LineConfigCallback", CallbackValue(MakeCallback(AncpAnLineConfig)));
  ancp.SetAnAttribute("MCastLineConfigCallback", CallbackValue(MakeCallback(AncpAnMCastLineConfig)));
  ancp.SetAnAttribute("MCastProfileCallback", CallbackValue(MakeCallback(AncpAnMCastProfile)));
  ancp.SetAnAttribute("MCastCommandCallback", CallbackValue(MakeCallback(AncpAnMCastCommand)));

  ApplicationContainer apps = ancp.InstallNas(csmaNodes.Get(0), csmaInterfaces.GetAddress(0));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(30.0));

  apps = ancp.InstallAccessNode(csmaNodes.Get(1));
  apps.Start(Seconds(1.0));
  apps.Stop(Seconds(28.0));

  anMacAddr = Mac48Address::ConvertFrom(csmaDevices.Get(1)->GetAddress());

  /* Do requests */
  Simulator::Schedule(Seconds(2.0), &SendMCastProvisioning);
  Simulator::Schedule(Seconds(5.0), &SendPortUp);
  Simulator::Schedule(Seconds(8.0), &SendMCastOnJoin);
  Simulator::Schedule(Seconds(12.0), &SendMCastOnLeave);
  Simulator::Schedule(Seconds(15.0), &SendPortDown);

  /* Enable packet capture */
  csma.EnablePcap("ancp-nas", csmaDevices.Get(0), true);
  csma.EnablePcap("ancp-an", csmaDevices.Get(1), true);

  /* Run Simulation */
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
