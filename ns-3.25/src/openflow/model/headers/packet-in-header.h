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

#ifndef PACKET_IN_HEADER_H
#define PACKET_IN_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/openflow-lib.h"

#define PACKET_IN_LENGTH 18

namespace ns3
{
class OpenflowHeader;

/**
 * \class PacketInHeader
 * \brief When packets are received by the datapath and sent to the controller a packet in message is used.
 */
class PacketInHeader : public Header,
                       public SimpleRefCount<PacketInHeader>
{
public:

  /**
   * \brief  Convert a packet in header in NS3 format to struct ofp_packet_in.
   */
  static void ConvertToStruct(ofp_packet_in &ofi, Ptr<OpenflowHeader> header);

  /**
   * \brief  Convert a packet in header in NS3 format to struct ofp_packet_in.
   *         WARNING: header parameter won't be set, to do so use the method
   *         with OpenflowHeader.
   */
  static void ConvertToStruct(ofp_packet_in &ofi, Ptr<PacketInHeader> header);

  /**
   * Default constructor
   */
  PacketInHeader (void);

  /**
   * Constructor
   * \param bufferId           ID assigned by datapath
   * \param inPort             Port on which frame was received
   * \param reason             Reason packet is being sent (one of OFPR_*)
   */
  PacketInHeader (uint32_t bufferId, uint16_t inPort, uint8_t reason);

  /**
   * Constructor
   * \param bufferId           ID assigned by datapath
   * \param totalLen           Full length of frame
   * \param inPort             Port on which frame was received
   * \param reason             Reason packet is being sent (one of OFPR_*)
   * \param data               Pointer to the packet being forwarded to the controller
   */
  PacketInHeader (uint32_t bufferId, uint16_t totalLen, uint16_t inPort,
                  uint8_t reason, uint8_t* data);

  /**
   * Default destructor
   */
  virtual ~PacketInHeader ();

  /* Getters and Setters*/
  uint32_t GetBufferId() const;
  void SetBufferId(uint32_t bufferId);

  uint16_t GetTotalLen() const;

  uint16_t GetInPort() const;
  void SetInPort(uint16_t inPort);

  uint8_t GetReason() const;
  void SetReason(uint8_t reason);

  void GetData(uint8_t *data, uint16_t dataLength) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint32_t m_bufferId;      /* ID assigned by datapath. */
  uint16_t m_totalLen;      /* Full length of frame. */
  uint16_t m_inPort;        /* Port on which frame was received. */
  uint8_t m_reason;         /* Reason packet is being sent (one of OFPR_*) */
  uint8_t* m_data;          /* Ethernet frame, halfway through 32-bit word,
                               so the IP header is 32-bit aligned.  The
                               amount of data is inferred from the length
                               field in the header.  Because of padding,
                               offsetof(struct ofp_packet_in, data) ==
                               sizeof(struct ofp_packet_in) - 2. */
};
} // namespace ns3
#endif  /* PACKET_IN_HEADER_H */
