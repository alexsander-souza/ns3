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
#ifndef BNG_HELPER_H
#define BNG_HELPER_H

#include <string>
#include <list>
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"
#include "ns3/bng-control.h"

namespace ns3 {
class BngHelper {
public:

  /**
   * Create an BNG helper
   */
  BngHelper(void);

  ApplicationContainer InstallBngControl(Ptr<Node>bngNode,
                                         Ptr<NetDevice>aggr_netdev,
                                         Ptr<NetDevice>reg_netdev) const;

  void SetAttribute(std::string name,
                    const AttributeValue& value);

  void AddServiceProfile(const Mac48Address& circuitId,
                         const std::string& accessProfileName,
                         const std::string& mcastProfileName);

  void SetupMulticastProfile(const struct BngControl::McastProfile& profile);

private:

  ObjectFactory m_bng_factory;                     // !< BNG factory
  ObjectFactory m_ancp_factory;                    // !< ANCP agent factory
  ObjectFactory m_dhcp_factory;                    // !< DHCP server factory
  ObjectFactory m_radius_factory;                  // !< RADIUS client factory
  std::map<Mac48Address, std::string> m_accessProfileMap;   // !< Service profiles
  std::map<Mac48Address, std::string> m_mcastProfileMap;   // !< Service profiles
  std::list<struct BngControl::McastProfile> m_mcastProfiles;
};
}
#endif /* BNG_HELPER_H */
