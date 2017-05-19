/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB, 2014 UFRGS
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 * Contributor(s):
 *  Alexsander de Souza <asouza@inf.ufrgs.br>
 *  Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *
 */

#ifndef DHCP_HEADER_H
#define DHCP_HEADER_H

#include <string>
#include "ns3/header.h"
#include "ns3/tlv-header.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac48-address.h"

namespace ns3
{

#define DHCP_BOOTPS_PORT    (67)
#define DHCP_BOOTPC_PORT    (68)

/**
 * \ingroup dhcpclientserver
 * \class DhcpOption
 * \brief DHCP protocol options (RFC2132)
 */
class DhcpOption : public GenericTlvBase<uint8_t,uint8_t>
{
public:
  /**
   * DHCP option codes
   */
  enum
  {
    DHCP_OPT_SUBNET_MASK = 1,
    DHCP_OPT_ROUTER = 3,
    DHCP_OPT_DOMAIN_NAME_SERVER = 6,
    DHCP_OPT_DOMAIN_NAME = 15,
    DHCP_OPT_NETWORK_TIME_PROTOCOL_SERVERS = 42,
    DHCP_OPT_VENDOR_SPECIFIC = 43,
    DHCP_OPT_WINS_NBNS_SERVERS = 44,
    DHCP_OPT_WINS_NBT_NODE_TYPE = 46,
    DHCP_OPT_NETBIOS_SCOPE_ID = 47,
    DHCP_OPT_REQUESTED_IP_ADDRESS = 50,
    DHCP_OPT_IP_ADDRESS_LEASE_TIME = 51,
    DHCP_OPT_MESSAGE_TYPE = 53,
    DHCP_OPT_DHCP_SERVER_IDENTIFIER = 54,
    DHCP_OPT_PARAMETER_REQUEST_LIST = 55,
    DHCP_OPT_RENEWAL_TIME_VALUE = 58,
    DHCP_OPT_REBINDING_TIME_VALUE = 59,
    DHCP_OPT_CLIENT_IDENTIFIER = 61,
    DHCP_OPT_END = 255
  };

  /**
   * DHCP_OPT_MESSAGE_TYPE values
   */
  enum
  {
    DHCP_TYPE_DISCOVER = 1,
    DHCP_TYPE_OFFER = 2,
    DHCP_TYPE_REQUEST = 3,
    DHCP_TYPE_ACK = 5,
    DHCP_TYPE_NACK = 6,
    DHCP_TYPE_RELEASE = 7,
  };

  DhcpOption(): GenericTlvBase<uint8_t,uint8_t>(0) {};

  /* Inherit constructors (C++11) */
  using GenericTlvBase<uint8_t,uint8_t>::GenericTlvBase;

  virtual ~DhcpOption() {};

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
};

/**
 * ingroup dhcpclientserver
 * \class DhcpHeader
 * \brief DHCP protocol packet (RFC2132)
 */
class DhcpHeader : public Header
{
public:
  typedef std::list<DhcpOption> DhcpOptionList;
  typedef std::list<DhcpOption>::const_iterator DhcpOptionListCIT;

  /**
   * Message type
   */
  enum
  {
    BOOT_REQUEST = 1,
    BOOT_REPLY = 2,
  };

  /**
   * Default constructor
   */
  DhcpHeader (void);

  /**
   * Default destructor
   */
  virtual ~DhcpHeader ();

  /**
   * Set message type
   * \param  type             BOOT_REQUEST or BOOT_REPLY
   */
  void SetOp (uint8_t type);

  /**
   * Get message type
   */
  uint8_t GetOp (void) const;

  /**
   * Set transaction ID
   * \param xid              Transaction ID
   */
  void SetTransactionId (uint32_t xid);

  /**
   * Get transaction ID
   */
  uint32_t GetTransactionId (void) const;

  /**
   * Set Client IP address
   * \param addr       IPv4 address
   *
   * Filled by client only if it can respond to ARP requests (during RENEW).
   */
  void SetCIAddr (Ipv4Address addr);

  /**
   * Get client IP address
   */
  Ipv4Address GetCIAddr (void) const;

  /**
   * Set Your IP address
   * \param addr      IPv4 address
   *
   * Filled by server with the offered address
   */
  void SetYIAddr (Ipv4Address addr);

  /**
   * Get Your IP address
   */
  Ipv4Address GetYIAddr (void) const;

  /**
   * Set Relay Agent address
   * \param addr      IPv4 address
   *
   * Set by server in OFFER and ACK messages
   */
  void SetSIAddr (Ipv4Address addr);

  /**
   * Get Relay Agent address
   */
  Ipv4Address GetSIAddr (void) const;

  /**
   * Set client HW address
   * \param mac           MAC address
   */
  void SetCHAddr (Mac48Address mac);

  /**
   * Get client HW address
   */
  Mac48Address GetCHAddr (void);

  /**
   * Add DHCP option to message
   * \param option           DHCP option
   */
  void AddOption(DhcpOption &option);

  /**
   * Add DHCP option list to message
   * \param opt_list        DHCP option list
   */
  void AddOptionList(DhcpOptionList &opt_list);

  /**
   * Get Option list
   */
  DhcpOptionList GetOptionList(void) const;

  /**
   * Search for an option in the message
   * \param type             Option type
   *
   * \returns a pointer to the option, or (0) when not found
   */
  const DhcpOption* GetOptionByType(uint8_t type) const;

  /* ns3::Header methods */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint8_t               m_Op;             /**< Message Type */
  uint32_t              m_XID;            /**< Transaction ID */
  Ipv4Address           m_CIAddr;         /**< Client IP address (only if bound) */
  Ipv4Address           m_YIAddr;         /**< your (client) IP address */
  Ipv4Address           m_SIAddr;         /**< IP address of next server to use in bootstrap */
  Ipv4Address           m_GIAddr;         /**< Relay agent IP address */
  Mac48Address          m_CHAddr;         /**< Client hardware address */
  DhcpOptionList        m_Options_list;   /**< Optional parameters list */
};

} // namespace ns3

#endif /* DHCP_HEADER_H */
