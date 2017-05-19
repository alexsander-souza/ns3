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
#ifndef __ACCESS_PORT_H__
#define __ACCESS_PORT_H__

#include <map>
#include "ns3/address.h"
#include "ns3/nstime.h"

namespace ns3
{
class NetDevice;
class AccessNetDevice;
class AccessProfile;
class Packet;

class AccessPort
{
public:
  static const uint QUERY_INTERVAL;   /*!< IGMP Query interval */

  enum PacketDir {
    DOWNSTREAM,
    UPSTREAM
  };
  AccessPort();
  ~AccessPort();

  void SetTtl(Time ttl);
  Time GetTtl() const;

  void SetShowTime(bool enabled);
  bool GetShowTime() const;

  void SetAccessPort(Ptr<AccessNetDevice>anDev,
                     Ptr<NetDevice>accessDev,
                     const Mac48Address &source);
  Ptr<NetDevice>GetAccessPort() const;

  int ApplyProfile(Ptr<AccessProfile>prof);

  int ApplyProfile(Ptr<MCastProfile>prof);

  const std::string GetAccessProfileName() const;
  const std::string GetMcastProfileName() const;

  void AccPacket(enum PacketDir dir, Ptr<const Packet>pkt);

  void NasInitiatedMulticastGroup(const Address &mcGroup, bool allow);
  bool EnterMulticastGroup(const Address &mcGroup);
  bool LeaveMulticastGroup(const Address &mcGroup);
  bool IsMember(const Address &mcGroup) const;
  void CleanupMulticastMembership();

private:
  void LinkStateChangedHandler(void);

  Ptr<AccessNetDevice>m_accessNode;
  Ptr<NetDevice>m_port;
  Time m_ttl;
  bool m_showtime;
  std::string m_circuitId;
  DataRate m_attainableRate;

  uint64_t m_usBytes;
  uint64_t m_usPkts;
  uint64_t m_dsBytes;
  uint64_t m_dsPkts;

  Ptr<AccessProfile> m_access_prof;
  Ptr<MCastProfile> m_mcast_prof;

  std::map < Address, Time > m_mcastGroups;
};
} //  namespace ns3
#endif /* __ACCESS_PORT_H__ */
