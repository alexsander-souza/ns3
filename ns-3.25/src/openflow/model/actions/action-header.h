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

#ifndef ACTION_HEADER_H
#define ACTION_HEADER_H

#include <ns3/header.h>
#include <ns3/simple-ref-count.h>
#include <ns3/action-dl-address-header.h>
#include <ns3/action-nw-address-header.h>
#include <ns3/action-output-header.h>
#include <ns3/action-tp-port-header.h>
#include <ns3/action-vendor-header.h>
#include <ns3/action-vlan-pcp-header.h>
#include <ns3/action-vlan-vid-header.h>
#include <ns3/openflow-lib.h>

namespace ns3
{
enum ofp_action_type {
  OFPAT_OUTPUT,             /* Output to switch port. */
  OFPAT_SET_VLAN_VID,       /* Set the 802.1q VLAN id. */
  OFPAT_SET_VLAN_PCP,       /* Set the 802.1q priority. */
  OFPAT_STRIP_VLAN,         /* Strip the 802.1q header. */
  OFPAT_SET_DL_SRC,         /* Ethernet source address. */
  OFPAT_SET_DL_DST,         /* Ethernet destination address. */
  OFPAT_SET_NW_SRC,         /* IP source address. */
  OFPAT_SET_NW_DST,         /* IP destination address. */
  OFPAT_SET_TP_SRC,         /* TCP/UDP source port. */
  OFPAT_SET_TP_DST,         /* TCP/UDP destination port. */
  OFPAT_VENDOR = 0xffff
};

enum ofp_action_flags {
  OFPAF_OUTPUT        = 1 << 0,     /* Output to switch port. */
  OFPAF_SET_VLAN_VID  = 1 << 1,     /* Set the 802.1q VLAN id. */
  OFPAF_SET_VLAN_PCP  = 1 << 2,     /* Set the 802.1q priority. */
  OFPAF_STRIP_VLAN    = 1 << 3,     /* Strip the 802.1q header. */
  OFPAF_SET_DL_SRC    = 1 << 4,     /* Ethernet source address. */
  OFPAF_SET_DL_DST    = 1 << 5,     /* Ethernet destination address. */
  OFPAF_SET_NW_SRC    = 1 << 6,     /* IP source address. */
  OFPAF_SET_NW_DST    = 1 << 7,     /* IP destination address. */
  OFPAF_SET_TP_SRC    = 1 << 9,     /* TCP/UDP source port. */
  OFPAF_SET_TP_DST    = 1 << 10,    /* TCP/UDP destination port. */
};

/**
 * \class ActionHeader
 * \brief Action header that is common to all actions.
 */
class ActionHeader : public Header,
                     public SimpleRefCount<ActionHeader>
{
public:
  /**
   * Default constructor
   */
  ActionHeader (void);

  /**
   * Constructor
   * \param header   Pointer to struct ofp_action_header
   */
  ActionHeader (ofp_action_header* header);

  /**
   * Constructor
   * \param type             One of OFPAT_*
   * \param length           Length of action, including this header. This is the length of action,
                            including any padding to make it 64-bit aligned
   */
  ActionHeader (uint16_t type, uint16_t length);

  /**
   * Default destructor
   */
  virtual ~ActionHeader ();

  /* Getters and Setters*/
  uint16_t GetType(void) const;
  void SetType(uint16_t type);

  uint16_t GetLength(void) const;
  void SetLength(uint16_t length);

  Ptr<ActionOutputHeader> GetOutput() const;
  void SetOutput(Ptr<ActionOutputHeader> header);

  Ptr<ActionVlanVidHeader> GetVlanVid() const;
  void SetVlanVid(Ptr<ActionVlanVidHeader> header);

  Ptr<ActionVlanPcpHeader> GetVlanPcp() const;
  void SetVlanPcp(Ptr<ActionVlanPcpHeader> header);

  Ptr<ActionDlAddressHeader> GetDlSrcAddress() const;
  void SetDlSrcAddress(Ptr<ActionDlAddressHeader> header);

  Ptr<ActionDlAddressHeader> GetDlDstAddress() const;
  void SetDlDstAddress(Ptr<ActionDlAddressHeader> header);

  Ptr<ActionNwAddressHeader> GetNwSrcAddress() const;
  void SetNwSrcAddress(Ptr<ActionNwAddressHeader> header);

  Ptr<ActionNwAddressHeader> GetNwDstAddress() const;
  void SetNwDstAddress(Ptr<ActionNwAddressHeader> header);

  Ptr<ActionTpPortHeader> GetTpSrc() const;
  void SetTpSrc(Ptr<ActionTpPortHeader> header);

  Ptr<ActionTpPortHeader> GetTpDst() const;
  void SetTpDst(Ptr<ActionTpPortHeader> header);

  Ptr<ActionVendorHeader> GetVendor() const;
  void SetVendor(Ptr<ActionVendorHeader> header);

  bool operator<(const ActionHeader &other) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_type;        /* One of OFPAT_*. */
  uint16_t m_length;      /* Length of action, including this header.  This is the length of action,
                             including any padding to make it 64-bit aligned. */

  Ptr<ActionOutputHeader> m_OHeader;
  Ptr<ActionVlanVidHeader> m_VVidHeader;
  Ptr<ActionVlanPcpHeader> m_VPcpHeader;
  Ptr<ActionDlAddressHeader> m_DlSrcAHeader;
  Ptr<ActionDlAddressHeader> m_DlDstAHeader;
  Ptr<ActionNwAddressHeader> m_NwSrcAHeader;
  Ptr<ActionNwAddressHeader> m_NwDstAHeader;
  Ptr<ActionTpPortHeader> m_TpSrcHeader;
  Ptr<ActionTpPortHeader> m_TpDstHeader;
  Ptr<ActionVendorHeader> m_VHeader;
};
} // namespace ns3
#endif  /* ACTION_HEADER_H */
