/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Alexsander de Souza
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

#ifndef __RADIUS_HEADER_H__
#define __RADIUS_HEADER_H__

#include <ns3/header.h>
#include <ns3/tlv-header.h>

namespace ns3
{

/**
 * RADIUS protocol Attribute-Value Pair
 *
 */
class RadiusAVP : public GenericTlvBase<uint8_t,uint8_t>
{
public:
  enum
  {
    RAD_ATTR_USER_NAME = 1,
    RAD_ATTR_USER_PASSWORD = 2,
    RAD_ATTR_CHAP_PASSWORD = 3,
    RAD_ATTR_NAS_IP_ADDRESS = 4,
    RAD_ATTR_NAS_PORT = 5,
    RAD_ATTR_SERVICE_TYPE = 6,
    RAD_ATTR_FRAMED_PROTOCOL = 7,
    RAD_ATTR_FRAMED_IP_ADDRESS = 8,
    RAD_ATTR_FRAMED_IP_NETMASK = 9,
    RAD_ATTR_FRAMED_ROUTING = 10,
    RAD_ATTR_FILTER_ID = 11,
    RAD_ATTR_FRAMED_MTU = 12,
    RAD_ATTR_FRAMED_COMPRESSION = 13,
    RAD_ATTR_LOGIN_IP_HOST = 14,
    RAD_ATTR_LOGIN_SERVICE = 15,
    RAD_ATTR_LOGIN_TCP_PORT = 16,
    RAD_ATTR_REPLY_MESSAGE = 18,
    RAD_ATTR_CALLBACK_NUMBER = 19,
    RAD_ATTR_CALLBACK_ID = 20,
    RAD_ATTR_FRAMED_ROUTE = 22,
    RAD_ATTR_FRAMED_IPX_NETWORK = 23,
    RAD_ATTR_STATE = 24,
    RAD_ATTR_CLASS = 25,
    RAD_ATTR_VENDOR_SPECIFIC = 26,
    RAD_ATTR_SESSION_TIMEOUT = 27,
    RAD_ATTR_IDLE_TIMEOUT = 28,
    RAD_ATTR_TERMINATION_ACTION = 29,
    RAD_ATTR_CALLED_STATION_ID = 30,
    RAD_ATTR_CALLING_STATION_ID = 31,
    RAD_ATTR_NAS_IDENTIFIER = 32,
    RAD_ATTR_PROXY_STATE = 33,
    RAD_ATTR_LOGIN_LAT_SERVICE = 34,
    RAD_ATTR_LOGIN_LAT_NODE = 35,
    RAD_ATTR_LOGIN_LAT_GROUP = 36,
    RAD_ATTR_FRAMED_APPLETALK_LINK = 37,
    RAD_ATTR_FRAMED_APPLETALK_NETWORK = 38,
    RAD_ATTR_FRAMED_APPLETALK_ZONE = 39,
    RAD_ATTR_ACCT_STATUS_TYPE = 40,
    RAD_ATTR_ACCT_DELAY_TIME = 41,
    RAD_ATTR_ACCT_INPUT_OCTETS = 42,
    RAD_ATTR_ACCT_OUTPUT_OCTETS = 43,
    RAD_ATTR_ACCT_SESSION_ID = 44,
    RAD_ATTR_ACCT_AUTHENTIC = 45,
    RAD_ATTR_ACCT_SESSION_TIME = 46,
    RAD_ATTR_ACCT_INPUT_PACKETS = 47,
    RAD_ATTR_ACCT_OUTPUT_PACKETS = 48,
    RAD_ATTR_ACCT_TERMINATE_CAUSE = 49,
    RAD_ATTR_ACCT_MULTI_SESSION_ID = 50,
    RAD_ATTR_ACCT_LINK_COUNT = 51,
    RAD_ATTR_CHAP_CHALLENGE = 60,
    RAD_ATTR_NAS_PORT_TYPE = 61,
    RAD_ATTR_PORT_LIMIT = 62,
    RAD_ATTR_LOGIN_LAT_PORT = 63,
  };

