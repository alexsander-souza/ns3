/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 UFRGS
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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/assert.h"
#include "ns3/simulator.h"
#include "ns3/address-utils.h"
#include "ancp-header.h"

NS_LOG_COMPONENT_DEFINE("AncpHeader");

#define ANCP_PROTO_ID      (0x880C)
#define ANCP_PROTO_VERSION     (50)
#define ANCP_TCPIP_HEADER       (4)
#define ANCP_ADJ_FIXED_HEADER  (36)
#define ANCP_GEN_FIXED_HEADER  (12)

namespace ns3
{
/******************************************************************************/
AncpTlv::AncpTlv() : GenericTlvBase<uint16_t, uint16_t>(0)
{
}

uint32_t AncpTlv::GetSerializedSize(void) const
{
  /* From RFC: The Value field in each TLV MUST be padded with zeroes as
   * required to align with a 4-byte word boundary. */

  uint32_t real_size = GenericTlvBase<uint16_t, uint16_t>::GetSerializedSize();

  return((real_size + 3) & ~3);
}

void AncpTlv::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  uint32_t real_size = GenericTlvBase<uint16_t, uint16_t>::GetSerializedSize();
  uint32_t padded_size = ((real_size + 3) & ~3);

  GenericTlvBase<uint16_t, uint16_t>::Serialize(start);

  if (real_size < padded_size)
    {
      uint8_t pad[4] = { 0, 0, 0, 0 };

      start.Next(real_size);
      start.Write(pad, (padded_size - real_size));
    }
}

int AncpTlv::PeekTlvType(Buffer::Iterator start)
{
  decltype(m_type)tlvType = start.ReadNtohU16();

  return tlvType;
}

std::ostream & operator <<(std::ostream & os, const AncpTlv & h)
{
  h.Print(os);
  return os;
}

/******************************************************************************/
struct dsl_attr_st {
  uint16_t dsl_tlv_id;
  uint16_t dsl_tlv_len;
  uint32_t dsl_data;
  uint16_t uprate_tlv_id;
  uint16_t uprate_tlv_len;
  uint32_t uprate_data;
  uint16_t downrate_tlv_id;
  uint16_t downrate_tlv_len;
  uint32_t downrate_data;
  uint16_t showtime_tlv_id;
  uint16_t showtime_tlv_len;
  uint32_t showtime_data;
  uint16_t tagmode_tlv_id;
  uint16_t tagmode_tlv_len;
  uint8_t tagmode_data_link;
  uint8_t tagmode_data_encap1;
  uint8_t tagmode_data_encap2;
  uint8_t tagmode_data_pad;
};

AncpTlvDslAttr::AncpTlvDslAttr() :
  AncpTlv(TLV_DSL_LINE_ATTRIBUTES)
{
  NS_LOG_FUNCTION(this);

  m_len = sizeof(struct dsl_attr_st);
  m_data = new uint8_t[m_len];

  /* initialize */
  struct dsl_attr_st *dsl_data = (struct dsl_attr_st *)m_data;

  dsl_data->dsl_tlv_id = htons(TLV_DSL_TYPE);
  dsl_data->dsl_tlv_len = htons(4);
  dsl_data->dsl_data = 0; /* other */
  dsl_data->uprate_tlv_id = htons(TLV_ACTUAL_NET_DATA_RATE_UP);
  dsl_data->uprate_tlv_len = htons(4);
  dsl_data->downrate_tlv_id = htons(TLV_ACTUAL_NET_DATA_RATE_DOWN);
  dsl_data->downrate_tlv_len = htons(4);
  dsl_data->showtime_tlv_id = htons(TLV_DSL_LINE_STATE);
  dsl_data->showtime_tlv_len = htons(4);
  dsl_data->tagmode_tlv_id = htons(TLV_ACCESS_LOOP_ENCAP);
  dsl_data->tagmode_tlv_len = htons(3);
  dsl_data->tagmode_data_link = 1; /* Ethernet */
  dsl_data->tagmode_data_encap2 = 0; /* NA */
  dsl_data->tagmode_data_pad = 0;
}

