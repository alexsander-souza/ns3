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

#ifndef IGMPV2_H
#define IGMPV2_H

#include "ns3/header.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-address.h"
#include <stdint.h>

namespace ns3
{

class Packet;

class Igmpv2Header : public Header
{
public:
  enum
  {
    MEMBERSHIP_QUERY = 0x11,
    MEMBERSHIP_REPORT_V2 = 0x16,
    LEAVE_GROUP = 0x17
  };

  /**
  * Enables IGMPv2 Checksum calculation
  */
  void EnableChecksum(void);

  /**
  * Set IGMPv2 type
  * \param type the IGMPv2 type
  */
  void SetType(uint8_t type);

  /**
  * Set IGMPv2 max response time
  * \param code the IGMP max response time
  */
  void SetMaxRespTime(uint8_t maxRespTime);

  /**
  * Set IPv4 group address
  * \param IPv4 address of the group
  */
  void SetGroupAddress(Ipv4Address groupAddress);

  /**
  * Get IGMPv2 type
  * \returns the IGMPv2 type
  */
  uint8_t GetType(void) const;

  /**
  * Get IGMPv2 max response time
  * \returns the IGMPv2 max response time
  */
  uint8_t GetMaxRespTime(void) const;

  /**
  * Get IPv4 group address
  * \returns the IPv4 address of the group
  */
  Ipv4Address GetGroupAddress(void) const;

  /**
  * \brief Get the type ID.
  * \return the object TypeId
  */
  static TypeId GetTypeId(void);
  Igmpv2Header();
  virtual ~Igmpv2Header ();

  virtual TypeId GetInstanceTypeId (void) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

private:
  uint8_t m_type; //!< IGMPv2 type
  uint8_t m_maxRespTime; //!< IGMPv2 max response time
  bool m_calcChecksum;  //!< true if checksum is calculated
  Ipv4Address m_groupAddress; //!< IPv4 group address
};

} // namespace ns3

#endif /* IGMPV2_H */
