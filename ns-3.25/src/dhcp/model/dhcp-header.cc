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

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "ns3/address-utils.h"
#include "dhcp-header.h"

NS_LOG_COMPONENT_DEFINE("DhcpHeader");

namespace ns3
{
/*******************************************************************************
 * DHCP Options
 ******************************************************************************/
uint32_t DhcpOption::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  if (GetType() == DhcpOption::DHCP_OPT_END)
    return 1;

  return GenericTlvBase<uint8_t, uint8_t>::GetSerializedSize();
}

void DhcpOption::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  if (GetType() == DhcpOption::DHCP_OPT_END)
    {
      start.WriteU8(GetType());
      return;
    }

  GenericTlvBase<uint8_t, uint8_t>::Serialize(start);
}

uint32_t DhcpOption::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  SetType(start.PeekU8());

  if (GetType() == DhcpOption::DHCP_OPT_END)
    return GetSerializedSize();

  return GenericTlvBase<uint8_t, uint8_t>::Deserialize(start);
}

std::ostream & operator <<(std::ostream & os, const DhcpOption & h)
{
  h.Print(os);
  return os;
}

/*******************************************************************************
 * DHCP packet
 ******************************************************************************/
#define MAGIC_COOKIE                    (0x63825363)
#define ETHERNET                        1
#define MAC_ADDRESS_LEN                 6
#define SERVER_NAME_LENGTH              (64)
#define BOOT_FILE_NAME_LENGTH           (128)

NS_OBJECT_ENSURE_REGISTERED(DhcpHeader);

TypeId DhcpHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::DhcpHeader")
                      .SetParent<Header> ()
                      .AddConstructor<DhcpHeader> ()
  ;

  return tid;
}

TypeId DhcpHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

DhcpHeader::DhcpHeader () :
  m_CIAddr(Ipv4Address::GetAny()),
  m_YIAddr(Ipv4Address::GetAny()),
  m_SIAddr(Ipv4Address::GetAny()),
  m_GIAddr(Ipv4Address::GetAny())
{
  NS_LOG_FUNCTION(this);
}

DhcpHeader::~DhcpHeader ()
{
  NS_LOG_FUNCTION(this);
  m_Options_list.clear();
}

void DhcpHeader::Print(std::ostream &os) const
{
  os << " xid " << m_XID;
  os << " ciaddr " << m_CIAddr;
  os << " yiaddr " << m_YIAddr;
  os << " siaddr " << m_SIAddr;
  os << " giaddr " << m_GIAddr;
  os << " chaddr " << m_CHAddr;
  for (DhcpOptionListCIT iter = m_Options_list.begin();
       iter != m_Options_list.end(); iter++)
    {
      os << " | ";
      iter->Print(os);
    }
}

uint32_t DhcpHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  uint32_t msg_size = 1 + 1 + 1 + 1 + 4 + 2 + 2 + 4 + 4 + 4 + 4 + 6
                      + (16 - MAC_ADDRESS_LEN)
                      + SERVER_NAME_LENGTH + BOOT_FILE_NAME_LENGTH
                      + 4; /* Base header (RFC2131) */

  for (DhcpOptionListCIT iter = m_Options_list.begin();
       iter != m_Options_list.end(); iter++)
    {
      msg_size += iter->GetSerializedSize();
    }

  return msg_size;
}

void DhcpHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteU8(m_Op);
  start.WriteU8(uint8_t(ETHERNET));
  start.WriteU8(uint8_t(MAC_ADDRESS_LEN));
  start.WriteU8(uint8_t(0));
  start.WriteU32(m_XID);
  start.WriteHtonU16(uint16_t(0));
  start.WriteU16(uint16_t(0)); /* Always Unicast */

  WriteTo(start, m_CIAddr);
  WriteTo(start, m_YIAddr);
  WriteTo(start, m_SIAddr);
  WriteTo(start, m_GIAddr);
  WriteTo(start, m_CHAddr);

  /* add padding and fill unused fields */
  uint8_t buf[BOOT_FILE_NAME_LENGTH];
  memset(buf, 0, sizeof(buf));

  start.Write(buf, (16 - MAC_ADDRESS_LEN));  /* padding */
  start.Write(buf, (SERVER_NAME_LENGTH));    /* server hostname */
  start.Write(buf, (BOOT_FILE_NAME_LENGTH)); /* boot file name */

  start.WriteHtonU32(uint32_t(MAGIC_COOKIE));

  for (DhcpOptionListCIT iter = m_Options_list.begin();
       iter != m_Options_list.end(); iter++)
    {
      iter->Serialize(start);
      start.Next(iter->GetSerializedSize());
    }
}

uint32_t DhcpHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_Op = start.ReadU8();

  start.Next(3); /* skip htype, hlen and hops */

  m_XID = start.ReadU32();

  start.Next(4); /* skip secs and flags */

  ReadFrom(start, m_CIAddr);
  ReadFrom(start, m_YIAddr);
  ReadFrom(start, m_SIAddr);
  ReadFrom(start, m_GIAddr);
  ReadFrom(start, m_CHAddr);

  start.Next((16 - MAC_ADDRESS_LEN));  /* skip padding */
  start.Next((SERVER_NAME_LENGTH));    /* skip server hostname */
  start.Next((BOOT_FILE_NAME_LENGTH)); /* skip boot file name */
  start.Next(4);                       /* skip magic cookie */

  /* Read options */
  bool hasEndOpt = false;
  while (!hasEndOpt && start.IsEnd() == false)
    {
      DhcpOption option;
      start.Next(option.Deserialize(start));
      m_Options_list.push_back(option);

      hasEndOpt = (option.GetType() == DhcpOption::DHCP_OPT_END);
    }

  NS_ASSERT_MSG(hasEndOpt, "Missing END option");

  return GetSerializedSize();
}

void DhcpHeader::SetOp(uint8_t type)
{
  NS_LOG_FUNCTION(this << type);
  m_Op = type;
}

uint8_t DhcpHeader::GetOp(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_Op;
}

void DhcpHeader::SetTransactionId(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);
  m_XID = xid;
}

uint32_t DhcpHeader::GetTransactionId(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_XID;
}

void DhcpHeader::SetCIAddr(Ipv4Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_CIAddr = addr;
}

Ipv4Address DhcpHeader::GetCIAddr(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_CIAddr;
}

void DhcpHeader::SetYIAddr(Ipv4Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_YIAddr = addr;
}

Ipv4Address DhcpHeader::GetYIAddr(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_YIAddr;
}

void DhcpHeader::SetSIAddr(Ipv4Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_SIAddr = addr;
}

Ipv4Address DhcpHeader::GetSIAddr(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_SIAddr;
}

void DhcpHeader::SetCHAddr(Mac48Address addr)
{
  NS_LOG_FUNCTION(this << addr);
  m_CHAddr = addr;
}

Mac48Address DhcpHeader::GetCHAddr(void)
{
  NS_LOG_FUNCTION_NOARGS();
  return m_CHAddr;
}

void DhcpHeader::AddOption(DhcpOption &option)
{
  NS_LOG_FUNCTION(this << option);
  m_Options_list.push_back(option);
}

void DhcpHeader::AddOptionList(DhcpOptionList &opt_list)
{
  NS_LOG_FUNCTION(this);
  m_Options_list.merge(opt_list);
}

DhcpHeader::DhcpOptionList DhcpHeader::GetOptionList(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_Options_list;
}

const DhcpOption* DhcpHeader::GetOptionByType(uint8_t type) const
{
  NS_LOG_FUNCTION(this << type);

  for (DhcpOptionListCIT iter = m_Options_list.begin();
       iter != m_Options_list.end(); iter++)
    {
      if (iter->GetType() == type)
        return &(*iter);
    }
  return 0;
}
} // namespace ns3
