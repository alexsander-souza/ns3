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

#ifndef STATS_REQUEST_HEADER_H
#define STATS_REQUEST_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define STATS_REQUEST_LENGTH 4

namespace ns3
{
class FlowStatsRequestHeader;
class PortStatsRequestHeader;

enum ofp_stats_types {
  /* Description of this OpenFlow switch.
   * The request body is empty.
   * The reply body is struct ofp_desc_stats. */
  OFPST_DESC,

  /* Individual flow statistics.
   * The request body is struct ofp_flow_stats_request.
   * The reply body is an array of struct ofp_flow_stats. */
  OFPST_FLOW,

  /* Aggregate flow statistics.
   * The request body is struct ofp_aggregate_stats_request.
   * The reply body is struct ofp_aggregate_stats_reply. */
  OFPST_AGGREGATE,

  /* Flow table statistics.
   * The request body is empty.
   * The reply body is an array of struct ofp_table_stats. */
  OFPST_TABLE,

  /* Physical port statistics.
   * The request body is struct ofp_port_stats_request.
   * The reply body is an array of struct ofp_port_stats. */
  OFPST_PORT,

  /* Queue statistics for a port
   * The request body defines the port
   * The reply body is an array of struct ofp_queue_stats */
  OFPST_QUEUE,

  /* Vendor extension.
   * The request and reply bodies begin with a 32-bit vendor ID, which takes
   * the same form as in "struct ofp_vendor_header". The request and reply
   * bodies are otherwise vendor-defined. */
  OFPST_VENDOR = 0xffff
};

/**
 * \class StatsRequestHeader
 * \brief Openflow protocol stats packet used by stats request
 */
class StatsRequestHeader : public Header,
                           public SimpleRefCount<StatsRequestHeader>
{
public:

  /**
   * Default constructor
   */
  StatsRequestHeader (void);

  /**
   * Constructor
   * \param type               One of the OFPST_* constants
   * \param flags              OFPSF_REQ_* flags (none yet defined)
   */
  StatsRequestHeader (uint16_t type, uint16_t flags);

  /**
   * Default destructor
   */
  virtual ~StatsRequestHeader ();

  /* Getters and Setters*/
  uint16_t GetType() const;
  void SetType(uint16_t type);

  uint16_t GetFlags() const;
  void SetFlags(uint16_t flags);

  Ptr<FlowStatsRequestHeader> GetFlowStatsRequest() const;
  void SetFlowStatsRequest(Ptr<FlowStatsRequestHeader> header);

  Ptr<PortStatsRequestHeader> GetPortStatsRequest() const;
  void SetPortStatsRequest(Ptr<PortStatsRequestHeader> header);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_type;                          /* One of the OFPST_* constants. */
  uint16_t m_flags;                         /* OFPSF_REQ_* flags (none yet defined). */
  Ptr<FlowStatsRequestHeader> m_FSReqHeader;    /* Body for ofp_stats_request of type OFPST_FLOW. */
  Ptr<PortStatsRequestHeader> m_PSReqHeader;    /* Body for ofp_stats_request of type OFPST_PORT. */
};
} // namespace ns3
#endif  /* STATS_REQUEST_HEADER_H */
