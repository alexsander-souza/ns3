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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#ifdef NS3_OPENFLOW

#include <ns3/controller.h>
#include <ns3/controller-application.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/vertex-host.h>
#include <ns3/object-base.h>

NS_LOG_COMPONENT_DEFINE("ControllerApplication");

namespace ns3 {
TypeId
ControllerApplication::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ControllerApplication")
                      .SetParent<Object>()
                      .AddConstructor<ControllerApplication>()
  ;

  return tid;
}

ControllerApplication::ControllerApplication()
{
  NS_LOG_FUNCTION_NOARGS();
}

ControllerApplication::~ControllerApplication()
{
  NS_LOG_FUNCTION_NOARGS();
}

bool
ControllerApplication::ReceiveFromSwitch(Ptr<OfSwitch>origin,
                                         unsigned portIn, unsigned bufferId,
                                         Ptr<Packet>packet)
{
  NS_LOG_FUNCTION(this);
  return false; /* not handled */
}

void
ControllerApplication::InitSwitch(Ptr<OfSwitch>origin)
{
  NS_LOG_FUNCTION(this << origin);
}

void
ControllerApplication::HostInstalled(Ptr<HostVertex>host)
{
  NS_LOG_FUNCTION(this << *host);
}

void
ControllerApplication::HostUninstalled(Ptr<HostVertex>host)
{
  NS_LOG_FUNCTION(this << *host);
}

void
ControllerApplication::SyncCompleted(int statedId)
{
  NS_LOG_FUNCTION(this << statedId);
}

void
ControllerApplication::AddApplicationToController(Ptr<Node>controllerNode)
{
  NS_LOG_FUNCTION(this);

  TypeId tid = Controller::GetTypeId();
  Ptr<Application> controllerApp;

  for (uint32_t i = 0; i < controllerNode->GetNApplications(); ++i)
  {
    if (tid == controllerNode->GetApplication(i)->GetInstanceTypeId())
    {
      controllerApp = controllerNode->GetApplication(i);
      break;
    }
  }

  m_controller = DynamicCast<Controller>(controllerApp);
  m_controller->AddApplication(this);
}

void ControllerApplication::SetController(Ptr<Controller> controller)
{
  m_controller = controller;
}

void
ControllerApplication::InitMe()
{
  NS_LOG_FUNCTION(this);
}

int
ControllerApplication::GetPriority() const
{
  NS_LOG_FUNCTION(this << " using default priority");
  return 500;
}

bool
operator<(const ControllerApplication& app1,
          const ControllerApplication& app2)
{
  return app1.GetPriority() < app2.GetPriority();
}
} // namespace ns3
#endif // NS3_OPENFLOW
