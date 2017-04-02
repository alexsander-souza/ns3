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
#include "ns3/action-header.h"
#include "ns3/packet-out-header.h"

NS_LOG_COMPONENT_DEFINE("PacketOutHeader");

namespace ns3
{
using namespace action_utils;

NS_OBJECT_ENSURE_REGISTERED(PacketOutHeader);

PacketOutHeader::PacketOutHeader (void) :
  m_bufferId(0),
  m_inPort(0),
  m_actionsLen(0),
  m_data(nullptr),
  m_dataLength(0)
{
  NS_LOG_FUNCTION(this);
}

PacketOutHeader::PacketOutHeader (uint32_t bufferId, uint16_t inPort, uint16_t actionsLen) :
  m_bufferId(bufferId),
  m_inPort(inPort),
  m_actionsLen(actionsLen),
  m_data(nullptr),
  m_dataLength(0)
{
  NS_LOG_FUNCTION(this);
}

PacketOutHeader::PacketOutHeader (uint32_t bufferId, uint16_t inPort, uint16_t actionsLen,
                                  Ptr<Packet> packet) :
  m_bufferId(bufferId),
  m_inPort(inPort),
  m_actionsLen(actionsLen),
  m_data(nullptr),
  m_dataLength(packet->GetSize())
{
  NS_LOG_FUNCTION(this);

  if (m_dataLength > 0)
    {
      m_data = new u_int8_t[m_dataLength];
      packet->CopyData(m_data, m_dataLength);
    }
}

// PacketOutHeader::PacketOutHeader (const PacketOutHeader& p)
// {
//   NS_LOG_FUNCTION(this);
//
//   m_bufferId = p.GetBufferId();
//   m_inPort = p.GetInPort();
//   m_actionsLen = p.GetActionsLen();
//   m_actionsList = p.GetActionsList();
//   m_dataLength = p.GetDataLength();
//
//   if (m_dataLength > 0)
//     {
//       m_data = new u_int8_t[m_dataLength];
//       p.GetData(m_data, m_dataLength);
//     }
// }

PacketOutHeader::~PacketOutHeader ()
{
  NS_LOG_FUNCTION(this);

  m_actionsList.clear();

  if (m_data != nullptr)
    delete [] m_data;
}

uint32_t PacketOutHeader::GetBufferId() const
{
  return m_bufferId;
}

void PacketOutHeader::SetBufferId(uint32_t bufferId)
{
  m_bufferId = bufferId;
}

uint16_t PacketOutHeader::GetInPort() const
{
  return m_inPort;
}

void PacketOutHeader::SetInPort(uint16_t inPort)
{
  m_inPort = inPort;
}

uint16_t PacketOutHeader::GetActionsLen() const
{
  return m_actionsLen;
}

void PacketOutHeader::SetActionsLen(uint16_t actionsLen)
{
  m_actionsLen = actionsLen;
}

void PacketOutHeader::AddAction(Ptr<ActionHeader> action)
{
  NS_LOG_FUNCTION(this << *action);
  m_actionsList.push_back(action);
}

void PacketOutHeader::AddActionsList(ActionsList actionsList)
{
  NS_LOG_FUNCTION(this);
  m_actionsList.merge(actionsList);
}

const ActionsList& PacketOutHeader::GetActionsList(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_actionsList;
}

Ptr<ActionHeader> PacketOutHeader::GetActionByType(uint16_t type) const
{
  NS_LOG_FUNCTION(this << type);

  for (auto &act: m_actionsList)
    {
      if (act->GetType() == type)
        return act;
    }
  NS_LOG_WARN("Action not found");
  return nullptr;
}

uint16_t PacketOutHeader::GetDataLength() const
{
  return m_dataLength;
}

void PacketOutHeader::GetData(uint8_t *data, uint16_t dataLength) const
{
  NS_ASSERT(dataLength >= m_dataLength);
  memcpy(data, m_data, m_dataLength);
}

void PacketOutHeader::PrintActionsList(std::ostream &os) const
{
  for (auto &act: m_actionsList)
    {
      act->Print(os);

      switch (act->GetType())
        {
        case OFPAT_OUTPUT:
          act->GetOutput()->Print(os);
          break;

        case OFPAT_SET_VLAN_VID:
          act->GetVlanVid()->Print(os);
          break;

        case OFPAT_SET_VLAN_PCP:
          act->GetVlanPcp()->Print(os);
          break;

        case OFPAT_SET_DL_SRC:
          act->GetDlSrcAddress()->Print(os);
          break;

        case OFPAT_SET_DL_DST:
          act->GetDlDstAddress()->Print(os);
          break;

        case OFPAT_SET_NW_SRC:
          act->GetNwSrcAddress()->Print(os);
          break;

        case OFPAT_SET_NW_DST:
          act->GetNwDstAddress()->Print(os);
          break;

        case OFPAT_SET_TP_SRC:
          act->GetTpSrc()->Print(os);
          break;

        case OFPAT_SET_TP_DST:
          act->GetTpDst()->Print(os);
          break;

        case OFPAT_VENDOR:
          act->GetVendor()->Print(os);
          break;
        }
    }
}

TypeId PacketOutHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::PacketOutHeader")
                      .SetParent<Header> ()
                      .AddConstructor<PacketOutHeader> ()
  ;