void AncpTlvDslAttr::SetLineRate(uint32_t upstream, uint32_t downstream)
{
  struct dsl_attr_st *dsl_data = (struct dsl_attr_st *)m_data;

  dsl_data->uprate_data = htonl(upstream);
  dsl_data->downrate_data = htonl(downstream);
}

void AncpTlvDslAttr::SetTagMode(uint32_t mode)
{
  struct dsl_attr_st *dsl_data = (struct dsl_attr_st *)m_data;

  dsl_data->tagmode_data_encap1 = mode;
}

void AncpTlvDslAttr::SetShowtime(bool enabled)
{
  struct dsl_attr_st *dsl_data = (struct dsl_attr_st *)m_data;

  dsl_data->showtime_data = (enabled ? htonl(1) : 0);
}

uint32_t AncpTlvDslAttr::GetUpstreamRate() const
{
  struct dsl_attr_st * dsl_data = (struct dsl_attr_st *)m_data;

  return ntohl(dsl_data->uprate_data);
}

uint32_t AncpTlvDslAttr::GetDownstreamRate() const
{
  struct dsl_attr_st * dsl_data = (struct dsl_attr_st *)m_data;

  return ntohl(dsl_data->downrate_data);
}

bool AncpTlvDslAttr::IsShowtime() const
{
  struct dsl_attr_st * dsl_data = (struct dsl_attr_st *)m_data;

  return(dsl_data->showtime_data != 0);
}

uint32_t AncpTlvDslAttr::GetTagMode() const
{
  struct dsl_attr_st * dsl_data = (struct dsl_attr_st *)m_data;

  return dsl_data->tagmode_data_encap1;
}

/******************************************************************************/
AncpTlvMCastSrvProfName::AncpTlvMCastSrvProfName() :
  AncpTlv(TLV_MCAST_SRV_PROF_NAME)
{
}

void AncpTlvMCastSrvProfName::SetSrvProfileName(const std::string &profName)
{
  if (m_len > 0)
    delete [] m_data;

  m_len = profName.size();
  m_data = new uint8_t[m_len];
  std::memcpy(m_data, profName.c_str(), m_len);
}

std::string AncpTlvMCastSrvProfName::GetSrvProfileName() const
{
  return std::string(reinterpret_cast<const char*>(m_data), m_len);
}

/******************************************************************************/
struct mcast_list_action_st {
  uint8_t oper;
  uint8_t listType;
  uint16_t res1;
  uint16_t family;
  uint16_t nFlows;
};

AncpTlvMCastListAction::AncpTlvMCastListAction() :
  AncpTlv(TLV_LIST_ACTION)
{
  NS_LOG_FUNCTION(this);

  m_len = sizeof(struct mcast_list_action_st);
  m_data = new uint8_t[m_len];

  std::memset(m_data, 0, m_len);
  m_data[5] = IPv4;
}

AncpTlvMCastListAction::~AncpTlvMCastListAction()
{
  flows.clear();
}

void AncpTlvMCastListAction::SetOperation(int oper)
{
  m_data[0] = uint8_t(oper);
}

int AncpTlvMCastListAction::GetOperation() const
{
  return m_data[0];
}

void AncpTlvMCastListAction::SetListType(int listType)
{
  m_data[1] = uint8_t(listType);
}

int AncpTlvMCastListAction::GetListType() const
{
  return m_data[1];
}

void AncpTlvMCastListAction::AddFlow(const Address &group)
{
  NS_ASSERT(group.GetLength() == 4 || group.GetLength() == 16);
  flows.push_back(group);
  m_data[5] = (group.GetLength() == 4) ? IPv4 : IPv6;
  m_data[7] += 1;
}

Address AncpTlvMCastListAction::GetFlow(int pos) const
{
  return flows[pos];
}

int AncpTlvMCastListAction::GetNFlows() const
{
  return flows.size();
}

uint32_t AncpTlvMCastListAction::GetSerializedSize(void) const
{
  int flowsSize = flows.size() * (1 + ((m_data[5] == IPv4) ? 4 : 16));

  return AncpTlv::GetSerializedSize() + flowsSize;
}

