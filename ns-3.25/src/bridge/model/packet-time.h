/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 */

#ifndef PACKET_TIME_H
#define PACKET_TIME_H

#include "ns3/net-device.h"
#include "ns3/mac48-address.h"

#include <iostream>
#include <string>
#include <set>

#include <ns3/nstime.h>

namespace ns3
{

class PacketTime
{
private:
  Ptr<NetDevice> incomingPort;
  Ptr<const Packet> packet;
  uint16_t protocol;
  Mac48Address src;
  Mac48Address dst;
  Time timestamp;

public:
  PacketTime(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
             uint16_t protocol, Mac48Address src, Mac48Address dst, Time& timestamp);
  PacketTime(Ptr<const Packet> packet, Time& timestamp);

  Ptr<NetDevice> GetIncomingPort() const;
  Ptr<const Packet> GetPacket() const;
  uint16_t GetProtocol(void) const;
  Mac48Address GetSrc(void) const;
  Mac48Address GetDst(void) const;
  Time GetTimestamp(void) const;

  friend bool operator<(const PacketTime&, const PacketTime&);
};

} //namespace ns3

#endif /* PACKET_TIME_H */