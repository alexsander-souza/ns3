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

#include "openflow-controller-helper.h"
#include "ns3/controller.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE("OpenFlowControllerHelper");

namespace ns3 {
OpenFlowControllerHelper::OpenFlowControllerHelper(void)
{
  m_factory.SetTypeId(Controller::GetTypeId());
}

void
OpenFlowControllerHelper::SetAttribute(
  std::string           name,
  const AttributeValue& value)
{
  m_factory.Set(name, value);
}

ApplicationContainer
OpenFlowControllerHelper::Install(Ptr<Node>node) const
{
  Ptr<Application> app = m_factory.Create<Controller>();
  node->AddApplication(app);

  return ApplicationContainer(app);
}
} // namespace ns3
