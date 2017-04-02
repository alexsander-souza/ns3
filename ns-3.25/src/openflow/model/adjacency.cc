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
#include "adjacency.h"

NS_LOG_COMPONENT_DEFINE("Adjacency");

namespace ns3 {
Adjacency::Adjacency(Ptr<Vertex>origin,
                     Ptr<Vertex>destination,
                     uint32_t   weight) :
  m_origin(origin),
  m_destination(destination),
  m_portNumber(-1),
  m_weight(weight)
{
  NS_LOG_FUNCTION(origin << destination);
}

Ptr<Vertex>Adjacency::GetOrigin() const
{
  return m_origin;
}

Ptr<Vertex>Adjacency::GetDestination() const
{
  return m_destination;
}

uint16_t
Adjacency::GetPortNumber() const
{
  return m_portNumber;
}

void
Adjacency::SetPortNumber(uint16_t portNumber)
{
  m_portNumber = portNumber;
}

uint32_t
Adjacency::GetWeight() const
{
  return m_weight;
}

void
Adjacency::SetWeight(uint32_t weight)
{
  m_weight = weight;
}

Ptr<Adjacency>
Adjacency::Invert() const
{
  Ptr<Adjacency> adj = Create<Adjacency>(m_destination, m_origin, m_weight);
  return adj;
}

bool
operator<(const Adjacency& adj1, const Adjacency& adj2)
{
  return *adj1.m_origin < *adj2.m_origin;
}

bool
operator==(const Adjacency& adj1, const Adjacency& adj2)
{
  return adj1.m_origin == adj2.m_origin
         && adj1.m_destination == adj2.m_destination;
}

std::ostream&
operator<<(std::ostream   & strm,
           Adjacency const& adj)
{
  strm << *adj.m_origin << "-->" << *adj.m_destination << "[" << adj.m_portNumber << "]";
  return strm;
}
} // namespace ns3
#endif // NS3_OPENFLOW
