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

#include <string>
#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/ancp-module.h"
#include "ancp-helper.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AncpHelper");

AncpHelper::AncpHelper(void) :
  m_nas_addr(Ipv4Address::GetAny())
{
  m_nas_factory.SetTypeId("ns3::AncpNasAgent");
  m_an_factory.SetTypeId("ns3::AncpAnAgent");
}

void AncpHelper::SetNasAttribute(std::string name, const AttributeValue &value)
{
  m_nas_factory.Set(name, value);
}

void AncpHelper::SetAnAttribute(std::string name, const AttributeValue &value)
{
  m_an_factory.Set(name, value);
}

ApplicationContainer AncpHelper::InstallNas(Ptr<Node> node, Ipv4Address nas_addr)
{
  m_nas_addr = nas_addr;

  Ptr<Application> app = m_nas_factory.Create<Application> ();
  app->SetAttribute("LocalAddress", Ipv4AddressValue(m_nas_addr));

  node->AddApplication(app);
  return ApplicationContainer(app);
}

ApplicationContainer AncpHelper::InstallAccessNode(NodeContainer c) const
{
  ApplicationContainer apps;

  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      apps.Add(AttachClient(*i));
    }

  return apps;
}

ApplicationContainer AncpHelper::InstallAccessNode(Ptr<Node> node) const
{
  return ApplicationContainer(AttachClient(node));
}

Ptr<Application> AncpHelper::AttachClient(Ptr<Node> node) const
{
  Ptr<Application> app = m_an_factory.Create<Application> ();

  /* select local address */
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  Ipv4Address local = ipv4->SelectSourceAddress(0, m_nas_addr,
                                                Ipv4InterfaceAddress::GLOBAL);
  NS_LOG_LOGIC("ANCP connect " << local << " to " << m_nas_addr);
  app->SetAttribute("LocalAddress", Ipv4AddressValue(local));
  app->SetAttribute("NasAddress", Ipv4AddressValue(m_nas_addr));
  node->AddApplication(app);
  return app;
}
}
