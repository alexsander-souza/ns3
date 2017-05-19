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
#include "ns3/ancp-an-agent.h"
#include "access-node-helper.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AccessNodeHelper");

AccessNodeHelper::AccessNodeHelper(void)
{
  NS_LOG_FUNCTION_NOARGS();
  m_an_factory.SetTypeId("ns3::AccessNetDevice");
  m_ancp_factory.SetTypeId("ns3::AncpAnAgent");
}

void
AccessNodeHelper::SetDeviceAttribute(std::string n1, const AttributeValue &v1)
{
  NS_LOG_FUNCTION_NOARGS();
  m_an_factory.Set(n1, v1);
}

void
AccessNodeHelper::SetupServiceProfile(Ptr<AccessProfile> profile)
{
  m_profiles.push_back(profile);
}

NetDeviceContainer AccessNodeHelper::CreateAccessNodeDevice(Ptr<Node> anNode,
                                                            NetDeviceContainer &accessPorts,
                                                            Ptr<NetDevice> uplink)
{
  NS_LOG_FUNCTION(this << anNode);

  Ptr<AccessNetDevice>anDev = m_an_factory.Create<AccessNetDevice> ();
  anNode->AddDevice(anDev);

  anDev->SetUplinkPort(uplink);

  for (auto i = accessPorts.Begin(); i != accessPorts.End(); ++i)
    {
      NS_LOG_LOGIC("Add AccessPort " << *i);
      anDev->AddAccessPort(*i);
    }

  return NetDeviceContainer(anDev);
}

ApplicationContainer AccessNodeHelper::InstallAccessNodeControl(Ptr<Node> anNode,
                                                                Ipv4Address nasAddr)
{
  NS_LOG_FUNCTION(this << anNode);

  Ptr<AccessNetDevice> anDev = GetAnNetDevice(anNode);

  NS_ASSERT(anDev != nullptr);

  /* Setup Profiles */
  for (auto prof = m_profiles.begin(); prof != m_profiles.end(); ++prof)
    {
      NS_LOG_LOGIC("Add AccessProfile " << *prof);
      anDev->AddAccessProfile(*prof);
    }


  /* Create ANCP AN agent here */
  Ptr<Application> app = m_ancp_factory.Create<Application> ();

  Ptr<Ipv4> ipv4 = anNode->GetObject<Ipv4> ();
  Ipv4Address local = ipv4->SelectSourceAddress(0, nasAddr,
                                                Ipv4InterfaceAddress::GLOBAL);

  NS_LOG_LOGIC("ANCP connect " << local << " to " << nasAddr);

  app->SetAttribute("LocalAddress", Ipv4AddressValue(local));
  app->SetAttribute("NasAddress", Ipv4AddressValue(nasAddr));
  anNode->AddApplication(app);

  Ptr<AncpAnAgent> anAgent = DynamicCast<AncpAnAgent, Application>(app);
  anDev->SetAncpAgent(anAgent);

  anDev->StartIpServices();

  return ApplicationContainer(app);
}

Ptr<AccessNetDevice> AccessNodeHelper::GetAnNetDevice(Ptr<Node>anNode) const
{
  NS_LOG_FUNCTION(this << anNode);

  const TypeId &anTid = AccessNetDevice::GetTypeId();

  for (uint32_t i = 0; i < anNode->GetNDevices(); ++i)
    {
      Ptr<NetDevice> netdev = anNode->GetDevice(i);
      if (netdev->GetInstanceTypeId() == anTid)
        return DynamicCast<AccessNetDevice, NetDevice>(netdev);
    }
  return nullptr;
}
}
