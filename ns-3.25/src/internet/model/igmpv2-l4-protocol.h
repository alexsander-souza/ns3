/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 */

#ifndef Igmpv2_L4_PROTOCOL_H
#define Igmpv2_L4_PROTOCOL_H

#include "ip-l4-protocol.h"
#include "igmpv2.h"
#include "ns3/ipv4-address.h"

namespace ns3
{

class Node;
class Ipv4Interface;
class Ipv4Route;

class Igmpv2L4Protocol : public IpL4Protocol
{
public:

  /**
  * \brief Get the type ID.
  * \return the object TypeId
  */
  static TypeId GetTypeId(void);
  static const uint8_t PROT_NUMBER; //!< IGMPv2 protocol number (0x2)

  Igmpv2L4Protocol();
  virtual ~Igmpv2L4Protocol();

  /**
  * \brief Set the node the protocol is associated with.
  * \param node the node
  */
  void SetNode(Ptr<Node> node);

  /**
  * Get the protocol number
  * \returns the protocol number
  */
  static uint16_t GetStaticProtocolNumber(void);

  /**
  * Get the protocol number
  * \returns the protocol number
  */
  virtual int GetProtocolNumber(void) const;

  /**
  * \brief Receive method.
  * \param p the packet
  * \param header the IPv4 header
  * \param incomingInterface the interface from which the packet is coming
  * \returns the receive status
  */
  virtual enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
      Ipv4Header const &header,
      Ptr<Ipv4Interface> incomingInterface);

  /**
  * \brief Receive method.
  * \param p the packet
  * \param header the IPv6 header
  * \param incomingInterface the interface from which the packet is coming
  * \returns the receive status
  */
  virtual enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
      Ipv6Header const &header,
      Ptr<Ipv6Interface> incomingInterface);

  // From IpL4Protocol
  virtual void SetDownTarget(IpL4Protocol::DownTargetCallback cb);
  virtual void SetDownTarget6(IpL4Protocol::DownTargetCallback6 cb);
  // From IpL4Protocol
  virtual IpL4Protocol::DownTargetCallback GetDownTarget(void) const;
  virtual IpL4Protocol::DownTargetCallback6 GetDownTarget6(void) const;

protected:

  /*
  * This function will notify other components connected to the node that a new stack member is now connected
  * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
  */
  virtual void NotifyNewAggregate();

private:

  /**
  * \brief Handles an incoming IGMPv2 Echo packet
  * \param p the packet
  * \param header the IP header
  * \param source the source address
  * \param destination the destination address
  */
  void HandleMembershipQuery(Ptr<Packet> p,
                             Igmpv2Header header,
                             Ipv4Address source,
                             Ipv4Address destination);
  /**
  * \brief Handles an incoming IGMPv2 Destination Unreachable packet
  * \param p the packet
  * \param header the IP header
  * \param source the source address
  * \param destination the destination address
  */
  void HandleMembershipReportv2(Ptr<Packet> p,
                                Igmpv2Header header,
                                Ipv4Address source,
                                Ipv4Address destination);
  /**
  * \brief Handles an incoming IGMPv2 Time Exceeded packet
  * \param p the packet
  * \param IGMPv2 the IGMPv2 header
  * \param source the source address
  * \param destination the destination address
  */
  void HandleLeaveGroup(Ptr<Packet> p,
                        Igmpv2Header IGMPv2,
                        Ipv4Address source,
                        Ipv4Address destination);

  /**
  * \brief Send a generic IGMPv2 packet
  *
  * \param packet the packet
  * \param dest the destination
  * \param type the IGMPv2 type
  * \param code the IGMPv2 code
  */
  void SendMessage (Ptr<Packet> packet, Ipv4Address dest,
                    uint8_t type, uint8_t maxRespTime, uint16_t checksum,
                    Ipv4Address groupAddress);

  void SendMessage(Ptr<Packet> packet, Ipv4Address source,
                   Ipv4Address dest, uint8_t type, uint8_t maxRespTime,
                   Ipv4Address groupAddress, Ptr<Ipv4Route> route);

  virtual void DoDispose (void);

  Ptr<Node> m_node; //!< the node this protocol is associated with
  IpL4Protocol::DownTargetCallback m_downTarget; //!< callback to Ipv4::Send
};

} // namespace ns3

#endif /* Igmpv2_L4_PROTOCOL_H */
