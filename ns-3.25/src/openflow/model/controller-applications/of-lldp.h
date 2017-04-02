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

#ifndef LLDP_HANDLER_H
#define LLDP_HANDLER_H

#include <ns3/event-id.h>
#include <ns3/mac48-address.h>
#include <ns3/controller-application.h>

namespace ns3 {
class LldpHandler : public ControllerApplication {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  LldpHandler();

  /**
   * \brief Destructor.
   */
  ~LldpHandler();

  virtual bool ReceiveFromSwitch(Ptr<OfSwitch>origin,
                                 unsigned     portIn,
                                 unsigned     bufferId,
                                 Ptr<Packet>  packet);

  virtual void InitSwitch(Ptr<OfSwitch>origin);

  virtual void InitMe();

protected:

  virtual int GetPriority() const;

private:

  void RefreshTopology();

  Mac48Address m_address;
  EventId m_topoRefreshEvent;
  Time    m_refreshInterval;
};
} // namespace ns3
#endif  /* LLDP_HANDLER_H */