void AncpTlvMCastListAction::Serialize(Buffer::Iterator start) const
{
  int flowsSize = flows.size() * ((m_data[5] == IPv4) ? 4 : 16);

  WriteSubField(start, m_type);
  WriteSubField(start, uint16_t(m_len + flowsSize));

  start.Write(m_data, m_len);

  for (auto &g: flows)
    {
      std::array<uint8_t, 16> prefix;

      uint8_t pLen = g.CopyTo(prefix.data());

      WriteSubField(start, pLen);
      start.Write(prefix.data(), pLen);
    }
}

uint32_t AncpTlvMCastListAction::Deserialize(Buffer::Iterator start)
{
  ReadSubField(start, m_type);

  decltype(m_len)dummyLen;
  ReadSubField(start, dummyLen);

  start.Read(m_data, m_len);

  int nFlows = m_data[7];
  for (int i = 0; i < nFlows; ++i)
    {
      uint8_t pLen;
      ReadSubField(start, pLen);

      std::array<uint8_t, 16> prefix;
      prefix.fill(0);
      start.Read(prefix.data(), pLen);

      Address group;

      if (m_data[5] == IPv4)
        group = Ipv4Address::Deserialize(prefix.data());
      else
        group = Ipv6Address::Deserialize(prefix.data());

      flows.push_back(group);
    }

  return GetSerializedSize();
}

/******************************************************************************/
struct mcast_flow_st {
  uint8_t flowType;
  uint8_t family;
  uint16_t nSources; // max 1
  uint8_t addresses[16 * 2];
};

AncpTlvMCastFlow::AncpTlvMCastFlow() :
  AncpTlv(TLV_MCAST_FLOW)
{
  NS_LOG_FUNCTION(this);

  m_len = sizeof(struct mcast_flow_st);
  m_data = new uint8_t[m_len];

  /* initialize */
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;
  flow->flowType = ASM;
  flow->family = IPv6;
  flow->nSources = 0;
}

void AncpTlvMCastFlow::SetFlowType(int type)
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  flow->flowType = type;
}

int AncpTlvMCastFlow::GetFlowType() const
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  return flow->flowType;
}

void AncpTlvMCastFlow::SetGroupAddress(const Address& addr)
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  int addrLen = addr.CopyTo(&flow->addresses[0]);

  NS_ASSERT(addrLen == 4 || addrLen == 16);

  flow->family = (addrLen == 4) ? IPv4 : IPv6;

  m_len = 4 + ((flow->nSources + 1) * addrLen);
}

Address AncpTlvMCastFlow::GetGroupAddress() const
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;
  Address group;

  if (flow->family == IPv4)
    group = Ipv4Address::Deserialize(&flow->addresses[0]);
  else
    group = Ipv6Address::Deserialize(&flow->addresses[0]);

  return group;
}

void AncpTlvMCastFlow::SetGroupSourceAddress(const Address& addr)
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  flow->flowType = SSM;
  flow->nSources++;

  int addrLen = (flow->family == IPv4) ? 4 : 16;
  int pos = (addrLen * flow->nSources);

  addr.CopyTo(&flow->addresses[pos]);
  m_len = 4 + ((flow->nSources + 1) * addrLen);
}

Address AncpTlvMCastFlow::GetGroupSourceAddress(int num) const
{
  Address source;
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  if (num > flow->nSources)
    return source;

  int addrLen = (flow->family == IPv4) ? 4 : 16;
  int pos = (addrLen * num);

  if (flow->family == IPv4)
    source = Ipv4Address::Deserialize(&flow->addresses[pos]);
  else
    source = Ipv6Address::Deserialize(&flow->addresses[pos]);
  return source;
}

int AncpTlvMCastFlow::GetGroupSourceNAddresses() const
{
  struct mcast_flow_st *flow = (struct mcast_flow_st *)m_data;

  return flow->nSources;
}

/******************************************************************************/
AncpTlvMCastCommand::AncpTlvMCastCommand() :
  AncpTlv(TLV_COMMAND)
{
  NS_LOG_FUNCTION(this);
  m_len = 4;
  m_data = new uint8_t[m_len];
  std::memset(m_data, 0, m_len);
}

uint32_t AncpTlvMCastCommand::GetSerializedSize(void) const
{
  return AncpTlv::GetSerializedSize() + flow.GetSerializedSize();
}

