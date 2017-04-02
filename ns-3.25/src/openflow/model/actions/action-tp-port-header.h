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

#ifndef ACTION_TP_PORT_HEADER_H
#define ACTION_TP_PORT_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

namespace ns3
{
/**
 * \class ActionTpPortHeader
 * \brief Action header for  OFPAT_SET_TP_SRC/DST
 */
class ActionTpPortHeader : public Header,
                           public SimpleRefCount<ActionTpPortHeader>
{
public:
  /**
   * Default constructor
   */
  ActionTpPortHeader (void);

  /**
   * Constructor
   * \param tpPort          TCP/UDP port
   */
  ActionTpPortHeader (uint16_t tpPort);

  /**
   * Default destructor
   */
  virtual ~ActionTpPortHeader ();

  /* Getters and Setters*/
  uint16_t GetTpPort(void) const;
  void SetTpPort(uint16_t tpPort);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_tpPort;          /* TCP/UDP port. */
};
} // namespace ns3
#endif  /* ACTION_TP_PORT_HEADER_H */
