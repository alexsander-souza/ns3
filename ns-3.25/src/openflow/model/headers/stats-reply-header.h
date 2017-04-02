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

#ifndef STATS_REPLY_HEADER_H
#define STATS_REPLY_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define STATS_REPLY_LENGTH 4

namespace ns3
{
class FlowStatsHeader;
class PortStatsHeader;

/**
 * \class StatsReplyHeader
 * \brief Openflow protocol stats packet used by stats request
 */
class StatsReplyHeader : public Header,
                         public SimpleRefCount<StatsReplyHeader>
{
public:

  /**
   * Default constructor
   */
  StatsReplyHeader (void);

  /**
   * Constructor
   * \param type               One of the OFPST_* constants
   * \param flags              OFPSF_REPLY_* flags
   */
  StatsReplyHeader (uint16_t type, uint16_t flags);

  /**
   * Default destructor
   */
  virtual ~StatsReplyHeader ();

  /* Getters and Setters*/
  uint16_t GetType() const;
  void SetType(uint16_t type);

  uint16_t GetFlags() const;
  void SetFlags(uint16_t flags);

  Ptr<FlowStatsHeader> GetFlowStatsReply() const;
  void SetFlowStatsReply(Ptr<FlowStatsHeader> header);

  Ptr<PortStatsHeader> GetPortStatsReply() const;
  void SetPortStatsReply(Ptr<PortStatsHeader> header);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_type;                          /* One of the OFPST_* constants. */
  uint16_t m_flags;                         /* OFPSF_REPLY_* flags. */
  Ptr<FlowStatsHeader> m_FSReplyHeader;         /* Body of reply to OFPST_FLOW request. */
  Ptr<PortStatsHeader> m_PSReplyHeader;         /* Body of reply to OFPST_PORT request. */
};
} // namespace ns3
#endif  /* STATS_REPLY_HEADER_H */
