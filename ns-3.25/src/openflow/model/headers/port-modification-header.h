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

#ifndef PORT_MODIFICATION_HEADER_H
#define PORT_MODIFICATION_HEADER_H

#include "ns3/header.h"
#include <ns3/simple-ref-count.h>
#include "ns3/openflow-lib.h"

#define PORT_MODIFICATION_LENGTH 32

namespace ns3
{
/**
 * \class PortModificationHeader
 * \brief Openflow protocol port modification packet
 */
class PortModificationHeader : public Header,
                               public SimpleRefCount<PortModificationHeader>
{
public:

  /**
   * Default constructor
   */
  PortModificationHeader (void);

  /**
   * Constructor
   * \param portNumber         Number of the port to be modified
   * \param hwAddr             The hardware address is not configurable. This is used to
                              sanity-check the request, so it must be the same as returned
                              in an ofp_phy_port struct
   * \param config             Bitmap of OFPPC_* flags
   * \param mask               Bitmap of OFPPC_* flags to be changed
   * \param advertise          Bitmap of "ofp_port_features"s. Zero all
                              bits to prevent any action taking place
   */

  PortModificationHeader (uint16_t portNumber, uint8_t hwAddr[], uint32_t config,
                          uint32_t mask, uint32_t advertise);

  /**
   * Default destructor
   */
  virtual ~PortModificationHeader ();

  /* Getters and Setters*/
  uint16_t GetPortNumber() const;
  void SetPortNumber(uint16_t portNumber);

  const uint8_t* GetHwAddr() const;
  void SetHwAddr(uint8_t* hwAddr);

  uint32_t GetConfig() const;
  void SetConfig(uint32_t config);

  uint32_t GetMask() const;
  void SetMask(uint32_t mask);

  uint32_t GetAdvertise() const;
  void SetAdvertise(uint32_t advertise);

  /* ns3::Header methods */
  static TypeId GetTypeId(void);
  virtual TypeId GetInstanceTypeId(void) const;
  virtual void Print(std::ostream &os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:
  uint16_t m_portNumber;
  uint8_t m_hwAddr[OFP_ETH_ALEN];   /* The hardware address is not configurable. This is used to
                                       sanity-check the request, so it must be the same as returned
                                       in an ofp_phy_port struct. */
  uint32_t m_config;                /* Bitmap of OFPPC_* flags. */
  uint32_t m_mask;                  /* Bitmap of OFPPC_* flags to be changed. */
  uint32_t m_advertise;             /* Bitmap of "ofp_port_features"s. Zero all
                                       bits to prevent any action taking place. */
};
} // namespace ns3
#endif  /* PORT_MODIFICATION_HEADER_H */
