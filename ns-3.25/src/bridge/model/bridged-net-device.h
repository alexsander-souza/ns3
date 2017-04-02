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

#ifndef __BRIDGED_NET_DEVICE_H__
#define __BRIDGED_NET_DEVICE_H__

#include "ns3/net-device.h"

namespace ns3
{

class BridgedNetDevice : public NetDevice
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual uint32_t GetNBridgePorts (void) const = 0;

  virtual Ptr<NetDevice> GetBridgePort (uint32_t n) const = 0;

  virtual bool IsBridge (void) const;
};

} // ns3

#endif /* __BRIDGED_NET_DEVICE_H__ */