void AncpTlvMCastCommand::Serialize(Buffer::Iterator start) const
{
  WriteSubField(start, m_type);
  decltype(m_len)dummyLen = GetSerializedSize();
  WriteSubField(start, dummyLen);

  start.Write(m_data, m_len);

  flow.Serialize(start);
}

uint32_t AncpTlvMCastCommand::Deserialize(Buffer::Iterator start)
{
  decltype(m_len)dummyLen;

  ReadSubField(start, m_type);
  ReadSubField(start, dummyLen);

  start.Read(m_data, m_len);

  flow.Deserialize(start);

  return GetSerializedSize();
}

void AncpTlvMCastCommand::SetCommand(uint16_t cmd)
{
  m_data[0] = (cmd >> 8);
  m_data[1] = (cmd & 0x00ff);
}

uint16_t AncpTlvMCastCommand::GetCommand() const
{
  uint16_t val = (m_data[0] << 8) | m_data[1];

  return val;
}

void AncpTlvMCastCommand::SetFlow(const Address &group)
{
  flow.SetFlowType(AncpTlvMCastFlow::ASM);
  flow.SetGroupAddress(group);
}

void AncpTlvMCastCommand::SetFlow(const Address &group, const Address &source)
{
  flow.SetFlowType(AncpTlvMCastFlow::SSM);
  flow.SetGroupAddress(group);
  flow.SetGroupSourceAddress(source);
}

const Address AncpTlvMCastCommand::GetFlowGroup() const
{
  return flow.GetGroupAddress();
}

const Address AncpTlvMCastCommand::GetFlowSource() const
{
  return flow.GetGroupSourceAddress(0);
}

/******************************************************************************/
AncpTlvMCastServiceProfile::AncpTlvMCastServiceProfile() :
  AncpTlv(TLV_MCAST_SRV_PROF)
{
}

AncpTlvMCastSrvProfName& AncpTlvMCastServiceProfile::GetFlowName()
{
  return m_name;
}

const AncpTlvMCastSrvProfName& AncpTlvMCastServiceProfile::GetFlowName() const
{
  return m_name;
}

AncpTlvMCastListAction& AncpTlvMCastServiceProfile::GetAction(int color)
{
  return m_action[color - 1];
}

const AncpTlvMCastListAction& AncpTlvMCastServiceProfile::GetAction(int color) const
{
  return m_action[color - 1];
}

uint32_t AncpTlvMCastServiceProfile::GetSerializedSize(void) const
{
  return sizeof(m_type) + sizeof(m_len)
         + m_name.GetSerializedSize()
         + m_action[0].GetSerializedSize()
         + m_action[1].GetSerializedSize()
         + m_action[2].GetSerializedSize();
}

void AncpTlvMCastServiceProfile::Serialize(Buffer::Iterator start) const
{
  decltype(m_len)tlvLen = m_name.GetSerializedSize() + m_action[0].GetSerializedSize()
                          + m_action[1].GetSerializedSize() + m_action[2].GetSerializedSize();
  WriteSubField(start, m_type);
  WriteSubField(start, tlvLen);

  m_name.Serialize(start);
  start.Next(m_name.GetSerializedSize());

  for (int i = 0; i < 3; ++i)
    {
      m_action[i].Serialize(start);
      start.Next(m_action[i].GetSerializedSize());
    }
}

uint32_t AncpTlvMCastServiceProfile::Deserialize(Buffer::Iterator start)
{
  ReadSubField(start, m_type);
  decltype(m_len)tlvLen;
  ReadSubField(start, tlvLen);

  start.Next(m_name.Deserialize(start));
  for (int i = 0; i < 3; ++i)
    {
      start.Next(m_action[i].Deserialize(start));
    }

  /* FIXME sort actions */

  return GetSerializedSize();
}

/******************************************************************************/
uint32_t AncpCapability::GetSerializedSize(void) const
{
  /* From RFC: The Value field in each TLV MUST be padded with zeroes as
   * required to align with a 4-byte word boundary. */

  uint32_t real_size = GenericTlvBase<uint16_t, uint16_t>::GetSerializedSize();

  return((real_size + 3) & ~3);
}

