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

#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/multicast-source.h"
#include "multicast-source-helper.h"

namespace ns3
{

MulticastSourceHelper::MulticastSourceHelper()
{
  m_factory.SetTypeId(MulticastSource::GetTypeId());
}

void MulticastSourceHelper::SetAttribute(std::string name, const AttributeValue &value)
{
  m_factory.Set(name, value);
}

void MulticastSourceHelper::SetFill(Ptr<Application> app, std::string fill)
{
  app->GetObject<MulticastSource>()->SetFill(fill);
}

void MulticastSourceHelper::SetFill(Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<MulticastSource>()->SetFill(fill, dataLength);
}

void MulticastSourceHelper::SetFill(Ptr<Application> app, uint8_t *fill, uint32_t fillLength,
                                      uint32_t dataLength)
{
  app->GetObject<MulticastSource>()->SetFill(fill, fillLength, dataLength);
}

ApplicationContainer MulticastSourceHelper::Install(Ptr<Node> node) const
{
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MulticastSourceHelper::Install(std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MulticastSourceHelper::Install(NodeContainer c) const
{
  ApplicationContainer apps;

  for(NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
      apps.Add(InstallPriv(*i));
    }

  return apps;
}

Ptr<Application> MulticastSourceHelper::InstallPriv(Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<MulticastSource>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
