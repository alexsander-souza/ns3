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

#ifndef PACKET_OUT_HEADER_H
#define PACKET_OUT_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/action-utils.h"

#define PACKET_OUT_LENGTH 16

namespace ns3
{
class ActionHeader;

/**
 * \class PacketOutHeader
 * \brief When packets are sent to the switch a packet out message is used.
 */
class PacketOutHeader : public Header,
                        public SimpleRefCount<PacketOutHeader>
{
public:
  /**
   * Default constructor
   */
  PacketOutHeader (void);

  /**
   * Constructor
   * \param bufferId           ID assigned by datapath (-1 if none)
   * \param inPort             Packet’s input port (OFPP_NONE if none)
   * \param actionsLen         Size of action array in bytes
   */
  PacketOutHeader (uint32_t bufferId, uint16_t inPort, uint16_t actionsLen);

  /**
   * Constructor
   * \param bufferId           ID assigned by datapath
   * \param inPort             Packet’s input port (OFPP_NONE if none)
   * \param actionsLen         Size of action array in bytes
   * \param packet             Pointer to the packet being forwarded to the controller
   */
  PacketOutHeader (uint32_t bufferId, uint16_t inPort, uint16_t actionsLen,
                   Ptr<Packet> packet);

  /**
   * Default destructor
   */
  virtual ~PacketOutHeader ();

  /* Getters and Setters*/
  uint32_t GetBufferId() const;
  void SetBufferId(uint32_t bufferId);

  uint16_t GetInPort() const;
  void SetInPort(uint16_t inPort);

  uint16_t GetActionsLen() const;
  void SetActionsLen(uint16_t actionsLen);

  /**
   * Add action to message
   * \param action           ActionHeader
   */
  void AddAction(Ptr<ActionHeader>action);

  /**
   * Add actions list to message
   * \param actions_list        Actions list
   */
  void AddActionsList(action_utils::ActionsList actions_list);

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

  uint16_t GetDataLength() const;

  void GetData(uint8_t *data, uint16_t dataLength) const;

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

  // PacketOutHeader& operator=(const PacketOutHeader& other);

private:
  uint32_t m_bufferId;            /* ID assigned by datapath (-1 if none). */
  uint16_t m_inPort;              /* Packet’s input port (OFPP_NONE if none). */
  uint16_t m_actionsLen;          /* Size of action array in bytes. */
  action_utils::ActionsList m_actionsList;      /* Actions. */
  uint8_t* m_data;                /* Packet data. The length is inferred
                                     from the length field in the header.
                                     (Only meaningful if buffer_id == -1.) */

  uint16_t m_dataLength;          /* Added to allocate the exact amount of memory needed for this header */
};
} // namespace ns3
#endif  /* PACKET_OUT_HEADER_H */