  enum
  {
    RAD_ACCT_START = 1,
    RAD_ACCT_STOP = 2,
    RAD_ACCT_UPDATE = 3,
    /* don't care about other status */
  };

  enum
  {
    RAD_TERM_CAUSE_USER_REQUEST = 1,
    RAD_TERM_CAUSE_LOST_CARRIER = 2,
    RAD_TERM_CAUSE_LOST_SERVICE = 3,
    RAD_TERM_CAUSE_IDLE_TIMEOUT = 4,
    RAD_TERM_CAUSE_SESSION_TIMEOUT = 5,
    RAD_TERM_CAUSE_ADMIN_RESET = 6,
    RAD_TERM_CAUSE_ADMIN_REBOOT = 7,
    RAD_TERM_CAUSE_PORT_ERROR = 8,
    RAD_TERM_CAUSE_NAS_ERROR = 9,
    RAD_TERM_CAUSE_NAS_REQUEST = 10,
    RAD_TERM_CAUSE_NAS_REBOOT = 11,
    RAD_TERM_CAUSE_PORT_UNNEEDED = 12,
    RAD_TERM_CAUSE_PORT_PREEMPTED = 13,
    RAD_TERM_CAUSE_PORT_SUSPENDED = 14,
    RAD_TERM_CAUSE_SERVICE_UNAVAILABLE = 15,
    RAD_TERM_CAUSE_CALLBACK = 16,
    RAD_TERM_CAUSE_USER_ERROR = 17,
    RAD_TERM_CAUSE_HOST_REQUEST = 18,
  };

  RadiusAVP(): GenericTlvBase<uint8_t,uint8_t>(0) {};

  /* Inherit constructors (C++11) */
  using GenericTlvBase<uint8_t,uint8_t>::GenericTlvBase;

  virtual ~RadiusAVP() {};

  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
};

/**
 * RADIUS protocol message (RFC2865)
 */
class RadiusMessage : public Header
{
public:
  enum
  {
    RAD_INVALID = 0,
    RAD_ACCESS_REQUEST = 1,
    RAD_ACCESS_ACCEPT = 2,
    RAD_ACCESS_REJECT = 3,
    RAD_ACCOUNTING_REQUEST = 4,
    RAD_ACCOUNTING_RESPONSE = 5,
    RAD_ACCESS_CHALLENGE = 11,
  };

  typedef std::list<RadiusAVP> RadiusAvpList;
  typedef std::list<RadiusAVP>::const_iterator RadiusAvpListCIT;

  /**
     * Default constructor
     */
  RadiusMessage(void);

  /**
     * Default destructor
     */
  virtual ~RadiusMessage(void);

  /**
   * Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Return the instance type identifier.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * \param os output stream
   * This method is used by Packet::Print to print the
   * content of a trailer as ascii data to a c++ output stream.
   */
  virtual void Print (std::ostream& os) const;

  /**
   * Get the serialized size of the packet.
   * \return size
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serialize the packet.
   * \param start Buffer iterator
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * Deserialize the packet.
   * \param start Buffer iterator
   * \return size of the packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  void SetMessageCode(uint8_t code);

  uint8_t GetMessageCode(void) const;

  void SetMessageID(uint8_t identifier);

  uint8_t GetMessageID(void) const;

  void AddAttribute(RadiusAVP &attribute);

  void AddAttributeList(RadiusAvpList &attr_list);

  uint16_t GetAttributeNumber (void) const;

  RadiusAvpList GetAttributeList(void) const;

  const RadiusAVP* GetAttributeByType(uint8_t type) const;

  void GetAutheticator(uint8_t authenticator[16]) const;

  void SetAutheticator(uint8_t authenticator[16]);

private:
  uint8_t                    m_code;
  uint8_t                    m_identifier;
  uint8_t                    m_authenticator[16];
  RadiusAvpList              m_avp_list;
};

}

#endif /* __RADIUS_HEADER_H__ */
