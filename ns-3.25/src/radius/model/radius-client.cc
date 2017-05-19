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
#include <arpa/inet.h>
#include "radius-header.h"
#include "radius-client.h"

NS_LOG_COMPONENT_DEFINE("RadiusClient");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(RadiusClient);

TypeId RadiusClient::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::RadiusClient")
                      .SetParent<Header> ()
                      .AddConstructor<RadiusClient> ()
                      .AddAttribute("ServerAddress",
                                    "RADIUS server IPv4 Address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&RadiusClient::m_server_addr),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("AuthPort",
                                    "RADIUS server UDP port",
                                    UintegerValue(1812),
                                    MakeUintegerAccessor(&RadiusClient::m_server_port),
                                    MakeUintegerChecker<uint16_t> ())
                      .AddAttribute("AccPort",
                                    "RADIUS server Accounting UDP port",
                                    UintegerValue(1813),
                                    MakeUintegerAccessor(&RadiusClient::m_server_acc_port),
                                    MakeUintegerChecker<uint16_t> ())
                      .AddAttribute("ServerSecret",
                                    "RADIUS server secret",
                                    StringValue(""),
                                    MakeStringAccessor(&RadiusClient::m_server_secret),
                                    MakeStringChecker());

  return tid;
}

RadiusClient::RadiusClient ()
  : m_socket(0),
  m_request_counter(0),
  m_cb_request_completed(0)
{
  NS_LOG_FUNCTION(this);
}

RadiusClient::~RadiusClient ()
{
  NS_LOG_FUNCTION(this);
}

void RadiusClient::DoDispose()
{
  NS_LOG_FUNCTION(this);
  Application::DoDispose();
}

void RadiusClient::SetRequestDoneCb(RadiusRequestCompletedCb cb)
{
  NS_LOG_FUNCTION(this);
  m_cb_request_completed = cb;
}

void RadiusClient::StartApplication()
{
  NS_LOG_FUNCTION(this);

  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket(GetNode(), tid);
      m_socket->Bind();
    }

  m_socket->SetRecvCallback(MakeCallback(&RadiusClient::NetHandler, this));
}

void RadiusClient::StopApplication()
{
  NS_LOG_FUNCTION(this);

  if (m_socket != 0)
    {
      m_socket->Close();
      m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }
  m_pending_reqs.clear();
}

int RadiusClient::SendRequest(uint8_t code, RadiusMessage::RadiusAvpList &avp_list)
{
  NS_LOG_FUNCTION(this);

  Ptr<Packet> packet = Create<Packet>();
  RadiusMessage rad_req;

  rad_req.SetMessageCode(code);
  rad_req.SetMessageID(++m_request_counter);
  rad_req.AddAttributeList(avp_list);
  packet->AddHeader(rad_req);

  if (code != RadiusMessage::RAD_ACCOUNTING_REQUEST)
    m_socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::ConvertFrom(m_server_addr), m_server_port));
  else
    m_socket->SendTo(packet, 0, InetSocketAddress(Ipv4Address::ConvertFrom(m_server_addr), m_server_acc_port));

  m_pending_reqs[rad_req.GetMessageID()] = rad_req;

  return 0;
}

void RadiusClient::NetHandler(Ptr<Socket> socket)
{
  NS_LOG_FUNCTION(this << socket);

  Ptr<Packet> packet = 0;
  Address from;

  while ((packet = socket->RecvFrom(from)))
    {
      RadiusMessage response;

      if (packet->GetSize() <= 0)
        continue;

      packet->RemoveHeader(response);

      RadiusClientPendingListIT it = m_pending_reqs.find(response.GetMessageID());

      if (it == m_pending_reqs.end())
        {
          NS_LOG_WARN(this << " Got unexpected response " << response);
          continue;
        }

      m_pending_reqs.erase(it);

      if (!m_cb_request_completed.IsNull())
        m_cb_request_completed(response.GetMessageCode(),
                               response.GetAttributeList());
    }
}

int RadiusClient::DoAuthentication(const std::string& username, const std::string& passwd,
                                   const std::string& nas_id, uint32_t nas_port)
{
  NS_LOG_FUNCTION(this << username << nas_id);

  RadiusMessage::RadiusAvpList avp_list;
  uint32_t srv_type = 2UL; /* Framed */

  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_USER_NAME,
                               username.length(),
                               (const uint8_t*)username.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_USER_PASSWORD,
                               passwd.length(),
                               (const uint8_t*)passwd.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_NAS_IDENTIFIER,
                               nas_id.length(),
                               (const uint8_t*)nas_id.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_NAS_PORT, nas_port));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_SERVICE_TYPE, srv_type));

  return SendRequest(RadiusMessage::RAD_ACCESS_REQUEST, avp_list);
}

int RadiusClient::DoStartAccounting(uint32_t acc_event, const std::string &session_id,
                                    const std::string& username)
{
  NS_LOG_FUNCTION(this << acc_event << session_id << username);
  RadiusMessage::RadiusAvpList avp_list;

  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_USER_NAME,
                               username.length(),
                               (const uint8_t*)username.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_SESSION_ID,
                               session_id.length(),
                               (const uint8_t*)session_id.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_STATUS_TYPE, acc_event));

  return SendRequest(RadiusMessage::RAD_ACCOUNTING_REQUEST, avp_list);
}

int RadiusClient::DoStopAccounting(uint32_t acc_event, const std::string &session_id,
                                   uint32_t session_time, uint32_t termination_cause)
{
  NS_LOG_FUNCTION(this << acc_event << session_id);
  RadiusMessage::RadiusAvpList avp_list;

  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_SESSION_ID,
                               session_id.length(),
                               (const uint8_t*)session_id.c_str()));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_STATUS_TYPE, acc_event));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_SESSION_TIME, session_time));
  avp_list.push_back(RadiusAVP(RadiusAVP::RAD_ATTR_ACCT_TERMINATE_CAUSE, termination_cause));

  return SendRequest(RadiusMessage::RAD_ACCOUNTING_REQUEST, avp_list);
}
}
