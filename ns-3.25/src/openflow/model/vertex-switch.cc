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
 *
 */

#include "controller-ofswitch.h"
#include "vertex-switch.h"

namespace ns3 {
SwitchVertex::SwitchVertex(Ptr<OfSwitch>dev) :
  Vertex(dev->GetChassisId(), false),
  m_dev(dev)
{}

Ptr<OfSwitch>
SwitchVertex::GetSwitch() const
{
  return m_dev;
}

const Mac48Address&
SwitchVertex::GetChassisId() const
{
  return m_dev->GetChassisId();
}
} // namespace ns3
