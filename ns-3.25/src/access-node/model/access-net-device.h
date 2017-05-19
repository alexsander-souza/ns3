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
#ifndef __ACCESS_NET_DEVICE_H__
#define __ACCESS_NET_DEVICE_H__

#include "ns3/bridged-net-device.h"
#include "ns3/access-channel.h"
#include "ns3/mac48-address.h"
#include "ns3/data-rate.h"
#include "ns3/access-profile.h"
#include "ns3/mcast-profile.h"
#include "ns3/access-port.h"

namespace ns3
{
class AncpAnAgent;
class AccessPort;
class Packet;
class Socket;

class AccessNetDevice : public BridgedNetDevice
{
public:
/**
 * \brief Get the type ID.
 * \return the object TypeId
 */
  static TypeId GetTypeId(void);

  AccessNetDevice();

  virtual ~AccessNetDevice();

/**
 * \brief Set Uplink port
 */
  void SetUplinkPort(Ptr< NetDevice > uplink);

/*
 * \brief Add access port
 */
  void AddAccessPort(Ptr< NetDevice > port);

/**
 * \brief Get ANCP agent pointer
 */
  Ptr<AncpAnAgent> GetAncpAgent();

/**
 * \brief Assign an ANCP agent to this AN
 */
  void SetAncpAgent(Ptr<AncpAnAgent> agent);

/**
 * \brief Create an Access Profile
 * \param profile          Access Profile
 */
  void AddAccessProfile(Ptr<AccessProfile>profile);

/**
 * Inherited from BridgedNetDevice
 */
  virtual uint32_t GetNBridgePorts(void) const;
  virtual Ptr<NetDevice> GetBridgePort(uint32_t n) const;

  void StartIpServices();

/**
 * Inherited from NetDevice
 */
  virtual void SetIfIndex(const uint32_t index);
  virtual uint32_t GetIfIndex(void) const;
  virtual Ptr<Channel> GetChannel(void) const;
  virtual void SetAddress(Address address);
  virtual Address GetAddress(void) const;
  virtual bool SetMtu(const uint16_t mtu);
  virtual uint16_t GetMtu(void) const;
  virtual bool IsLinkUp(void) const;
  virtual void AddLinkChangeCallback(Callback<void> callback);
  virtual bool IsBroadcast(void) const;
  virtual Address GetBroadcast(void) const;
  virtual bool IsMulticast(void) const;
  virtual Address GetMulticast(Ipv4Address multicastGroup) const;
  virtual Address GetMulticast(Ipv6Address addr) const;
  virtual bool IsPointToPoint(void) const;
  virtual bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual bool SendFrom(Ptr<Packet> packet, const Address& source,
                        const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode(void) const;
  virtual void SetNode(Ptr<Node> node);
  virtual bool NeedsArp(void) const;
  virtual void SetReceiveCallback(ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom(void) const;

protected:
  virtual void DoDispose(void);

  void LearnAccessPort(Mac48Address &source, Ptr<NetDevice> port, Ptr<const Packet> pkt);

  Ptr<NetDevice> SelectAccessPortForMac(Mac48Address &source, Ptr<const Packet> pkt);

private:

  void ReceiveFromAccessPort(Ptr<NetDevice> device, Ptr<const Packet> packet,
                             uint16_t protocol, Address const &source,
                             Address const &destination, PacketType packetType);

  void ReceiveFromUplinkPort(Ptr<NetDevice> device, Ptr<const Packet> packet,
                             uint16_t protocol, Address const &source,
                             Address const &destination, PacketType packetType);

  void ForwardUnicast(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst);

  void ForwardMulticast(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst);

  void ForwardBroadcast(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst);

  void ForwardUpstream(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src, Mac48Address &dst);

  void DoMulticastSnooping(Ptr<const Packet> packet, uint16_t protocol, Mac48Address &src);

  void SendIgmpJoinUpstream(const Address& group, const Mac48Address &src);
  void SendIgmpLeaveUpstream(const Address& group, const Mac48Address &src);
  void SendIgmpLeaveUpstream(const Address& group);

  int HandleLineConfig(const std::string &circuitId, const std::string &profileName);
  int HandleMCastConfig(const std::string &circuitId, const std::string &profileName);

  int HandleMCastProfile(const std::string&prof_name, const std::list<Address>&whitelist,
                         const std::list<Address>&greylist, const std::list<Address>&blacklist,
                         bool doWhitelistCac, bool doMRepCtlCac);

  int HandleMCastCommand(const std::string&circuitId, int oper, const Address &group);

  void HandleMCastQueryTimer(void);

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;

  Time m_portMapTtl;
  std::map<Mac48Address, AccessPort > m_portMap;

  Ptr< NetDevice > m_uplink;
  std::vector< Ptr<NetDevice> > m_access_ports;

  Ptr< AccessChannel > m_channel;
  Ptr<Node> m_node;
  Mac48Address m_address;
  uint32_t m_ifIndex;
  uint16_t m_mtu;

  Ptr<AncpAnAgent> m_agent;

  std::map<std::string, Ptr<AccessProfile> > m_accessProfiles;
  std::map<std::string, Ptr<MCastProfile> > m_mcastProfiles;

  EventId m_mcastQueryTimer;

  std::map<Address, std::set<Mac48Address> > m_mcastActiveGroups;
  Ptr < Socket > m_sock_igmp;     //!< IGMP socket
};
}
#endif /* __ACCESS_NET_DEVICE_H__ */
