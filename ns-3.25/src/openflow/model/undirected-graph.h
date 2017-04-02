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

#ifndef UNDIRECTED_GRAPH_H
#define UNDIRECTED_GRAPH_H

#include <map>
#include <set>
#include <list>
#include <ns3/mac48-address.h>

namespace ns3 {
class Adjacency;
class Vertex;
class OfSwitch;
class HostVertex;
class ServerVertex;
class SwitchVertex;

class UndirectedGraph {
public:

  typedef std::list<Ptr<Adjacency> >GraphPath;

  /**
   * \brief Constructor.
   */
  UndirectedGraph();

  /**
   * \brief Add a server vertex to the map.
   * \param mac      Vertex MAC to be added
   */
  Ptr<ServerVertex>AddServer(const Mac48Address& mac);

  /**
   * \brief Add a host vertex to the map.
   * \param mac      Vertex MAC to be added
   */
  Ptr<HostVertex>AddHost(const Mac48Address& mac);

  /**
   * \brief Add a Switch vertex to the map.
   * \param dev      OF-Switch device
   */
  Ptr<SwitchVertex>AddSwitch(Ptr<OfSwitch>dev);


  /**
   * \brief Removes a vertex from the map.
   * \param mac      Vertex MAC to be removed
   */
  void RemoveVertex(const Mac48Address& mac);

  /**
   * \brief Adds adjacency to vertex adjacency list.
   * \param from        Origin Vertex
   * \param to          Destination Vertex
   */
  Ptr<Adjacency>AddEdge(const Mac48Address& from,
                        const Mac48Address& to);

  Ptr<Adjacency>AddEdge(Ptr<Vertex>from,
                        Ptr<Vertex>to);

  Ptr<Adjacency>AddEdge(Ptr<Adjacency>adj);

  GraphPath FindShortestPath(const Mac48Address& from,
                             const Mac48Address& to) const;

  GraphPath FindShortestPath(Ptr<Vertex>from,
                             Ptr<Vertex>to) const;

  Ptr<Vertex>LookupVertex(const Mac48Address& mac) const;

  Ptr<Vertex>LookupVertex(const Address& addr) const;

private:

  typedef std::map<Mac48Address, Ptr<Vertex> >Graph;

  // from current to previous ID in path
  typedef std::map<Ptr<Vertex>, Ptr<Vertex> >PreviousNodeMap;

  // from node to its distance from source
  typedef std::map<Ptr<Vertex>, int>SourceDistanceMap;

  typedef std::set<Ptr<Vertex> >VertexSet;

  void CalculatePathsFromSource(PreviousNodeMap& previousNodes,
                                Ptr<Vertex>      source) const;

  Ptr<Vertex>FindMinimumInSet(SourceDistanceMap& sourceDistances,
                              VertexSet          nodesSet) const;

  Graph m_graph;
};
} //  namespace ns3
#endif  /* UNDIRECTED_GRAPH_H */
