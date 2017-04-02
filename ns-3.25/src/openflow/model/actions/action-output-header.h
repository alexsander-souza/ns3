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

#ifndef ACTION_OUTPUT_HEADER_H
#define ACTION_OUTPUT_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "openflow/openflow.h"

#define OFP_NO_BUFFER (0xffffffff)

namespace ns3 {
class ActionHeader;

/**
 * \class ActionOutputHeader
 * \brief Action header for OFPAT_OUTPUT, which sends packets out ’port’.
 */
class ActionOutputHeader : public Header,
                           public SimpleRefCount<ActionOutputHeader>{
public:

  /* Port numbering.  Physical ports are numbered starting from 0. */
  enum {
    /* Maximum number of physical switch ports. */
    OFPP_MAX = 0xff00,

    /* Fake output "ports". */
    OFPP_IN_PORT    = 0xfff8, /* Send the packet out the input port.  This
                                 virtual port must be explicitly used
                                 in order to send back out of the input
                                 port. */
    OFPP_TABLE      = 0xfff9, /* Perform actions in flow table.
                                 NB: This can only be the destination
                                 port for packet-out messages. */
    OFPP_NORMAL     = 0xfffa, /* Process with normal L2/L3 switching. */
    OFPP_FLOOD      = 0xfffb, /* All physical ports except input port and
                                 those disabled by STP. */
    OFPP_ALL        = 0xfffc, /* All physical ports except input port. */
    OFPP_CONTROLLER = 0xfffd, /* Send to controller. */
    OFPP_LOCAL      = 0xfffe, /* Local openflow "port". */
    OFPP_NONE       = 0xffff, /* Not associated with a physical port. */
  };

  /**
   * \brief  Convert a action header of type output in NS3 format to struct
   * ofp_action_output
   */
  static void ConvertToStruct(ofp_action_output &ofout, Ptr<ActionHeader>header);

  /**
   * Default constructor
   */
  ActionOutputHeader(void);

  /**
   * Constructor
   * \param header   Pointer to struct ofp_action_output
   */
  ActionOutputHeader(ofp_action_output *header);

  /**
   * Constructor
   * \param port             Output port
   * \param maxLen           Max length to send to controller
   */
  ActionOutputHeader(uint16_t port,
                     uint16_t maxLen);

  /**
   * Default destructor
   */
  virtual
  ~ActionOutputHeader();

  /* Getters and Setters*/
  uint16_t GetPort(void) const;
  void     SetPort(uint16_t port);

  uint16_t GetMaxLength(void) const;
  void     SetMaxLength(uint16_t length);

  /* ns3::Header methods */
  static TypeId    GetTypeId(void);
  virtual TypeId   GetInstanceTypeId(void) const;
  virtual void     Print(std::ostream& os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void     Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:

  uint16_t m_port;   /* Output port. */
  uint16_t m_maxLen; /* Max length to send to controller. */
};
} // namespace ns3
#endif  /* ACTION_OUTPUT_HEADER_H */
