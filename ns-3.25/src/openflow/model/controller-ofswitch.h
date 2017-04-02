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
 * Author: Andrey Blazejuk  <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 *
 */

#ifndef CONTROLLER_OFSWITCH_H
#define CONTROLLER_OFSWITCH_H

#include <map>
#include <ns3/ptr.h>
#include <ns3/simple-ref-count.h>
#include <ns3/inet-socket-address.h>
#include <ns3/ipv4-address.h>
#include <ns3/mac48-address.h>
#include <ns3/action-utils.h>

namespace ns3 {
class ActionHeader;
class ActionDlAddressHeader;
class ActionEnqueueHeader;
class ActionNwAddressHeader;
class ActionNwTosHeader;
class ActionOutputHeader;
class ActionTpPortHeader;
class ActionVendorHeader;
class ActionVlanVidHeader;
class ActionVlanPcpHeader;
class ControllerApplication;
class ErrorMsgHeader;
class FlowMatchHeader;
class FlowRemovedHeader;
class FlowStatsHeader;
class PacketInHeader;
class PortStatsHeader;
class PortStatsRequestHeader;
class PortStatusHeader;
class StatsReplyHeader;
class SwitchConfigurationHeader;
class SwitchFeaturesHeader;
class Packet;
class Socket;
class ControllerSbi;

/**
 * \class OfSwitch
 * \brief An abstraction of a single openflow switch from the controller
 * perspective.
 *
 */
class OfSwitch : public SimpleRefCount<OfSwitch>{
public:

  /**
   * \brief Constructor.
   */
  OfSwitch(ControllerSbi *control,
           Ptr<Socket>    socket,
           const Address& address);

  /**
   * \brief Destructor.
   */
  virtual ~OfSwitch();

  /**
   * \brief Start protocol state machine
   */
  void Initialize();

  /**
   * \brief Send OpenFlow message to this Switch
   * \param packet   OpenFlow 1.0 message
   */
  void SendMessage(Ptr<Packet>packet);

  void UpdateXid(uint32_t xid);

  uint32_t GetLastXid() const;

  void SyncSwitch(int stateId);

  const Mac48Address& GetChassisId() const;

  int GetNPorts() const;

  const Mac48Address& GetPortAddress(int port) const;

  bool IsFloodPort(int port) const;

  void SetFloodPort(int  port,
                    bool enabled);

  friend bool operator==(const OfSwitch   & ofswitch,
                         const Ipv4Address& addr);

private:

  /**
   * \param xid         Transaction ID received from hello
   * \brief Handle a hello message received
   */
  void HandleHello(uint32_t xid);

  /**
   * \param xid         Transaction ID received
   * \brief Handle a Barrier message received
   */
  void HandleBarrier(uint32_t xid);

  /**
   * \param header      Error message header
   * \brief Handle a error message received
   */
  void HandleErrorMsg(Ptr<ErrorMsgHeader>header);

  /**
   * \param xid         Transaction ID received from features reply
   * \param header      Switch features header
   * \brief Handle a features reply message received
   */
  void HandleFeaturesReply(uint32_t                 xid,
                           Ptr<SwitchFeaturesHeader>header);

  /**
   * \param xid         Transaction ID received from get configuration reply
   * \param header      Switch configuration header
   * \brief Handle a get configuration reply message received
   */
  void HandleConfigurationReply(uint32_t                      xid,
                                Ptr<SwitchConfigurationHeader>header);

  /**
   * \param header      Packet in header
   * \brief Handle a packet in message received
   */
  void HandlePacketIn(Ptr<PacketInHeader>header);

  /**
   * \param header      Flow removed header
   * \brief Handle a flow removed message received
   */
  void HandleFlowRemoved(Ptr<FlowRemovedHeader>header);

  /**
   * \param header      Port status header
   * \brief Handle a port status message received
   */
  void HandlePortStatus(Ptr<PortStatusHeader>header);

  /**
   * \param xid         Transaction ID received from features reply
   * \param header      Stats reply header
   * \brief Handle a stats reply message received and calls the appropriate
   * handler according to type
   */
  void HandleStatsReply(uint32_t             xid,
                        Ptr<StatsReplyHeader>header);

  /**
   * \param header      Flow stats header
   * \brief Handle a flow stats reply message received
   */
  void HandleFlowStatsReply(Ptr<FlowStatsHeader>header);

  /**
   * \param header      Port stats header
   * \brief Handle a port stats reply message received
   */
  void HandlePortStatsReply(Ptr<PortStatsHeader>header);

  void OpenflowHandler(Ptr<Socket>socket);

  Ptr<Socket> m_socket;
  Address     m_address;
  Mac48Address   m_chassisId;
  uint32_t       m_xid;
  ControllerSbi *m_control;
  Ptr<Packet>    m_fragment;
  std::map<int, Mac48Address> m_port_map;
  std::map<int, bool> m_flood_map;
  std::map<uint32_t, int> m_barrier_map;
};
} // namespace ns3
#endif  /* CONTROLLER_OFSWITCH_H */