void AncpCapability::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  uint32_t real_size = GenericTlvBase<uint16_t, uint16_t>::GetSerializedSize();
  uint32_t padded_size = ((real_size + 3) & ~3);

  GenericTlvBase<uint16_t, uint16_t>::Serialize(start);

  if (real_size < padded_size)
    {
      uint8_t pad[4] = { 0, 0, 0, 0 };

      start.Next(real_size);
      start.Write(pad, (padded_size - real_size));
    }
}

std::ostream & operator <<(std::ostream & os, const AncpCapability & h)
{
  h.Print(os);
  return os;
}

/******************************************************************************/
NS_OBJECT_ENSURE_REGISTERED(AncpHeader);

TypeId AncpHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AncpHeader")
                      .SetParent<Header> ()
                      .AddConstructor<AncpHeader> ()
  ;

  return tid;
}

TypeId AncpHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

AncpHeader::AncpHeader(void) :
  m_MsgType(0),
  m_AdjCode(0),
  m_Result(RSP_IGNORE),
  m_ResultCode(RES_NO_RESULT),
  m_PartitionId(0),
  m_TransactionId(0),
  m_SenderName(Mac48Address("00:00:00:00:00:00")),
  m_ReceiverName(Mac48Address("00:00:00:00:00:00")),
  m_SenderPort(0),
  m_ReceiverPort(0),
  m_SenderInstance(0),
  m_ReceiverInstance(0),
  m_IsNAS(false)
{
  NS_LOG_FUNCTION(this);
}

AncpHeader::~AncpHeader(void)
{
  NS_LOG_FUNCTION(this);
  m_Tlvs.remove_if([](AncpTlv *tlv){
      delete tlv;
      return true;
    });
  m_Capabilities.clear();
}

uint8_t AncpHeader::GetMsgType() const
{
  NS_LOG_FUNCTION(this);
  return(m_MsgType);
}

void AncpHeader::SetMsgType(uint8_t msgType)
{
  NS_LOG_FUNCTION(this << msgType);
  m_MsgType = msgType;

  /* Adjust some defaults */
  switch (msgType)
    {
    case MSG_PORT_MANAGEMENT:
    case MSG_MCAST_REPLICATION_CONTROL:
      m_Result = RSP_NACK;
      break;

    case MSG_MCAST_ADMISSION_CONTROL:
      m_Result = RSP_IGNORE;
      break;

    case MSG_PROVISIONING:
    default:
      break;
    }
}

uint8_t AncpHeader::GetAdjCode() const
{
  NS_LOG_FUNCTION(this);
  return(m_AdjCode);
}

void AncpHeader::SetAdjCode(uint8_t code)
{
  NS_LOG_FUNCTION(this << code);
  m_AdjCode = code;
}

uint8_t AncpHeader::GetResponseMode() const
{
  NS_LOG_FUNCTION(this);
  return(m_Result);
}

void AncpHeader::SetResponseMode(uint8_t mode)
{
  NS_LOG_FUNCTION(this << mode);
  m_Result = (mode & 0x7F); /* Mode is a 7-bit field */
}

uint16_t AncpHeader::GetResultCode() const
{
  NS_LOG_FUNCTION(this);
  return(m_ResultCode);
}

void AncpHeader::SetResultCode(uint16_t result)
{
  NS_LOG_FUNCTION(this << result);
  m_ResultCode = result;
}

uint8_t AncpHeader::GetPartitionId() const
{
  NS_LOG_FUNCTION(this);
  return(m_PartitionId);
}

void AncpHeader::SetPartitionId(uint8_t partId)
{
  NS_LOG_FUNCTION(this << partId);
  m_PartitionId = partId;
}

uint32_t AncpHeader::GetTransactionId() const
{
  NS_LOG_FUNCTION(this);
  return(m_TransactionId);
}

void AncpHeader::SetTransactionId(uint32_t transId)
{
  NS_LOG_FUNCTION(this << transId);
  m_TransactionId = transId;
}

bool AncpHeader::GetNAS() const
{
  NS_LOG_FUNCTION(this);
  return(m_IsNAS);
}

void AncpHeader::SetNAS(bool isNas)
{
  NS_LOG_FUNCTION(this << isNas);
  m_IsNAS = isNas;
}

Mac48Address AncpHeader::GetSenderName() const
{
  NS_LOG_FUNCTION(this);
  return(m_SenderName);
}

