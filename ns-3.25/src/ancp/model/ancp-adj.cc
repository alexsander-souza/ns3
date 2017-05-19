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

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/net-device.h"
#include "ns3/channel.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include <algorithm>

#include "ancp-adj.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("AncpAdjacency");

AncpAdjacency::AncpAdjacency(Mac48Address &mac, uint32_t port,
                             Ptr<Socket> &ancp_sock, bool is_nas) :
  m_IsNAS(is_nas),
  m_State(ADJ_ST_SYNSENT),
  m_AdjTimeout(ANCP_TIMER),
  m_AdjTimeoutCnt(0),
  m_lastAck(0),
  m_MyMac(mac),
  m_MyInstance(1),
  m_MyPort(port),
  m_TheirInstance(0),
  m_TheirMac(Mac48Address("00:00:00:00:00:00")),
  m_TheirPort(0),
  m_socket(ancp_sock),
  m_transactionId(0),
  m_fragment(nullptr)
{
  NS_LOG_FUNCTION(this << "ANCP ADJ " << m_MyMac << " NAS=" << m_IsNAS);

  m_socket->SetRecvCallback(MakeCallback(&AncpAdjacency::NetHandler, this));
}

AncpAdjacency::~AncpAdjacency()
{
  NS_LOG_FUNCTION(this);

  Simulator::Cancel(m_AdjTimer);

  if (m_socket != 0)
    {
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
      m_socket->Close();
      m_socket = 0;
    }
}

void AncpAdjacency::SetCapabilities(AncpHeader::AncpCapList &capList)
{
  NS_LOG_FUNCTION(this);
  m_MyCapList.merge(capList);
}

void AncpAdjacency::SetAdjProtoTimeout(uint timeout)
{
  NS_LOG_FUNCTION(this);
  m_AdjTimeout = timeout;
}

void AncpAdjacency::NetHandler(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = nullptr;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      while (packet->GetSize() > 0)
        {
          if (m_fragment != nullptr)
            {
              m_fragment->AddAtEnd(packet);
              packet = m_fragment;
              m_fragment = nullptr;
            }

          AncpHeader request;
          if (packet->RemoveHeader(request) == 0)
            {
              m_fragment = packet;
              break;
            }

          /* ignore all messages until Adjacency is established */
          if (!IsEstablished() && request.GetMsgType() != AncpHeader::MSG_ADJ_PROTOCOL)
            {
              NS_LOG_INFO(this << " Not established, ignoring");
              continue;
            }

          switch (request.GetMsgType())
            {
            case AncpHeader::MSG_ADJ_PROTOCOL:
              HandleAdjProtocol(request);
              break;

            default:
              /*FIXME Return the appropriate error message */
              if (m_AncpHandler(m_TheirMac, request) < 0)
                NS_LOG_WARN(this << " Failed to handle request " << request);
            }
        }
    }
}

/**
 * If incoming message is an RSTACK:
 *  If (A && C && !SYNSENT) Reset the link
 *  Else discard the message.
 * If incoming message is a SYN, SYNACK, or ACK:
 *  Response defined by the following state tables.
 */
void AncpAdjacency::HandleAdjProtocol(AncpHeader &msg)
{
  NS_LOG_FUNCTION(this << msg);

  bool condA = (m_TheirInstance == msg.GetSenderInstance());
  bool condB = ((condA) && (m_TheirMac == msg.GetSenderName())
                && (m_TheirPort == msg.GetSenderPort())); /* AND Partition is the same */
  bool condC = ((m_MyMac == msg.GetReceiverName())
                && (m_MyInstance == msg.GetReceiverInstance())
                && (m_MyPort == msg.GetReceiverPort()));

  /* Process RSTACK */
  if (msg.GetAdjCode() == AncpHeader::ADJ_RSTACK)
    {
      if (condA && condB && (m_State != ADJ_ST_SYNSENT))
        ResetTheLink();
      return;
    }

  /* Process other messages */
  switch (m_State)
    {
    case ADJ_ST_SYNSENT:
      switch (msg.GetAdjCode())
        {
        case AncpHeader::ADJ_SYNACK:
          if (condC)
            {
              RecordAdjState(msg);
              SendMessage(AncpHeader::ADJ_ACK);
              SetState(ADJ_ST_ESTAB);
            }
          else
            {
              SendRstAck(&msg);
            }
          break;

        case AncpHeader::ADJ_SYN:
          RecordAdjState(msg);
          SendMessage(AncpHeader::ADJ_SYNACK);
          SetState(ADJ_ST_SYNRCVD);
          break;

        default:
          SendRstAck(&msg);
        }
      break;

    case ADJ_ST_DEAD: /* resurrect */
    case ADJ_ST_SYNRCVD:
      switch (msg.GetAdjCode())
        {
        case AncpHeader::ADJ_SYNACK:
          if (condC)
            {
              RecordAdjState(msg);
              SendMessage(AncpHeader::ADJ_ACK);
              SetState(ADJ_ST_ESTAB);
            }
          else
            {
              SendRstAck(&msg);
            }
          break;

        case AncpHeader::ADJ_SYN:
          RecordAdjState(msg);
          SendMessage(AncpHeader::ADJ_SYNACK);
          SetState(ADJ_ST_SYNRCVD);
          break;

        case AncpHeader::ADJ_ACK:
          if (condB && condC)
            {
              SendMessage(AncpHeader::ADJ_ACK);
              SetState(ADJ_ST_ESTAB);
            }
          else
            {
              SendRstAck(&msg);
            }
        }
      break;

    case ADJ_ST_ESTAB:
      switch (msg.GetAdjCode())
        {
        case AncpHeader::ADJ_SYN:
        case AncpHeader::ADJ_SYNACK:
          m_AdjTimeoutCnt = 0;
          SendMessage(AncpHeader::ADJ_ACK);
          SetState(ADJ_ST_ESTAB);
          break;

        case AncpHeader::ADJ_ACK:
          if (condB && condC)
            {
              m_AdjTimeoutCnt = 0;
              SendMessage(AncpHeader::ADJ_ACK);
              SetState(ADJ_ST_ESTAB);
            }
          else
            {
              SendRstAck(&msg);
            }
        }
      break;
    }
}

