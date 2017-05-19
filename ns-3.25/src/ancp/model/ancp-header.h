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
#ifndef __ANCP_HEADER_H__
#define __ANCP_HEADER_H__

#include <string>
#include <deque>
#include "ns3/header.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"
#include "ns3/tlv-header.h"

#define ANCP_TIMER            (250) /* ms */

namespace ns3
{
/**
 * ANCP TLV
 *
 * MUST be padded to 4-byte boundary
 */
class AncpTlv : public GenericTlvBase<uint16_t, uint16_t>
{
public:
  /**
   * ANCP TLV codes
   */
  enum {
    TLV_ACCESS_LOOP_CIRCUIT_ID = 0X0001,
    TLV_ACCESS_LOOP_REMOTE_ID = 0X0002,
    TLV_ACCESS_AGGR_CIRCUIT_ID_ASCII = 0X0003,
    TLV_DSL_LINE_ATTRIBUTES = 0X0004,
    TLV_SRV_PROF_NAME = 0X0005,
    TLV_ACCESS_AGGR_CIRCUIT_ID_BINARY = 0X0006,
    TLV_OAM_LB_TEST_PARAM = 0X0007,
    TLV_OPAQUE_DATA = 0X0008,
    TLV_OAM_LB_TEST_RSP = 0X0009,
    TLV_COMMAND = 0X0011,
    TLV_MCAST_SRV_PROF = 0X0013,
    TLV_BW_ALLOCATION = 0X0015,
    TLV_BW_REQUEST = 0X0016,
    TLV_MCAST_SRV_PROF_NAME = 0X0018,
    TLV_MCAST_FLOW = 0X0019,
    TLV_LIST_ACTION = 0X0021,
    TLV_SEQUENCE_NUMBER = 0X0022,
    TLV_WHITE_LIST_CAC = 0X0024,
    TLV_MREPCTL_CAC = 0X0025,
    TLV_ACTUAL_NET_DATA_RATE_UP = 0X0081,
    TLV_ACTUAL_NET_DATA_RATE_DOWN = 0X0082,
    TLV_MIN_NET_DATA_RATE_UP = 0X0083,
    TLV_MIN_NET_DATA_RATE_DOWN = 0X0084,
    TLV_ATT_NET_DATA_RATE_UP = 0X0085,
    TLV_ATT_NET_DATA_RATE_DOWN = 0X0086,
    TLV_MAX_NET_DATA_RATE_UP = 0X0087,
    TLV_MAX_NET_DATA_RATE_DOWN = 0X0088,
    TLV_MIN_NET_LOW_POWER_DATA_RATE_UP = 0X0089,
    TLV_MIN_NET_LOW_POWER_DATA_RATE_DOWN = 0X008A,
    TLV_MAX_INTERLEAVING_DELAY_UP = 0X008B,
    TLV_ACTUAL_INTERLEAVING_DELAY_UP = 0X008C,
    TLV_MAX_INTERLEAVING_DELAY_DOWN = 0X008D,
    TLV_ACTUAL_INTERLEAVING_DELAY_DOWN = 0X008E,
    TLV_DSL_LINE_STATE = 0X008F,
    TLV_ACCESS_LOOP_ENCAP = 0X0090,
    TLV_DSL_TYPE = 0X0091,
    TLV_REQUEST_SOURCE_IP = 0X0092,
    TLV_REQUEST_SOURCE_MAC = 0X0093,
    TLV_REPORT_BUFFERING_TIME = 0X0094,
    TLV_COMMITTED_BW = 0X0095,
    TLV_REQUEST_SOURCE_DEV_ID = 0X0096,
    TLV_STATUS_INFO = 0X0106,
    TLV_TARGET = 0X1000,
  };

  AncpTlv();
  AncpTlv(const AncpTlv& other) = delete;

  /* Inherit constructors (C++11) */
  using GenericTlvBase<uint16_t, uint16_t>::GenericTlvBase;

  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;

  static int PeekTlvType(Buffer::Iterator start);
};

/******************************************************************************/
class AncpTlvDslAttr : public AncpTlv
{
public:
  AncpTlvDslAttr();

  void     SetLineRate(uint32_t upstream, uint32_t downstream);
  void     SetTagMode(uint32_t mode);
  void     SetShowtime(bool enabled);

  uint32_t GetUpstreamRate() const;
  uint32_t GetDownstreamRate() const;
  bool     IsShowtime() const;
  uint32_t GetTagMode() const;
};

/******************************************************************************/
class AncpTlvMCastSrvProfName : public AncpTlv
{
public:
  AncpTlvMCastSrvProfName();

  void SetSrvProfileName(const std::string &profName);
  std::string GetSrvProfileName() const;
};

/******************************************************************************/
class AncpTlvMCastListAction : public AncpTlv
{
public:
  typedef enum {
    ADD = 1,
    DELETE = 2,
    REPLACE = 3
  } OperationType;

  typedef enum {
    IPv4 = 1,
    IPv6 = 2
  } ListType;

