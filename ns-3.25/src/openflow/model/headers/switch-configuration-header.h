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

#ifndef SWITCH_CONFIGURATION_HEADER_H
#define SWITCH_CONFIGURATION_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define SWITCH_SET_CONFIGURATION_LENGTH         12
#define SWITCH_GET_CONFIGURATION_REPLY_LENGTH   12

namespace ns3
{
enum ofp_config_flags {
  /* Handling of IP fragments. */
  OFPC_FRAG_NORMAL  = 0,    /* No special handling for fragments. */
  OFPC_FRAG_DROP    = 1,    /* Drop fragments. */
  OFPC_FRAG_REASM   = 2,    /* Reassemble (only if OFPC_IP_REASM set). */
  OFPC_FRAG_MASK    = 3
};

/**
 * \class SwitchConfigurationHeader
 * \brief Openflow protocol switch configuration packet
 */
class SwitchConfigurationHeader : public Header,
                                  public SimpleRefCount<SwitchConfigurationHeader>
{
public:
  /**
   * Default constructor
   */
  SwitchConfigurationHeader (void);

  /**
   * Constructor
   * \param flags          OFPC flags
   * \param length         Max flow length
   */
  SwitchConfigurationHeader (uint16_t flags, uint16_t length);

  /**
   * Default destructor
   */
  virtual ~SwitchConfigurationHeader ();

  /* Getters and Setters*/
  uint16_t GetFlags() const;
  void SetFlags(uint16_t flags);

  uint16_t GetMaxFlowLen() const;
  void SetMaxFlowLen(uint16_t length);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_flags;             /* OFPC_* flags. */
  uint16_t m_maxFlowLen;    /* Max bytes of new flow that datapath should send to the controller. */
};
} // namespace ns3
#endif  /* SWITCH_CONFIGURATION_HEADER_H */
