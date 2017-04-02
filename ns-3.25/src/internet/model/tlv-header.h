/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 UFRGS
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

#ifndef __TLV_HEADER_H__
#define __TLV_HEADER_H__

#include <string>
#include <arpa/inet.h>
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/buffer.h"

namespace ns3
{

/**
 * \ingroup TLV
 * \class GenericTlvBase
 * \brief Generic Type-Length-Value object
 */

template <class T, class L> class GenericTlvBase
{
public:
  GenericTlvBase(T type, L length, const uint8_t *value);
  GenericTlvBase(T type);
  GenericTlvBase(T type, uint8_t value);
  GenericTlvBase(T type, uint16_t value);
  GenericTlvBase(T type, uint32_t value);
  GenericTlvBase(const GenericTlvBase<T, L> &other);
  virtual ~GenericTlvBase();

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
  * Get type
  */
  T GetType(void) const;

  /**
   * Set TLV type
   */
  void SetType(T type);

  /**
   * Get value legth
   */
  L GetDataLength(void) const;

  /**
   * Get value
   * \param data           Output buffer
   * \param data_legth     Buffer length
   */
  void GetData(uint8_t *data, uint8_t data_length) const;

  /**
   * Print TLV to stream
   */
  virtual void Print (std::ostream &os) const;

  // Operator overload
  operator uint32_t(void) const;
  operator uint16_t(void) const;
  operator uint8_t(void) const;
  operator std::string(void) const;
  bool operator< (const GenericTlvBase<T, L> &other) const;

protected:
  void WriteSubField(Buffer::Iterator &start, uint8_t value) const;
  void WriteSubField(Buffer::Iterator &start, uint16_t value) const;
  void ReadSubField(Buffer::Iterator &start, uint8_t &value) const;
  void ReadSubField(Buffer::Iterator &start, uint16_t &value) const;
  T         m_type;
  L         m_len;
  uint8_t  *m_data;
};

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(T type, L length, const uint8_t *value):
  m_type(type),
  m_len(length),
  m_data(0)
{
  if (m_len > 0)
    {
      m_data = new uint8_t[m_len];
      memcpy(m_data, value, m_len);
    }
}

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(T type):
  m_type(type),
  m_len(0),
  m_data(0)
{
}

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(T type, uint8_t value):
  m_type(type),
  m_len(1)
{
  m_data = new uint8_t[m_len];
  *m_data = value;
}

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(T type, uint16_t value):
  m_type(type),
  m_len(2)
{
  m_data = new uint8_t[m_len];
  *(uint16_t*)m_data = htons(value);
}

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(T type, uint32_t value):
  m_type(type),
  m_len(4)
{
  m_data = new uint8_t[m_len];
  *(uint32_t*)m_data = htonl(value);
}

template <class T, class L>
GenericTlvBase<T, L>::GenericTlvBase(const GenericTlvBase<T, L> &other):
  m_type(other.m_type),
  m_len(other.m_len),
  m_data(0)
{
  if (m_len > 0)
    {
      m_data = new uint8_t[m_len];
      memcpy(m_data, other.m_data, m_len);
    }
}

template <class T, class L>
GenericTlvBase<T, L>::~GenericTlvBase ()
{
  if (m_data != 0)
    {
      delete [] m_data;
    }
}

template <class T, class L>
uint32_t GenericTlvBase<T,L>::GetSerializedSize (void) const
{
  return (sizeof(T) + sizeof(L) + m_len);
}

template <class T, class L>
void GenericTlvBase<T,L>::WriteSubField(Buffer::Iterator &start, uint8_t value) const
{
  start.WriteU8(value);
}

template <class T, class L>
void GenericTlvBase<T,L>::WriteSubField(Buffer::Iterator &start, uint16_t value) const
{
  start.WriteHtonU16(value);
}

template <class T, class L>
void GenericTlvBase<T,L>::ReadSubField(Buffer::Iterator &start, uint8_t &value) const
{
  value = start.ReadU8();
}

template <class T, class L>
void GenericTlvBase<T,L>::ReadSubField(Buffer::Iterator &start, uint16_t &value) const
{
  value = start.ReadNtohU16();
}

template <class T, class L>
void GenericTlvBase<T,L>::Serialize (Buffer::Iterator start) const
{
  WriteSubField(start, m_type);
  WriteSubField(start, m_len);

  if (m_len > 0)
    {
      start.Write(m_data, m_len);
    }
}

template <class T, class L>
uint32_t GenericTlvBase<T,L>::Deserialize (Buffer::Iterator start)
{
  ReadSubField(start, m_type);
  ReadSubField(start, m_len);

  if (m_len > 0)
    {
      m_data = new uint8_t[m_len];
      start.Read(m_data, m_len);
    }

  return GetSerializedSize();
}

template <class T, class L>
T GenericTlvBase<T,L>::GetType(void) const
{
  return m_type;
}

template <class T, class L>
void GenericTlvBase<T,L>::SetType(T type)
{
  m_type = type;
}

template <class T, class L>
L GenericTlvBase<T,L>::GetDataLength(void) const
{
  return m_len;
}

template <class T, class L>
void GenericTlvBase<T,L>::GetData(uint8_t *data, uint8_t data_length) const
{
  NS_ASSERT(data_length >= m_len);
  memcpy(data, m_data, m_len);
}


template <class T, class L>
GenericTlvBase<T,L>::operator uint32_t(void) const
{
  NS_ASSERT(m_len == sizeof(uint32_t));
  return uint32_t(ntohl(*(uint32_t*)m_data));
}

template <class T, class L>
GenericTlvBase<T,L>::operator uint16_t(void) const
{
  NS_ASSERT(m_len == sizeof(uint16_t));
  return uint16_t(ntohs(*(uint16_t*)m_data));
}

template <class T, class L>
GenericTlvBase<T,L>::operator uint8_t(void) const
{
  NS_ASSERT(m_len == sizeof(uint8_t));
  return uint8_t(*m_data);
}

template <class T, class L>
GenericTlvBase<T,L>::operator std::string(void) const
{
  std::string tmp;
  NS_ASSERT(m_data != 0);
  tmp.assign(reinterpret_cast<char* const>(m_data), int(m_len));
  return tmp;
}

template <class T, class L>
bool GenericTlvBase<T,L>::operator< (const GenericTlvBase<T, L> &other) const
{
  if ( int(m_type) - int(other.m_type) < 0)
    return true;
  else
    return false;
}

template <class T, class L>
void GenericTlvBase<T,L>::Print (std::ostream& os) const
{
  os << "[TLV " << std::to_string(m_type) << " datalen=" << std::to_string(m_len)
    << " total=" << std::to_string(GetSerializedSize()) << "]";
}


} // namespace ns3

#endif /* __TLV_HEADER_H__ */
