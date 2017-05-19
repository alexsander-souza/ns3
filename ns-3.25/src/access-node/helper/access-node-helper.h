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
#ifndef ACCESS_NODE_HELPER_H
#define ACCESS_NODE_HELPER_H

#include <string>
#include <list>
#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"
#include "ns3/access-net-device.h"

namespace ns3
{
class AccessNodeHelper
{
public:
  /**
   * Create an AccessNode helper
   */
  AccessNodeHelper(void);

  NetDeviceContainer CreateAccessNodeDevice(Ptr<Node>anNode,
                                            NetDeviceContainer &accessPorts,
                                            Ptr<NetDevice>uplink);

  ApplicationContainer InstallAccessNodeControl(Ptr<Node>anNode,
                                                Ipv4Address nasAddr);

  void SetDeviceAttribute(std::string n1, const AttributeValue &v1);

  void SetupServiceProfile(Ptr<AccessProfile> profile);

private:
  Ptr<AccessNetDevice> GetAnNetDevice(Ptr<Node>anNode) const;

  ObjectFactory m_an_factory;                         //!< Access Node factory
  ObjectFactory m_ancp_factory;                       //!< ANCP agent factory
  std::list<Ptr<AccessProfile> > m_profiles;          //!< Service profiles
};
}
#endif /* ACCESS_NODE_HELPER_H */
