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

#ifndef FLOW_REMOVED_HEADER_H
#define FLOW_REMOVED_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>

#define FLOW_REMOVED_LENGTH 88

namespace ns3
{
/* Why was this flow removed? */
enum ofp_flow_removed_reason {
  OFPRR_IDLE_TIMEOUT,     /* Flow idle time exceeded idle_timeout. */
  OFPRR_HARD_TIMEOUT,     /* Time exceeded hard_timeout. */
  OFPRR_DELETE            /* Evicted by a DELETE flow mod. */
};

class FlowMatchHeader;

/**
 * \class FlowRemovedHeader
 * \brief Openflow protocol flow removed packet
 */
class FlowRemovedHeader : public Header,
                          public SimpleRefCount<FlowRemovedHeader>
{
public:

  /**
   * Default constructor
   */
  FlowRemovedHeader (void);

  /**
   * Constructor
   * \param cookie         Opaque controller-issued identifier
   * \param priority       Priority level of flow entry
   * \param reason         One of OFPRR_*
   * \param durationSec    Time flow was alive in seconds
   * \param durationNSec   Time flow was alive in nanoseconds beyond duration_sec
   * \param idleTimeout    Idle timeout from original flow mod
   * \param packetCount
   * \param byteCount
   */
  FlowRemovedHeader (uint64_t cookie, uint16_t priority, uint8_t reason,
                     uint32_t durationSec, uint32_t durationNSec, uint16_t idleTimeout,
                     uint64_t packetCount, uint64_t byteCount);

  /**
   * Default destructor
   */
  virtual ~FlowRemovedHeader ();

  /* Getters and Setters*/
  Ptr<FlowMatchHeader> GetFlowMatch() const;
  void SetFlowMatch(Ptr<FlowMatchHeader> FMatchHeader);

  uint64_t GetCookie() const;
  void SetCookie(uint64_t cookie);

  uint16_t GetPriority() const;
  void SetPriority(uint16_t priority);

  uint8_t GetReason() const;
  void SetReason(uint8_t reason);

  uint32_t GetDurationSec() const;
  void SetDurationSec(uint32_t durationSec);

  uint32_t GetDurationNSec() const;
  void SetDurationNSec(uint32_t durationNSec);

  uint16_t GetIdleTimeout() const;
  void SetIdleTimeout(uint16_t idleTimeout);

  uint64_t GetPacketCount() const;
  void SetPacketCount(uint64_t packetCount);

  uint64_t GetByteCount() const;
  void SetByteCount(uint64_t byteCount);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  Ptr<FlowMatchHeader> m_FMatchHeader;      /* Description of fields. */
  uint64_t m_cookie;                    /* Opaque controller-issued identifier. */
  uint16_t m_priority;                  /* Priority level of flow entry. */
  uint8_t m_reason;                     /* One of OFPRR_*. */
  uint32_t m_durationSec;               /* Time flow was alive in seconds. */
  uint32_t m_durationNSec;              /* Time flow was alive in nanoseconds beyond duration_sec. */
  uint16_t m_idleTimeout;               /* Idle timeout from original flow mod. */
  uint64_t m_packetCount;
  uint64_t m_byteCount;
};
} // namespace ns3
#endif  /* FLOW_REMOVED_HEADER_H */