void AncpHeader::SetSenderName(Mac48Address &sender)
{
  NS_LOG_FUNCTION(this << sender);
  m_SenderName = sender;
}

Mac48Address AncpHeader::GetReceiverName() const
{
  NS_LOG_FUNCTION(this);
  return(m_ReceiverName);
}

void AncpHeader::SetReceiverName(Mac48Address &receiver)
{
  NS_LOG_FUNCTION(this << receiver);
  m_ReceiverName = receiver;
}

uint32_t AncpHeader::GetSenderPort() const
{
  NS_LOG_FUNCTION(this);
  return(m_SenderPort);
}
void AncpHeader::SetSenderPort(uint32_t port)
{
  NS_LOG_FUNCTION(this << port);
  m_SenderPort = port;
}

uint32_t AncpHeader::GetReceiverPort() const
{
  NS_LOG_FUNCTION(this);
  return(m_ReceiverPort);
}

void AncpHeader::SetReceiverPort(uint32_t port)
{
  NS_LOG_FUNCTION(this << port);
  m_ReceiverPort = port;
}

uint32_t AncpHeader::GetSenderInstance() const
{
  NS_LOG_FUNCTION(this);
  return(m_SenderInstance);
}

void AncpHeader::SetSenderInstance(uint32_t instance)
{
  NS_LOG_FUNCTION(this << instance);
  m_SenderInstance = (instance & 0x0FFF); /* instance is a 24-bit field */
}

uint32_t AncpHeader::GetReceiverInstance() const
{
  NS_LOG_FUNCTION(this);
  return(m_ReceiverInstance);
}
void AncpHeader::SetReceiverInstance(uint32_t instance)
{
  NS_LOG_FUNCTION(this);
  m_ReceiverInstance = (instance & 0x0FFF); /* instance is a 24-bit field */
}

void AncpHeader::AddTlv(AncpTlv *tlv)
{
  NS_ABORT_IF(m_MsgType == MSG_ADJ_PROTOCOL);
  NS_LOG_FUNCTION(this << *tlv);
  m_Tlvs.push_back(tlv);
}

const AncpTlv* AncpHeader::GetTlvByType(uint16_t tlvType) const
{
  NS_LOG_FUNCTION(this << tlvType);

  if (m_MsgType == MSG_ADJ_PROTOCOL)
    return nullptr;

  for (auto &tlv: m_Tlvs)
    {
      if (tlv->GetType() == tlvType)
        return tlv;
    }
  return nullptr;
}

void AncpHeader::AddCapability(AncpCapability &cap)
{
  NS_ABORT_IF(m_MsgType != MSG_ADJ_PROTOCOL);
  NS_LOG_FUNCTION(this << cap);
  m_Capabilities.push_back(cap);
}

void AncpHeader::AddCapabilityList(const AncpCapList &capList)
{
  NS_ABORT_IF(m_MsgType != MSG_ADJ_PROTOCOL);
  NS_LOG_FUNCTION(this);
  m_Capabilities = capList;
}

AncpHeader::AncpCapList AncpHeader::GetCapabilityList() const
{
  NS_ABORT_IF(m_MsgType != MSG_ADJ_PROTOCOL);
  NS_LOG_FUNCTION(this);
  return(m_Capabilities);
}

uint32_t AncpHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  uint32_t msg_size = ANCP_TCPIP_HEADER; /* ANCP over TCP/IP encap header */

  switch (m_MsgType)
    {
    case MSG_ADJ_PROTOCOL:
      msg_size += ANCP_ADJ_FIXED_HEADER; /* fixed fields */
      for (auto &cap: m_Capabilities)
        {
          msg_size += cap.GetSerializedSize();
        }
      break;


    case MSG_PORT_UP:
    case MSG_PORT_DOWN:
    case MSG_PORT_MANAGEMENT:
    case MSG_PROVISIONING:
    case MSG_MCAST_REPLICATION_CONTROL:
    case MSG_MCAST_ADMISSION_CONTROL:

      msg_size += ANCP_GEN_FIXED_HEADER; /* fixed fields */
      msg_size += 20; /* Reserved block */
      msg_size += 8; /* Extension block header */
      for (auto &tlv: m_Tlvs)
        {
          msg_size += tlv->GetSerializedSize();
        }
      break;

    default:
      NS_LOG_WARN("Don't known message size!");
      break;
    }

  return(msg_size);
}

void AncpHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this);

  uint32_t total_size = GetSerializedSize();

  /* ANCP encap Header */
  start.WriteHtonU16(ANCP_PROTO_ID);
  start.WriteHtonU16(total_size - 4);

  /* Common fields */
  start.WriteU8(ANCP_PROTO_VERSION);
  start.WriteU8(m_MsgType);

  switch (m_MsgType)
    {
    case MSG_ADJ_PROTOCOL:
      start.WriteU8(ANCP_TIMER); /* Hard-coded at the moment */

      if (m_IsNAS)
        start.WriteU8(0x80 | m_AdjCode); /* Set M-Bit */
      else
        start.WriteU8(m_AdjCode);

      WriteTo(start, m_SenderName);
      WriteTo(start, m_ReceiverName);
      start.WriteU32(m_SenderPort);
      start.WriteU32(m_ReceiverPort);

      start.WriteU32(0x01000000 | m_SenderInstance); /* New Adjacency + Sender Instance */
      start.WriteU32((m_PartitionId << 24) | m_ReceiverInstance);
      start.WriteU8(0x0); /* Reserved */

      /* Add capabilities */
      start.WriteU8(uint8_t(m_Capabilities.size()));
      start.WriteHtonU16(total_size - (ANCP_ADJ_FIXED_HEADER + ANCP_TCPIP_HEADER));

      for (auto &cap: m_Capabilities)
        {
          cap.Serialize(start);
          start.Next(cap.GetSerializedSize());
        }
      break;

    case MSG_PORT_UP:
    case MSG_PORT_DOWN:
      start.WriteU16((m_Result << 12)); /* ResultCode MUST be zero */
      start.WriteU32((m_PartitionId << 24)); /* TransactionId MUST be zero */
      start.WriteHtonU16(0x8000); /* no frags */
      start.WriteHtonU16(total_size - (ANCP_TCPIP_HEADER));

      for (int i = 0; i < 5; ++i)
        start.WriteU32(0); /* Reserved */

      start.WriteU8(0); /* reserved */
      start.WriteU8(m_MsgType);
      start.WriteU8(TECH_DSL);
      start.WriteU8(0); /* reserved */

      /* Add TLVs */
      start.WriteHtonU16(uint16_t(m_Tlvs.size()));
      start.WriteHtonU16(total_size - (ANCP_GEN_FIXED_HEADER + ANCP_TCPIP_HEADER + 20));
      for (auto &tlv: m_Tlvs)
        {
          tlv->Serialize(start);
          start.Next(tlv->GetSerializedSize());
        }
      break;

    case MSG_PROVISIONING:
    case MSG_MCAST_REPLICATION_CONTROL:
    case MSG_MCAST_ADMISSION_CONTROL:
    case MSG_PORT_MANAGEMENT:
      start.WriteHtonU16((m_Result << 12)); /* ResultCode MUST be zero */
      start.WriteHtonU32((m_PartitionId << 24) | (m_TransactionId & 0X00FFFFFF));
      start.WriteHtonU16(0x8000); /* no frags */
      start.WriteHtonU16(total_size - (ANCP_TCPIP_HEADER));

      for (int i = 0; i < 3; ++i)
        start.WriteU32(0); /* Reserved */

      start.WriteU16(0); /* reserved */
      start.WriteU8(8); /* Function=8 */
      start.WriteU8(0); /* X-Func=0 */
      start.WriteU32(0); /* reserved */
      start.WriteU8(0); /* reserved */
      start.WriteU8(m_MsgType);
      start.WriteU16(0); /* reserved */

      /* Add TLVs */
      start.WriteHtonU16(uint16_t(m_Tlvs.size()));
      start.WriteHtonU16(total_size - (ANCP_GEN_FIXED_HEADER + ANCP_TCPIP_HEADER + 20));
      for (auto &tlv: m_Tlvs)
        {
          tlv->Serialize(start);
          start.Next(tlv->GetSerializedSize());
        }
      break;

    default:
      NS_LOG_WARN("Unable to serialize ANCP message code " << uint(m_MsgType));
      break;
    }
}

