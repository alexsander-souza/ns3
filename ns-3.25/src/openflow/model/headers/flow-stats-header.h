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

#ifndef FLOW_STATS_HEADER_H
#define FLOW_STATS_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/action-header.h"
#include "ns3/action-utils.h"

#define FLOW_STATS_REPLY_LENGTH 100

namespace ns3
{
class FlowMatchHeader;

/**
 * \class FlowStatsHeader
 * \brief Openflow protocol flow stats reply packet
 */
class FlowStatsHeader : public Header,
                        public SimpleRefCount<FlowStatsHeader>
{
public:
  /**
   * Default constructor
   */
  FlowStatsHeader (void);

  /**
   * Constructor
   * \param length             Length of this entry
   * \param tableId            ID of table flow came from
   * \param durationSec        Time flow has been alive in seconds
   * \param durationNsec       Time flow has been alive in nanoseconds beyond duration_sec
   * \param priority           Priority of the entry. Only meaningful when this is not an exact-match entry
   * \param idleTimeout        Number of seconds idle before expiration
   * \param hardTimeout        Number of seconds before expiration
   * \param cookie             Opaque controller-issued identifier
   * \param packetCount        Number of packets in flow
   * \param byteCount          Number of bytes in flow
   */
  FlowStatsHeader (uint16_t length, uint8_t tableId, uint32_t durationSec,
                   uint32_t durationNsec, uint16_t priority, uint16_t idleTimeout,
                   uint16_t hardTimeout, uint64_t cookie, uint64_t packetCount,
                   uint64_t byteCount);

  /**
   * Default destructor
   */
  virtual ~FlowStatsHeader ();

  /* Getters and Setters*/
  uint16_t GetLength() const;
  void SetLength(uint16_t length);

  uint8_t GetTableId() const;
  void SetTableId(uint8_t tableId);

  Ptr<FlowMatchHeader> GetFlowMatch() const;
  void SetFlowMatch(Ptr<FlowMatchHeader> header);

  uint32_t GetDurationSec() const;
  void SetDurationSec(uint32_t durationSec);

  uint32_t GetDurationNsec() const;
  void SetDurationNsec(uint32_t durationNsec);

  uint16_t GetPriority() const;
  void SetPriority(uint16_t priority);

  uint16_t GetIdleTimeout() const;
  void SetIdleTimeout(uint16_t idleTimeout);

  uint16_t GetHardTimeout() const;
  void SetHardTimeout(uint16_t hardTimeout);

  uint64_t GetCookie() const;
  void SetCookie(uint64_t cookie);

  uint64_t GetPacketCount() const;
  void SetPacketCount(uint64_t packetCount);

  uint64_t GetByteCount() const;
  void SetByteCount(uint64_t byteCount);

  /**
   * Add action to message
   * \param action           ActionHeader
   */
  void AddAction(Ptr<ActionHeader> action);

  /**
   * Add actions list to message
   * \param actions_list        Actions list
   */
  void AddActionsList(action_utils::ActionsList &actions_list);

  /**
   * Get actions list
   */
  const action_utils::ActionsList& GetActionsList(void) const;

  /**
   * Search for an action in the message
   * \param type             Action type
   * \returns a pointer to the action, or (0) when not found
   */
  Ptr<ActionHeader> GetActionByType(uint16_t type) const;

  /**
   * Prints entire list of actions on screen
   */
  void PrintActionsList(std::ostream &os) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_length;                  /* Length of this entry. */
  uint8_t m_tableId;                  /* ID of table flow came from. */
  Ptr<FlowMatchHeader> m_FMatchHeader;    /* Description of fields. */
  uint32_t m_durationSec;             /* Time flow has been alive in seconds. */
  uint32_t m_durationNsec;            /* Time flow has been alive in nanoseconds beyond
                                         durationSec. */
  uint16_t m_priority;                /* Priority of the entry. Only meaningful
                                         when this is not an exact-match entry. */
  uint16_t m_idleTimeout;             /* Number of seconds idle before expiration. */
  uint16_t m_hardTimeout;             /* Number of seconds before expiration. */
  uint64_t m_cookie;                  /* Opaque controller-issued identifier. */
  uint64_t m_packetCount;             /* Number of packets in flow. */
  uint64_t m_byteCount;               /* Number of bytes in flow. */
  action_utils::ActionsList m_actionsList;          /* Actions. */
};
} // namespace ns3
#endif  /* FLOW_STATS_HEADER_H */
