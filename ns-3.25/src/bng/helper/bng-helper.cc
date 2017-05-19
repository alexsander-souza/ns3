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

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/node.h"
#include "ns3/ancp-nas-agent.h"
#include "ns3/dhcp-server.h"
#include "ns3/radius-client.h"
#include "bng-helper.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("BngHelper");

BngHelper::BngHelper(void)
{
  NS_LOG_FUNCTION_NOARGS();
  m_bng_factory.SetTypeId("ns3::BngControl");
  m_ancp_factory.SetTypeId("ns3::AncpNasAgent");
  m_dhcp_factory.SetTypeId("ns3::DhcpServer");
  m_radius_factory.SetTypeId("ns3::RadiusClient");
}

ApplicationContainer BngHelper::InstallBngControl(Ptr<Node>     bngNode,
                                                  Ptr<NetDevice>aggr_netdev,
                                                  Ptr<NetDevice>reg_netdev) const
{
  NS_LOG_FUNCTION(this << bngNode);
  ApplicationContainer helperApps;

  /* Create NAS control */
  Ptr<Application> appNas = m_bng_factory.Create<Application>();
  bngNode->AddApplication(appNas);
  helperApps.Add(appNas);

  /* Create ANCP AN agent */
  Ptr<Application> appAncp = m_ancp_factory.Create<Application>();
  bngNode->AddApplication(appAncp);
  helperApps.Add(appAncp);

  /* Create DHCP server */
  Ptr<Application> appDhcp = m_dhcp_factory.Create<Application>();
  bngNode->AddApplication(appDhcp);
  helperApps.Add(appDhcp);

  /* Create RADIUS client */
  Ptr<Application> appRadius = m_radius_factory.Create<Application>();
  bngNode->AddApplication(appRadius);
  helperApps.Add(appRadius);

  /* Connect everyone */
  Ptr<BngControl> bng = DynamicCast<BngControl, Application>(appNas);

  bng->SetAccessNetPort(aggr_netdev);
  bng->SetRegionalNetPort(reg_netdev);

  Ptr<AncpNasAgent> ancp = DynamicCast<AncpNasAgent, Application>(appAncp);
  bng->SetAncpAgent(ancp);
  Ptr<DhcpServer> dhcp = DynamicCast<DhcpServer, Application>(appDhcp);
  bng->SetDhcpServer(dhcp);
  Ptr<RadiusClient> radius = DynamicCast<RadiusClient, Application>(appRadius);
  bng->SetRadiusClient(radius);

  /* Create profiles */
  for (auto &p: m_accessProfileMap)
    bng->AddBandwidthProfile(p.first, p.second);


  for (auto &p: m_mcastProfileMap)
    bng->AddMcastProfile(p.first, p.second);

  for (auto &p: m_mcastProfiles)
    bng->SetupMcastProfile(p);

  return helperApps;
}

void BngHelper::SetAttribute(std::string name, const AttributeValue& value)
{
  NS_LOG_FUNCTION_NOARGS();

  if (name == "ServerAddress")
    {
      m_radius_factory.Set(name, value);
    }
  else if (name == "ServerSecret")
    {
      m_radius_factory.Set(name, value);
    }
  else if (name == "PoolAddresses")
    {
      m_dhcp_factory.Set(name, value);
    }
  else if (name == "PoolMask")
    {
      m_dhcp_factory.Set(name, value);
    }
  else if (name == "LeaseTime")
    {
      m_dhcp_factory.Set(name, value);
    }
  else if (name == "LocalAddress")
    {
      m_ancp_factory.Set(name, value);
    }
  else if (name == "PortUpCallback")
    {
      m_ancp_factory.Set(name, value);
    }
  else if (name == "PortDownCallback")
    {
      m_ancp_factory.Set(name, value);
    }
  else
    {
      m_bng_factory.Set(name, value);
    }
}

void BngHelper::AddServiceProfile(const Mac48Address& circuitId,
                                  const std::string& accessProfileName,
                                  const std::string& mcastProfileName)
{
  NS_LOG_FUNCTION(circuitId << accessProfileName << mcastProfileName);
  m_accessProfileMap[circuitId] = accessProfileName;
  m_mcastProfileMap[circuitId] = mcastProfileName;
}

void BngHelper::SetupMulticastProfile(const struct BngControl::McastProfile& profile)
{
  NS_LOG_FUNCTION(profile.name);
  m_mcastProfiles.push_back(profile);
}
}