uint32_t AncpHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this);

  uint8_t n_caps = 0;
  uint16_t n_tlvs = 0;

  if (start.GetSize() < ANCP_TCPIP_HEADER)
    return 0;

  /* ANCP TCP/IP encap header */
  uint16_t ancp_proto_id = start.ReadNtohU16();
  uint16_t total_size = start.ReadNtohU16();
  uint16_t proto_ver = start.ReadU8();

  if ((ancp_proto_id != ANCP_PROTO_ID) ||
      (proto_ver != ANCP_PROTO_VERSION) ||
      (start.GetSize() < (total_size + 4U)))
    return 0;

  m_MsgType = start.ReadU8();

  switch (m_MsgType)
    {
    case MSG_ADJ_PROTOCOL:
      start.Next(1); /* Timer is hard-coded */

      if (start.PeekU8() & 0x80)
        m_IsNAS = true;

      m_AdjCode = start.ReadU8() & (~0x80);

      ReadFrom(start, m_SenderName);
      ReadFrom(start, m_ReceiverName);
      m_SenderPort = start.ReadU32();
      m_ReceiverPort = start.ReadU32();

      /* ignore P-Type and P flag */
      m_SenderInstance = start.ReadU32() & (0x00FFFFFF);

      m_PartitionId = start.PeekU8();
      m_ReceiverInstance = start.ReadU32() & (0x00FFFFFF);

      start.Next(1); /* Reserved */
      n_caps = start.ReadU8();
      start.Next(2); /* CAPs size */

      /* Read Caps */
      while (n_caps-- > 0)
        {
          AncpCapability cap;
          start.Next(cap.Deserialize(start));
          m_Capabilities.push_back(cap);
        }
      break;

    case MSG_PORT_MANAGEMENT:
    case MSG_PROVISIONING:
    case MSG_MCAST_REPLICATION_CONTROL:
    case MSG_MCAST_ADMISSION_CONTROL:
    /* XXX not really the same format, but all relevant fields
     * are in the same position */
    /* fall-through */
    case MSG_PORT_UP:
    case MSG_PORT_DOWN:
      m_Result = start.PeekU8() >> 4;
      m_ResultCode = start.ReadNtohU16() & 0x0FFF;
      m_PartitionId = start.PeekU8();
      m_TransactionId = start.ReadNtohU32() & 0x00FFFFFF;
      start.Next(2); /* no frags */
      start.Next(2); /* Header size */
      start.Next(20); /* Reserved */
      start.Next(4); /* Extension block header */

      n_tlvs = start.ReadNtohU16();
      start.Next(2); /* TLVs size */

      /* Read TLVs */
      while (n_tlvs-- > 0)
        {
          AncpTlv *tlv = nullptr;
          switch (AncpTlv::PeekTlvType(start))
            {
            case AncpTlv::TLV_DSL_LINE_ATTRIBUTES:
              tlv = new AncpTlvDslAttr();
              break;

            case AncpTlv::TLV_MCAST_SRV_PROF_NAME:
              tlv = new AncpTlvMCastSrvProfName();
              break;

            case AncpTlv::TLV_LIST_ACTION:
              tlv = new AncpTlvMCastListAction();
              break;

            case AncpTlv::TLV_MCAST_FLOW:
              tlv = new AncpTlvMCastFlow();
              break;

            case AncpTlv::TLV_COMMAND:
              tlv = new AncpTlvMCastCommand();
              break;

            case AncpTlv::TLV_MCAST_SRV_PROF:
              tlv = new AncpTlvMCastServiceProfile();
              break;

            default:
              tlv = new AncpTlv();
            }

          start.Next(tlv->Deserialize(start));
          m_Tlvs.push_back(tlv);
        }

      break;

    default:
      NS_LOG_WARN("Unable to deserialize ANCP message code " << uint(m_MsgType));
      break;
    }

  uint16_t actual_size = GetSerializedSize();

  NS_ABORT_IF(actual_size != (total_size + 4));

  return actual_size;
}

void AncpHeader::Print(std::ostream &os) const
{
  os << "ANCP Type " << uint(m_MsgType) << " LEN " << uint(GetSerializedSize());

  for (auto &tlv: m_Tlvs)
    {
      os << *tlv;
    }
}
}
