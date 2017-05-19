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

#ifndef __RADIUS_CLIENT_H__
#define __RADIUS_CLIENT_H__

#include <ns3/callback.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ptr.h>
#include <ns3/radius-header.h>

namespace ns3
{
class Socket;
class Packet;

class RadiusClient : public Application
{
public:
  typedef Callback<int, uint8_t,
                   RadiusMessage::RadiusAvpList > RadiusRequestCompletedCb;

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  RadiusClient (void);

  /**
   * \brief Destructor.
   */
  virtual ~RadiusClient (void);

  void SetRequestDoneCb(RadiusRequestCompletedCb cb);

  int SendRequest(uint8_t code, RadiusMessage::RadiusAvpList &avp_list);

  int DoAuthentication(const std::string& username, const std::string& passwd,
                       const std::string& nas_id, uint32_t nas_port);

  int DoStartAccounting(uint32_t acc_event, const std::string& session_id,
                        const std::string& username);

  int DoStopAccounting(uint32_t acc_event, const std::string& session_id,
                       uint32_t session_time, uint32_t termination_cause);

protected:
  /**
   * \brief Dispose the instance.
   */
  virtual void DoDispose();

private:
  typedef std::map<uint8_t, RadiusMessage> RadiusClientPendingList;
  typedef std::map<uint8_t, RadiusMessage>::iterator RadiusClientPendingListIT;

  /**
   * \brief Start the application.
   */
  virtual void StartApplication();

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication();

  void NetHandler(Ptr<Socket> socket);

  Ptr<Socket>                     m_socket;
  Ipv4Address m_server_addr;
  uint16_t m_server_port;
  uint16_t m_server_acc_port;
  std::string m_server_secret;
  uint8_t m_request_counter;
  RadiusClientPendingList m_pending_reqs;
  RadiusRequestCompletedCb m_cb_request_completed;
};
}
#endif  /* __RADIUS_CLIENT_H__ */
