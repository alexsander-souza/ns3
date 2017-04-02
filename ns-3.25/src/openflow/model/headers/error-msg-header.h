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

#ifndef ERROR_MSG_HEADER_H
#define ERROR_MSG_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/openflow-lib.h"

#define ERROR_MSG_LENGTH 12

namespace ns3
{
/* Values for ’type’ in ofp_error_message. These values are immutable: they
 * will not change in future versions of the protocol (although new values may
 * be added). */
enum ofp_error_type {
  OFPET_HELLO_FAILED,       /* Hello protocol failed. */
  OFPET_BAD_REQUEST,        /* Request was not understood. */
  OFPET_BAD_ACTION,         /* Error in action description. */
  OFPET_FLOW_MOD_FAILED,    /* Problem modifying flow entry. */
  OFPET_PORT_MOD_FAILED,    /* Port mod request failed. */
  OFPET_QUEUE_OP_FAILED     /* Queue operation failed. */
};

/* ofp_error_msg ’code’ values for OFPET_HELLO_FAILED. ’data’ contains an
 * ASCII text string that may give failure details. */
enum ofp_hello_failed_code {
  OFPHFC_INCOMPATIBLE,    /* No compatible version. */
  OFPHFC_EPERM            /* Permissions error. */
};

/* ofp_error_msg ’code’ values for OFPET_BAD_REQUEST. ’data’ contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_request_code {
  OFPBRC_BAD_VERSION,     /* ofp_header.version not supported. */
  OFPBRC_BAD_TYPE,        /* ofp_header.type not supported. */
  OFPBRC_BAD_STAT,        /* ofp_stats_request.type not supported. */
  OFPBRC_BAD_VENDOR,      /* Vendor not supported (in ofp_vendor_header
                           * or ofp_stats_request or ofp_stats_reply). */
  OFPBRC_BAD_SUBTYPE,     /* Vendor subtype not supported. */
  OFPBRC_EPERM,           /* Permissions error. */
  OFPBRC_BAD_LEN,         /* Wrong request length for type. */
  OFPBRC_BUFFER_EMPTY,    /* Specified buffer has already been used. */
  OFPBRC_BUFFER_UNKNOWN   /* Specified buffer does not exist. */
};

/* ofp_error_msg ’code’ values for OFPET_BAD_ACTION. ’data’ contains at least
 * the first 64 bytes of the failed request. */
enum ofp_bad_action_code {
  OFPBAC_BAD_TYPE,          /* Unknown action type. */
  OFPBAC_BAD_LEN,           /* Length problem in actions. */
  OFPBAC_BAD_VENDOR,        /* Unknown vendor id specified. */
  OFPBAC_BAD_VENDOR_TYPE,   /* Unknown action type for vendor id. */
  OFPBAC_BAD_OUT_PORT,      /* Problem validating output action. */
  OFPBAC_BAD_ARGUMENT,      /* Bad action argument. */
  OFPBAC_EPERM,             /* Permissions error. */
  OFPBAC_TOO_MANY,          /* Can’t handle this many actions. */
  OFPBAC_BAD_QUEUE          /* Problem validating output queue. */
};

/* ofp_error_msg ’code’ values for OFPET_FLOW_MOD_FAILED. ’data’ contains
 * at least the first 64 bytes of the failed request. */
enum ofp_flow_mod_failed_code {
  OFPFMFC_ALL_TABLES_FULL,      /* Flow not added because of full tables. */
  OFPFMFC_OVERLAP,              /* Attempted to add overlapping flow with
                                 * CHECK_OVERLAP flag set. */
  OFPFMFC_EPERM,                /* Permissions error. */
  OFPFMFC_BAD_EMERG_TIMEOUT,    /* Flow not added because of non-zero idle/hard
                                 * timeout. */
  OFPFMFC_BAD_COMMAND,          /* Unknown command. */
  OFPFMFC_UNSUPPORTED           /* Unsupported action list - cannot process in
                                 * the order specified. */
};

/* ofp_error_msg ’code’ values for OFPET_PORT_MOD_FAILED. ’data’ contains
 * at least the first 64 bytes of the failed request. */
enum ofp_port_mod_failed_code {
  OFPPMFC_BAD_PORT,       /* Specified port does not exist. */
  OFPPMFC_BAD_HW_ADDR,    /* Specified hardware address is wrong. */
};

/* ofp_error msg ’code’ values for OFPET_QUEUE_OP_FAILED. ’data’ contains
 * at least the first 64 bytes of the failed request */
enum ofp_queue_op_failed_code {
  OFPQOFC_BAD_PORT,     /* Invalid port (or port does not exist). */
  OFPQOFC_BAD_QUEUE,    /* Queue does not exist. */
  OFPQOFC_EPERM         /* Permissions error. */
};

/**
 * \class ErrorMsgHeader
 * \brief Openflow protocol packet
 */
class ErrorMsgHeader : public Header,
                       public SimpleRefCount<ErrorMsgHeader>
{
public:
  /**
   * Default constructor
   */
  ErrorMsgHeader ();

  /**
   * Constructor
   * \param type
   * \param code
   */
  ErrorMsgHeader (uint16_t type, uint16_t code);

  /**
   * Constructor
   * \param type
   * \param code
   * \param data        Variable-length data. Interpreted based on the type and code
   * \param dataLength  Size of data
   */
  ErrorMsgHeader (uint16_t type, uint16_t code, uint8_t *data, uint16_t dataLength);

  /**
   * Default destructor
   */
  virtual ~ErrorMsgHeader ();

  /* Getters and Setters*/
  uint16_t GetType() const;
  void SetType(uint16_t type);

  uint16_t GetCode() const;
  void SetCode(uint16_t code);

  uint16_t GetDataLength() const;

  void GetData(uint8_t *data, uint16_t dataLength) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_type;
  uint16_t m_code;
  uint8_t *m_data;          /* Variable-length data. Interpreted based
                               on the type and code. */
  uint16_t m_dataLength;    /* Added to allocate the exact amount of memory needed for this header */
};
} // namespace ns3
#endif  /* ERROR_MSG_HEADER_H */
