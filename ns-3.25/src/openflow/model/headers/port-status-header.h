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
 * Author: Andrey Blazejuk  <andrey.blazejuk@inf.ufrgs.br>
 *
 */

#ifndef PORT_STATUS_HEADER_H
#define PORT_STATUS_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define PORT_STATUS_LENGTH 64

namespace ns3
{
class PhysicalPortHeader;

/**
 * \class PortStatusHeader
 * \brief Openflow protocol port status packet
 */
class PortStatusHeader : public Header,
                         public SimpleRefCount<PortStatusHeader>
{
public:

  /**
   * Default constructor
   */
  PortStatusHeader (void);

  /**
   * Constructor
   * \param reason         One of OFPPR_*
   */
  PortStatusHeader (uint8_t reason);

  /**
   * Default destructor
   */
  virtual ~PortStatusHeader ();

  /* Getters and Setters*/
  uint8_t GetReason() const;
  void SetReason(uint8_t reason);

  Ptr<PhysicalPortHeader> GetPhysicalPort() const;
  void SetPhysicalPort(Ptr<PhysicalPortHeader> PhyPortHeader);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint8_t m_reason;                       /* One of OFPPR_*. */
  Ptr<PhysicalPortHeader> m_PhyPortHeader;
};
} // namespace ns3
#endif  /* PORT_STATUS_HEADER_H */
