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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#include "ns3/log.h"
#include "access-channel.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AccessChannel");

NS_OBJECT_ENSURE_REGISTERED(AccessChannel);

TypeId
AccessChannel::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::AccessChannel")
                      .SetParent<Channel> ()
                      .AddConstructor<AccessChannel> ()
  ;

  return tid;
}

AccessChannel::AccessChannel ()
  : Channel()
{
  NS_LOG_FUNCTION_NOARGS();
}

AccessChannel::~AccessChannel ()
{
  NS_LOG_FUNCTION_NOARGS();

  for (ChannelList::iterator iter = m_channels.begin();
       iter != m_channels.end(); iter++)
    {
      *iter = 0;
    }
  m_channels.clear();
}


void
AccessChannel::AddChannel(Ptr<Channel> accessChannel)
{
  m_channels.push_back(accessChannel);
}

uint32_t
AccessChannel::GetNDevices(void) const
{
  uint32_t ndevices = 0;

  for (ChannelList::const_iterator iter = m_channels.begin();
       iter != m_channels.end(); iter++)
    {
      ndevices += (*iter)->GetNDevices();
    }
  return ndevices;
}


Ptr<NetDevice>
AccessChannel::GetDevice(uint32_t i) const
{
  uint32_t ndevices = 0;

  for (ChannelList::const_iterator iter = m_channels.begin();
       iter != m_channels.end(); iter++)
    {
      if ((i - ndevices) < (*iter)->GetNDevices())
        {
          return (*iter)->GetDevice(i - ndevices);
        }
      ndevices += (*iter)->GetNDevices();
    }
  return 0;
}
} // namespace ns3
