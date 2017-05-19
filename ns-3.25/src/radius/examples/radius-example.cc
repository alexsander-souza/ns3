/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Alexsander de Souza
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
#include "ns3/radius-module.h"
#include "ns3/radius-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RadiusServiceTest");

NodeContainer csmaNodes;

void SetupRadiusDB(Ptr<RadiusDB> radiusDB)
{
  /* User 1 */
  RadiusUserEntry user1("user1");
  user1.SetUserPassword("senha1");

  RadiusUserEntry user2("user2");
  user2.SetUserPassword("senha2");

  radiusDB->AddUser(user1);
  radiusDB->AddUser(user2);
}

int AuthenticationDone(uint8_t msg_code, RadiusMessage::RadiusAvpList avps)
{
  if (msg_code == RadiusMessage::RAD_ACCESS_ACCEPT)
    NS_LOG_FUNCTION ("Success");
  else
    NS_LOG_FUNCTION ("Failed");
  return (0);
}

int AccountDone(uint8_t msg_code, RadiusMessage::RadiusAvpList avps)
{
  NS_LOG_FUNCTION_NOARGS();
  return (0);
}

void AuthenticateUser1(void)
{
  Ptr<RadiusClient> radClient = csmaNodes.Get(1)->GetApplication(0)->GetObject<RadiusClient>();

  radClient->SetRequestDoneCb(MakeCallback(&AuthenticationDone));
  radClient->DoAuthentication("user1", "senha1", "nas134134", 1);
}

void AuthenticateUser2(void)
{
  Ptr<RadiusClient> radClient = csmaNodes.Get(1)->GetApplication(0)->GetObject<RadiusClient>();

  radClient->SetRequestDoneCb(MakeCallback(&AuthenticationDone));
  radClient->DoAuthentication("user2", "wrongpasswd", "nas134134", 1);
}

void StartAccountUser1(void)
{
  Ptr<RadiusClient> radClient = csmaNodes.Get(1)->GetApplication(0)->GetObject<RadiusClient>();

  radClient->SetRequestDoneCb(MakeCallback(&AccountDone));
  radClient->DoStartAccounting(RadiusAVP::RAD_ACCT_START, "123456789", "user1");
}

void StopAccountUser1(void)
{
  Ptr<RadiusClient> radClient = csmaNodes.Get(1)->GetApplication(0)->GetObject<RadiusClient>();

  radClient->SetRequestDoneCb(MakeCallback(&AccountDone));
  radClient->DoStopAccounting(RadiusAVP::RAD_ACCT_STOP, "123456789",
                              5000, RadiusAVP::RAD_TERM_CAUSE_USER_REQUEST);
}

int
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc,argv);

  LogComponentEnable ("RadiusServiceTest", LOG_LEVEL_ALL);
  if (verbose)
    {
      LogComponentEnable ("RadiusClient", LOG_LEVEL_ALL);
      LogComponentEnable ("RadiusServer", LOG_LEVEL_ALL);
      //~ LogComponentEnable ("RadiusDB", LOG_LEVEL_ALL);
      //~ LogComponentEnable ("RadiusHeader", LOG_LEVEL_ALL);
    }

  /* Create simple Ethernet topology */
  csmaNodes.Create (2);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Setup RADIUS Service */
  Ptr<RadiusDB> radiusDB = CreateObject<RadiusDB>();
  SetupRadiusDB(radiusDB);

  RadiusHelper radius;
  radius.SetAttribute("ServerAddress", Ipv4AddressValue(csmaInterfaces.GetAddress(0)));
  radius.SetAttribute("ServerSecret", StringValue("segredo"));
  radius.SetDatabase(radiusDB);

  ApplicationContainer apps = radius.InstallServer(csmaNodes.Get(0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  apps = radius.InstallClient(csmaNodes.Get(1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

  /* Do a request */
  Simulator::Schedule (Seconds (3.0), &AuthenticateUser1);
  Simulator::Schedule (Seconds (3.0), &AuthenticateUser2);
  Simulator::Schedule (Seconds (4.0), &StartAccountUser1);
  Simulator::Schedule (Seconds (8.0), &StopAccountUser1);

  /* Enable packet capture */
  csma.EnablePcap ("radius", csmaDevices.Get (0), true);

  /* Run Simulation */
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