  typedef enum {
    WHITELIST = 1,
    BLACKLIST = 2,
    GREYLIST = 3
  } ListClass;

  AncpTlvMCastListAction();
  AncpTlvMCastListAction(const AncpTlv* tlv);
  virtual ~AncpTlvMCastListAction();

  void SetOperation(int oper);
  int  GetOperation() const;
  void SetListType(int listType);
  int  GetListType() const;

  void AddFlow(const Address &group);
  Address GetFlow(int pos) const;
  int GetNFlows() const;

  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);
private:
  std::deque<Address> flows;
};

/******************************************************************************/
class AncpTlvMCastFlow : public AncpTlv
{
public:
  enum {
    ASM = 1,
    SSM = 2
  };

  enum {
    IPv4 = 1,
    IPv6 = 2
  };

  AncpTlvMCastFlow();

  void SetFlowType(int type);
  int GetFlowType() const;
  void SetGroupAddress(const Address& addr);
  Address GetGroupAddress() const;
  void SetGroupSourceAddress(const Address& addr);
  Address GetGroupSourceAddress(int num) const;
  int GetGroupSourceNAddresses() const;
};

/******************************************************************************/
class AncpTlvMCastCommand : public AncpTlv
{
public:
  /**
   * ANCP Command codes [RFC7256]
   */
  typedef enum {
    CMD_ADD = 1,
    CMD_DELETE = 2,
    CMD_DELETE_ALL = 3,
    CMD_ADMISSION_CTRL_REJ = 4,
    CMD_COND_ACCESS_REJ = 5,
  } CommandType;

  AncpTlvMCastCommand();

  void SetCommand(uint16_t cmd);
  uint16_t GetCommand() const;
  void SetFlow(const Address &group);
  void SetFlow(const Address &group, const Address &source);
  const Address  GetFlowGroup() const;
  const Address  GetFlowSource() const;

  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);
private:
  uint16_t command;
  AncpTlvMCastFlow flow;
};

/******************************************************************************/

class AncpTlvMCastServiceProfile : public AncpTlv
{
public:
  AncpTlvMCastServiceProfile();

  AncpTlvMCastSrvProfName& GetFlowName();
  const AncpTlvMCastSrvProfName& GetFlowName() const;
  AncpTlvMCastListAction& GetAction(int color);
  const AncpTlvMCastListAction& GetAction(int color) const;

  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  AncpTlvMCastSrvProfName m_name;
  AncpTlvMCastListAction m_action[3];
};

/******************************************************************************/

/**
 * ANCP Capability
 *
 * MUST be padded to 4-byte boundary
 */
class AncpCapability : public GenericTlvBase<uint16_t, uint16_t>
{
public:
  /**
   * ANCP Capabilities
   */
  enum {
    CAP_DSL_TOPO_DISCOVERY = 1,
    CAP_DSL_LINE_CONFIG = 2,
    CAP_NAS_MCAST_REPLICATION = 3,
    CAP_DSL_LINE_TESTING = 4,
    CAP_COMMITTED_BW_REPORTING = 5,
    CAP_CAC_WHITE_BLACK = 6,
    CAP_CAC_GREY = 7,
    CAP_BW_DELEGATION = 8,
  };


  AncpCapability() : GenericTlvBase<uint16_t, uint16_t>(0)
  {
  }

  /* Inherit constructors (C++11) */
  using GenericTlvBase<uint16_t, uint16_t>::GenericTlvBase;

  virtual ~AncpCapability()
  {
  }

  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
};

/**
 * ANCP protocol message
 */
class AncpHeader : public Header
{
public:
  typedef std::list<AncpTlv*> AncpTlvList;
  typedef std::list<AncpCapability> AncpCapList;

  /**
   * ANCP message types
   */
  enum ancp_msg_type {
    MSG_ADJ_PROTOCOL = 10, /*[RFC6320]*/
    MSG_PORT_MANAGEMENT = 32, /*[RFC6320]*/
    MSG_PORT_UP = 80, /*[RFC6320]*/
    MSG_PORT_DOWN = 81,/*[RFC6320]*/
    MSG_ADJ_UPDATE = 85,/*[RFC6320]*/
    MSG_GEN_RESPONSE = 91,/*[RFC6320]*/
    MSG_PROVISIONING = 93,/*[RFC6320]*/
    MSG_MCAST_REPLICATION_CONTROL = 144,/*[RFC7256]*/
    MSG_MCAST_ADMISSION_CONTROL = 145,/*[RFC7256]*/
    MSG_BW_REALLOCATION_REQUEST = 146,/*[RFC7256]*/
    MSG_BW_TRANSFER_= 147,/*[RFC7256]*/
    MSG_DELEGATED_BW_QUERY = 148,/*[RFC7256]*/
    MSG_MCAST_FLOW_QUERY = 149,/*[RFC7256]*/
    MSG_COMMITTED_BW_REPORT = 150,/*[RFC7256]*/
  };


