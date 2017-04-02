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

#include "ns3/log.h"
#include "ns3/flow-match-header.h"
#include "ns3/mac48-address.h"
#include "ns3/address-utils.h"

NS_LOG_COMPONENT_DEFINE("FlowMatchHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(FlowMatchHeader);

void FlowMatchHeader::ConvertToStruct(ofp_match &ofm, Ptr<FlowMatchHeader>header)
{
  ofm.wildcards = header->GetWildcards();
  ofm.in_port = header->GetInPort();
  const Mac48Address dlSrc = header->GetDlSrc();
  dlSrc.CopyTo(ofm.dl_src);
  const Mac48Address dlDst = header->GetDlDst();
  dlDst.CopyTo(ofm.dl_dst);
  ofm.dl_vlan = header->GetDlVlan();
  // VlanPcp not implemented
  ofm.dl_type = header->GetDlType();
  // nwToS not implemented
  ofm.nw_proto = header->GetNwProto();
  ofm.nw_src = header->GetNwSrc();
  ofm.nw_dst = header->GetNwDst();
  ofm.tp_src = header->GetTpSrc();
  ofm.tp_dst = header->GetTpDst();
}

FlowMatchHeader::FlowMatchHeader (void) :
  m_wildcards(0),
  m_inPort(0),
  m_dlSrc(Mac48Address("00:00:00:00:00:00")),
  m_dlDst(Mac48Address("00:00:00:00:00:00")),
  m_dlVlan(0),
  m_dlVlanPcp(0),
  m_dlType(0),
  m_nwTos(0),
  m_nwProto(0),
  m_nwSrc(0),
  m_nwDst(0),
  m_tpSrc(0),
  m_tpDst(0)
{
  NS_LOG_FUNCTION(this);
}

FlowMatchHeader::FlowMatchHeader (ofp_match* header) :
  m_dlVlanPcp(0),
  m_nwTos(0)
{
  NS_LOG_FUNCTION(this);

  m_wildcards = header->wildcards;
  m_inPort = header->in_port;
  m_dlSrc.CopyFrom(header->dl_src);
  m_dlDst.CopyFrom(header->dl_dst);
  m_dlVlan = header->dl_vlan;
  m_dlType = header->dl_type;
  m_nwProto = header->nw_proto;
  m_nwSrc = header->nw_src;
  m_nwDst = header->nw_dst;
  m_tpSrc = header->tp_src;
  m_tpDst = header->tp_dst;
}

FlowMatchHeader::FlowMatchHeader (uint32_t wildcards, uint16_t inPort, const Mac48Address& dlSrc,
                                  const Mac48Address &dlDst, uint16_t dlVlan, uint8_t dlVlanPcp,
                                  uint16_t dlType, uint8_t nwTos, uint8_t nwProto,
                                  uint32_t nwSrc, uint32_t nwDst, uint16_t tpSrc,
                                  uint16_t tpDst) :
  m_wildcards(wildcards),
  m_inPort(inPort),
  m_dlSrc(dlSrc),
  m_dlDst(dlDst),
  m_dlVlan(dlVlan),
  m_dlVlanPcp(dlVlanPcp),
  m_dlType(dlType),
  m_nwTos(nwTos),
  m_nwProto(nwProto),
  m_nwSrc(nwSrc),
  m_nwDst(nwDst),
  m_tpSrc(tpSrc),
  m_tpDst(tpDst)
{
  NS_LOG_FUNCTION(this);
}

FlowMatchHeader::~FlowMatchHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint32_t FlowMatchHeader::GetWildcards() const
{
  return m_wildcards;
}

void FlowMatchHeader::SetWildcards(uint32_t wildcards)
{
  m_wildcards = wildcards;
}

uint16_t FlowMatchHeader::GetInPort() const
{
  return m_inPort;
}

void FlowMatchHeader::SetInPort(uint16_t inPort)
{
  m_inPort = inPort;
}

const Mac48Address& FlowMatchHeader::GetDlSrc() const
{
  return m_dlSrc;
}

void FlowMatchHeader::SetDlSrc(Mac48Address dlSrc)
{
  m_dlSrc = dlSrc;
}

const Mac48Address& FlowMatchHeader::GetDlDst() const
{
  return m_dlDst;
}

void FlowMatchHeader::SetDlDst(Mac48Address dlDst)
{
  m_dlDst = dlDst;
}

uint16_t FlowMatchHeader::GetDlVlan() const
{
  return m_dlVlan;
}

void FlowMatchHeader::SetDlVlan(uint16_t dlVlan)
{
  m_dlVlan = dlVlan;
}

uint8_t FlowMatchHeader::GetDlVlanPcp() const
{
  return m_dlVlanPcp;
}

void FlowMatchHeader::SetDlVlanPcp(uint8_t dlVlanPcp)
{
  m_dlVlanPcp = dlVlanPcp;
}

uint16_t FlowMatchHeader::GetDlType() const
{
  return m_dlType;
}

void FlowMatchHeader::SetDlType(uint16_t dlType)
{
  m_dlType = dlType;
}

uint8_t FlowMatchHeader::GetNwTos() const
{
  return m_nwTos;
}

void FlowMatchHeader::SetNwTos(uint8_t nwTos)
{
  m_nwTos = nwTos;
}

uint8_t FlowMatchHeader::GetNwProto() const
{
  return m_nwProto;
}

void FlowMatchHeader::SetNwProto(uint8_t nwProto)
{
  m_nwProto = nwProto;
}

uint32_t FlowMatchHeader::GetNwSrc() const
{
  return m_nwSrc;
}

void FlowMatchHeader::SetNwSrc(uint32_t nwSrc)
{
  m_nwSrc = nwSrc;
}

uint32_t FlowMatchHeader::GetNwDst() const
{
  return m_nwDst;
}

void FlowMatchHeader::SetNwDst(uint32_t nwDst)
{
  m_nwDst = nwDst;
}

uint16_t FlowMatchHeader::GetTpSrc() const
{
  return m_tpSrc;
}

void FlowMatchHeader::SetTpSrc(uint16_t tpSrc)
{
  m_tpSrc = tpSrc;
}

uint16_t FlowMatchHeader::GetTpDst() const
{
  return m_tpDst;
}

void FlowMatchHeader::SetTpDst(uint16_t tpDst)
{
  m_tpDst = tpDst;
}

TypeId FlowMatchHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::FlowMatchHeader")
                      .SetParent<Header> ()
                      .AddConstructor<FlowMatchHeader> ()
  ;

  return tid;
}

