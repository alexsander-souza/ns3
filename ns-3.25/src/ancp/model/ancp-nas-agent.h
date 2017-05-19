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
 * Author: Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#ifndef __ANCP_NAS_AGENT_H__
#define __ANCP_NAS_AGENT_H__

#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ancp-header.h>
#include <ns3/ancp-adj.h>

namespace ns3
{
#define ANCP_NAS_PORT               (6068)

class Socket;
class Packet;
class Address;
class AncpHeader;

/**
 * ANCP agent (NAS)
 *
 */
class AncpNasAgent : public Application
{
public:
  typedef Callback<int, const Mac48Address&, const std::string&, uint32_t, uint32_t, uint32_t > AncpNasPortUpCb;
  typedef Callback<int, const Mac48Address&, const std::string& > AncpNasPortDownCb;
  typedef Callback<int, const Mac48Address&, const std::string&, const Address&, bool> AncpNasMCastAdmissionCb;
  typedef Callback<int, const Mac48Address&> AncpNasNewAdjacencyCb;

  static TypeId GetTypeId(void);

  AncpNasAgent();

  virtual ~AncpNasAgent();

  void StartNetwork(Ptr<NetDevice> netdev);

  virtual void Print(std::ostream& os) const;

  int SendPortConfigCommand(const Mac48Address &anName, const std::string &circuitId,
                            const std::string &profName);

  int SendMCastPortConfigCommand(const Mac48Address &anName, const std::string &circuitId,
                                 const std::string &profName);

  int SendMCastServiceProfile(const Mac48Address &anName, const std::string &profName,
                              const std::list<Address> &whitelist,
                              const std::list<Address> &greylist,
                              const std::list<Address> &blacklist,
                              bool doWhitelistCac, bool doMRepCtlCac);

  int SendMCastCommand(const Mac48Address &anName, const std::string &circuitId,
                       int command, const Address &group);

protected:
  typedef std::map<Address, Ptr<AncpAdjacency> > AdjacencyList;

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  int HandleAncpMessage(Mac48Address &adj_name, AncpHeader &msg);

private:
  /**
   * \brief Receive packets
   */
  void AcceptHandler(Ptr<Socket> socket, const Address& from);

  Ptr<AncpAdjacency> GetAdjacency(const Mac48Address &anName) const;

  Ptr<Socket>              m_Sock;                 /**< AF_INET TCP listen-socket */
  Mac48Address m_MyMac;                            /**< local interface MAC address */
  Ipv4Address m_MyAddr;                            /**< local interface IPv4 address */
  uint32_t m_MyPort;                               /**< local port number */
  AdjacencyList m_AdjList;
  AncpHeader::AncpCapList m_capList;               /**< Node capability list */

  AncpNasPortUpCb m_portUpHandler;
  AncpNasPortDownCb m_portDownHandler;
  AncpNasMCastAdmissionCb m_admissionCacHandler;
  AncpNasNewAdjacencyCb m_newAdjacencyHandler;
};
}
#endif /* __ANCP_NAS_AGENT_H__ */
