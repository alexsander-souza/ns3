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

#ifndef CONTROLLER_APPLICATION_H
#define CONTROLLER_APPLICATION_H

#include <ns3/object.h>
#include <ns3/address.h>
#include <ns3/packet.h>

namespace ns3 {
class Address;
class Controller;
class Node;
class Packet;
class OfSwitch;
class HostVertex;

/**
 * \ingroup openflow
 * \class ControllerApplication
 * \brief A controller application virtual class. It must be extended by
 *        each class that implements an controller application so it can be
 *        added to the list of applications that run on a openflow controller.
 */
class ControllerApplication : public Object {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  ControllerApplication();

  /**
   * \brief Destructor.
   */
  virtual ~ControllerApplication();

  /**
   * \brief When the controller receives a Packet In message, this method is
   *        called to forward the packet and the address of the switch that
   *        sent this packet to this application.
   * \param origin      Switch that sent this packet
   * \param packet      Copy of the openflow packet received by this controller
   * \return true if the packet was consumed, and the controller should stop
   *              processing it.
   */
  virtual bool ReceiveFromSwitch(Ptr<OfSwitch>origin,
                                 unsigned     portIn,
                                 unsigned     bufferId,
                                 Ptr<Packet>  packet);

  void AddApplicationToController(Ptr<Node>controllerNode);

  void SetController(Ptr<Controller> controller);

  virtual void InitMe();

  virtual void InitSwitch(Ptr<OfSwitch>origin);

  virtual void HostInstalled(Ptr<HostVertex>host);

  virtual void HostUninstalled(Ptr<HostVertex>host);

  virtual void SyncCompleted(int statedId);

  friend bool operator<(const ControllerApplication& app1,
                        const ControllerApplication& app2);

protected:

  /**
   * \brief Get application priority, used to order the processing queue
   * lower values means higher precedence.
   */
  virtual int GetPriority() const;

  Ptr<Controller> m_controller;
};
} // namespace ns3
#endif  /* CONTROLLER_APPLICATION_H */
