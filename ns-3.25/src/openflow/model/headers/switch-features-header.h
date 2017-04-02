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

#ifndef SWITCH_FEATURES_HEADER_H
#define SWITCH_FEATURES_HEADER_H

#include <ns3/header.h>
#include <ns3/mac48-address.h>
#include <ns3/simple-ref-count.h>

#define FEATURES_REPLY_LENGTH 32

namespace ns3
{
class PhysicalPortHeader;

/* Capabilities supported by the datapath. */
enum ofp_capabilities {
  OFPC_FLOW_STATS = 1 << 0,     /* Flow statistics. */
  OFPC_TABLE_STATS = 1 << 1,    /* Table statistics. */
  OFPC_PORT_STATS = 1 << 2,     /* Port statistics. */
  OFPC_STP = 1 << 3,            /* 802.1d spanning tree. */
  OFPC_RESERVED = 1 << 4,       /* Reserved, must be zero. */
  OFPC_IP_REASM = 1 << 5,       /* Can reassemble IP fragments. */
  OFPC_QUEUE_STATS = 1 << 6,    /* Queue statistics. */
  OFPC_ARP_MATCH_IP = 1 << 7    /* Match IP addresses in ARP pkts. */
};

/**
 * \class SwitchFeaturesHeader
 * \brief Openflow protocol switch configuration packet
 */
class SwitchFeaturesHeader : public Header,
                             public SimpleRefCount<SwitchFeaturesHeader>
{
public:
  typedef std::list<Ptr<PhysicalPortHeader> > PhysicalPortsList;

  /**
   * Default constructor
   */
  SwitchFeaturesHeader (void);

  /**
   * Constructor
   * \param datapathId           Datapath unique ID.
   * \param nBuffers             Max packets buffered at once.
   * \param nTables              Number of tables supported by datapath.
   * \param capabilities         Bitmap of support "ofp_capabilities".
   * \param actions              Bitmap of supported "ofp_action_type"s.
   */
  SwitchFeaturesHeader (uint64_t datapathId, uint32_t nBuffers, uint8_t nTables,
                        uint32_t capabilities, uint32_t actions);

  /**
   * Default destructor
   */
  virtual ~SwitchFeaturesHeader ();

  /* Getters and Setters*/
  uint64_t GetDatapathId() const;
  void SetDatapathId(uint64_t datapathId);

  Mac48Address GetDatapathMac() const;

  uint32_t GetNBuffers() const;
  void SetNBuffers(uint32_t nBuffers);

  uint8_t GetNTables() const;
  void SetNTables(uint8_t nTables);

  uint32_t GetCapabilities() const;
  void SetCapabilities(uint32_t capabilities);

  uint32_t GetActions() const;
  void SetActions(uint32_t actions);

  /**
   * Add physical port to message
   * \param port           PhysicalPortHeader
   */
  void AddPort(Ptr<PhysicalPortHeader>port);

  /**
   * Add physical ports list to message
   * \param ports_list        Physical ports list
   */
  void AddPortsList(PhysicalPortsList &ports_list);

  /**
   * Get physical ports list
   */
  const PhysicalPortsList& GetPortsList(void) const;

  /**
   * Search for a port in the message
   * \param number             Port number
   * \returns a pointer to the physical port, or (0) when not found
   */
  Ptr<PhysicalPortHeader> GetPortByNumber(uint16_t number) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint64_t m_datapathId;     /* Datapath unique ID.  Only the lower 48-bits are meaningful. */
  uint32_t m_nBuffers;       /* Max packets buffered at once. */
  uint8_t m_nTables;         /* Number of tables supported by datapath. */

  /* Features. */
  uint32_t m_capabilities;    /* Bitmap of support "ofp_capabilities". */
  uint32_t m_actions;         /* Bitmap of supported "ofp_action_type"s. */

  PhysicalPortsList m_portsList;    /* Port definitions.  The number of ports is inferred from
                                       the length field in the header. */
};
} // namespace ns3
#endif  /* SWITCH_FEATURES_HEADER_H */
