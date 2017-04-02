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
#include "ns3/flow-match-header.h"
#include "ns3/action-header.h"
#include "ns3/flow-modification-header.h"

NS_LOG_COMPONENT_DEFINE("FlowModificationHeader");

namespace ns3
{
using namespace action_utils;

NS_OBJECT_ENSURE_REGISTERED(FlowModificationHeader);

FlowModificationHeader::FlowModificationHeader (void) :
  m_FMatchHeader(nullptr),
  m_cookie(0),
  m_command(0),
  m_idleTimeout(0),
  m_hardTimeout(0),
  m_priority(0),
  m_bufferId(0),
  m_outPort(0),
  m_flags(0)
{
  NS_LOG_FUNCTION(this);
}

FlowModificationHeader::FlowModificationHeader (ofp_flow_mod* header) :
  m_FMatchHeader(nullptr),
  m_cookie(0),
  m_command(header->command),
  m_idleTimeout(header->idle_timeout),
  m_hardTimeout(header->hard_timeout),
  m_priority(header->priority),
  m_bufferId(header->buffer_id),
  m_outPort(header->out_port),
  m_flags(0)
{
  NS_LOG_FUNCTION(this);
}

FlowModificationHeader::FlowModificationHeader (uint64_t cookie, uint16_t command, uint16_t idleTimeout,
                                                uint16_t hardTimeout, uint16_t priority, uint32_t bufferId,
                                                uint16_t outPort, uint16_t flags) :
  m_FMatchHeader(nullptr),
  m_cookie(cookie),
  m_command(command),
  m_idleTimeout(idleTimeout),
  m_hardTimeout(hardTimeout),
  m_priority(priority),
  m_bufferId(bufferId),
  m_outPort(outPort),
  m_flags(flags)
{
  NS_LOG_FUNCTION(this);
}

FlowModificationHeader::~FlowModificationHeader ()
{
  NS_LOG_FUNCTION(this);

  m_actionsList.clear();
}

uint16_t FlowModificationHeader::GetCommand() const
{
  return m_command;
}

void FlowModificationHeader::SetCommand(uint16_t command)
{
  m_command = command;
}

uint16_t FlowModificationHeader::GetIdleTimeout() const
{
  return m_idleTimeout;
}

void FlowModificationHeader::SetIdleTimeout(uint16_t idleTimeout)
{
  m_idleTimeout = idleTimeout;
}

uint16_t FlowModificationHeader::GetHardTimeout() const
{
  return m_hardTimeout;
}

void FlowModificationHeader::SetHardTimeout(uint16_t hardTimeout)
{
  m_hardTimeout = hardTimeout;
}

uint16_t FlowModificationHeader::GetPriority() const
{
  return m_priority;
}

void FlowModificationHeader::SetPriority(uint16_t priority)
{
  m_priority = priority;
}

uint32_t FlowModificationHeader::GetBufferId() const
{
  return m_bufferId;
}

void FlowModificationHeader::SetBufferId(uint32_t bufferId)
{
  m_bufferId = bufferId;
}

uint16_t FlowModificationHeader::GetOutPort() const
{
  return m_outPort;
}

void FlowModificationHeader::SetOutPort(uint16_t outPort)
{
  m_outPort = outPort;
}

Ptr<FlowMatchHeader> FlowModificationHeader::GetFlowMatch() const
{
  return m_FMatchHeader;
}

void FlowModificationHeader::SetFlowMatch(Ptr<FlowMatchHeader> header)
{
  m_FMatchHeader = header;
}

void FlowModificationHeader::AddAction(Ptr<ActionHeader> action)
{
  NS_LOG_FUNCTION(this << *action);
  m_actionsList.push_back(action);
}

void FlowModificationHeader::AddActionsList(ActionsList actionsList)
{
  NS_LOG_FUNCTION(this);
  m_actionsList.merge(actionsList);
}

const ActionsList& FlowModificationHeader::GetActionsList(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_actionsList;
}

Ptr<ActionHeader> FlowModificationHeader::GetActionByType(uint16_t type) const
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

void FlowModificationHeader::PrintActionsList(std::ostream &os) const
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

TypeId FlowModificationHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::FlowModificationHeader")
                      .SetParent<Header> ()
                      .AddConstructor<FlowModificationHeader> ()
  ;

  return tid;
}

TypeId FlowModificationHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void FlowModificationHeader::Print(std::ostream &os) const
{
  os << "Flow Modification Header" << std::endl
     << " Cookie: " << uint(m_cookie) << std::endl
     << " Command: " << uint(m_command) << std::endl
     << " Idle Timeout: " << uint(m_idleTimeout) << std::endl
     << " Hard Timeout: " << uint(m_hardTimeout) << std::endl
     << " Priority: " << uint(m_priority) << std::endl
     << " Buffer ID: " << uint(m_bufferId) << std::endl
     << " Out Port: " << uint(m_outPort) << std::endl
     << " Flags: " << uint(m_flags) << std::endl
     << std::endl;
}

uint32_t FlowModificationHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  uint32_t msgSize = 64;   // (FlowMatch) 40 + 8 + 2 + 2 + 2 + 2 + 4 + 2 + 2 = 64

  for (auto &act: m_actionsList)
    {
      msgSize += act->GetSerializedSize();
    }

  return msgSize;
}

void FlowModificationHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader->Serialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  start.WriteHtonU64(m_cookie);
  start.WriteHtonU16(m_command);
  start.WriteHtonU16(m_idleTimeout);
  start.WriteHtonU16(m_hardTimeout);
  start.WriteHtonU16(m_priority);
  start.WriteHtonU32(m_bufferId);
  start.WriteHtonU16(m_outPort);
  start.WriteHtonU16(m_flags);

  for (auto &act: m_actionsList)
    {
      act->Serialize(start);
      start.Next(act->GetSerializedSize());
    }
}

uint32_t FlowModificationHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_FMatchHeader = Create<FlowMatchHeader>();
  m_FMatchHeader->Deserialize(start);
  start.Next(m_FMatchHeader->GetSerializedSize());
  m_cookie = start.ReadNtohU64();
  m_command = start.ReadNtohU16();
  m_idleTimeout = start.ReadNtohU16();
  m_hardTimeout = start.ReadNtohU16();
  m_priority = start.ReadNtohU16();
  m_bufferId = start.ReadNtohU32();
  m_outPort = start.ReadNtohU16();
  m_flags = start.ReadNtohU16();

  /* Read actions */
  while (start.IsEnd() == false)
    {
      Ptr<ActionHeader> action = Create<ActionHeader>();
      start.Next(action->Deserialize(start));
      m_actionsList.push_back(action);
    }

  return GetSerializedSize();
}
} // namespace ns3
