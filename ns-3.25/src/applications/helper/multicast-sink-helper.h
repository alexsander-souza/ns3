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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br
 *
 */

#ifndef MULTICAST_SINK_HELPER_H
#define MULTICAST_SINK_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"

namespace ns3
{
/**
 * \brief A helper to make it easier to instantiate an ns3::MulticastSink
 * on a set of nodes.
 */
class MulticastSinkHelper
{
public:

  /**
   * Create a MulticastSinkHelper to make it easier to work with MulticastSink Applications
   *
   * \param group the address of the multicast group
   */
  MulticastSinkHelper(Ipv4Address group);

  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name the name of the application attribute to set
   * \param value the value of the application attribute to set
   */
  void SetAttribute(std::string name, const AttributeValue &value);

  /**
   * Install an ns3::MulticastSink on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which a MulticastSink
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install(NodeContainer c) const;

  /**
   * Install an ns3::MulticastSink on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param node The node on which a MulticastSink will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install(Ptr < Node > node) const;

  /**
   * Install an ns3::MulticastSink on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param nodeName The name of the node on which a MulticastSink will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer Install(std::string nodeName) const;

private:

  /**
   * Install an ns3::MulticastSink on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an MulticastSink will be installed.
   * \returns Ptr to the application installed.
   */
  Ptr < Application > InstallPriv(Ptr < Node > node) const;
  ObjectFactory m_factory;   //!< Object factory.
};
} // namespace ns3
#endif /* MULTICAST_SINK_HELPER_H */
