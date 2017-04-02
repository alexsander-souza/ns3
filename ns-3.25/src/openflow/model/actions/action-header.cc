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

#include <ns3/action-header.h>
#include <ns3/log.h>

NS_LOG_COMPONENT_DEFINE("ActionHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ActionHeader);

ActionHeader::ActionHeader (void) :
  m_type(0),
  m_length(0)
{
  NS_LOG_FUNCTION(this);
}

ActionHeader::ActionHeader (ofp_action_header* header) :
  m_type(header->type),
  m_length(header->len)
{
  NS_LOG_FUNCTION(this);
}

ActionHeader::ActionHeader (uint16_t type, uint16_t length) :
  m_type(type),
  m_length(length)
{
  NS_LOG_FUNCTION(this);
}

ActionHeader::~ActionHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint16_t ActionHeader::GetType(void) const
{
  return m_type;
}

void ActionHeader::SetType(uint16_t type)
{
  m_type = type;
}

uint16_t ActionHeader::GetLength(void) const
{
  return m_length;
}

void ActionHeader::SetLength(uint16_t length)
{
  m_length = length;
}

Ptr<ActionOutputHeader> ActionHeader::GetOutput() const
{
  return m_OHeader;
}

void ActionHeader::SetOutput(Ptr<ActionOutputHeader> header)
{
  m_OHeader = header;
}

Ptr<ActionVlanVidHeader> ActionHeader::GetVlanVid() const
{
  return m_VVidHeader;
}

void ActionHeader::SetVlanVid(Ptr<ActionVlanVidHeader> header)
{
  m_VVidHeader = header;
}

Ptr<ActionVlanPcpHeader> ActionHeader::GetVlanPcp() const
{
  return m_VPcpHeader;
}

void ActionHeader::SetVlanPcp(Ptr<ActionVlanPcpHeader> header)
{
  m_VPcpHeader = header;
}

Ptr<ActionDlAddressHeader> ActionHeader::GetDlSrcAddress() const
{
  return m_DlSrcAHeader;
}

void ActionHeader::SetDlSrcAddress(Ptr<ActionDlAddressHeader> header)
{
  m_DlSrcAHeader = header;
}

Ptr<ActionDlAddressHeader> ActionHeader::GetDlDstAddress() const
{
  return m_DlDstAHeader;
}

void ActionHeader::SetDlDstAddress(Ptr<ActionDlAddressHeader> header)
{
  m_DlDstAHeader = header;
}

Ptr<ActionNwAddressHeader> ActionHeader::GetNwSrcAddress() const
{
  return m_NwSrcAHeader;
}

void ActionHeader::SetNwSrcAddress(Ptr<ActionNwAddressHeader> header)
{
  m_NwSrcAHeader = header;
}

Ptr<ActionNwAddressHeader> ActionHeader::GetNwDstAddress() const
{
  return m_NwDstAHeader;
}

void ActionHeader::SetNwDstAddress(Ptr<ActionNwAddressHeader> header)
{
  m_NwDstAHeader = header;
}

Ptr<ActionTpPortHeader> ActionHeader::GetTpSrc() const
{
  return m_TpSrcHeader;
}

void ActionHeader::SetTpSrc(Ptr<ActionTpPortHeader> header)
{
  m_TpSrcHeader = header;
}

Ptr<ActionTpPortHeader> ActionHeader::GetTpDst() const
{
  return m_TpDstHeader;
}

void ActionHeader::SetTpDst(Ptr<ActionTpPortHeader> header)
{
  m_TpDstHeader = header;
}

Ptr<ActionVendorHeader> ActionHeader::GetVendor() const
{
  return m_VHeader;
}

void ActionHeader::SetVendor(Ptr<ActionVendorHeader> header)
{
  m_VHeader = header;
}

bool ActionHeader::operator<(const ActionHeader &other) const
{
  if (int(m_type) - int(other.m_type) < 0)
    return true;
  else
    return false;
}

TypeId ActionHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ActionHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ActionHeader> ()
  ;

  return tid;
}

TypeId ActionHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ActionHeader::Print(std::ostream &os) const
{
  os << "Action Header (default)" << std::endl
     << " Type: " << uint(m_type) << std::endl
     << " Length: " << uint(m_length) << std::endl
     << std::endl;
}

uint32_t ActionHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this << m_length);
  return m_length;
}

void ActionHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_type);
  start.WriteHtonU16(m_length);

  switch (m_type)
    {
    case OFPAT_OUTPUT:
      m_OHeader->Serialize(start);
      break;

    case OFPAT_SET_VLAN_VID:
      m_VVidHeader->Serialize(start);
      break;

    case OFPAT_SET_VLAN_PCP:
      m_VPcpHeader->Serialize(start);
      break;

    case OFPAT_SET_DL_SRC:
      m_DlSrcAHeader->Serialize(start);
      break;

    case OFPAT_SET_DL_DST:
      m_DlDstAHeader->Serialize(start);
      break;

    case OFPAT_SET_NW_SRC:
      m_NwSrcAHeader->Serialize(start);
      break;

    case OFPAT_SET_NW_DST:
      m_NwDstAHeader->Serialize(start);
      break;

    case OFPAT_SET_TP_SRC:
      m_TpSrcHeader->Serialize(start);
      break;

    case OFPAT_SET_TP_DST:
      m_TpDstHeader->Serialize(start);
      break;

    case OFPAT_VENDOR:
      m_VHeader->Serialize(start);
      break;

    default:
      start.WriteU8(0, 4);
      break;
    }
}

uint32_t ActionHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_type = start.ReadNtohU16();
  m_length = start.ReadNtohU16();

  switch (m_type)
    {
    case OFPAT_OUTPUT:
      m_OHeader = Create<ActionOutputHeader>();
      m_OHeader->Deserialize(start);
      break;

    case OFPAT_SET_VLAN_VID:
      m_VVidHeader = Create<ActionVlanVidHeader>();
      m_VVidHeader->Deserialize(start);
      break;

    case OFPAT_SET_VLAN_PCP:
      m_VPcpHeader = Create<ActionVlanPcpHeader>();
      m_VPcpHeader->Deserialize(start);
      break;

    case OFPAT_SET_DL_SRC:
      m_DlSrcAHeader = Create<ActionDlAddressHeader>();
      m_DlSrcAHeader->Deserialize(start);
      break;

    case OFPAT_SET_DL_DST:
      m_DlDstAHeader = Create<ActionDlAddressHeader>();
      m_DlDstAHeader->Deserialize(start);
      break;

    case OFPAT_SET_NW_SRC:
      m_NwSrcAHeader = Create<ActionNwAddressHeader>();
      m_NwSrcAHeader->Deserialize(start);
      break;

    case OFPAT_SET_NW_DST:
      m_NwDstAHeader = Create<ActionNwAddressHeader>();
      m_NwDstAHeader->Deserialize(start);
      break;

    case OFPAT_SET_TP_SRC:
      m_TpSrcHeader = Create<ActionTpPortHeader>();
      m_TpSrcHeader->Deserialize(start);
      break;

    case OFPAT_SET_TP_DST:
      m_TpDstHeader = Create<ActionTpPortHeader>();
      m_TpDstHeader->Deserialize(start);
      break;

    case OFPAT_VENDOR:
      m_VHeader = Create<ActionVendorHeader>();
      m_VHeader->Deserialize(start);
      break;

    default:
      start.Next(4);
      break;
    }

  return GetSerializedSize();
}
} // namespace ns3
