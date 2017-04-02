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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#include "ns3/string.h"
#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/multicast-sink.h"
#include "multicast-sink-helper.h"

namespace ns3
{
MulticastSinkHelper::MulticastSinkHelper(Ipv4Address group)
{
  m_factory.SetTypeId(MulticastSink::GetTypeId());
  m_factory.Set("Group", Ipv4AddressValue(group));
}

void MulticastSinkHelper::SetAttribute(std::string name,
                                         const AttributeValue &value)
{
  m_factory.Set(name, value);
}

ApplicationContainer MulticastSinkHelper::Install(Ptr<Node> node) const
{
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MulticastSinkHelper::Install(std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node>(nodeName);

  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MulticastSinkHelper::Install(NodeContainer c) const
{
  ApplicationContainer apps;

  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      apps.Add(InstallPriv(*i));
    }

  return apps;
}

Ptr<Application> MulticastSinkHelper::InstallPriv(Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application>();
  node->AddApplication(app);

  return app;
}
} // namespace ns3
