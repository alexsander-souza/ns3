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
#include "ns3/switch-features-header.h"
#include "ns3/physical-port-header.h"

NS_LOG_COMPONENT_DEFINE("SwitchFeaturesHeader");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(SwitchFeaturesHeader);

SwitchFeaturesHeader::SwitchFeaturesHeader (void) :
  m_datapathId(0),
  m_nBuffers(0),
  m_nTables(0),
  m_capabilities(0),
  m_actions(0)
{
  NS_LOG_FUNCTION(this);
}

SwitchFeaturesHeader::SwitchFeaturesHeader (uint64_t datapathId, uint32_t nBuffers, uint8_t nTables,
                                            uint32_t capabilities, uint32_t actions) :
  m_datapathId(datapathId),
  m_nBuffers(nBuffers),
  m_nTables(nTables),
  m_capabilities(capabilities),
  m_actions(actions)
{
  NS_LOG_FUNCTION(this);
}

SwitchFeaturesHeader::~SwitchFeaturesHeader ()
{
  NS_LOG_FUNCTION(this);
  m_portsList.clear();
}

uint64_t SwitchFeaturesHeader::GetDatapathId() const
{
  return m_datapathId;
}

void SwitchFeaturesHeader::SetDatapathId(uint64_t datapathId)
{
  m_datapathId = datapathId;
}

Mac48Address SwitchFeaturesHeader::GetDatapathMac() const
{
  uint8_t ea[ETH_ADDR_LEN];

  eth_addr_from_uint64(m_datapathId, ea);

  Mac48Address dpId;
  dpId.CopyFrom (ea);

  return dpId;
}

uint32_t SwitchFeaturesHeader::GetNBuffers() const
{
  return m_nBuffers;
}

void SwitchFeaturesHeader::SetNBuffers(uint32_t nBuffers)
{
  m_nBuffers = nBuffers;
}

uint8_t SwitchFeaturesHeader::GetNTables() const
{
  return m_nTables;
}

void SwitchFeaturesHeader::SetNTables(uint8_t nTables)
{
  m_nTables = nTables;
}

uint32_t SwitchFeaturesHeader::GetCapabilities() const
{
  return m_capabilities;
}

void SwitchFeaturesHeader::SetCapabilities(uint32_t capabilities)
{
  m_capabilities = capabilities;
}

uint32_t SwitchFeaturesHeader::GetActions() const
{
  return m_actions;
}

void SwitchFeaturesHeader::SetActions(uint32_t actions)
{
  m_actions = actions;
}

void SwitchFeaturesHeader::AddPort(Ptr<PhysicalPortHeader> port)
{
  NS_LOG_FUNCTION(this << *port);
  m_portsList.push_back(port);
}

void SwitchFeaturesHeader::AddPortsList(PhysicalPortsList &ports_list)
{
  NS_LOG_FUNCTION(this);
  m_portsList.merge(ports_list);
}

const SwitchFeaturesHeader::PhysicalPortsList& SwitchFeaturesHeader::GetPortsList(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_portsList;
}

Ptr<PhysicalPortHeader> SwitchFeaturesHeader::GetPortByNumber(uint16_t number) const
{
  NS_LOG_FUNCTION(this << number);

  for (auto &port: m_portsList)
    {
      if (port->GetPortNumber() == number)
        return(port);
    }
  NS_LOG_WARN("Port not found");
  return nullptr;
}

TypeId SwitchFeaturesHeader::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::SwitchFeaturesHeader")
                      .SetParent<Header> ()
                      .AddConstructor<SwitchFeaturesHeader> ()
  ;

  return tid;
}

TypeId SwitchFeaturesHeader::GetInstanceTypeId(void) const
{
  NS_LOG_FUNCTION(this);
  return GetTypeId();
}

void SwitchFeaturesHeader::Print(std::ostream &os) const
{

  os << "Switch Features Header" << std::endl
     << " Datapath ID: " << GetDatapathMac() << std::endl
     << " NBuffers: " << uint(m_nBuffers) << std::endl
     << " NTables: " << uint(m_nTables) << std::endl
     << " Capabilities: " << uint(m_capabilities) << std::endl
     << " Actions: " << uint(m_actions) << std::endl
     << std::endl;
}

uint32_t SwitchFeaturesHeader::GetSerializedSize(void) const
{
  NS_LOG_FUNCTION(this);

  uint32_t msgSize = 24 - 3;   // 8 + 4 + 1 + 3 (pad) + 4 + 4 = 24

  for (auto &port: m_portsList)
    {
      msgSize += port->GetSerializedSize();
    }

  return msgSize;
}

void SwitchFeaturesHeader::Serialize(Buffer::Iterator start) const
{
  NS_LOG_FUNCTION(this << &start);

  start.WriteHtonU64(m_datapathId);
  start.WriteHtonU32(m_nBuffers);
  start.WriteU8(m_nTables);

  start.WriteHtonU32(m_capabilities);
  start.WriteHtonU32(m_actions);

  for (auto &port: m_portsList)
    {
      port->Serialize(start);
      start.Next(port->GetSerializedSize());
    }
}

uint32_t SwitchFeaturesHeader::Deserialize(Buffer::Iterator start)
{
  NS_LOG_FUNCTION(this << &start);

  m_datapathId = start.ReadNtohU64();
  m_nBuffers = start.ReadNtohU32();
  m_nTables = start.ReadU8();

  m_capabilities = start.ReadNtohU32();
  m_actions = start.ReadNtohU32();

  /* Read options */
  while (start.IsEnd() == false)
    {
      Ptr<PhysicalPortHeader> port = Create<PhysicalPortHeader>();
      start.Next(port->Deserialize(start));
      m_portsList.push_back(port);
    }

  return GetSerializedSize();
}
} // namespace ns3
