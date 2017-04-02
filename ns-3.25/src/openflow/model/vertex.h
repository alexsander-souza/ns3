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

#ifndef VERTEX_H
#define VERTEX_H

#include <set>
#include <ns3/address.h>
#include <ns3/mac48-address.h>
#include <ns3/simple-ref-count.h>
#include <ns3/adjacency.h>

namespace ns3 {
class Vertex : public SimpleRefCount<Vertex>{
public:

  Vertex(const Mac48Address& mac,
         bool                leaf);

  virtual ~Vertex() {}

  const Mac48Address& GetHwAddress() const;

  bool IsLeaf() const;

  void AddAddress(const Address& addr);
  void RemoveAddress(const Address& addr);
  bool HasAddress(const Address& addr) const;

  void AddEdge(Ptr<Adjacency>adj);

  Ptr<Adjacency>GetEdge(Ptr<Vertex>destination);

  const std::set<Ptr<Adjacency> >GetEdges() const;

  friend bool operator<(const Vertex& v1,
                        const Vertex& v2);
  friend bool operator==(const Vertex& v1,
                         const Vertex& v2);

  friend bool operator==(const Vertex      & v1,
                         const Mac48Address& mac);

  friend std::ostream& operator<<(std::ostream& strm,
                                  Vertex const& vertex);

private:

  Mac48Address m_mac;
  bool m_isleaf;
  std::set<Address> m_addresses;
  std::set<Ptr<Adjacency> > m_adjacencies;
};
} //  namespace ns3
#endif  /* VERTEX_H */
