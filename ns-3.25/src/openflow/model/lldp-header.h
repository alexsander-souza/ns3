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
 * Author: Alexsander de Souza  <asouza@inf.ufrgs.br>
 *
 */

#ifndef OF_LLDP_HEADER_H
#define OF_LLDP_HEADER_H

#include <ns3/header.h>
#include <ns3/mac48-address.h>

namespace ns3 {
class BasicLldpHeader : public Header {
public:
  static const uint16_t PROT_NUMBER; //!< LLDP protocol number (0x88cc)

  BasicLldpHeader();

  BasicLldpHeader(const Mac48Address& chassis_id,
                  int                 port,
                  int ttl);

  const Mac48Address& GetChassisId() const;
  int                 GetPortId() const;

  /* ns3::Header methods */
  static TypeId    GetTypeId(void);
  virtual TypeId   GetInstanceTypeId(void) const;
  virtual void     Print(std::ostream& os) const;
  virtual uint32_t GetSerializedSize(void) const;
  virtual void     Serialize(Buffer::Iterator start) const;
  virtual uint32_t Deserialize(Buffer::Iterator start);

private:

  Mac48Address m_chassis_id;
  int m_port;
  int m_ttl;
};
}
#endif // ifndef OF_LLDP_HEADER_H
