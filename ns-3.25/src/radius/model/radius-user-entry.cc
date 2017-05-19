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
#include "radius-user-entry.h"

NS_LOG_COMPONENT_DEFINE ("RadiusUserEntry");

namespace ns3
{
RadiusUserEntry::RadiusUserEntry (std::string username)
{
  NS_LOG_FUNCTION (this << username);
  m_user_name = username;
}

RadiusUserEntry::~RadiusUserEntry (void)
{
  NS_LOG_FUNCTION (this);
}

std::string RadiusUserEntry::GetUserName (void) const
{
  NS_LOG_FUNCTION (this);
  return m_user_name;
}

void RadiusUserEntry::SetUserPassword (std::string passwd)
{
  NS_LOG_FUNCTION (this << passwd);
  m_user_password = passwd;
}

std::string RadiusUserEntry::GetUserPassword (void) const
{
  NS_LOG_FUNCTION (this);
  return m_user_password;
}

void RadiusUserEntry::SetCalledId (std::string id)
{
  NS_LOG_FUNCTION (this << id);
  m_called_id = id;
}

std::string RadiusUserEntry::GetCalledId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_called_id;
}

void RadiusUserEntry::SetCallingId (std::string id)
{
  NS_LOG_FUNCTION (this << id);
  m_calling_id = id;
}

std::string RadiusUserEntry::GetCallingId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_calling_id;
}

void RadiusUserEntry::SetNasIdentifier (std::string id)
{
  NS_LOG_FUNCTION (this << id);
  m_nas_identifier = id;
}

std::string RadiusUserEntry::GetNasIdentifier (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas_identifier;
}

void RadiusUserEntry::SetSessionId (std::string id)
{
  NS_LOG_FUNCTION (this << id);
  m_session_id = id;
}

std::string RadiusUserEntry::GetSessionId (void) const
{
  NS_LOG_FUNCTION (this);
  return m_session_id;
}

void RadiusUserEntry::SetNasPort (uint32_t port)
{
  NS_LOG_FUNCTION (this << port);
  m_nas_port = port;
}

uint32_t RadiusUserEntry::GetNasPort (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas_port;
}

void RadiusUserEntry::SetNasPortType (uint32_t port_type)
{
  NS_LOG_FUNCTION (this << port_type);
  m_nas_port_type = port_type;
}

uint32_t RadiusUserEntry::GetNasPortType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas_port_type;
}

void RadiusUserEntry::SetSessionTime (uint32_t time)
{
  NS_LOG_FUNCTION (this << time);
  m_session_time = time;
}

uint32_t RadiusUserEntry::GetSessionTime (void) const
{
  NS_LOG_FUNCTION (this);
  return m_session_time;
}

void RadiusUserEntry::SetTerminateCause (uint32_t cause)
{
  NS_LOG_FUNCTION (this << cause);
  m_terminate_cause = cause;
}

uint32_t RadiusUserEntry::GetTerminateCause (void) const
{
  NS_LOG_FUNCTION (this);
  return m_terminate_cause;
}

std::ostream & operator << (std::ostream & os, const RadiusUserEntry & h)
{
  os << "[RAD_USER:" << h.GetUserName() <<"]";
  return os;
}

}
