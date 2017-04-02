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

#ifndef FLOW_MODIFICATION_HEADER_H
#define FLOW_MODIFICATION_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/action-utils.h"
#include "ns3/openflow-lib.h"

namespace ns3
{
class FlowMatchHeader;
class ActionHeader;

/**
 * OFPFF_* codes
 */
enum ofp_flow_mod_flags {
  OFPFF_SEND_FLOW_REM = 1 << 0,   /* Send flow removed message when flow expires or is deleted. */
  OFPFF_CHECK_OVERLAP = 1 << 1,   /* Check for overlapping entries first. */
  OFPFF_EMERG = 1 << 2            /* Remark this is for emergency. */
};

/**
 * \class FlowModificationHeader
 * \brief Openflow protocol flow modification packet
 */
class FlowModificationHeader : public Header,
                               public SimpleRefCount<FlowModificationHeader>

{
public:
  /**
   * Default constructor
   */
  FlowModificationHeader (void);

  /**
   * Constructor
   * \param header   Pointer to struct ofp_flow_mod
   */
  FlowModificationHeader (ofp_flow_mod* header);

  /**
   * Constructor
   * \param cookie             Opaque controller-issued identifier
   * \param command            One of OFPFC_*
   * \param idleTimeout        Idle time before discarding (seconds)
   * \param hardTimeout        Max time before discarding (seconds)
   * \param priority           Priority level of flow entry
   * \param bufferId           Buffered packet to apply to (or -1). Not meaningful for OFPFC_DELETE*
   * \param outPort            For OFPFC_DELETE* commands, require matching entries to include this as an
                              output port. A value of OFPP_ANY indicates no restriction
   * \param flags              One of OFPFF_*
   */
  FlowModificationHeader (uint64_t cookie, uint16_t command, uint16_t idleTimeout,
                          uint16_t hardTimeout, uint16_t priority, uint32_t bufferId,
                          uint16_t outPort, uint16_t flags);

  /**
   * Default destructor
   */
  virtual ~FlowModificationHeader ();

  /* Getters and Setters*/
  Ptr<FlowMatchHeader> GetFlowMatch() const;
  void SetFlowMatch(Ptr<FlowMatchHeader> header);

  uint64_t GetCookie() const;
  void SetCookie(uint64_t cookie);

  uint16_t GetCommand() const;
  void SetCommand(uint16_t command);

  uint16_t GetIdleTimeout() const;
  void SetIdleTimeout(uint16_t idleTimeout);

  uint16_t GetHardTimeout() const;
  void SetHardTimeout(uint16_t hardTimeout);

  uint16_t GetPriority() const;
  void SetPriority(uint16_t priority);

  uint32_t GetBufferId() const;
  void SetBufferId(uint32_t bufferId);

  uint16_t GetOutPort() const;
  void SetOutPort(uint16_t outPort);

  uint16_t GetFlags() const;
  void SetFlags(uint16_t flags);

  /**
   * Add action to message
   * \param action           ActionHeader
   */
  void AddAction(Ptr<ActionHeader> action);

  /**
   * Add actions list to message
   * \param actionsList        Actions list
   */
  void AddActionsList(action_utils::ActionsList actionsList);

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
  Ptr<FlowMatchHeader> m_FMatchHeader;    /* Fields to match */
  uint64_t m_cookie;                  /* Opaque controller-issued identifier. */
  /* Flow actions. */
  uint16_t m_command;                 /* One of OFPFC_*. */
  uint16_t m_idleTimeout;             /* Idle time before discarding (seconds). */
  uint16_t m_hardTimeout;             /* Max time before discarding (seconds). */
  uint16_t m_priority;                /* Priority level of flow entry. */
  uint32_t m_bufferId;                /* Buffered packet to apply to (or -1). Not meaningful for OFPFC_DELETE*. */
  uint16_t m_outPort;                 /* For OFPFC_DELETE* commands, require matching entries to include this as
                                         an output port. A value of OFPP_ANY indicates no restriction. */
  uint16_t m_flags;                   /* One of OFPFF_*. */
  action_utils::ActionsList m_actionsList;          /* The action length is inferred from the length field in
                                                       the header. */
};
} // namespace ns3
#endif  /* FLOW_MODIFICATION_HEADER_H */
