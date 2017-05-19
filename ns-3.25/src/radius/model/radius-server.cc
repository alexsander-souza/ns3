/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Alexsander de Souza
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

#include <ns3/ipv4.h>
#include <ns3/ipv4-address.h>
#include <ns3/socket-factory.h>
#include <ns3/packet.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/pointer.h>
#include <ns3/log.h>
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <arpa/inet.h>
#include "radius-header.h"
#include "radius-server.h"

NS_LOG_COMPONENT_DEFINE("RadiusServer");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(RadiusServer);

TypeId RadiusServer::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::RadiusServer")
                      .SetParent<Header> ()
                      .AddConstructor<RadiusServer> ()
                      .AddAttribute("AuthPort",
                                    "RADIUS server Authentication UDP port",
                                    UintegerValue(1812),
                                    MakeUintegerAccessor(&RadiusServer::m_srv_auth_port),
                                    MakeUintegerChecker<uint16_t> ())
                      .AddAttribute("AccPort",
                                    "RADIUS server Accounting UDP port",
                                    UintegerValue(1813),
                                    MakeUintegerAccessor(&RadiusServer::m_srv_acc_port),
                                    MakeUintegerChecker<uint16_t> ())
                      .AddAttribute("ServerSecret",
                                    "RADIUS server secret",
                                    StringValue(""),
                                    MakeStringAccessor(&RadiusServer::m_server_secret),
                                    MakeStringChecker())
                      .AddAttribute("ServerDelay",
                                    "RADIUS server response delay (ms)",
                                    UintegerValue(500),
                                    MakeUintegerAccessor(&RadiusServer::m_server_delay),
                                    MakeUintegerChecker<uint64_t> ());

  return tid;
}

RadiusServer::RadiusServer ()
  : m_auth_sock(0),
  m_acc_sock(0)
{
  NS_LOG_FUNCTION(this);
}

RadiusServer::~RadiusServer ()
{
  NS_LOG_FUNCTION(this);
}

void RadiusServer::SetDatabase(Ptr<RadiusDB> database)
{
  NS_LOG_FUNCTION(this << database);
  m_rad_db = database;
}

void RadiusServer::DoDispose()
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void RadiusServer::StartApplication()
{
  NS_LOG_FUNCTION(this);

  if (m_auth_sock == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
      InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_srv_auth_port);
      m_auth_sock = Socket::CreateSocket(GetNode(), tid);
      m_auth_sock->Bind(local);
      m_auth_sock->Listen();
    }
  m_auth_sock->SetRecvCallback(MakeCallback(&RadiusServer::HandleAuthentication, this));

  if (m_acc_sock == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
      InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_srv_acc_port);
      m_acc_sock = Socket::CreateSocket(GetNode(), tid);
      m_acc_sock->Bind(local);
      m_acc_sock->Listen();
    }
  m_acc_sock->SetRecvCallback(MakeCallback(&RadiusServer::HandleAccounting, this));

  if (m_rad_db == 0)
    {
      m_rad_db = CreateObject<RadiusDB>();
    }
}

void RadiusServer::StopApplication()
{
  NS_LOG_FUNCTION(this);

  if (m_auth_sock != 0)
    {
      m_auth_sock->Close();
      m_auth_sock->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
      m_auth_sock = 0;
    }

  if (m_acc_sock != 0)
    {
      m_acc_sock->Close();
      m_acc_sock->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
      m_acc_sock = 0;
    }

  while (!m_peding_reqs.empty())
    {
      EventId evt = m_peding_reqs.front();

      Simulator::Cancel(evt);
      m_peding_reqs.pop();
    }

  if (m_rad_db != 0)
    {
      m_rad_db = 0;
    }
}

