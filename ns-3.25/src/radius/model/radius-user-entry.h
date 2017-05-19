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

#ifndef __RADIUS_USER_ENTRY_H__
#define __RADIUS_USER_ENTRY_H__

#include <ns3/object.h>
#include <ns3/string.h>

namespace ns3
{

class RadiusUserEntry
{
public:
  RadiusUserEntry (std::string username);
  virtual ~RadiusUserEntry (void);

  //~ void SetUserName (std::string &username);
  std::string GetUserName (void) const;

  void SetUserPassword (std::string passwd);
  std::string GetUserPassword (void) const;

  void SetCalledId (std::string id);
  std::string GetCalledId (void) const;

  void SetCallingId (std::string id);
  std::string GetCallingId (void) const;

  void SetNasIdentifier (std::string id);
  std::string GetNasIdentifier (void) const;

  void SetSessionId (std::string id);
  std::string GetSessionId (void) const;

  void SetNasPort (uint32_t port);
  uint32_t GetNasPort (void) const;

  void SetNasPortType (uint32_t port_type);
  uint32_t GetNasPortType (void) const;

  void SetSessionTime (uint32_t time);
  uint32_t GetSessionTime (void) const;

  void SetTerminateCause (uint32_t cause);
  uint32_t GetTerminateCause (void) const;

private:
  std::string m_user_name;
  std::string m_user_password;

  std::string m_session_id;
  std::string m_called_id;
  std::string m_calling_id;
  std::string m_nas_identifier;
  uint32_t m_nas_port;
  uint32_t m_nas_port_type;
  uint32_t m_session_time;
  uint32_t m_terminate_cause;
};

std::ostream & operator << (std::ostream & os, const RadiusUserEntry & h);

}

#endif /* __RADIUS_USER_ENTRY_H__ */
