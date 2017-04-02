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

#ifndef OF_ROUTING_H
#define OF_ROUTING_H

#include <map>
#include <ns3/controller.h>
#include <ns3/controller-application.h>
#include <ns3/undirected-graph.h>

namespace ns3 {
class FlowMatchHeader;

class OFRouting : public ControllerApplication {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  OFRouting();

  /**
   * \brief Destructor.
   */
  ~OFRouting();

  virtual bool ReceiveFromSwitch(Ptr<OfSwitch>origin,
                                 unsigned     portIn,
                                 unsigned     bufferId,
                                 Ptr<Packet>  packet);

  virtual void SyncCompleted(int statedId);

  void SetupPath(Ptr<FlowMatchHeader>        matchHeader,
                 UndirectedGraph::GraphPath& path);

  void TearDownPath(Ptr<FlowMatchHeader>        matchHeader,
                    UndirectedGraph::GraphPath& path);

  void RelayPacket(Ptr<OfSwitch>origin,
                   unsigned     bufferId,
                   Ptr<Packet>  packet);

protected:

  virtual int GetPriority() const;

private:

  typedef struct {
    unsigned     bufferId;
    Ptr<OfSwitch>origin;
    Ptr<Packet>  packet;
  } PendingPacketType;


  Ipv4Address m_nspGw;
  Ipv4Address m_accessNetPrefix;
  Ipv4Mask    m_accessNetMask;
  uint16_t    m_flowLifetime;
  uint16_t    m_flowPrio;

  std::map<int, PendingPacketType> m_pendingPacket;
};
} // namespace ns3
#endif  /* OF_ROUTING_H */
