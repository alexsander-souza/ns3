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

#ifndef __ANCP_ADJACENCY_H__
#define __ANCP_ADJACENCY_H__

#include <list>
#include <ns3/simple-ref-count.h>
#include <ns3/string.h>
#include <ns3/event-id.h>
#include <ns3/nstime.h>
#include <ns3/callback.h>
#include <ns3/ptr.h>
#include <ns3/ipv4-address.h>
#include <ns3/ancp-header.h>

namespace ns3
{
class Address;
class Socket;

class AncpAdjacency : public SimpleRefCount<AncpAdjacency>
{
public:
  typedef Callback<int, Mac48Address&, AncpHeader& > AncpHandlerCb;

  enum {
    ADJ_ST_SYNSENT,
    ADJ_ST_SYNRCVD,
    ADJ_ST_ESTAB,
    ADJ_ST_DEAD, /* implementation state */
  };

  /**
   * Adjacency constructor
   * \param mac         NAS MAC address
   * \param port        NAS port index
   * \param ancp_sock   ANCP connection
   * \param is_nas      The node is a NAS
   */
  AncpAdjacency(Mac48Address &mac, uint32_t port, Ptr<Socket> &ancp_sock, bool is_nas);

  virtual ~AncpAdjacency();

  /**
   * \brief Start Adjacency Protocol
   */
  void Start(AncpHandlerCb cb);

  void SetAdjProtoTimeout(uint timeout);

  void SetCapabilities(AncpHeader::AncpCapList &capList);

  bool IsEstablished() const;
  bool IsDead() const;

  int SendControlMessage(AncpHeader &msg);

  Mac48Address GetAdjacencyName() const;

protected:
  void NetHandler(Ptr<Socket> socket);

  /**
   * \brief Protocol message handler
   */
  void HandleAdjProtocol(AncpHeader &msg);

  /**
   * \brief Protocol timeout handler
   */
  void AdjProtoTimeoutHandler(void);

  /* Operations defined in the standard */
  void RecordAdjState(const AncpHeader &msg);
  void ResetTheLink();
  bool VerifyAdjState(const AncpHeader &msg);

  void SendMessage(uint msg_code);
  void SendRstAck(const AncpHeader *msg);

  void SetState(uint state);

  uint32_t GetTransactionId(uint8_t msg_type);

private:
  void ProccessOutputQueue();

  bool m_IsNAS;                               /**< Whether this adjacency is a NAS */
  uint m_State;                               /**< Adjacency state */
  uint m_AdjTimeout;                          /**< State machine timeout (ms) */
  uint m_AdjTimeoutCnt;                       /**< Retransmission timeout count */
  EventId m_AdjTimer;                         /**< State machine Timer */
  Time m_lastAck;                             /**< Timestamp of the last ACK sent */

  Mac48Address m_MyMac;                       /**< local interface MAC address */
  uint32_t m_MyInstance;                      /**< local instance number */
  uint32_t m_MyPort;                          /**< local port number */
  AncpHeader::AncpCapList m_MyCapList;        /**< Node capability list */

  uint32_t m_TheirInstance;                   /**< Remote Instance number */
  Mac48Address m_TheirMac;                    /**< Remote interface MAC address */
  uint32_t m_TheirPort;                       /**< Remote port number */
  AncpHeader::AncpCapList m_TheirCapList;     /**< Remote capability list */

  Ptr<Socket>    m_socket;                    /**< Adjacency connection */

  AncpHandlerCb m_AncpHandler;                /**< Handles ANCP protocol */

  uint32_t m_transactionId;                   /**< TransactionId Counter */

  std::list<Ptr<Packet> >  m_output_queue;    /**< Adjacency output queue */

  Ptr<Packet> m_fragment;
};
}
#endif /* __ANCP_ADJACENCY_H__ */
