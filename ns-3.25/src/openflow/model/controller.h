/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2014 UFRGS
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
 * Author: Blake Hurd  <naimorai@gmail.com>
 * Contributor: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <map>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/application.h>
#include <ns3/controller-sbi.h>
#include <ns3/undirected-graph.h>

namespace ns3 {
class Socket;
class ControllerApplication;
class OfSwitch;
class HostVertex;
class SwitchVertex;
class NetDevice;

/**
 * \brief An interface for a Controller of OpenFlowSwitchNetDevices
 *
 * Follows the OpenFlow specification for a controller.
 */
class Controller : public Application,
                   public ControllerSbi {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  Controller();

  /**
   * \brief Destructor.
   */
  virtual
  ~Controller();

  /**
   * \brief Adds an new application to add to the list of applications of this
   * controller
   * \param newApp  New application to add to the list of applications of this
   * controller
   */
  void AddApplication(Ptr<ControllerApplication>newApp);

  Ptr<NetDevice>GetInbandNetDevice();

  Ptr<ServerVertex>GetInbandVertex();

  void SetAccessProfile(const std::string &profName);

  /*
   * \brief Search for an application on controller's list of applications.
   * \param appTypeId   Type Id of the controller application to search for
   * \return Pointer to application if found, or NULL if not found
   */
  Ptr<ControllerApplication>GetAppByTypeId(TypeId appTypeId);

  virtual void Print(std::ostream& os) const;

  /*
   * \brief Forwards the openflow message of type Packet In received to all
   *        applications on the list of controller applications of this
   *        controller.
   */
  virtual void ForwardToApps(Ptr<OfSwitch>origin,
                             unsigned     portIn,
                             unsigned     bufferId,
                             Ptr<Packet>  packet);

  virtual void NewSwitch(Ptr<OfSwitch>origin);

  int SwitchSync();

  virtual void SwitchSyncCompleted(Ptr<OfSwitch>origin,
                                   int          stateId);

  Ptr<HostVertex>AddHost(const Mac48Address& mac);

  void RemoveHost(const Mac48Address& mac);

  void AddHostMapping(const Mac48Address& mac,
                      const Address     & addr);

  void AddHostMapping(Ptr<HostVertex>host,
                      const Address& addr);

  void RemoveHostMapping(const Mac48Address& mac,
                         const Address     & addr);

  void RemoveHostMapping(const Address& addr);

  /**
   * \brief Adds adjacency to vertex adjacency list.
   */
  Ptr<Adjacency>AddEdge(Ptr<Adjacency>adj);

  /**
   * \brief Adds adjacency to vertex adjacency list.
   * \param from        Origin Vertex
   * \param to          Destination Vertex
   * \param portNum     Switch port ID
   */
  Ptr<Adjacency>AddEdge(Ptr<Vertex>from,
                        Ptr<Vertex>to,
                        uint16_t   portNum);

  /**
   * \brief Adds adjacency to vertex adjacency list.
   * \param from        Origin Vertex
   * \param to          Destination Vertex
   */
  Ptr<Adjacency>AddEdge(Ptr<Vertex>from,
                        Ptr<Vertex>to);

  /**
   * \brief Adds adjacency to vertex adjacency list.
   * \param from        Origin Vertex
   * \param to          Destination Vertex
   */
  Ptr<Adjacency>AddEdge(const Mac48Address& from,
                        const Mac48Address& to);

  Ptr<SwitchVertex>LookupSwitch(const Mac48Address& mac) const;

  Ptr<HostVertex>LookupHost(const Mac48Address& mac) const;

  Ptr<HostVertex>LookupHost(const Address& addr) const;

  UndirectedGraph::GraphPath FindShortestPath(const Mac48Address& from,
                                              const Mac48Address& to) const;

  UndirectedGraph::GraphPath FindShortestPath(Ptr<Vertex>from,
                                              Ptr<Vertex>to) const;

  Ipv4Address GetLocalAddress(Ptr<NetDevice>device) const;

protected:

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

private:

  typedef std::list<Ptr<ControllerApplication> >AppsList;
  typedef std::multimap<int, Ptr<OfSwitch> >    SyncMap;

  Ipv4Address m_myAddress;
  AppsList    m_applications;
  UndirectedGraph m_nbi;
  SyncMap m_syncMap;
  int     m_stateId;
  Ipv4Address    m_inbandAddress;
  Ptr<NetDevice> m_inband;
  std::string m_dftAccessProfile;
  std::string m_dftMcastProfile;
};
} // namespace ns3
#endif  /* CONTROLLER_H */
