/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2011 UPB, 2014 UFRGS
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 * Contributor(s):
 *  Alexsander de Souza <asouza@inf.ufrgs.br>
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *  Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 *
 */

#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include <map>
#include "ns3/application.h"
#include "ns3/simple-ref-count.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "ns3/traced-value.h"

#include "ns3/abstract-dhcp-server.h"

namespace ns3 {
class Socket;
class Packet;
class NetDevice;
class DhcpHeader;

/**
 * \ingroup dhcpclientserver
 * \class DhcpServer
 * \brief A Dhcp server.
 */
class DhcpServer : public Application,
                   public AbstractDhcpServer {
public:

  /* ns3::Object method */
  static TypeId GetTypeId(void);

  /**
   * Default constructor
   */
  DhcpServer();

  /**
   * Default destructor
   */
  virtual
  ~DhcpServer();

protected:

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:

  /**
   * Create server socket based on the Lease Pool network
   */
  void CreateServerSocket(void);

  void NetHandler(Ptr<Socket>socket);

  /**
   * Send response to client
   * \param client                   Client HW address
   * \param response                 DHCP response
   */
  void SendMessage(Mac48Address& clientId,
                   DhcpHeader  & response);

  Ptr<Socket> m_sock_pkt; /**< AF_PACKET-like socket */
  Ptr<Socket> m_sock_udp; /**< AF_INET UDP socket */

  Ptr<NetDevice> m_netdev;
};
} // namespace ns3

#endif /* DHCP_SERVER_H */
