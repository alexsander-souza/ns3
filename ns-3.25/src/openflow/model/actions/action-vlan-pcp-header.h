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

#ifndef ACTION_VLAN_PCP_HEADER_H
#define ACTION_VLAN_PCP_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

namespace ns3
{
/**
 * \class ActionVlanPcpHeader
 * \brief Action header for OFPAT_SET_VLAN_PCP
 */
class ActionVlanPcpHeader : public Header,
                            public SimpleRefCount<ActionVlanPcpHeader>
{
public:
  /**
   * Default constructor
   */
  ActionVlanPcpHeader (void);

  /**
   * Constructor
   * \param vlanPcp          VLAN PCP
   */
  ActionVlanPcpHeader (uint8_t vlanPcp);

  /**
   * Default destructor
   */
  virtual ~ActionVlanPcpHeader ();

  /* Getters and Setters*/
  uint8_t GetVlanPcp(void) const;
  void SetVlanPcp(uint8_t vlanPcp);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint8_t m_vlanPcp;          /* VLAN PCP. */
};
} // namespace ns3
#endif  /* ACTION_VLAN_PCP_HEADER_H */
