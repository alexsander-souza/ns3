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

#ifndef ARPING_H
#define ARPING_H

#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/application.h>
#include <ns3/ipv4-address.h>


namespace ns3 {
class Socket;
class NetDevice;

class Arping : public Application {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  Arping();

  /**
   * \brief Destructor.
   */
  ~Arping();

protected:

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:

  void GratuitousArpSender();

  Ipv4Address    m_advAddress;
  Ptr<NetDevice> m_netdev;
  Ptr<Socket>    m_sock_pkt;
  EventId m_arpEvent;
  Time    m_arpInterval;
};
}
#endif // ifndef ARPING_H
