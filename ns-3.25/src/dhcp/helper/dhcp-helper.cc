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
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/dhcp-server.h"
#include "ns3/dhcp-client.h"
#include "dhcp-helper.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("DhcpHelper");

DhcpClientHelper::DhcpClientHelper(void)
{
  m_factory.SetTypeId(DhcpClient::GetTypeId());
}

DhcpServerHelper::DhcpServerHelper(Ipv4Address pool_addr, Ipv4Mask pool_mask)
{
  m_factory.SetTypeId(DhcpServer::GetTypeId());
  SetAttribute("PoolAddresses", Ipv4AddressValue(pool_addr));
  SetAttribute("PoolMask",      Ipv4MaskValue(pool_mask));
}

void
DhcpClientHelper::SetAttribute(
  std::string           name,
  const AttributeValue& value)
{
  m_factory.Set(name, value);
}

void
DhcpServerHelper::SetAttribute(
  std::string           name,
  const AttributeValue& value)
{
  m_factory.Set(name, value);
}

ApplicationContainer
DhcpClientHelper::Install(Ptr<Node>node) const
{
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
DhcpServerHelper::Install(Ptr<Node>node) const
{
  return ApplicationContainer(InstallPriv(node));
}

Ptr<Application>DhcpClientHelper::InstallPriv(Ptr<Node>node) const
{
  Ptr<Ipv4> ipv4n4      = node->GetObject<Ipv4>();
  Ptr<NetDevice> device = node->GetDevice(0);

  NS_LOG_DEBUG("Installing DHCP client on node " << node->GetId() << " DEV " << device->GetAddress());

  NS_ASSERT_MSG(ipv4n4->GetInterfaceForDevice(device) == -1, "Interface already has IP");

  uint32_t ifIndex = ipv4n4->AddInterface(device);
  ipv4n4->SetUp(ifIndex);

  Ptr<Application> app = m_factory.Create<DhcpClient>();
  node->AddApplication(app);
  return app;
}

Ptr<Application>DhcpServerHelper::InstallPriv(Ptr<Node>node) const
{
  Ptr<Application> app = m_factory.Create<DhcpServer>();
  node->AddApplication(app);

  return app;
}
} // namespace ns3
