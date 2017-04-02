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

#ifndef OPENFLOW_HEADER_H
#define OPENFLOW_HEADER_H

#include <ns3/header.h>
#include <ns3/simple-ref-count.h>
#include <ns3/openflow-lib.h>

#define OFP_VERSION1            1
#define OPENFLOW_HEADER_LENGTH  8

namespace ns3
{
class ErrorMsgHeader;
class SwitchConfigurationHeader;
class SwitchFeaturesHeader;
class FlowMatchHeader;
class ActionHeader;
class FlowRemovedHeader;
class PortStatusHeader;
class PortModificationHeader;
class StatsRequestHeader;
class StatsReplyHeader;
class FlowModificationHeader;
class PacketOutHeader;
class PacketInHeader;

/**
 * \class OpenflowHeader
 * \brief Openflow protocol packet
 */
class OpenflowHeader : public Header,
                       public SimpleRefCount<OpenflowHeader>
{
public:
  /**
   * \brief  Convert a openflow header in NS3 format to a pointer to struct ofp_header
   */
  static void ConvertToStruct(ofp_header &ofh, Ptr<OpenflowHeader> header);

  /**
   * Default constructor
   */
  OpenflowHeader ();

  /**
   * Constructor
   * \param header   Pointer to struct ofp_header
   */
  OpenflowHeader (ofp_header* header);

  /**
   * Constructor
   * \param type        Type of OF message
   * \param xid         Transaction ID
   * \brief Version is assumed to be 1.0
   */
  OpenflowHeader (uint8_t type, uint32_t xid);

  /**
   * Constructor
   * \param version     OF version
   * \param type        Type of OF message
   * \param xid         Transaction ID
   */
  OpenflowHeader (uint8_t version, uint8_t type, uint32_t xid);

  /**
   * Default destructor
   */
  virtual ~OpenflowHeader ();

  /* Getters and Setters*/
  uint8_t GetVersion() const;
  void SetVersion(uint8_t version);

  uint8_t GetType() const;
  void SetType(uint8_t type);

  uint16_t GetLength() const;
  // void SetLength(uint16_t length);

  uint32_t GetXId() const;
  void SetXId(uint32_t xid);

  void SetHeader(ofp_header* header);

  Ptr<ErrorMsgHeader> GetErrorMsg() const;
  void SetErrorMsg(Ptr<ErrorMsgHeader> header);

  Ptr<SwitchConfigurationHeader> GetSwitchConfiguration() const;
  void SetSwitchConfiguration(Ptr<SwitchConfigurationHeader>header);

  Ptr<SwitchFeaturesHeader> GetSwitchFeatures() const;
  void SetSwitchFeatures(Ptr<SwitchFeaturesHeader>header);

  Ptr<FlowModificationHeader> GetFlowModification() const;
  void SetFlowModification(Ptr<FlowModificationHeader>header);

  Ptr<PacketInHeader> GetPacketIn() const;
  void SetPacketIn(Ptr<PacketInHeader>header);

  Ptr<FlowRemovedHeader> GetFlowRemoved() const;
  void SetFlowRemoved(Ptr<FlowRemovedHeader>header);

  Ptr<PortStatusHeader> GetPortStatus() const;
  void SetPortStatus(Ptr<PortStatusHeader>header);

  Ptr<PacketOutHeader> GetPacketOut() const;
  void SetPacketOut(Ptr<PacketOutHeader>header);

  Ptr<PortModificationHeader> GetPortModification() const;
  void SetPortModification(Ptr<PortModificationHeader>header);

  Ptr<StatsRequestHeader> GetStatsRequest() const;
  void SetStatsRequest(Ptr<StatsRequestHeader>header);

  Ptr<StatsReplyHeader> GetStatsReply() const;
  void SetStatsReply(Ptr<StatsReplyHeader>header);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  OpenflowHeader(const OpenflowHeader&other) = delete;
  OpenflowHeader& operator=(const OpenflowHeader&other) = delete;

  uint8_t m_version;
  uint8_t m_type;
  uint16_t m_length;
  uint32_t m_xid;       // Transaction ID

  Ptr<ErrorMsgHeader>              m_EMsgHeader;
  Ptr<SwitchConfigurationHeader>   m_SCHeader;
  Ptr<SwitchFeaturesHeader>        m_SFHeader;
  Ptr<FlowModificationHeader>      m_FModHeader;
  Ptr<PacketInHeader>              m_PIHeader;
  Ptr<FlowRemovedHeader>           m_FRemHeader;
  Ptr<PortStatusHeader>            m_PSHeader;
  Ptr<PacketOutHeader>             m_POHeader;
  Ptr<PortModificationHeader>      m_PModHeader;
  Ptr<StatsRequestHeader>          m_SReqHeader;
  Ptr<StatsReplyHeader>            m_SReplyHeader;
};
} // namespace ns3
#endif  /* OPENFLOW_HEADER_H */
