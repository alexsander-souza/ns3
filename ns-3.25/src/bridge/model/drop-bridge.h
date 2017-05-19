/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Kazuki Yokoyama <kmyokoyama@inf.ufrgs.br>
 */

#ifndef DROP_BRIDGE_H
#define DROP_BRIDGE_H

#include "ns3/nstime.h"
#include <stdint.h>
#include <string>
#include <ns3/nstime.h>

#include <iostream>

namespace ns3
{

class DropBridge : public Object
{
public:
  static TypeId GetTypeId (void);

  DropBridge();
  ~DropBridge();

  bool ShouldDrop(void);
  void SetProb(double prob);

protected:
  bool drop;
  double dropProb;
};

} // namespace ns3

#endif /* DROP_BRIDGE_H */
