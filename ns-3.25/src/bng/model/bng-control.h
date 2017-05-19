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
#ifndef __BNG_NET_DEVICE_H__
#define __BNG_NET_DEVICE_H__

#include <list>
#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/mac48-address.h"
#include "ns3/data-rate.h"
#include "ns3/packet.h"
#include "ns3/net-device.h"
#include <ns3/event-id.h>
#include "ns3/bng-session.h"

namespace ns3 {
class AncpNasAgent;
class DhcpServer;
class RadiusClient;
class Packet;
class Ipv4Interface;
class Socket;

class BngControl : public Application {
public:

  struct McastProfile {
    std::string name;
    std::list<Address> whitelist;
    std::list<Address> greylist;
    std::list<Address> blacklist;
  };

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);

  BngControl();

  virtual ~BngControl();

  /**
   * \brief Get ANCP agent pointer
   */
  Ptr<AncpNasAgent> GetAncpAgent();

  /**
   * \brief Assign an ANCP agent to this AN
   */
  void                  SetAncpAgent(Ptr<AncpNasAgent> agent);

  /**
   * \brief Assign a RADIUS client to this AN
   */
  void                  SetRadiusClient(Ptr<RadiusClient> radClient);

  /**
   * \brief Assign a DHCP server to this AN
   */
  void                  SetDhcpServer(Ptr<DhcpServer> dhcpSrv);

  /**
   * \brief Set Regional domain facing port
   */
  void SetRegionalNetPort(Ptr<NetDevice> regionalNetPort);

  /*
   * \brief Set access domain facing port
   */
  void SetAccessNetPort(Ptr<NetDevice> accessNetPort);

  /* ANCP callbacks */
  int SubscriberPortUp(const Mac48Address &anId, const std::string &circuit_id,
                       uint32_t rate_up, uint32_t rate_down, uint32_t tag);

  int SubscriberPortDown(const Mac48Address &anId, const std::string &circuit_id);

  int SubscriberMRepCtlHandler(const Mac48Address &anId, const std::string &circuit_id,
                               const Address&group, bool join);
  int DoAnProvisioning(const Mac48Address &anId);

  /* DHCP callbacks */
  int SubscriberIpSessionSetup(const Mac48Address &cpeHwId, const Address &ip);
  int SubscriberIpSessionTearDown(const Mac48Address &cpeHwId, const Address &ip);

  /* Profile management */
  void AddBandwidthProfile(const Mac48Address &cpeHwId, const std::string &profName);
  void AddMcastProfile(const Mac48Address &cpeHwId, const std::string &profName);
  void SetupMcastProfile(const struct McastProfile &profile);

protected:

  virtual void DoDispose(void);

private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  void ActivityMonitor(Ptr<NetDevice> device, Ptr<const Packet> packet,
                       uint16_t protocol, Address const &source,
                       Address const &destination, NetDevice::PacketType packetType);

  void IdleSessionSweep();
  Ipv4Address discoverLocalAddress(Ptr<NetDevice> device);
  void HandleReadIgmp(Ptr<Socket> socket);
  void SendIgmpJoinUpstream(const Address& group, const Address &src);
  void SendIgmpLeaveUpstream(const Address& group, const Address &src);

  Ptr<NetDevice> m_regionalNetPort;
  Ptr<NetDevice> m_accessNetPort;

  Mac48Address m_address;

  Ptr<AncpNasAgent> m_agent;
  Ptr<RadiusClient> m_radClient;
  Ptr<DhcpServer> m_dhcp;

  std::map<Mac48Address, SubscriberSession> m_sessionMap;
  std::map<Mac48Address, std::string> m_accessProfileMap;
  std::map<Mac48Address, std::string> m_mcastProfileMap;
  std::map<std::string, struct McastProfile> m_mcastProfiles;

  EventId m_sessionSweepEvent;
  Time m_sessionTimeout;

  Ptr<Socket> m_sock_igmp_up;
  Ptr<Socket> m_sock_igmp_down;
  std::map<Address, std::set<Address> > m_mcastActiveGroups;
};
}
#endif /* __BNG_NET_DEVICE_H__ */