  return tid;
}

TypeId PacketOutHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void PacketOutHeader::Print(std::ostream &os) const
{
  os << "Packet Out Header" << std::endl
     << " Buffer ID: " << uint(m_bufferId) << std::endl
     << " In Port: " << uint(m_inPort) << std::endl
     << " Actions Length: " << uint(m_actionsLen) << std::endl
     << " Packet Length: " << uint(m_dataLength) << std::endl
     << std::endl;
}

uint32_t PacketOutHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  return 8 + m_actionsLen + m_dataLength;   // 4 + 2 + 2 = 8
}

void PacketOutHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU32(m_bufferId);
  start.WriteHtonU16(m_inPort);
  start.WriteHtonU16(m_actionsLen);

  for (auto &act: m_actionsList)
    {
      act->Serialize(start);
      start.Next(act->GetSerializedSize());
    }

  if (m_dataLength > 0)
    start.Write(m_data, m_dataLength);
}

uint32_t PacketOutHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  if (m_data != nullptr)
    {
      delete [] m_data;
      m_data = nullptr;
    }

  m_bufferId = start.ReadNtohU32();
  m_inPort = start.ReadNtohU16();
  m_actionsLen = start.ReadNtohU16();

  uint16_t actionsReadedSize = 0;

  /* Read options */
  while (actionsReadedSize < m_actionsLen)
    {
      Ptr<ActionHeader> action = Create<ActionHeader>();
      uint16_t actionSize = action->Deserialize(start);
      start.Next(actionSize);
      actionsReadedSize += actionSize;
      m_actionsList.push_back(action);
    }

  int dataBytesCounter = 0;

  while (start.IsEnd() == false)
    {
      start.ReadU8();
      dataBytesCounter++;
    }

  start.Prev(dataBytesCounter);
  m_dataLength = dataBytesCounter;

  if (m_dataLength > 0)
    {
      m_data = new u_int8_t[m_dataLength];
      start.Read(m_data, m_dataLength);
    }

  return GetSerializedSize();
}

// PacketOutHeader& PacketOutHeader::operator=(const PacketOutHeader& other)
// {
//   m_bufferId = other.GetBufferId();
//   m_inPort = other.GetInPort();
//   m_actionsLen = other.GetActionsLen();
//   m_actionsList = other.GetActionsList();
//   m_dataLength = other.GetDataLength();
//
//   if (m_dataLength > 0)
//     {
//       m_data = new u_int8_t[m_dataLength];
//       other.GetData(m_data, m_dataLength);
//     }
//
//   return *this;
// }
} // namespace ns3
