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
#include "ns3/error-msg-header.h"
#include "ns3/switch-configuration-header.h"
#include "ns3/switch-features-header.h"
#include "ns3/physical-port-header.h"
#include "ns3/flow-match-header.h"
#include "ns3/action-header.h"
#include "ns3/flow-modification-header.h"
#include "ns3/packet-in-header.h"
#include "ns3/flow-removed-header.h"
#include "ns3/port-status-header.h"
#include "ns3/packet-out-header.h"
#include "ns3/port-modification-header.h"
#include "ns3/flow-stats-request-header.h"
#include "ns3/stats-request-header.h"
#include "ns3/stats-reply-header.h"
#include "ns3/openflow-header.h"

NS_LOG_COMPONENT_DEFINE("OpenflowHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(OpenflowHeader);

void OpenflowHeader::ConvertToStruct(ofp_header &ofh, Ptr<OpenflowHeader>header)
{
  ofh.version = OFP_VERSION; //header->GetVersion();
  ofh.type = header->GetType();
  ofh.length = header->GetLength();
  ofh.xid = header->GetXId();
}

OpenflowHeader::OpenflowHeader () :
  m_version(0),
  m_type(0),
  m_length(OPENFLOW_HEADER_LENGTH),
  m_xid(0),
  m_EMsgHeader(nullptr),
  m_SCHeader(nullptr),
  m_SFHeader(nullptr),
  m_FModHeader(nullptr),
  m_PIHeader(nullptr),
  m_FRemHeader(nullptr),
  m_PSHeader(nullptr),
  m_POHeader(nullptr),
  m_PModHeader(nullptr),
  m_SReqHeader(nullptr),
  m_SReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

OpenflowHeader::OpenflowHeader(ofp_header* header) :
  m_version(header->version),
  m_type(header->type),
  m_length(header->length),
  m_xid(header->xid),
  m_EMsgHeader(nullptr),
  m_SCHeader(nullptr),
  m_SFHeader(nullptr),
  m_FModHeader(nullptr),
  m_PIHeader(nullptr),
  m_FRemHeader(nullptr),
  m_PSHeader(nullptr),
  m_POHeader(nullptr),
  m_PModHeader(nullptr),
  m_SReqHeader(nullptr),
  m_SReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

OpenflowHeader::OpenflowHeader (uint8_t type, uint32_t xid) :
  m_version(OFP_VERSION1),
  m_type(type),
  m_length(OPENFLOW_HEADER_LENGTH),
  m_xid(xid),
  m_EMsgHeader(nullptr),
  m_SCHeader(nullptr),
  m_SFHeader(nullptr),
  m_FModHeader(nullptr),
  m_PIHeader(nullptr),
  m_FRemHeader(nullptr),
  m_PSHeader(nullptr),
  m_POHeader(nullptr),
  m_PModHeader(nullptr),
  m_SReqHeader(nullptr),
  m_SReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

OpenflowHeader::OpenflowHeader (uint8_t version, uint8_t type, uint32_t xid) :
  m_version(version),
  m_type(type),
  m_length(OPENFLOW_HEADER_LENGTH),
  m_xid(xid),
  m_EMsgHeader(nullptr),
  m_SCHeader(nullptr),
  m_SFHeader(nullptr),
  m_FModHeader(nullptr),
  m_PIHeader(nullptr),
  m_FRemHeader(nullptr),
  m_PSHeader(nullptr),
  m_POHeader(nullptr),
  m_PModHeader(nullptr),
  m_SReqHeader(nullptr),
  m_SReplyHeader(nullptr)
{
  NS_LOG_FUNCTION(this);
}

OpenflowHeader::~OpenflowHeader ()
{
  NS_LOG_FUNCTION(this);
}

uint8_t OpenflowHeader::GetVersion() const
{
  return m_version;
}

void OpenflowHeader::SetVersion(uint8_t version)
{
  m_version = version;
}

uint8_t OpenflowHeader::GetType() const
{
  return m_type;
}

void OpenflowHeader::SetType(uint8_t type)
{
  m_type = type;
}

uint16_t OpenflowHeader::GetLength() const
{
  return m_length;
}

uint32_t OpenflowHeader::GetXId() const
{
  return m_xid;
}

void OpenflowHeader::SetXId(uint32_t xid)
{
  m_xid = xid;
}

void OpenflowHeader::SetHeader(ofp_header* header)
{
  NS_LOG_FUNCTION(this << &header);

  m_version = header->version;
  m_type = header->type;
  m_length = header->length;
  m_xid = header->xid;
}

Ptr<ErrorMsgHeader> OpenflowHeader::GetErrorMsg()  const
{
  NS_ASSERT(m_EMsgHeader != nullptr);
  return m_EMsgHeader;
}

void OpenflowHeader::SetErrorMsg(Ptr<ErrorMsgHeader> header)
{
  m_EMsgHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<SwitchConfigurationHeader> OpenflowHeader::GetSwitchConfiguration()  const
{
  NS_ASSERT(m_SCHeader != nullptr);
  return m_SCHeader;
}

void OpenflowHeader::SetSwitchConfiguration(Ptr<SwitchConfigurationHeader>header)
{
  m_SCHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<SwitchFeaturesHeader> OpenflowHeader::GetSwitchFeatures() const
{
  NS_ASSERT(m_SFHeader != nullptr);
  return m_SFHeader;
}

void OpenflowHeader::SetSwitchFeatures(Ptr<SwitchFeaturesHeader>header)
{
  m_SFHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<FlowModificationHeader> OpenflowHeader::GetFlowModification() const
{
  NS_ASSERT(m_FModHeader != nullptr);
  return m_FModHeader;
}

void OpenflowHeader::SetFlowModification(Ptr<FlowModificationHeader>header)
{
  m_FModHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<PacketInHeader> OpenflowHeader::GetPacketIn() const
{
  NS_ASSERT(m_PIHeader != nullptr);
  return m_PIHeader;
}

void OpenflowHeader::SetPacketIn(Ptr<PacketInHeader>header)
{
  m_PIHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<FlowRemovedHeader> OpenflowHeader::GetFlowRemoved() const
{
  NS_ASSERT(m_FRemHeader != nullptr);
  return m_FRemHeader;
}

void OpenflowHeader::SetFlowRemoved(Ptr<FlowRemovedHeader> header)
{
  m_FRemHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<PortStatusHeader> OpenflowHeader::GetPortStatus() const
{
  NS_ASSERT(m_PSHeader != nullptr);
  return m_PSHeader;
}

void OpenflowHeader::SetPortStatus(Ptr<PortStatusHeader> header)
{
  m_PSHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<PacketOutHeader> OpenflowHeader::GetPacketOut() const
{
  NS_ASSERT(m_POHeader != nullptr);
  return m_POHeader;
}

void OpenflowHeader::SetPacketOut(Ptr<PacketOutHeader> header)
{
  m_POHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<PortModificationHeader> OpenflowHeader::GetPortModification() const
{
  NS_ASSERT(m_PModHeader != nullptr);
  return m_PModHeader;
}

void OpenflowHeader::SetPortModification(Ptr<PortModificationHeader> header)
{
  m_PModHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<StatsRequestHeader> OpenflowHeader::GetStatsRequest() const
{
  NS_ASSERT(m_SReqHeader != nullptr);
  return m_SReqHeader;
}

void OpenflowHeader::SetStatsRequest(Ptr<StatsRequestHeader> header)
{
  m_SReqHeader = header;
  m_length += header->GetSerializedSize();
}

Ptr<StatsReplyHeader> OpenflowHeader::GetStatsReply() const
{
  NS_ASSERT(m_SReplyHeader != nullptr);
  return m_SReplyHeader;
}

void OpenflowHeader::SetStatsReply(Ptr<StatsReplyHeader> header)
{
  m_SReplyHeader = header;
  m_length += header->GetSerializedSize();
}

TypeId OpenflowHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::OpenflowHeader")
                      .SetParent<Header> ()
                      .AddConstructor<OpenflowHeader> ()
  ;

  return tid;
}

TypeId OpenflowHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void OpenflowHeader::Print(std::ostream &os) const
{
  os << "Openflow Header" << std::endl
     << " Version: " << uint(m_version) << std::endl
     << " Type: " << uint(m_type) << std::endl
     << " Length: " << uint(m_length) << std::endl
     << " Transaction ID: " << uint(m_xid) << std::endl
     << std::endl;
}

uint32_t OpenflowHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return m_length;
}

void OpenflowHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteU8(m_version);
  start.WriteU8(m_type);
  start.WriteHtonU16(m_length);
  start.WriteHtonU32(m_xid);

  switch (m_type)
    {
    case OFPT_ERROR:
      m_EMsgHeader->Serialize(start);
      break;

    case OFPT_FEATURES_REPLY:
      m_SFHeader->Serialize(start);
      break;

    case OFPT_SET_CONFIG:
    case OFPT_GET_CONFIG_REPLY:
      m_SCHeader->Serialize(start);
      break;

    case OFPT_PACKET_IN:
      m_PIHeader->Serialize(start);
      break;

    case OFPT_FLOW_EXPIRED:
      m_FRemHeader->Serialize(start);
      break;

    case OFPT_PORT_STATUS:
      m_PSHeader->Serialize(start);
      break;

    case OFPT_PACKET_OUT:
      m_POHeader->Serialize(start);
      break;

    case OFPT_FLOW_MOD:
      m_FModHeader->Serialize(start);
      break;

    case OFPT_PORT_MOD:
      m_PModHeader->Serialize(start);
      break;

    case OFPT_STATS_REQUEST:
      m_SReqHeader->Serialize(start);
      break;

    case OFPT_STATS_REPLY:
      m_SReplyHeader->Serialize(start);
      break;
    }
  /* Todos os OFPT_* a partir de OFPT_FEATURES_REPLY (6) foram somados em dois pela adição de
     OFPT_VPORT_TABLE_FEATURES_REQUEST e OFPT_VPORT_TABLE_FEATURES_REPLY. Estes tipos foram implementados
     na biblioteca sem seguir nenhuma especificação. Por isso valores constantes estão sendo
     utilizados */
}

uint32_t OpenflowHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_version = start.ReadU8();
  m_type = start.ReadU8();
  m_length = start.ReadNtohU16();
  m_xid = start.ReadNtohU32();

  if (start.GetSize() < (m_length))
    return 0;

  switch (m_type)
    {
    case OFPT_ERROR:
      m_EMsgHeader = Create<ErrorMsgHeader>();
      m_EMsgHeader->Deserialize(start);
      break;

    case OFPT_FEATURES_REPLY:
      m_SFHeader = Create<SwitchFeaturesHeader>();
      m_SFHeader->Deserialize(start);
      break;

    case OFPT_SET_CONFIG:
    case OFPT_GET_CONFIG_REPLY:
      m_SCHeader = Create<SwitchConfigurationHeader>();
      m_SCHeader->Deserialize(start);
      break;

    case OFPT_PACKET_IN:
      m_PIHeader = Create<PacketInHeader>();
      m_PIHeader->Deserialize(start);
      break;

    case OFPT_FLOW_EXPIRED:
      m_FRemHeader = Create<FlowRemovedHeader>();
      m_FRemHeader->Deserialize(start);
      break;

    case OFPT_PORT_STATUS:
      m_PSHeader = Create<PortStatusHeader>();
      m_PSHeader->Deserialize(start);
      break;

    case OFPT_PACKET_OUT:
      m_POHeader = Create<PacketOutHeader>();
      m_POHeader->Deserialize(start);
      break;

    case OFPT_FLOW_MOD:
      m_FModHeader = Create<FlowModificationHeader>();
      m_FModHeader->Deserialize(start);
      break;

    case OFPT_PORT_MOD:
      m_PModHeader = Create<PortModificationHeader>();
      m_PModHeader->Deserialize(start);
      break;

    case OFPT_STATS_REQUEST:
      m_SReqHeader = Create<StatsRequestHeader>();
      m_SReqHeader->Deserialize(start);
      break;

    case OFPT_STATS_REPLY:
      m_SReplyHeader = Create<StatsReplyHeader>();
      m_SReplyHeader->Deserialize(start);
      break;
    }

  return GetSerializedSize();
}
}