/**
 * Timer Expires: Reset Timer
 *  If state = SYNSENT Send SYN
 *  If state = SYNRCVD Send SYNACK
 *  If state = ESTAB Send ACK
 */
void AncpAdjacency::AdjProtoTimeoutHandler(void)
{
  NS_LOG_FUNCTION(this << " State " << m_State);

  switch (m_State)
    {
    case ADJ_ST_SYNSENT:
      SendMessage(AncpHeader::ADJ_SYN);
      break;

    case ADJ_ST_SYNRCVD:
      SendMessage(AncpHeader::ADJ_SYNACK);
      break;

    case ADJ_ST_ESTAB:
      if (m_AdjTimeoutCnt++ < 3)
        {
          SendMessage(AncpHeader::ADJ_ACK);
        }
      else
        {
          SendRstAck(0);
          SetState(ADJ_ST_DEAD);
        }
      break;

    default:
      /* Should never happen */
      NS_LOG_WARN(this << " Protocol timeout in unexpected state");
      break;
    }
}

/**
 * Start the Adjacency Protocol
 *
 * MUST be called only from the AN node
 */
void AncpAdjacency::Start(AncpHandlerCb cb)
{
  NS_LOG_FUNCTION(this << " Starting ANCP ADJ protocol");

  m_AncpHandler = cb;

  if (m_IsNAS)
    {
      /* read incomming request */
      NetHandler(m_socket);
    }
  else
    {
      /* start new connection */
      SetState(ADJ_ST_SYNSENT);
      SendMessage(AncpHeader::ADJ_SYN);
    }
}

bool AncpAdjacency::VerifyAdjState(const AncpHeader &msg)
{
  NS_LOG_FUNCTION(this);

  /* Check Receiver ID */
  if ((m_MyMac != msg.GetReceiverName())
      || (m_MyInstance != msg.GetReceiverInstance())
      || (m_MyPort != msg.GetReceiverPort()))
    return false;

  ///FIXME check Sender's ID
  ///FIXME check capabilities

  return true;
}

void AncpAdjacency::RecordAdjState(const AncpHeader &msg)
{
  NS_LOG_FUNCTION(this);

  // Version is always the same
  // Timer is hard-coded

  m_TheirMac = msg.GetSenderName();
  m_TheirInstance = msg.GetSenderInstance();
  m_TheirPort = msg.GetSenderPort();

  m_TheirCapList.clear();
  m_TheirCapList.merge(msg.GetCapabilityList());
}

void AncpAdjacency::ResetTheLink()
{
  NS_LOG_FUNCTION(this);
  NS_LOG_INFO(this << "Reset " << m_MyMac << " == " << m_TheirMac);

  // Create new instance ID */
  m_MyInstance = std::max(m_MyInstance + 1, 1U);

  /* Clear Remote ID */
  m_TheirMac = Mac48Address("00:00:00:00:00:00");
  m_TheirInstance = 0;
  m_TheirPort = 0;
  m_TheirCapList.clear();

  /* Send SYN */
  m_State = ADJ_ST_SYNSENT;
  SendMessage(AncpHeader::ADJ_SYN);
}

void AncpAdjacency::SendMessage(uint msg_code)
{
  NS_LOG_FUNCTION(this << " sending " << msg_code);

  if (msg_code == AncpHeader::ADJ_ACK)
    {
      if ((Simulator::Now() - m_lastAck).GetMilliSeconds() < m_AdjTimeout)
        return;

      m_lastAck = Simulator::Now();
    }

  AncpHeader message;

  message.SetMsgType(AncpHeader::MSG_ADJ_PROTOCOL);
  message.SetAdjCode(msg_code);

  message.SetSenderName(m_MyMac);
  message.SetSenderPort(m_MyPort);
  message.SetSenderInstance(m_MyInstance);

  message.SetReceiverName(m_TheirMac);
  message.SetReceiverPort(m_TheirPort);
  message.SetReceiverInstance(m_TheirInstance);
  message.SetPartitionId(0);

  message.AddCapabilityList(m_MyCapList);

  if (msg_code == AncpHeader::ADJ_SYN)
    message.SetNAS(m_IsNAS);

  Ptr<Packet> pkt = Create<Packet> ();
  pkt->AddHeader(message);

  if (m_socket->Send(pkt, 0) < 0)
    NS_LOG_WARN(this << " Failed to send");

  Simulator::Cancel(m_AdjTimer);
  m_AdjTimer = Simulator::Schedule(MilliSeconds(m_AdjTimeout),
                                   &AncpAdjacency::AdjProtoTimeoutHandler, this);
}

