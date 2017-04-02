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

#ifndef FLOW_MATCH_HEADER_H
#define FLOW_MATCH_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/openflow-lib.h"

#define FLOW_MATCH_LENGTH 40

namespace ns3
{
class Mac48Address;

/* Flow wildcards. */
enum ofp_flow_wildcards {
  OFPFW_IN_PORT = 1 << 0,     /* Switch input port. */
  OFPFW_DL_VLAN = 1 << 1,     /* VLAN id. */
  OFPFW_DL_SRC = 1 << 2,      /* Ethernet source address. */
  OFPFW_DL_DST = 1 << 3,      /* Ethernet destination address. */
  OFPFW_DL_TYPE = 1 << 4,     /* Ethernet frame type. */
  OFPFW_NW_PROTO = 1 << 5,    /* IP protocol. */
  OFPFW_TP_SRC = 1 << 6,      /* TCP/UDP source port. */
  OFPFW_TP_DST = 1 << 7,      /* TCP/UDP destination port. */

  /* IP source address wildcard bit count. 0 is exact match, 1 ignores the
   * LSB, 2 ignores the 2 least-significant bits, ..., 32 and higher wildcard
   * the entire field. This is the *opposite* of the usual convention where
   * e.g. /24 indicates that 8 bits (not 24 bits) are wildcarded. */
  OFPFW_NW_SRC_SHIFT = 8,
  OFPFW_NW_SRC_BITS = 6,
  OFPFW_NW_SRC_MASK = ((1 << OFPFW_NW_SRC_BITS) - 1) << OFPFW_NW_SRC_SHIFT,
  OFPFW_NW_SRC_ALL = 32 << OFPFW_NW_SRC_SHIFT,

  /* IP destination address wildcard bit count. Same format as source. */
  OFPFW_NW_DST_SHIFT = 14,
  OFPFW_NW_DST_BITS = 6,
  OFPFW_NW_DST_MASK = ((1 << OFPFW_NW_DST_BITS) - 1) << OFPFW_NW_DST_SHIFT,
  OFPFW_NW_DST_ALL = 32 << OFPFW_NW_DST_SHIFT,
  OFPFW_DL_VLAN_PCP = 1 << 20,    /* VLAN priority. */
  OFPFW_NW_TOS = 1 << 21,         /* IP ToS (DSCP field, 6 bits). */

  /* Wildcard all fields. */
  OFPFW_ALL = ((1 << 22) - 1)
};

/**
 * \class FlowMatchHeader
 * \brief Openflow protocol flow match definition packet
 */
class FlowMatchHeader : public Header,
public SimpleRefCount<FlowMatchHeader>

{
public:
  /**
   * \brief  Convert a match header in NS3 format to struct ofp_match
   */
  static void ConvertToStruct(ofp_match &ofm, Ptr<FlowMatchHeader> header);

  /**
   * Default constructor
   */
  FlowMatchHeader (void);

  /**
   * Constructor
   * \param header   Wildcard fields
   */
  FlowMatchHeader (ofp_match* header);

  /**
   * Constructor
   * \param wildcards          Wildcard fields
   * \param inPort             Input switch port
   * \param dlSrc              Ethernet source address
   * \param dlDst              Ethernet destination address
   * \param dlVlan             Input VLAN
   * \param dlVlanPcp          Input VLAN priority
   * \param dlType             Ethernet frame type
   * \param nwTos              IP ToS (actually DSCP field, 6 bits)
   * \param nwProto            IP protocol or lower 8 bits of ARP opcode
   * \param nwSrc              IP source address
   * \param nwDst              IP destination address
   * \param tpSrc              TCP/UDP source port
   * \param tpDst              TCP/UDP destination port
   */
  FlowMatchHeader (uint32_t wildcards, uint16_t inPort, const Mac48Address &dlSrc,
                   const Mac48Address& dlDst, uint16_t dlVlan, uint8_t dlVlanPcp,
                   uint16_t dlType, uint8_t nwTos, uint8_t nwProto,
                   uint32_t nwSrc, uint32_t nwDst, uint16_t tpSrc,
                   uint16_t tpDst);

  /**
   * Default destructor
   */
  virtual ~FlowMatchHeader ();

  /* Getters and Setters*/
  uint32_t GetWildcards() const;
  void SetWildcards(uint32_t wildcards);

  uint16_t GetInPort() const;
  void SetInPort(uint16_t inPort);

  const Mac48Address& GetDlSrc() const;
  void SetDlSrc(Mac48Address dlSrc);

  const Mac48Address& GetDlDst() const;
  void SetDlDst(Mac48Address dlDst);

  uint16_t GetDlVlan() const;
  void SetDlVlan(uint16_t dlVlan);

  uint8_t GetDlVlanPcp() const;
  void SetDlVlanPcp(uint8_t dlVlanPcp);

  uint16_t GetDlType() const;
  void SetDlType(uint16_t dlType);

  uint8_t GetNwTos() const;
  void SetNwTos(uint8_t nwTos);

  uint8_t GetNwProto() const;
  void SetNwProto(uint8_t nwProto);

  uint32_t GetNwSrc() const;
  void SetNwSrc(uint32_t nwSrc);

  uint32_t GetNwDst() const;
  void SetNwDst(uint32_t nwDst);

  uint16_t GetTpSrc() const;
  void SetTpSrc(uint16_t tpSrc);

  uint16_t GetTpDst() const;
  void SetTpDst(uint16_t tpDst);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint32_t m_wildcards;             /* Wildcard fields. */
  uint16_t m_inPort;                /* Input switch port. */
  Mac48Address m_dlSrc;             /* Ethernet source address. */
  Mac48Address m_dlDst;             /* Ethernet destination address. */
  uint16_t m_dlVlan;                /* Input VLAN  id. */
  uint8_t m_dlVlanPcp;              /* Input VLAN priority. */
  uint16_t m_dlType;                /* Ethernet frame type. */
  uint8_t m_nwTos;                  /* IP ToS (actually DSCP field, 6 bits). */
  uint8_t m_nwProto;                /* IP protocol or lower 8 bits of ARP opcode. */
  uint32_t m_nwSrc;                 /* IP source address. */
  uint32_t m_nwDst;                 /* IP destination address. */
  uint16_t m_tpSrc;                 /* TCP/UDP source port. */
  uint16_t m_tpDst;                 /* TCP/UDP destination port. */
};
} // namespace ns3
#endif  /* FLOW_MATCH_HEADER_H */
