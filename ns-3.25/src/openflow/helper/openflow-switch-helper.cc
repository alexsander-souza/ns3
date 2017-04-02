/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2011 Blake Hurd, 2014 UFRGS
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
 * Author: Blake Hurd  <naimorai@gmail.com>
 * Contributor(s):
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 */
#ifdef NS3_OPENFLOW

#include "openflow-switch-helper.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/names.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/loopback-net-device.h"
#include "ns3/openflow-client.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("OpenFlowSwitchHelper");


std::ostream&
operator<<(std::ostream   & strm,
           NetDevice const& netdev)
{
  strm << "PORT:" << netdev.GetAddress() << "(" << netdev.GetInstanceTypeId() << ")";
  return strm;
}

// OpenFlowSwitchNetDeviceHelper methods

OpenFlowSwitchNetDeviceHelper::OpenFlowSwitchNetDeviceHelper()
{
  NS_LOG_FUNCTION_NOARGS();
  m_deviceFactory.SetTypeId(OpenFlowSwitchNetDevice::GetTypeId());
  m_clientFactory.SetTypeId(OpenflowClient::GetTypeId());
}

void
OpenFlowSwitchNetDeviceHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
  NS_LOG_FUNCTION_NOARGS();
  m_deviceFactory.Set(n1, v1);
}

void
OpenFlowSwitchNetDeviceHelper::SetClientAttribute(std::string n1, const AttributeValue& v1)
{
  NS_LOG_FUNCTION_NOARGS();
  m_clientFactory.Set(n1, v1);
}

void
OpenFlowSwitchNetDeviceHelper::SetControllerAddress(const Ipv4Address& address)
{
  SetClientAttribute("ControllerAddress", Ipv4AddressValue(address));
  m_ctrl_address = address;
}

ApplicationContainer
OpenFlowSwitchNetDeviceHelper::Install(Ptr<Node>node, NetDeviceContainer& exclude)
{
  NS_LOG_FUNCTION(node);
  NS_LOG_INFO("**** Install switch device on node " << node->GetId());

  Ptr<OpenFlowSwitchNetDevice> dev = m_deviceFactory.Create<OpenFlowSwitchNetDevice>();
  node->AddDevice(dev);

  int nDevs = node->GetNDevices();

  for (int i = 0; i < nDevs; ++i)
  {
    Ptr<NetDevice> port = node->GetDevice(i);

    NS_LOG_LOGIC(*port);

    if (!IsExcluded(port, exclude))
      dev->AddSwitchPort(port);
  }

  /* Lookup mgmt IP */
  Ptr<Ipv4>   ipv4  = node->GetObject<Ipv4>();
  Ipv4Address local = ipv4->SelectSourceAddress(0, m_ctrl_address,
                                                Ipv4InterfaceAddress::GLOBAL);

  Ptr<Application> app = m_clientFactory.Create<OpenflowClient>();
  app->SetAttribute("LocalAddress", Ipv4AddressValue(local));

  node->AddApplication(app);

  return ApplicationContainer(app);
}

ApplicationContainer
OpenFlowSwitchNetDeviceHelper::Install(NodeContainer& c, NetDeviceContainer& exclude)
{
  ApplicationContainer controlApps;

  for (auto node = c.Begin(); node != c.End(); ++node)
  {
    controlApps.Add(Install(*node, exclude));
  }

  return controlApps;
}

bool
OpenFlowSwitchNetDeviceHelper::IsExcluded(Ptr<NetDevice>dev, NetDeviceContainer& exclude) const
{
  if ((!Mac48Address::IsMatchingType(dev->GetAddress()))
      || (dev->GetInstanceTypeId() == OpenFlowSwitchNetDevice::GetTypeId())
      || (dev->GetInstanceTypeId() == LoopbackNetDevice::GetTypeId()))
    return true;

  for (auto p = exclude.Begin(); p != exclude.End(); ++p) {
    if (dev == *p)
      return true;
  }

  return false;
}
} // namespace ns3
#endif // NS3_OPENFLOW
