/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2015 UFRGS
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
#ifndef __ACCESS_PROFILE_H__
#define __ACCESS_PROFILE_H__

#include <string>
#include "ns3/simple-ref-count.h"
#include "ns3/data-rate.h"

namespace ns3 {
class AccessProfile : public SimpleRefCount<AccessProfile>{
public:

  AccessProfile();
  AccessProfile(const std::string& name,
                DataRate upRate,
                DataRate downRate);
  const std::string& GetName() const;
  DataRate&    GetUpstreamRate();
  DataRate&    GetDownstreamRate();

private:

  std::string m_name;
  DataRate m_upRate;
  DataRate m_downRate;
};
std::ostream& operator<<(std::ostream       & os,
                         const AccessProfile& prof);
}
#endif /* __ACCESS_PROFILE_H__ */
