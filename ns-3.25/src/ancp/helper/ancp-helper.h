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

#ifndef ANCP_HELPER_H
#define ANCP_HELPER_H

#include <string>
#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"
#include "ns3/ancp-module.h"

namespace ns3
{
/**
 * \ingroup ANCP
 * \brief A helper to make it easier to instantiate a ns3::AncpAnAgent or
 *        ns3::AncpNasAgent on a set of nodes.
 */
class AncpHelper
{
public:
  /**
   * Create a ANCP agent helper
   */
  AncpHelper(void);

  void SetNasAttribute(std::string name, const AttributeValue &value);

  void SetAnAttribute(std::string name, const AttributeValue &value);

  /**
   * Install an ns3::AncpAnAgent on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an AncpAnAgent
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallAccessNode(NodeContainer c) const;

  /**
   * Install an ns3::AncpAnAgent on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node The node on which an AncpAnAgent will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallAccessNode(Ptr<Node> node) const;

  /**
   * Install an ns3::AncpNasAgent on the node configured with all the
   * attributes set with SetAttribute.
   *
   * \param node       The node on which an AncpNasAgent will be installed.
   * \param nas_addr   IPv4 address used by ANCP
   *
   * \returns Container of Ptr to the applications installed.
   */
  ApplicationContainer InstallNas(Ptr<Node> node, Ipv4Address nas_addr);

private:
  Ptr<Application> AttachClient(Ptr<Node> node) const;

  Ipv4Address m_nas_addr;
  ObjectFactory m_nas_factory;     //!< Object factory.
  ObjectFactory m_an_factory;      //!< Object factory.
};
}
#endif /* ANCP_HELPER_H */