TypeId FlowMatchHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void FlowMatchHeader::Print(std::ostream &os) const
{
  os << "Flow Match Header" << std::endl
     << " Wildcards: " << uint(m_wildcards) << std::endl
     << " In Port: " << uint(m_inPort) << std::endl
     << " DL Source: " << m_dlSrc << std::endl
     << " DL Destination: " << m_dlDst << std::endl
     << " DL VLan: " << uint(m_dlVlan) << std::endl
     << " DL VLan Priority: " << uint(m_dlVlanPcp) << std::endl
     << " DL Type: " << uint(m_dlType) << std::endl
     << " NW ToS: " << uint(m_nwTos) << std::endl
     << " NW Protocol: " << uint(m_nwProto) << std::endl
     << " NW Source: " << uint(m_nwSrc) << std::endl
     << " NW Destination: " << uint(m_nwDst) << std::endl
     << " TCP/UDP source port: " << uint(m_tpSrc) << std::endl
     << " TCP/UDP destination port: " << uint(m_tpDst) << std::endl
     << std::endl;
}

uint32_t FlowMatchHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 40;    // 4 + 2 + 6 + 6 + 2 + 1 + (pad) 1 + 2 + 1 + 1 + (pad) 2 + 4 + 4 + 2 + 2 = 40
}

void FlowMatchHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU32(m_wildcards);
  start.WriteHtonU16(m_inPort);
  WriteTo(start, m_dlSrc);
  WriteTo(start, m_dlDst);
  start.WriteHtonU16(m_dlVlan);
  start.WriteU8(m_dlVlanPcp);
  start.WriteU8(0);                       /* Align to 64-bits */
  start.WriteHtonU16(m_dlType);
  start.WriteU8(m_nwTos);
  start.WriteU8(m_nwProto);
  start.WriteU8(0, 2);                    /* Align to 64-bits */
  start.WriteHtonU32(m_nwSrc);
  start.WriteHtonU32(m_nwDst);
  start.WriteHtonU16(m_tpSrc);
  start.WriteHtonU16(m_tpDst);
}

uint32_t FlowMatchHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_wildcards = start.ReadNtohU32();
  m_inPort = start.ReadNtohU16();
  ReadFrom(start, m_dlSrc);
  ReadFrom(start, m_dlDst);
  m_dlVlan = start.ReadNtohU16();
  m_dlVlanPcp = start.ReadU8();
  start.Next();
  m_dlType = start.ReadNtohU16();
  m_nwTos = start.ReadU8();
  m_nwProto = start.ReadU8();
  start.Next(2);
  m_nwSrc = start.ReadNtohU32();
  m_nwDst = start.ReadNtohU32();
  m_tpSrc = start.ReadNtohU16();
  m_tpDst = start.ReadNtohU16();

  return GetSerializedSize();
}
} // namespace ns3
