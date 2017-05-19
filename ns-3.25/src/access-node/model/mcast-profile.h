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
#ifndef __MCAST_PROFILE_H__
#define __MCAST_PROFILE_H__

#include <string>
#include <set>
#include "ns3/simple-ref-count.h"
#include "ns3/address.h"
#include "ns3/mac48-address.h"
#include "ns3/ancp-header.h"

namespace ns3 {
using ListClass = AncpTlvMCastListAction::ListClass;

class MCastProfile : public SimpleRefCount<MCastProfile>{
public:
  MCastProfile(const std::string& name);
  const std::string& GetName() const;

  void UpdateFlowList(ListClass listClass,
                      bool insert,
                      const Address &group);

  void FlushFlowList(ListClass listClass);

  ListClass GetFlowPolicy(const Address &group) const;

private:
  typedef std::set<Address> FlowList;

  void DoUpdateFlowList(FlowList& flow,
                        bool insert,
                        const Address &group);

  void DoFlushFlowList(FlowList& flow);

  std::string m_name;
  FlowList whitelist;
  FlowList greylist;
  FlowList blacklist;
};

bool operator==(const MCastProfile& profA, const MCastProfile& profB);

bool operator<(const MCastProfile& profA, const MCastProfile& profB);

std::ostream& operator<<(std::ostream      & os,
                         const MCastProfile& prof);
}
#endif /* __MCAST_PROFILE_H__ */
