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
 * Author: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#ifndef OF_ARP_H
#define OF_ARP_H

#include <map>
#include <ns3/ipv4-address.h>
#include <ns3/mac48-address.h>
#include <ns3/controller-application.h>

namespace ns3 {
class ArpHeader;
class OpenflowHeader;

class ArpHandler : public ControllerApplication {
public:

  struct Request {
    Request(Ptr<OfSwitch>       origin,
            unsigned            portIn,
            const Mac48Address& senderHwAddr,
            const Ipv4Address & senderAddr);

    friend bool operator==(const Request& req1,
                           const Request& req2);
    friend bool operator<(const Request& req1,
                          const Request& req2);

    Ptr<OfSwitch>m_origin;
    unsigned     m_portIn;
    Time         m_startTime;
    Mac48Address m_senderHwAddr;
    Ipv4Address  m_senderAddr;
  };

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  ArpHandler();

  /**
   * \brief Destructor.
   */
  ~ArpHandler();

  /**
   * \brief When the controller receives a Packet In message, this method is
   *        called to forward the packet and the address of the switch that
   *        sent this packet to this application.
   */
  virtual bool ReceiveFromSwitch(Ptr<OfSwitch>origin,
                                 unsigned     portIn,
                                 unsigned     bufferId,
                                 Ptr<Packet>  packet);

  virtual void InitSwitch(Ptr<OfSwitch>origin);

protected:

  virtual int GetPriority() const;

private:

  void FloodRequest(Ptr<OfSwitch>origin,
                    unsigned     portIn,
                    Ptr<Packet>  packet);


  /**
   * \brief Save relevant fields in ARP request, so that ARP reply can be
   * created.
   * \param arpHeader      Header of arp request message received
   * \returns true if the request was handled, false when it must be flooded
   */
  bool HandleArpRequest(Ptr<OfSwitch>origin,
                        unsigned     portIn,
                        ArpHeader  & arpHeader);

  /**
   * \brief Parse ARP response and updates the HostDB
   * \param arpHeader      Header of arp reply message received
   * \returns true if the request was handled, false when it must be flooded
   */
  bool HandleArpReply(Ptr<OfSwitch>origin,
                      unsigned     portIn,
                      ArpHeader  & arpHeader);

  void SendReply(const Ipv4Address & targetAddr,
                 const Mac48Address& targetHwAddress,
                 const Request     & request);

  std::multimap<Ipv4Address, Request> m_pending;
};
} // namespace ns3
#endif  /* OF_ARP_H */
