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

#include <string>
#include <ns3/packet.h>
#include <ns3/log.h>
#include "radius-header.h"
#include <arpa/inet.h>

NS_LOG_COMPONENT_DEFINE("RadiusHeader");

namespace ns3
{
/*******************************************************************************
 * RADIUS AVP
 ******************************************************************************/
std::ostream & operator <<(std::ostream & os, const RadiusAVP & h)
{
  h.Print(os);
  return os;
}

void RadiusAVP::Serialize(Buffer::Iterator start) const
{
  WriteSubField(start, m_type);
  WriteSubField(start, (uint8_t)(m_len + 2));

  if (m_len > 0)
    {
      start.Write(m_data, m_len);
    }
}

uint32_t RadiusAVP::Deserialize(Buffer::Iterator start)
{
  ReadSubField(start, m_type);
  ReadSubField(start, m_len);
  m_len -= 2;

  if (m_len > 0)
    {
      m_data = new uint8_t[m_len];
      start.Read(m_data, m_len);
    }

  return GetSerializedSize();
}

/*******************************************************************************
 * RADIUS message
 ******************************************************************************/
NS_OBJECT_ENSURE_REGISTERED(RadiusMessage);

TypeId RadiusMessage::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::RadiusMessage")
                      .SetParent<Header> ()
                      .AddConstructor<RadiusMessage> ()
  ;

  return tid;
}

TypeId RadiusMessage::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

RadiusMessage::RadiusMessage ()
  : m_code(0),
  m_identifier(0)
{
  NS_LOG_FUNCTION(this);
}

RadiusMessage::~RadiusMessage ()
{
  NS_LOG_FUNCTION(this);
  m_avp_list.clear();
}

void RadiusMessage::Print(std::ostream& os) const
{
  NS_LOG_FUNCTION(this << &os);
  os << "code " << int(m_code);
  os << " id " << int(m_identifier) << " AVP {";
  for (RadiusAvpListCIT iter = m_avp_list.begin();
       iter != m_avp_list.end(); iter++)
    {
      iter->Print(os);
    }
  os << "} len = " << int(GetSerializedSize());
}

uint32_t RadiusMessage::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  uint32_t msg_size = 1 + 1 + 2 + 16; /* code + id + length + auth */

  if (m_code == RAD_INVALID)
    return 0;

  for (RadiusAvpListCIT iter = m_avp_list.begin();
       iter != m_avp_list.end(); iter++)
    {
      msg_size += iter->GetSerializedSize();
    }

  return msg_size;
}

void RadiusMessage::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  if (m_code == RAD_INVALID)
    return;

  start.WriteU8(m_code);
  start.WriteU8(m_identifier);
  start.WriteU16(htons(u_int16_t(GetSerializedSize())));
  start.Write(m_authenticator, sizeof(m_authenticator));

  for (RadiusAvpListCIT iter = m_avp_list.begin();
       iter != m_avp_list.end(); iter++)
    {
      iter->Serialize(start);
      start.Next(iter->GetSerializedSize());
    }
}

uint32_t RadiusMessage::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  if (start.GetSize() < 20)
    return(0);  /* Invalid message */

  m_code = start.ReadU8();
  m_identifier = start.ReadU8();

  //u_int16_t msg_size = start.ReadU16();
  start.Next(sizeof(u_int16_t)); /* skip message size */
  start.Read(m_authenticator, sizeof(m_authenticator));

  /* Remaining data */
  while (start.IsEnd() == false)
    {
      RadiusAVP avp;
      start.Next(avp.Deserialize(start));
      m_avp_list.push_back(avp);
    }

  return GetSerializedSize();
}

void RadiusMessage::SetMessageCode(uint8_t code)
{
  NS_LOG_FUNCTION(this);
  m_code = code;
}

uint8_t RadiusMessage::GetMessageCode() const
{
  NS_LOG_FUNCTION(this);
  return m_code;
}

void RadiusMessage::SetMessageID(uint8_t identifier)
{
  NS_LOG_FUNCTION(this);
  m_identifier = identifier;
}

uint8_t RadiusMessage::GetMessageID(void) const
{
  NS_LOG_FUNCTION(this);
  return(m_identifier);
}

void RadiusMessage::AddAttribute(RadiusAVP &attribute)
{
  NS_LOG_FUNCTION(this);
  m_avp_list.push_back(attribute);
}

void RadiusMessage::AddAttributeList(RadiusMessage::RadiusAvpList &attr_list)
{
  NS_LOG_FUNCTION(this);
  m_avp_list.merge(attr_list);
}

uint16_t RadiusMessage::GetAttributeNumber(void) const
{
  NS_LOG_FUNCTION(this);
  return m_avp_list.size();
}

RadiusMessage::RadiusAvpList RadiusMessage::GetAttributeList(void) const
{
  return m_avp_list;
}

const RadiusAVP* RadiusMessage::GetAttributeByType(uint8_t type) const
{
  NS_LOG_FUNCTION(this << type);

  for (RadiusAvpListCIT iter = m_avp_list.begin();
       iter != m_avp_list.end(); iter++)
    {
      if (iter->GetType() == type)
        return &(*iter);
    }
  return 0;
}

void RadiusMessage::GetAutheticator(uint8_t authenticator[16]) const
{
  NS_LOG_FUNCTION(this);
  memcpy(authenticator, m_authenticator, sizeof(m_authenticator));
}

void RadiusMessage::SetAutheticator(uint8_t authenticator[16])
{
  NS_LOG_FUNCTION(this);
  memcpy(m_authenticator, authenticator, sizeof(m_authenticator));
}
}