void AncpAdjacency::SendRstAck(const AncpHeader* msg)
{
  NS_LOG_INFO(this << " sending RSTACK");

  AncpHeader message;

  message.SetMsgType(AncpHeader::MSG_ADJ_PROTOCOL);
  message.SetAdjCode(AncpHeader::ADJ_RSTACK);

  if (msg != 0)
    {
      NS_LOG_FUNCTION(this << *msg);
      /* If we have a incoming request, copy values from it */
      Mac48Address senderMac = msg->GetSenderName();
      Mac48Address recvMac = msg->GetReceiverName();

      message.SetSenderName(recvMac);
      message.SetSenderPort(msg->GetReceiverPort());
      message.SetSenderInstance(msg->GetReceiverInstance());

      message.SetReceiverName(senderMac);
      message.SetReceiverPort(msg->GetSenderPort());
      message.SetReceiverInstance(msg->GetSenderInstance());
    }
  else
    {
      /* In case of timeouts, use the values we have */
      message.SetSenderName(m_MyMac);
      message.SetSenderPort(m_MyPort);
      message.SetSenderInstance(m_MyInstance);

      message.SetReceiverName(m_TheirMac);
      message.SetReceiverPort(m_TheirPort);
      message.SetReceiverInstance(m_TheirInstance);
    }

  message.SetPartitionId(0);
  message.AddCapabilityList(m_MyCapList);
  message.SetNAS(m_IsNAS);

  Ptr<Packet> pkt = Create<Packet> ();
  pkt->AddHeader(message);

  if (m_socket->Send(pkt, 0) < 0)
    NS_LOG_WARN(this << " Failed to send RSTACK");
}

void AncpAdjacency::SetState(uint state)
{
  if (m_State == state)
    return;

  NS_LOG_FUNCTION(this << "State transition: " << m_State << " to " << state);
  m_State = state;

  if (IsEstablished())
    {
      /* Notify new Adjacency*/
      AncpHeader message;
      message.SetMsgType(AncpHeader::MSG_ADJ_PROTOCOL);
      m_AncpHandler(m_TheirMac, message);
    }
}

bool AncpAdjacency::IsEstablished() const
{
  return(m_State == ADJ_ST_ESTAB);
}

bool AncpAdjacency::IsDead() const
{
  return(m_State == ADJ_ST_DEAD);
}

Mac48Address AncpAdjacency::GetAdjacencyName() const
{
  NS_LOG_FUNCTION(this << m_TheirMac);
  return(m_TheirMac);
}

uint32_t AncpAdjacency::GetTransactionId(uint8_t msg_type)
{
  /* XXX The RFC states that we should keep individual TransactionId counters
   * for each kind of ANCP message, but that's too much work. */
  uint32_t transId = 0;

  switch (msg_type)
    {
    case AncpHeader::MSG_ADJ_PROTOCOL:
    case AncpHeader::MSG_PORT_UP:
    case AncpHeader::MSG_PORT_DOWN:
    case AncpHeader::MSG_ADJ_UPDATE:
      /* Event messages don't have a transaction ID */
      break;

    default:
      m_transactionId = std::max(((m_transactionId + 1) & 0x00FFFFFF), 1U);
      transId = m_transactionId;
      break;
    }

  NS_LOG_FUNCTION(this << transId);

  return(transId);
}

int AncpAdjacency::SendControlMessage(AncpHeader &msg)
{
  NS_LOG_FUNCTION(this << msg);

  /* Set Transaction ID */
  msg.SetTransactionId(GetTransactionId(msg.GetMsgType()));

  Ptr<Packet> pkt = Create<Packet> ();
  pkt->AddHeader(msg);

  if (msg.GetMsgType() != AncpHeader::MSG_ADJ_PROTOCOL) {
    NS_LOG_WARN(msg);
  }

  if (m_output_queue.empty())
    Simulator::Schedule(MilliSeconds(1), &AncpAdjacency::ProccessOutputQueue, this);

  m_output_queue.push_front(pkt);

  return(0);
}

void AncpAdjacency::ProccessOutputQueue()
{
  NS_LOG_FUNCTION(this);

  Ptr<Packet> pkt = m_output_queue.back();

  if (m_socket->Send(pkt, 0) < 0)
    NS_LOG_WARN(this << " Failed to send Control Message");

  m_output_queue.pop_back();
  if (!m_output_queue.empty())
    Simulator::Schedule(MilliSeconds(1), &AncpAdjacency::ProccessOutputQueue, this);
}
}  // namespace ns3
