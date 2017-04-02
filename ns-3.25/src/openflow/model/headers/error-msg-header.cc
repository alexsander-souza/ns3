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

#include "ns3/log.h"
#include "ns3/error-msg-header.h"

NS_LOG_COMPONENT_DEFINE("ErrorMsgHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(ErrorMsgHeader);

ErrorMsgHeader::ErrorMsgHeader () :
  m_type(0),
  m_code(0),
  m_data(0),
  m_dataLength(0)
{
  NS_LOG_FUNCTION(this);
}

ErrorMsgHeader::ErrorMsgHeader (uint16_t type, uint16_t code) :
  m_type(type),
  m_code(code),
  m_data(0),
  m_dataLength(0)
{
  NS_LOG_FUNCTION(this);
}

ErrorMsgHeader::ErrorMsgHeader (uint16_t type, uint16_t code, uint8_t *data, uint16_t dataLength) :
  m_type(type),
  m_code(code),
  m_data(0),
  m_dataLength(dataLength)
{
  NS_LOG_FUNCTION(this);

  if (dataLength > 0)
    {
      m_data = new u_int8_t[dataLength];
      memcpy(m_data, data, dataLength);
    }
}

ErrorMsgHeader::~ErrorMsgHeader ()
{
  NS_LOG_FUNCTION(this);

  if (m_data != 0)
    delete [] m_data;
}

uint16_t ErrorMsgHeader::GetType() const
{
  return m_type;
}

void ErrorMsgHeader::SetType(uint16_t type)
{
  m_type = type;
}

uint16_t ErrorMsgHeader::GetCode() const
{
  return m_code;
}

void ErrorMsgHeader::SetCode(uint16_t code)
{
  m_code = code;
}

uint16_t ErrorMsgHeader::GetDataLength() const
{
  return m_dataLength;
}

void ErrorMsgHeader::GetData(uint8_t *data, uint16_t dataLength) const
{
  NS_ASSERT(dataLength >= m_dataLength);
  memcpy(data, m_data, m_dataLength);
}

TypeId ErrorMsgHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::ErrorMsgHeader")
                      .SetParent<Header> ()
                      .AddConstructor<ErrorMsgHeader> ()
  ;

  return tid;
}

TypeId ErrorMsgHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void ErrorMsgHeader::Print(std::ostream &os) const
{
  os << "Error Msg Header" << std::endl
     << " Type: " << uint(m_type) << std::endl
     << " Code: " << uint(m_code) << std::endl
     << " Data Length: " << uint(m_dataLength) << std::endl
     << std::endl;
}

uint32_t ErrorMsgHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);
  return 4 + m_dataLength;   // 2 + 2 = 4
}

void ErrorMsgHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU16(m_type);
  start.WriteHtonU16(m_code);

  if (m_dataLength > 0)
    start.Write(m_data, m_dataLength);
}

uint32_t ErrorMsgHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  if (m_data != 0)
    {
      delete [] m_data;
      m_data = 0;
    }

  m_type = start.ReadNtohU16();
  m_code = start.ReadNtohU16();

  if (m_dataLength > 0)
    {
      m_data = new u_int8_t[m_dataLength];
      start.Read(m_data, m_dataLength);
    }

  return GetSerializedSize();
}
}
