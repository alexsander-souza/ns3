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

#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mcast-profile.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("MCastProfile");

/******************************************************************************/

MCastProfile::MCastProfile(const std::string& name) :
  m_name(name)
{
  NS_LOG_FUNCTION(name);
}

const std::string& MCastProfile::GetName() const
{
  return m_name;
}

ListClass MCastProfile::GetFlowPolicy(const Address &group) const
{
  NS_LOG_FUNCTION(this << group);

  if (blacklist.find(group) != blacklist.end())
    return ListClass::BLACKLIST; /* drop */

  if (greylist.find(group) != greylist.end())
    return ListClass::GREYLIST; /* ask NAS */

  if (whitelist.find(group) != whitelist.end())
    return ListClass::WHITELIST; /* allow */

  return ListClass::BLACKLIST; /* ignore */
}

void MCastProfile::UpdateFlowList(ListClass listClass,
                                  bool insert,
                                  const Address &group)
{
  NS_LOG_FUNCTION(this << listClass << insert << group);
  switch (listClass)
    {
    case ListClass::WHITELIST:
      return DoUpdateFlowList(whitelist, insert, group);

    case ListClass::GREYLIST:
      return DoUpdateFlowList(greylist, insert, group);

    case ListClass::BLACKLIST:
      return DoUpdateFlowList(blacklist, insert, group);
    }
}

void MCastProfile::FlushFlowList(ListClass listClass)
{
  NS_LOG_FUNCTION(this << listClass);
  switch (listClass)
    {
    case ListClass::WHITELIST:
      return DoFlushFlowList(whitelist);

    case ListClass::GREYLIST:
      return DoFlushFlowList(greylist);

    case ListClass::BLACKLIST:
      return DoFlushFlowList(blacklist);
    }
}

void MCastProfile::DoUpdateFlowList(FlowList& flow,
                                    bool insert,
                                    const Address &group)
{
  if (insert)
    {
      flow.insert(group);
    }
  else
    {
      flow.erase(group);
    }
}

void MCastProfile::DoFlushFlowList(FlowList& flow)
{
  flow.clear();
}

std::ostream& operator<<(std::ostream& os, const MCastProfile& prof)
{
  os << "[AN-MC-Prof:" << prof.GetName() << "]";
  return os;
}

bool operator==(const MCastProfile& profA, const MCastProfile& profB)
{
  return profA.GetName() == profB.GetName();
}

bool operator<(const MCastProfile& profA, const MCastProfile& profB)
{
  return profA.GetName() < profB.GetName();
}
} // ns3