  /**
   * ANCP Response request mode
   */
  enum {
    RSP_IGNORE = 0x0,
    RSP_NACK = 0x01,
    RSP_ACKALL = 0x2,
    RSP_SUCCESS = 0x3,
    RSP_FAILURE = 0x4,
  };

  /**
   * ANCP Technologies [RFC6320]
   */
  enum {
    TECH_INDEPENDENT = 0,
    TECH_PON = 1,
    TECH_DSL = 5,
  };

  /**
   * ANCP Result codes
   */
  enum {
    RES_NO_RESULT = 0X0,
    RES_INV_REQ_MESSAGE = 0X2,
    RES_PORTS_DOWN = 0X6,
    RES_OUT_OF_RESOURCES = 0X13,
    RES_NOT_IMPLEMENTED = 0X51,
    RES_MALFORMED_MESSAGE = 0X53,
    RES_TLV_MISSING = 0X54,
    RES_INV_TLV_CONTENTS = 0X55,
    RES_COMMAND_ERROR = 0X64,
    RES_INV_FLOW_ADDRESS = 0X65,
    RES_MCAST_FLOW_NOT_EXIST = 0X66,
    RES_INV_PREF_BW_AMOUNT = 0X67,
    RES_INCONSISTENT_VIEWS = 0X68,
    RES_BW_REQ_CONFLICT = 0X69,
    RES_PORTS_NOT_EXIST = 0X500,
    RES_LOOPBACK_TEST_TIMEOUT = 0X501,
    RES_DSL_LINE_STATUS_SHOWTIME = 0X503,
    RES_DSL_LINE_STATUS_IDLE = 0X504,
    RES_DSL_LINE_STATUS_SILENT = 0X505,
    RES_DSL_LINE_STATUS_TRAINING = 0X506,
    RES_DSL_LINE_INTEGRITY_ERROR = 0X507,
    RES_DSLAM_RESOURCE_NOT_AVAILABLE = 0X508,
    RES_INV_TEST_PARAMETER = 0X509,
  };

  /**
   * ANCP Adjacency codes
   */
  enum {
    ADJ_SYN = 1,
    ADJ_SYNACK = 2,
    ADJ_ACK = 3,
    ADJ_RSTACK = 4
  };

  /**
   * Default constructor
   */
  AncpHeader(void);
  AncpHeader(const AncpHeader &other) = delete;

  /**
   * Default destructor
   */
  virtual ~AncpHeader(void);

  /* Setters/Getters */
  uint8_t GetMsgType() const;
  void SetMsgType(uint8_t msgType);

  uint8_t GetAdjCode() const;
  void SetAdjCode(uint8_t code);

  uint8_t GetResponseMode() const;
  void SetResponseMode(uint8_t mode);

  uint16_t GetResultCode() const;
  void SetResultCode(uint16_t result);

  uint8_t GetPartitionId() const;
  void SetPartitionId(uint8_t partId);

  uint32_t GetTransactionId() const;
  void SetTransactionId(uint32_t transId);

  bool GetNAS() const;
  void SetNAS(bool isNas);

  Mac48Address GetSenderName() const;
  void SetSenderName(Mac48Address &sender);

  Mac48Address GetReceiverName() const;
  void SetReceiverName(Mac48Address &receiver);

  uint32_t GetSenderPort() const;
  void SetSenderPort(uint32_t port);

  uint32_t GetReceiverPort() const;
  void SetReceiverPort(uint32_t port);

  uint32_t GetSenderInstance() const;
  void SetSenderInstance(uint32_t instance);

  uint32_t GetReceiverInstance() const;
  void SetReceiverInstance(uint32_t instance);

  void AddTlv(AncpTlv *tlv);
  const AncpTlv* GetTlvByType(uint16_t tlvType) const;

  void AddCapability(AncpCapability &cap);
  void AddCapabilityList(const AncpCapList &capList);
  AncpCapList GetCapabilityList() const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:

  uint8_t m_MsgType;                           /**< ANCP message type */
  uint8_t m_AdjCode;                           /**< ANCP adjacency message code */
  uint8_t m_Result;                            /**< Whether a response is expected */
  uint16_t m_ResultCode;                       /**< Result code */
  uint8_t m_PartitionId;                       /**< AN Partition ID */
  uint32_t m_TransactionId;                    /**< Transaction ID */
  Mac48Address m_SenderName;                   /**< Sender Name, as RFC */
  Mac48Address m_ReceiverName;                 /**< Receiver Name, as RFC */
  uint32_t m_SenderPort;                       /**< Sender port number */
  uint32_t m_ReceiverPort;                     /**< Receiver port number */
  uint32_t m_SenderInstance;                   /**< Sender port number */
  uint32_t m_ReceiverInstance;                 /**< Receiver port number */
  bool m_IsNAS;                                /**< The sender is a NAS */
  AncpTlvList m_Tlvs;                          /**< TLV list */
  AncpCapList m_Capabilities;                  /**< ANCP Agent capabilities */
};

}
#endif /* __ANCP_HEADER_H__ */
