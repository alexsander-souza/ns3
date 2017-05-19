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

#ifndef __ANCP_AN_AGENT_H__
#define __ANCP_AN_AGENT_H__

#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ancp-header.h>
#include <ns3/ancp-adj.h>

namespace ns3
{
#define ANCP_NAS_PORT               (6068)

class Address;
class AncpHeader;

/**
 * ANCP agent (AN)
 *
 */
class AncpAnAgent : public Application
{
public:
  typedef Callback<int, const std::string&, const std::string& > AncpAnLineConfigCb;

  typedef Callback<int, const std::string&, const std::string& > AncpAnMCastLineConfigCb;
  typedef Callback<int, const std::string&, const std::list<Address>&,
                   const std::list<Address>&, const std::list<Address>&,
                   bool, bool > AncpAnMCastProfileCb;
  typedef Callback<int, const std::string&, int, const Address & > AncpAnMCastCommandCb;

  static TypeId GetTypeId(void);

  AncpAnAgent();

  virtual ~AncpAnAgent();

  virtual void Print(std::ostream& os) const;

  void SendMCastAdmissionCtl(const std::string&circuit_id, int command, const Address &group);

  /* RFC6320 - sec 6 */
  virtual void SendPortUp(const std::string &circuit_id, uint32_t tag_mode,
                          uint32_t up_rate, uint32_t down_rate) const;
  virtual void SendPortDown(const std::string &circuit_id) const;

  bool IsEstablished() const;

protected:
  typedef std::map<Address, Ptr<AncpAdjacency> > AdjacencyList;

  /* ns3::Application methods */
  virtual void DoDispose(void);
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  int HandleAncpMessage(Mac48Address &adj_name, AncpHeader &msg);

  Ptr<AncpAdjacency> GetActiveAdjacency() const;

private:
  Ptr<AncpAdjacency> CreateAdjacency(Address &nas);

  Mac48Address m_MyMac;                            /**< local interface MAC address */
  Ipv4Address m_MyAddr;                            /**< local interface IPv4 address */
  uint32_t m_MyPort;                               /**< local port number */
  Ipv4Address m_NasAddr;                           /**< Primary NAS address */

  AdjacencyList m_AdjList;
  AncpHeader::AncpCapList m_capList;               /**< Node capability list */

  AncpAnLineConfigCb m_lineConfigHandler;
  AncpAnMCastLineConfigCb m_mcastLineConfigHandler;
  AncpAnMCastProfileCb m_mcastProfileHandler;
  AncpAnMCastCommandCb m_mcastCommandHandler;
};
}
#endif /* __ANCP_NAS_AGENT_H__ */
