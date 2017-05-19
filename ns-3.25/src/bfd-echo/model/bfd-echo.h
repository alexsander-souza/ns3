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
#ifndef BFD_ECHO_H
#define BFD_ECHO_H
#include <ns3/application.h>
#include <ns3/event-id.h>
#include "ns3/address.h"
#include "ns3/mac48-address.h"

namespace ns3 {
class Socket;
class Packet;
class NetDevice;

class BfdEchoAgent : public Application
{
public:
  static TypeId GetTypeId(void);

  BfdEchoAgent();

  virtual ~BfdEchoAgent();

  virtual void Print(std::ostream& os) const;

  bool IsUp() const;
protected:
  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:
  enum {
    BFD_ECHO_PORT = 3785
  };

  void LinkStateChangedHandler(void);

  void CreateServerSocket(void);

  void BuildEchoPacket(void);

  void NetHandler(Ptr<Socket> socket);

  void SendEcho();

  bool discoverLocalAddress();

  Ptr<NetDevice>           m_netdev;               /**< Binded netdev */
  Ptr<Socket>              m_sock_pkt;             /**< AF_PACKET-like socket */
  Ptr<Socket>              m_sock_udp;             /**< AF_INET UDP socket */

  Ipv4Address m_myAddress;
  Mac48Address m_gwHwAddress;
  Ptr<Packet>  m_echoPacket;

  EventId m_pingEvent;
  Time m_pingInterval;
  bool m_echo_prepared;
};
}
#endif /* BFD_ECHO_H */
