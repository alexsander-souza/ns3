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

#ifndef ADJACENCY_H
#define ADJACENCY_H

#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>

namespace ns3 {
class Vertex;

class Adjacency : public SimpleRefCount<Adjacency>{
public:

  /**
   * \brief Constructor
   */
  Adjacency(Ptr<Vertex>origin,
            Ptr<Vertex>destination,
            uint32_t   weight = 1);

  /* Getters and Setters */
  Ptr<Vertex>GetOrigin() const;
  Ptr<Vertex>GetDestination() const;

  void     SetPortNumber(uint16_t portNumber);
  uint16_t GetPortNumber() const;

  uint32_t GetWeight() const;
  void     SetWeight(uint32_t weight);

  Ptr<Adjacency>Invert() const;

  friend bool operator<(const Adjacency& adj1,
                        const Adjacency& adj2);
  friend bool operator==(const Adjacency& adj1,
                         const Adjacency& adj2);

  friend std::ostream& operator<<(std::ostream   & strm,
                                  Adjacency const& adj);

private:

  Ptr<Vertex> m_origin;
  Ptr<Vertex> m_destination;
  uint16_t    m_portNumber;
  uint32_t    m_weight;
};
} //  namespace ns3
#endif  /* ADJACENCY_H */
