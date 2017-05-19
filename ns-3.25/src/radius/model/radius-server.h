/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Alexsander de Souza
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

#ifndef __RADIUS_SERVER_H__
#define __RADIUS_SERVER_H__

#include <queue>
#include <ns3/callback.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ptr.h>
#include <ns3/radius-db.h>

namespace ns3
{

class Socket;
class Packet;

class RadiusServer : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor.
   */
  RadiusServer (void);

  /**
   * \brief Destructor.
   */
  virtual ~RadiusServer (void);

  void SetDatabase(Ptr<RadiusDB> database);

protected:
  /**
   * \brief Dispose the instance.
   */
  virtual void DoDispose ();

private:
  typedef std::queue<EventId> RadiusPendingRequests;

  /**
    * \brief Start the application.
    */
  virtual void StartApplication ();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication ();

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  void HandleAuthentication (Ptr<Socket> socket);

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  void HandleAccounting (Ptr<Socket> socket);

  void ScheduleTransmit (Time dt, Address client, RadiusMessage &response);

  void Send(Address client, RadiusMessage &response);

  Ptr<RadiusDB>                   m_rad_db;
  Ptr<Socket>                     m_auth_sock;       //!< Auth socket
  Ptr<Socket>                     m_acc_sock;        //!< Account socket
  uint16_t                        m_srv_auth_port;
  uint16_t                        m_srv_acc_port;
  std::string                     m_server_secret;
  uint64_t                        m_server_delay;
  RadiusPendingRequests           m_peding_reqs;
};

}

#endif /* __RADIUS_SERVER_H__ */
