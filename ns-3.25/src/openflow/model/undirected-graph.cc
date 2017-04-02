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
#include <climits>
#include <ns3/adjacency.h>
#include <ns3/log.h>
#include <ns3/simple-ref-count.h>
#include <ns3/controller-ofswitch.h>
#include <ns3/undirected-graph.h>
#include <ns3/vertex-host.h>
#include <ns3/vertex-server.h>
#include <ns3/vertex-switch.h>

NS_LOG_COMPONENT_DEFINE("UndirectedGraph");

namespace ns3 {
UndirectedGraph::UndirectedGraph()
{
  NS_LOG_FUNCTION_NOARGS();
}

Ptr<HostVertex>
UndirectedGraph::AddHost(const Mac48Address& mac)
{
  NS_LOG_FUNCTION(this << mac);
  Ptr<HostVertex> v = Create<HostVertex>(mac);

  const auto& pair = m_graph.emplace(mac, v);

  return StaticCast<HostVertex, Vertex>(pair.first->second);
}

Ptr<ServerVertex>
UndirectedGraph::AddServer(const Mac48Address& mac)
{
  NS_LOG_FUNCTION(this << mac);
  Ptr<ServerVertex> v = Create<ServerVertex>(mac);

  const auto& pair = m_graph.emplace(mac, v);

  return StaticCast<ServerVertex, Vertex>(pair.first->second);
}

Ptr<SwitchVertex>
UndirectedGraph::AddSwitch(Ptr<OfSwitch>dev)
{
  NS_LOG_FUNCTION(this << dev);
  Ptr<SwitchVertex> v = Create<SwitchVertex>(dev);

  const auto& pair = m_graph.emplace(v->GetChassisId(), v);

  return StaticCast<SwitchVertex, Vertex>(pair.first->second);
}

Ptr<Adjacency>
UndirectedGraph::AddEdge(Ptr<Vertex>from,
                         Ptr<Vertex>to)
{
  NS_LOG_FUNCTION(this << *from << *to);

  if (auto adj = from->GetEdge(to))
    return adj; /* already exists */

  Ptr<Adjacency> adj = Create<Adjacency>(from, to);

  m_graph[from->GetHwAddress()]->AddEdge(adj);

  return adj;
}

Ptr<Adjacency>
UndirectedGraph::AddEdge(Ptr<Adjacency>adj)
{
  NS_LOG_FUNCTION(this << *adj);

  return AddEdge(adj->GetOrigin(), adj->GetDestination());
}

Ptr<Adjacency>
UndirectedGraph::AddEdge(const Mac48Address& from,
                         const Mac48Address& to)
{
  NS_LOG_FUNCTION(this << from << to);
  Ptr<Vertex> vFrom = m_graph.at(from);
  Ptr<Vertex> vTo   = m_graph.at(to);

  return AddEdge(vFrom, vTo);
}

void
UndirectedGraph::RemoveVertex(const Mac48Address& mac)
{
  NS_LOG_FUNCTION(this << mac);
  m_graph.erase(mac);

  // FIXME remove edges pointing to this vertex
}

Ptr<Vertex>
UndirectedGraph::LookupVertex(const Mac48Address& mac) const
{
  auto it = m_graph.find(mac);

  if (it != m_graph.end())
    return it->second;

  return nullptr;
}

Ptr<Vertex>
UndirectedGraph::LookupVertex(const Address& addr) const
{
  for (auto& pair: m_graph) {
    if (pair.second->HasAddress(addr))
      return pair.second;
  }
  return nullptr;
}

UndirectedGraph::GraphPath
UndirectedGraph::FindShortestPath(const Mac48Address& from,
                                  const Mac48Address& to) const
{
  NS_LOG_FUNCTION(from << to);

  Ptr<Vertex> vFrom = m_graph.at(from);
  Ptr<Vertex> vTo   = m_graph.at(to);

  if ((vFrom != nullptr) && (vTo != nullptr))
    return FindShortestPath(vFrom, vTo);
  else
    return GraphPath();
}

UndirectedGraph::GraphPath
UndirectedGraph::FindShortestPath(Ptr<Vertex>from,
                                  Ptr<Vertex>to) const
{
  NS_ASSERT((from != nullptr) && (to != nullptr));

  NS_LOG_FUNCTION(*from << *to);
  GraphPath path;

  PreviousNodeMap previousNodes; ///< Map of node to its previous
  CalculatePathsFromSource(previousNodes, from);

  Ptr<Vertex> currentNode = to;

  while (currentNode != nullptr)
  {
    Ptr<Vertex> prev = previousNodes[currentNode];

    if (prev) {
      Ptr<Adjacency> adj = prev->GetEdge(currentNode);
      path.push_front(adj);
    }

    currentNode = prev;
  }

  if (path.empty() || (from != path.front()->GetOrigin())) {
    NS_LOG_WARN("Unable to find a path!");
    path.clear();
  }

  return path;
}

Ptr<Vertex>
UndirectedGraph::FindMinimumInSet(SourceDistanceMap& sourceDistances,
                                  VertexSet          nodesSet) const
{
  std::pair<Ptr<Vertex>, int> minNodeDistance(nullptr, INT_MAX);

  for (auto& v : nodesSet)  {
    int dist = sourceDistances[v];

    if ((unsigned int)(dist) < (unsigned int)minNodeDistance.second)
    {
      minNodeDistance.first  = v;
      minNodeDistance.second = dist;
    }
  }
  return minNodeDistance.first;
}

void
UndirectedGraph::CalculatePathsFromSource(PreviousNodeMap& previousNodes,
                                          Ptr<Vertex>      source) const
{
  SourceDistanceMap sourceDistances;
  VertexSet unvisitedNodesSet;

  for (auto& pair: m_graph) {
    Ptr<Vertex> vertex = pair.second;

    sourceDistances[vertex] = (vertex != source) ? INT_MAX : 0;
    previousNodes[vertex]   = nullptr;

    unvisitedNodesSet.insert(vertex);
  }

  while (!unvisitedNodesSet.empty())
  {
    Ptr<Vertex> closestVertex = FindMinimumInSet(sourceDistances, unvisitedNodesSet);

    if (closestVertex == nullptr) {
      NS_LOG_LOGIC("Graph has unconnected vertexes, stopping path calculation");
      break;
    }

    unvisitedNodesSet.erase(closestVertex);

    for (auto& adj: closestVertex->GetEdges()) {
      Ptr<Vertex> peer = adj->GetDestination();

      // where peer is still in unvisited nodes set.
      if (unvisitedNodesSet.count(peer) > 0)
      {
        int alternativeDistance = sourceDistances[closestVertex] + adj->GetWeight();

        // new path found?
        if ((unsigned int)alternativeDistance < (unsigned int)sourceDistances[peer])
        {
          sourceDistances[peer] = alternativeDistance;
          previousNodes[peer]   = closestVertex;
        }
      }
    }
  }
}
} // namespace ns3
#endif // NS3_OPENFLOW
