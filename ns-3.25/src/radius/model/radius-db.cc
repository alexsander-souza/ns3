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

#include <ns3/ipv4-address.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/pointer.h>
#include <ns3/log.h>
#include "radius-db.h"

NS_LOG_COMPONENT_DEFINE("RadiusDB");

namespace ns3
{
NS_OBJECT_ENSURE_REGISTERED(RadiusDB);

TypeId RadiusDB::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::RadiusDB")
                      .SetParent<Object> ()
                      .AddConstructor<RadiusDB> ();

  return tid;
}

RadiusDB::RadiusDB (void)
{
  NS_LOG_FUNCTION(this);
}

RadiusDB::~RadiusDB (void)
{
  NS_LOG_FUNCTION(this);
}

void RadiusDB::AddUser(RadiusUserEntry& user)
{
  NS_LOG_FUNCTION(this << user);

  m_users.insert(m_users.begin(), make_pair(user.GetUserName(), user));
}

RadiusUserEntry* RadiusDB::GetUser(const std::string& username)
{
  NS_LOG_FUNCTION(this << username);
  RadiusUserListIT it = m_users.find(username);

  if (it != m_users.end())
    return &(it->second);

  NS_LOG_DEBUG("User not found: " << username);
  return 0;
}

void RadiusDB::StartUserSession(std::string& session_id, std::string& username)
{
  NS_LOG_FUNCTION(this << session_id);

  RadiusUserEntry *user = GetUser(username);
  if (user == 0)
    return;

  m_sessions.insert(m_sessions.begin(), make_pair(session_id, user));

  user->SetSessionId(session_id);

  NS_LOG_DEBUG("Started session for user " << user << " SID" << session_id);
}

void RadiusDB::StopUserSession(std::string& session_id, uint32_t session_time, uint32_t terminate_cause)
{
  NS_LOG_FUNCTION(this << session_id);

  RadiusSessionListIT it = m_sessions.find(session_id);
  if (it == m_sessions.end())
    {
      NS_LOG_WARN("Session not found: " << session_id);
      return;
    }

  RadiusUserEntry* user = it->second;
  m_sessions.erase(it);

  user->SetSessionTime(session_time);
  user->SetTerminateCause(terminate_cause);

  NS_LOG_DEBUG("Ended session for user " << user);
}
}
