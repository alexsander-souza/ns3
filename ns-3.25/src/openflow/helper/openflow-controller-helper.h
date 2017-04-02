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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *
 */

#ifndef OPENFLOW_CONTROLLER_HELPER_H
#define OPENFLOW_CONTROLLER_HELPER_H

#include "ns3/application-container.h"
#include "ns3/object-factory.h"

namespace ns3 {
/**
 * \brief create an openflow controller service
 */

class OpenFlowControllerHelper {
public:

  /**
   * Create OpenFlowControllerHelper
   */
  OpenFlowControllerHelper(void);

  /**
   * Helper function used to set the underlying application attributes.
   *
   * \param name    the name of the attribute to set
   * \param value   the value of the attribute to set
   */
  void SetAttribute(std::string           name,
                    const AttributeValue& value);

  /**
   * This method creates an controller application using the openflow protocol
   * to communicate with openflow switches.
   *
   * \param node The node to install the application in
   * \returns A container holding the added application.
   */
  ApplicationContainer Install(Ptr<Node>node) const;

private:

  ObjectFactory m_factory;
};
} // namespace ns3
#endif  /* OPENFLOW_CONTROLLER_HELPER_H */
