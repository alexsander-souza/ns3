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

#include "ns3/simulator.h"
#include "ns3/packet.h"

#include "delayed-bridge-net-device.h"

namespace ns3
{

PacketTime::PacketTime(Ptr<const Packet> packet, Time& timestamp)
  : packet(packet), timestamp(timestamp)
{ }

PacketTime::PacketTime(Ptr<NetDevice> incomingPort, Ptr<const Packet> packet,
                       uint16_t protocol, Mac48Address src, Mac48Address dst, Time& timestamp)
  : incomingPort(incomingPort), packet(packet), protocol(protocol),
    src(src), dst(dst), timestamp(timestamp)
{ }

Ptr<NetDevice> PacketTime::GetIncomingPort() const
{
  return incomingPort;
}

Ptr<const Packet> PacketTime::GetPacket() const
{
  return packet;
}

uint16_t PacketTime::GetProtocol(void) const
{
  return protocol;
}

Mac48Address PacketTime::GetSrc(void) const
{
  return src;
}

Mac48Address PacketTime::GetDst(void) const
{
  return dst;
}

Time PacketTime::GetTimestamp(void) const
{
  return timestamp;
}

bool operator<(const PacketTime& pt1, const PacketTime& pt2)
{
  return (pt1.GetTimestamp() < pt2.GetTimestamp()) ? true : false;
}

} //namespace ns3