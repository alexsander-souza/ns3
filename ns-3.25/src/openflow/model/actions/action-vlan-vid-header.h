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

#ifndef ACTION_VLAN_VID_HEADER_H
#define ACTION_VLAN_VID_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

namespace ns3
{
/**
 * \class ActionVlanVidHeader
 * \brief Action header for OFPAT_SET_VLAN_VID
 */
class ActionVlanVidHeader : public Header,
                            public SimpleRefCount<ActionVlanVidHeader>
{
public:
  /**
   * Default constructor
   */
  ActionVlanVidHeader (void);

  /**
   * Constructor
   * \param vlanVid          VLAN id
   */
  ActionVlanVidHeader (uint16_t vlanVid);

  /**
   * Default destructor
   */
  virtual ~ActionVlanVidHeader ();

  /* Getters and Setters*/
  uint16_t GetVlanVid(void) const;
  void SetVlanVid(uint16_t vlanVid);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_vlanVid;          /* VLAN id. */
};
} // namespace ns3
#endif  /* ACTION_VLAN_VID_HEADER_H */
