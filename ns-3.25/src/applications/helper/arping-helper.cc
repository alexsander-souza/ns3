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

#include "arping-helper.h"
#include "ns3/arping.h"
#include "ns3/names.h"

namespace ns3 {
ArpingHelper::ArpingHelper()
{
  m_factory.SetTypeId("ns3::Arping");
}

void
ArpingHelper::SetAttribute(std::string name, const AttributeValue& value)
{
  m_factory.Set(name, value);
}

ApplicationContainer
ArpingHelper::Install(Ptr<Node>node) const
{
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
ArpingHelper::Install(std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
ArpingHelper::Install(NodeContainer c) const
{
  ApplicationContainer apps;

  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
  {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application>
ArpingHelper::InstallPriv(Ptr<Node>node) const
{
  Ptr<Arping> app = m_factory.Create<Arping>();
  node->AddApplication(app);

  return app;
}
} // namespace ns3
