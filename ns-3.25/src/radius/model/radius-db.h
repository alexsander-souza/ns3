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

#ifndef __RADIUS_DB_H__
#define __RADIUS_DB_H__

#include <map>
#include <ns3/object.h>
#include <ns3/string.h>
#include <ns3/ipv4-address.h>
#include <ns3/radius-user-entry.h>

namespace ns3
{

class RadiusDB : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Constructor.
   */
  RadiusDB (void);

  /**
   * \brief Destructor.
   */
  virtual ~RadiusDB (void);

  void AddUser(RadiusUserEntry& user);

  RadiusUserEntry* GetUser(const std::string& username);

  void StartUserSession(std::string& session_id, std::string& username);

  void StopUserSession(std::string& session_id, uint32_t session_time, uint32_t terminate_cause);

private:
  typedef std::map<std::string, RadiusUserEntry> RadiusUserList;
  typedef std::map<std::string, RadiusUserEntry>::iterator RadiusUserListIT;

  typedef std::map<std::string, RadiusUserEntry*> RadiusSessionList;
  typedef std::map<std::string, RadiusUserEntry*>::iterator RadiusSessionListIT;

  RadiusUserList                 m_users;
  RadiusSessionList              m_sessions;

};

}

#endif /* __RADIUS_DB_H__ */
