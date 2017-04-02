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
 *
 */

#include "vertex-host.h"

namespace ns3 {
HostVertex::HostVertex(const Mac48Address& mac) :
  Vertex(mac, true),
  m_accessProfName(""),
  m_mcastProfName("")
{}

Ptr<Adjacency>
HostVertex::GetDownstreamAdjacency()
{
  Ptr<Vertex> self(this);
  Ptr<Vertex> ofSw = GetUpstreamAdjacency()->GetDestination();

  return ofSw->GetEdge(self);
}

Ptr<Adjacency>
HostVertex::GetUpstreamAdjacency()
{
  return *(GetEdges().begin());
}

void HostVertex::SetAccessProfileName(const string& profName)
{
  m_accessProfName = profName;
}

const string& HostVertex::GetAccessProfileName() const
{
  return m_accessProfName;
}

void HostVertex::SetMCastProfileName(const string& profName)
{
  m_mcastProfName = profName;
}

const string & HostVertex::GetMCastProfileName() const
{
  return m_mcastProfName;
}

void HostVertex::UpdateLineStatus(bool active, uint32_t upRate, uint32_t downRate)
{
  ///TODO implementar sessão
}

} // namespace ns3
