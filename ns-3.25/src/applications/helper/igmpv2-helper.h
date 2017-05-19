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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br
 *
 */

#ifndef IGMPV2_HELPER_H
#define IGMPV2_HELPER_H

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

namespace ns3
{

/**
* \brief create an IGMPv2 service
*/
class Igmpv2Helper
{
public:

  /**
  * Create Igmpv2Helper
  */
  Igmpv2Helper(bool isPing);
  Igmpv2Helper(bool isPing, Ipv4Address address);
  Igmpv2Helper(bool isPing, Ipv4Address address, Ipv4Address groupAddress);
  void SetAttribute(std::string name, const AttributeValue &value);
  ApplicationContainer InstallPing(Ptr<Node> node) const;
  ApplicationContainer InstallPong(Ptr<Node> node) const;

private:

  Ptr<Application> InstallPrivPing(Ptr<Node> node) const;
  Ptr<Application> InstallPrivPong(Ptr<Node> node) const;
  ObjectFactory m_factory;
};

} // namespace ns3

#endif /* IGMPV2_HELPER_H */
