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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#ifdef NS3_OPENFLOW

#include <ns3/log.h>
#include "vertex.h"

NS_LOG_COMPONENT_DEFINE("Vertex");

namespace ns3 {
Vertex::Vertex(const Mac48Address& mac, bool leaf) :
  m_mac(mac),
  m_isleaf(leaf)
{
  NS_LOG_FUNCTION(m_mac);
}

const Mac48Address&
Vertex::GetHwAddress() const
{
  return m_mac;
}

bool
Vertex::IsLeaf() const
{
  return m_isleaf;
}

void
Vertex::AddAddress(const Address& addr)
{
  m_addresses.insert(addr);
}

void
Vertex::RemoveAddress(const Address& addr)
{
  m_addresses.erase(addr);
}

bool
Vertex::HasAddress(const Address& addr) const
{
  return m_addresses.find(addr) != m_addresses.end();
}

void
Vertex::AddEdge(Ptr<Adjacency>adj)
{
  m_adjacencies.insert(adj);
}

const std::set<Ptr<Adjacency> >
Vertex::GetEdges() const
{
  return m_adjacencies;
}

Ptr<Adjacency>
Vertex::GetEdge(Ptr<Vertex>destination)
{
  for (auto& adj: m_adjacencies) {
    if (adj->GetDestination() == destination)
      return adj;
  }
  return nullptr;
}

bool
operator==(const Vertex& v1, const Vertex& v2)
{
  return v1.m_mac == v2.m_mac;
}

bool
operator==(const Vertex      & v1,
           const Mac48Address& mac)
{
  return v1.m_mac == mac;
}

bool
operator<(const Vertex& v1, const Vertex& v2)
{
  return v1.m_mac < v2.m_mac;
}

std::ostream&
operator<<(std::ostream& strm,
           Vertex const& vertex)
{
  return strm << vertex.m_mac;
}
} // namespace ns3
#endif // NS3_OPENFLOW
