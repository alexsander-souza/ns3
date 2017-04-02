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

#include "ns3/log.h"
#include "ns3/bridged-net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("BridgedNetDevice");

NS_OBJECT_ENSURE_REGISTERED (BridgedNetDevice);

TypeId BridgedNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BridgedNetDevice")
                      .SetParent<NetDevice> ()
                      ;
  return tid;
}

bool BridgedNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

} // namespace ns3