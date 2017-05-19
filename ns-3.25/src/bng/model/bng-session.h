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
#ifndef __BNG_SESSION_H__
#define __BNG_SESSION_H__

#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/mac48-address.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"

namespace ns3 {
class RadiusClient;

class SubscriberSession {
public:
  enum PacketDir {
    DOWNSTREAM,
    UPSTREAM
  };
  SubscriberSession(const Mac48Address &anId, const std::string& circuitId, Ptr<RadiusClient> radClient);
  ~SubscriberSession();

  void SetAddressIp(const Address& ip4);

  Ipv4Address GetIpv4Address() const;
  Ipv6Address GetIpv6Address() const;
  Mac48Address GetAnAddress() const;

  bool IsActive() const;

  void UpdateLineStatus(bool active, uint32_t upRate, uint32_t downRate);

  void AccPacket(enum PacketDir dir, Ptr<const Packet> pkt);

  Time GetUptime() const;
  Time GetIdletime() const;

  const std::string &GetSessionId() const;
  const std::string &GetCircuitId() const;

  bool operator ==(const Ipv4Address &dst4) const;
  bool operator ==(const Ipv6Address &dst6) const;

  static std::string GenerateId(const Mac48Address &anId,
                                const std::string &circuitId);

private:
  std::string m_sessionId;
  Mac48Address m_anId;                  /**< AN hardware address */
  Ipv4Address m_address4;               /**< Client designated IPv4 address */
  Ipv6Address m_address6;               /**< Client designated IPv6 address */
  bool m_active;
  std::string m_circuitId;
  uint32_t m_upRate;
  uint32_t m_downRate;
  Time m_startTime;
  Time m_lastSeen;
  Ptr<RadiusClient> m_radiusClient;
};
} /* namespace ns3  */
#endif /* __BNG_SESSION_H__ */
