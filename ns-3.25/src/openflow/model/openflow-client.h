/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2014 UFRGS
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

#ifndef OPENFLOW_CLIENT_H
#define OPENFLOW_CLIENT_H

#include "ns3/application.h"
#include "ns3/mac48-address.h"
#include "ns3/openflow-lib.h"
#include "ns3/action-utils.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/switch-features-header.h"

namespace ns3 {
class Socket;
class Packet;
class OpenflowHeader;
class SwitchConfigurationHeader;
class FlowModificationHeader;
class FlowStatsRequestHeader;
class PortStatsRequestHeader;
class ActionOutputHeader;
class ActionVlanPcpHeader;
class ActionVlanVidHeader;
class ActionDlAddressHeader;
class ActionNwAddressHeader;
class ActionNwTosHeader;
class ActionTpPortHeader;
class ActionEnqueueHeader;
class ActionVendorHeader;

/**
 * \ingroup openflow
 * \class OpenflowClient
 * \brief A openflow client (Switch). It learns connects to Controller, creates
 * and sends packet-in
 * messages.
 *
 */
class OpenflowClient : public Application {
public:

  /**
   * \brief Get the type ID.
   * \return type ID
   */
  static TypeId GetTypeId(void);

  /**
   * \brief Constructor.
   */
  OpenflowClient();

  /**
   * \brief Destructor.
   */
  virtual
  ~OpenflowClient();

  /**
   * \param os    Output to print in
   * \brief Print the Node Id of the client.
   */
  virtual void Print(std::ostream& os) const;

  /**
   * \param type  One of OFPT_*
   * \brief Create packet only with openflow header according to type
   */
  void SendBasicMessage(uint8_t type);


  /**
   * \brief Create packet informing switch configuration
   * \param xid             The same xid of the get configuration request
   * received
   * \param flags           OFPC_* flags
   * \param maxFlowLen      Max bytes of new flow that datapath should send to
   * the controller
   */
  void SendConfigurationReply(uint32_t xid,
                              uint16_t flags,
                              uint16_t maxFlowLen);

  /**
   * \brief Create packet informing switch features
   * \param portsNum      Number of ports
   * \param xid           The same xid of the features request received
   * \param datapathId    Datapath unique ID.
   * \param nTables       Number of tables supported by datapath.
   * \param ports         Struct with info related to all ports of this switch
   */
  void SendFeaturesReply(long unsigned int                portsNum,
                         uint32_t                         xid,
                         uint64_t                         datapathId,
                         uint8_t                          nTables,
                         uint32_t                         nBuffers,
                         OpenFlowSwitchNetDevice::Ports_t ports);

  /**
   * \brief Create packet in, forwarding a packet received on switch to the
   * controller
   */
  void SendPacketIn(ofpbuf *packetInBuffer);

  /*
   * \param buffer Openflow buffer to send
   * \brief Receive openflow buffer to put it in NS3 header format and send
   *        it to controller
   */
  void ReceiveFromNetDevice(ofpbuf *buffer);

protected:

  /**
   * \brief Dispose the instance.
   */
  virtual void DoDispose(void);

private:

  /**
   * \brief Start the application.
   */
  virtual void StartApplication(void);

  /**
   * \brief Stop the application.
   */
  virtual void StopApplication(void);

  /**
   * \brief Fills ports list in header format with the right info about the
   * ports using
   * the net device struct maintained by OpenflowSwitchNetDevice
   * \param portsList     Pointer to ports list of PhysicalPort headers
   * \param ports         Struct with info related to all ports of this switch
   */
  void CreatePortsList(SwitchFeaturesHeader::PhysicalPortsList *portsList,
                       OpenFlowSwitchNetDevice::Ports_t         ports);

  /**
   * \param OFHeader    A pointer to the OpenflowHeader to send
   * \brief Send openflow message to controller
   */
  void SendOpenflowMessage(Ptr<OpenflowHeader>OFHeader);

  /**
   * \param  header       A pointer to the OpenflowHeader to convert
   *                      to ofp_match
   * \return ofp_match*   Pointer to the struct created with the parameters of
   *                      header set.
   * \brief  Convert a match header in NS3 format to struct ofp_match
   */
  ofp_match ActionHeadersToStruct(action_utils::ActionsList actionsList);

  /**
   * \param xid         Transaction ID received from hello
   * \brief Handle a hello message received
   */
  void SendHello(uint32_t xid);

  /**
   * \param xid         Transaction ID received from barrier
   * \brief Handle a barrier message received
   */
  void SendBarrier(uint32_t xid);

  /**
   * \param ofh         Pointer to openflow header of type set configuration
   *                    in struct format
   * \param header      Pointer to switch configuration header
   * \brief Handle a switch set configuration message received
   */
  void HandleSetConfiguration(ofp_header                   *ofh,
                              Ptr<SwitchConfigurationHeader>header);

  /**
   * \param ofh         Pointer to openflow header of type flow modification
   *                    in struct format
   * \param header      Pointer to flow modification header
   * \brief Handle a flow modification message received
   */
  void HandleFlowModification(ofp_header                *ofh,
                              Ptr<FlowModificationHeader>header);

  /**
   * \param ofh         Pointer to openflow header of type flow modification
   *                    in struct format
   * \param header      Pointer to packet out header
   * \brief Handle a packet out message received
   */
  void HandlePacketOut(ofp_header         *ofh,
                       Ptr<PacketOutHeader>header);

  /**
   * \param header      Port modification header
   * \brief Handle a port modification message received
   */
  void HandlePortModification(Ptr<PortModificationHeader>header);

  /**
   * \param xid         Transaction ID received from features request
   * \param header      Stats request header
   * \brief Handle a stats request message received
   */
  void HandleStatsRequest(uint32_t               xid,
                          Ptr<StatsRequestHeader>header);

  /**
   * \param header      Flow stats request header
   * \brief Handle a flow stats request message received
   */
  void HandleFlowStatsRequest(Ptr<FlowStatsRequestHeader>header);

  /**
   * \param header      Port stats request header
   * \brief Handle a port stats request message received
   */
  void HandlePortStatsRequest(Ptr<PortStatsRequestHeader>header);

  /**
   * \param socket      Pointer to the socket of TCP connection
   * \brief Receive packets
   */
  void OpenflowHandler(Ptr<Socket>socket);

  uint32_t m_xid;                            /**< Current transaction ID */
  Ptr<OpenFlowSwitchNetDevice> m_ofSwNetDev; /**< Openflow switch net device*/
  Ptr<Socket>  m_socket;                     /**< AF_INET TCP socket */
  Ipv4Address  m_ctrlAddress;                /**< Controller IP address */
  Ipv4Address  m_myAddress;                  /**< IP address given to client */
  Mac48Address m_myMacAddress;               /**< local interface MAC address */
  Ptr<Packet>  m_fragment;
};
} // namespace ns3
#endif /* OPENFLOW_CLIENT_H */
