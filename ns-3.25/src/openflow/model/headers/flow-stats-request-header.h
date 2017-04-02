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

#ifndef FLOW_STATS_REQUEST_HEADER_H
#define FLOW_STATS_REQUEST_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define FLOW_STATS_REQUEST_LENGTH 56

namespace ns3
{
class FlowMatchHeader;

/**
 * \class FlowStatsRequestHeader
 * \brief Openflow protocol flow stats request packet
 */
class FlowStatsRequestHeader : public Header,
                               public SimpleRefCount<FlowStatsRequestHeader>
{
public:

  /**
   * Default constructor
   */
  FlowStatsRequestHeader (void);

  /**
   * Constructor
   * \param tableId            ID of table to read (from ofp_table_stats), 0xff for all
                              tables or 0xfe for emergency
   * \param outPort            Require matching entries to include this as an output port.
                              A value of OFPP_NONE indicates no restriction
   */
  FlowStatsRequestHeader (uint8_t tableId, uint16_t outPort);

  /**
   * Default destructor
   */
  virtual ~FlowStatsRequestHeader ();

  /* Getters and Setters*/
  Ptr<FlowMatchHeader> GetFlowMatch() const;
  void SetFlowMatch(Ptr<FlowMatchHeader> header);

  uint8_t GetTableId() const;
  void SetTableId(uint8_t tableId);

  uint16_t GetOutPort() const;
  void SetOutPort(uint16_t outPort);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  Ptr<FlowMatchHeader> m_FMatchHeader;    /* Fields to match */
  uint8_t m_tableId;                  /* ID of table to read (from ofp_table_stats),
                                         0xff for all tables or 0xfe for emergency. */
  uint16_t m_outPort;                 /* Require matching entries to include this
                                         as an output port. A value of OFPP_NONE
                                         indicates no restriction. */
};
} // namespace ns3
#endif  /* FLOW_STATS_REQUEST_HEADER_H */
