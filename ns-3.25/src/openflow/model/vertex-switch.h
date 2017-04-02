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

#ifndef OPENFLOW_VERTEX_SWITCH_H
#define OPENFLOW_VERTEX_SWITCH_H

#include <ns3/ptr.h>
#include <ns3/mac48-address.h>
#include <ns3/vertex.h>

namespace ns3 {
class OfSwitch;

/**
 * \brief OF-NIB OF-Switch data
 */
class SwitchVertex : public Vertex {
public:

  /**
   * \brief Constructor
   */
  SwitchVertex(Ptr<OfSwitch>dev);

  Ptr<OfSwitch>GetSwitch() const;

  const Mac48Address& GetChassisId() const;

private:

  Ptr<OfSwitch> m_dev;
};
} // namespace ns3
#endif /* OPENFLOW_VERTEX_SWITCH_H */
