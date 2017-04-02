/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2011 Blake Hurd, 2014 UFRGS
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
 * Author: Blake Hurd  <naimorai@gmail.com>
 * Contributor(s):
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 */
#ifndef OPENFLOW_SWITCH_HELPER_H
#define OPENFLOW_SWITCH_HELPER_H

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/application-container.h"
#include "ns3/object-factory.h"
#include <string>

namespace ns3 {
class AttributeValue;
class Controller;

/**
 * \brief Add capability to switch multiple LAN segments (IEEE 802.1D bridging)
 */
class OpenFlowSwitchNetDeviceHelper {
public:

  /*
   * Construct a OpenFlowSwitchNetDeviceHelper
   */
  OpenFlowSwitchNetDeviceHelper();

  void SetDeviceAttribute(std::string           n1,
                          const AttributeValue& v1);

  void SetClientAttribute(std::string           n1,
                          const AttributeValue& v1);

  ApplicationContainer Install(Ptr<Node>           node,
                               NetDeviceContainer& exclude);

  ApplicationContainer Install(NodeContainer     & c,
                               NetDeviceContainer& exclude);

  void SetControllerAddress(const Ipv4Address& address);

private:

  bool IsExcluded(Ptr<NetDevice>      dev,
                  NetDeviceContainer& exclude) const;

  ObjectFactory m_deviceFactory;
  ObjectFactory m_clientFactory;
  Ipv4Address   m_ctrl_address;
};
} // namespace ns3
#endif /* OPENFLOW_SWITCH_HELPER_H */
