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

#ifndef PHYSICAL_PORT_HEADER_H
#define PHYSICAL_PORT_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/openflow-lib.h"

#define PHYSICAL_PORT_LENGTH 48

namespace ns3
{
/**
 * \class PhysicalPortHeader
 * \brief Openflow protocol physical port definition packet
 */
class PhysicalPortHeader : public Header,
                           public SimpleRefCount<PhysicalPortHeader>
{
public:
  /**
   * Default constructor
   */
  PhysicalPortHeader (void);

  /**
   * Constructor
   * \param portNumber         Port number
   * \param hwAddr             Mac Address
   * \param name               Port name
   * \param config             Bitmap of OFPPC_* flags
   * \param state              Bitmap of OFPPS_* flags
   * \param curr               Current features
   * \param advertised         Features being advertised by the port
   * \param supported          Features supported by the port
   * \param peer               Features advertised by peer
   */
  PhysicalPortHeader (uint16_t portNumber, uint8_t hwAddr[], uint8_t name[],
                      uint32_t config, uint32_t state, uint32_t curr,
                      uint32_t advertised, uint32_t supported, uint32_t peer);

  /**
   * Default destructor
   */
  virtual ~PhysicalPortHeader ();

  /* Getters and Setters*/
  uint16_t GetPortNumber() const;
  void SetPortNumber(uint16_t portNumber);

  const uint8_t* GetHwAddr() const;
  void SetHwAddr(uint8_t* hwAddr);

  const uint8_t* GetName() const;
  void SetName(uint8_t* name);

  uint32_t GetConfig() const;
  void SetConfig(uint32_t config);

  uint32_t GetState() const;
  void SetState(uint32_t state);

  uint32_t GetCurr() const;
  void SetCurr(uint32_t curr);

  uint32_t GetAdvertised() const;
  void SetAdvertised(uint32_t advertised);

  uint32_t GetSupported() const;
  void SetSupported(uint32_t supported);

  uint32_t GetPeer() const;
  void SetPeer(uint32_t peer);

  bool operator<(const PhysicalPortHeader &other) const;

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_portNumber;
  uint8_t m_hwAddr[OFP_ETH_ALEN];
  uint8_t m_name[OFP_MAX_PORT_NAME_LEN];   /* Null-terminated */

  uint32_t m_config;          /* Bitmap of OFPPC_* flags. */
  uint32_t m_state;           /* Bitmap of OFPPS_* flags. */

  /* Bitmaps of OFPPF_* that describe features.  All bits zeroed if
   * unsupported or unavailable. */
  uint32_t m_curr;            /* Current features. */
  uint32_t m_advertised;      /* Features being advertised by the port. */
  uint32_t m_supported;       /* Features supported by the port. */
  uint32_t m_peer;            /* Features advertised by peer. */
};
} // namespace ns3
#endif  /* PHYSICAL_PORT_HEADER_H */
