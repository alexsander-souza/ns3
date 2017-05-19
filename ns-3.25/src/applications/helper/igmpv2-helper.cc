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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#include "igmpv2-helper.h"
#include "ns3/igmpv2-ping.h"
#include "ns3/igmpv2-pong.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

#include "ns3/inet-socket-address.h"

#include <iostream>

namespace ns3
{

Igmpv2Helper::Igmpv2Helper(bool isPing)
{
  if(isPing)
    {
      m_factory.SetTypeId(Igmpv2Ping::GetTypeId());
    }
  else
    {
      m_factory.SetTypeId(Igmpv2Pong::GetTypeId());
    }
}

Igmpv2Helper::Igmpv2Helper(bool isPing, Ipv4Address address)
{
  if(isPing)
    {
      m_factory.SetTypeId(Igmpv2Ping::GetTypeId());
    }
  else
    {
      m_factory.SetTypeId(Igmpv2Pong::GetTypeId());
    }

  m_factory.Set("Local", Ipv4AddressValue(address));
}

Igmpv2Helper::Igmpv2Helper(bool isPing, Ipv4Address address, Ipv4Address groupAddress)
{
  if(isPing)
    {
      m_factory.SetTypeId(Igmpv2Ping::GetTypeId());
      m_factory.Set("Group", Ipv4AddressValue(groupAddress));
    }
  else
    {
      m_factory.SetTypeId(Igmpv2Pong::GetTypeId());
    }

  m_factory.Set("Local", Ipv4AddressValue(address));
}

void Igmpv2Helper::SetAttribute(std::string name, const AttributeValue &value)
{
  m_factory.Set(name, value);
}

ApplicationContainer Igmpv2Helper::InstallPing(Ptr<Node> node) const
{
  return ApplicationContainer(InstallPrivPing(node));
}

Ptr<Application> Igmpv2Helper::InstallPrivPing(Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Igmpv2Ping>();
  node->AddApplication(app);

  return app;
}

ApplicationContainer Igmpv2Helper::InstallPong(Ptr<Node> node) const
{
  return ApplicationContainer(InstallPrivPong(node));
}

Ptr<Application> Igmpv2Helper::InstallPrivPong(Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Igmpv2Pong>();
  node->AddApplication(app);

  return app;
}

} // namespace ns3