void RadiusServer::HandleAuthentication(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = 0;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      RadiusMessage request;
      RadiusMessage response;

      if (packet->GetSize() <= 0)
        continue;

      packet->RemoveHeader(request);

      NS_ASSERT(request.GetMessageCode() == RadiusMessage::RAD_ACCESS_REQUEST);
      NS_LOG_LOGIC("Message:" << request);

      /* Read AVPs */
      std::string user_name = *(request.GetAttributeByType(RadiusAVP::RAD_ATTR_USER_NAME));
      std::string user_password = *(request.GetAttributeByType(RadiusAVP::RAD_ATTR_USER_PASSWORD));

      RadiusUserEntry *user = m_rad_db->GetUser(user_name);
      if (user == 0 || user->GetUserPassword() != user_password)
        {
          response.SetMessageCode(RadiusMessage::RAD_ACCESS_REJECT);
        }
      else
        {
          response.SetMessageCode(RadiusMessage::RAD_ACCESS_ACCEPT);

          /* Update DB */
          const RadiusAVP* avp_calledid = request.GetAttributeByType(RadiusAVP::RAD_ATTR_CALLED_STATION_ID);
          const RadiusAVP* avp_callingid = request.GetAttributeByType(RadiusAVP::RAD_ATTR_CALLING_STATION_ID);
          const RadiusAVP* avp_nasid = request.GetAttributeByType(RadiusAVP::RAD_ATTR_NAS_IDENTIFIER);
          const RadiusAVP* avp_port = request.GetAttributeByType(RadiusAVP::RAD_ATTR_NAS_PORT);
          const RadiusAVP* avp_port_type = request.GetAttributeByType(RadiusAVP::RAD_ATTR_NAS_PORT_TYPE);

          /*TODO check mandatory AVPs*/
          if (avp_calledid != 0)
            user->SetCalledId(*avp_calledid);
          if (avp_callingid != 0)
            user->SetCallingId(*avp_callingid);
          if (avp_nasid != 0)
            user->SetNasIdentifier(*avp_nasid);
          if (avp_port != 0)
            user->SetNasPort(*avp_port);
          if (avp_port_type != 0)
            user->SetNasPortType(*avp_port_type);
        }

      response.SetMessageID(request.GetMessageID());

      Time t = MilliSeconds(m_server_delay);

      ScheduleTransmit(t, from, response);
    }
}

void RadiusServer::HandleAccounting(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = 0;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      RadiusMessage request;

      if (packet->GetSize() <= 0)
        continue;

      packet->RemoveHeader(request);

      NS_ASSERT(request.GetMessageCode() == RadiusMessage::RAD_ACCOUNTING_REQUEST);

      NS_LOG_LOGIC("Message:" << request);

      const RadiusAVP* avp_event = request.GetAttributeByType(RadiusAVP::RAD_ATTR_ACCT_STATUS_TYPE);
      const RadiusAVP* avp_session_id = request.GetAttributeByType(RadiusAVP::RAD_ATTR_ACCT_SESSION_ID);

      if (avp_event == 0 || avp_session_id == 0)
        {
          NS_LOG_WARN("Invalid request" << request);
          continue;
        }

      std::string session_id = *avp_session_id;

      switch (uint32_t(*avp_event))
        {
        case RadiusAVP::RAD_ACCT_START:
        {
          const RadiusAVP* avp_username = request.GetAttributeByType(RadiusAVP::RAD_ATTR_USER_NAME);
          NS_ASSERT(avp_username != 0);

          std::string username = *avp_username;
          m_rad_db->StartUserSession(session_id, username);
          break;
        }

        case RadiusAVP::RAD_ACCT_STOP:
        {
          const RadiusAVP* avp_session_time = request.GetAttributeByType(RadiusAVP::RAD_ATTR_ACCT_SESSION_TIME);
          const RadiusAVP* avp_cause = request.GetAttributeByType(RadiusAVP::RAD_ATTR_ACCT_TERMINATE_CAUSE);

          NS_ASSERT(avp_session_time != 0);
          NS_ASSERT(avp_cause != 0);

          m_rad_db->StopUserSession(session_id, *avp_session_time, *avp_cause);
          break;
        }

        case RadiusAVP::RAD_ACCT_UPDATE:
          NS_LOG_DEBUG("RADIUS ACC Interim-update handling not implemented");
          break;

        default:
        {
          NS_LOG_WARN("Unsuported Radius Event type:" << uint32_t(*avp_event));
          continue;
        }
        }

      /* Accounting-Resp doesn't have mandatory AVPs */
      RadiusMessage response;
      response.SetMessageCode(RadiusMessage::RAD_ACCOUNTING_RESPONSE);
      response.SetMessageID(request.GetMessageID());

      Time t = MilliSeconds(m_server_delay);

      ScheduleTransmit(t, from, response);
    }
}

void RadiusServer::ScheduleTransmit(Time dt, Address client, RadiusMessage &response)
{
  NS_LOG_FUNCTION(this << dt << client << response);

  EventId eventId = Simulator::Schedule(dt, &RadiusServer::Send, this, client, response);

  m_peding_reqs.push(eventId);
}

void RadiusServer::Send(Address client, RadiusMessage &response)
{
  Ptr<Packet> pkt = Create<Packet> ();

  pkt->AddHeader(response);

  if (response.GetMessageCode() == RadiusMessage::RAD_ACCOUNTING_RESPONSE)
    m_acc_sock->SendTo(pkt, 0, client);
  else
    m_auth_sock->SendTo(pkt, 0, client);

  /* assume requests are serviced in order */
  m_peding_reqs.pop();
}
}
