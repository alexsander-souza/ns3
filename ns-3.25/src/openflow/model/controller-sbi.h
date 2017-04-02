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
 * Author: Blake Hurd  <naimorai@gmail.com>
 * Contributor: Andrey Blazejuk <andrey.blazejuk@inf.ufrgs.br>
 *         Alexsander de Souza <asouza@inf.ufrgs.br>
 */

#ifndef CONTROLLER_SBI_H
#define CONTROLLER_SBI_H

#include <set>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/ptr.h>
#include <ns3/application.h>
#include <ns3/openflow-lib.h>
#include <ns3/action-utils.h>

namespace ns3 {
class OpenflowHeader;
class FlowMatchHeader;
class OfSwitch;
class Socket;

/**
 * \brief A South-bound interface for a Controller
 *
 * Follows the OpenFlow specification for a controller.
 */
class ControllerSbi {
public:

  typedef std::list<Ptr<OfSwitch> >SwitchList;

  /**
   * \brief Constructor.
   */
  ControllerSbi();

  /**
   * \brief Destructor.
   */
  virtual
  ~ControllerSbi();

  /**
   * \param type  One of OFPT_*
   * \brief Create packet only with openflow header according to type. New XID
   * generated.
   */
  Ptr<OpenflowHeader>CreateDefault(uint8_t type);

  /**
   * \param flags          OFPC flags
   * \param length         Max flow length
   * \brief Create packet to configure a switch
   */
  Ptr<OpenflowHeader>CreateSetConfiguration(uint16_t flags,
                                            uint16_t length);

  /**
   * \param argsNumber  Number of arguments passed to this method
   * \param ...         Arguments must be in flow match header order: wildcards,
   * inPort, dlSrc...
   * \brief Create a flow match packet according to wildcard
   */
  Ptr<FlowMatchHeader>CreateFlowMatch(int argsNumber,
                                      ...);

  /**
   * \param msg     Pointer to struct to convert to ns3 header format
   * \brief Create packet to modify switch's flow
   */
  Ptr<OpenflowHeader>CreateFlowModification(ofp_flow_mod *msg);

  /**
   * \param matchHeader        Pointer to match header to set in this header
   * \param cookie             Opaque controller-issued identifier
   * \param command            One of OFPFC_*
   * \param idleTimeout        Idle time before discarding (seconds)
   * \param hardTimeout        Max time before discarding (seconds)
   * \param priority           Priority level of flow entry
   * \param bufferId           Buffered packet to apply to (or -1). Not
   * meaningful for OFPFC_DELETE*
   * \param outPort            For OFPFC_DELETE* commands, require matching
   * entries to include this as an
                               output port. A value of OFPP_ANY indicates no
   * restriction
   * \param flags              One of OFPFF_*
   * \param actionsList        List of actions to be set in this header
   * \brief Create packet to modify switch's flow
   */
  Ptr<OpenflowHeader>CreateFlowModification(Ptr<FlowMatchHeader>             matchHeader,
                                            uint64_t                         cookie,
                                            uint16_t                         command,
                                            uint16_t                         idleTimeout,
                                            uint16_t                         hardTimeout,
                                            uint16_t                         priority,
                                            uint32_t                         bufferId,
                                            uint16_t                         outPort,
                                            uint16_t                         flags,
                                            const action_utils::ActionsList& actionsList);

  /**
   * \param wildcards          Wildcard fields
   * \param inPort             Input switch port
   * \param dlSrc              Ethernet source address
   * \param dlDst              Ethernet destination address
   * \param dlVlan             Input VLAN
   * \param dlVlanPcp          Input VLAN priority
   * \param dlType             Ethernet frame type
   * \param nwTos              IP ToS (actually DSCP field, 6 bits)
   * \param nwProto            IP protocol or lower 8 bits of ARP opcode
   * \param nwSrc              IP source address
   * \param nwDst              IP destination address
   * \param tpSrc              TCP/UDP source port
   * \param tpDst              TCP/UDP destination port
   * \param cookie             Opaque controller-issued identifier
   * \param command            One of OFPFC_*
   * \param idleTimeout        Idle time before discarding (seconds)
   * \param hardTimeout        Max time before discarding (seconds)
   * \param priority           Priority level of flow entry
   * \param bufferId           Buffered packet to apply to (or -1). Not
   * meaningful for OFPFC_DELETE*
   * \param outPort            For OFPFC_DELETE* commands, require matching
   * entries to include this as an
                               output port. A value of OFPP_ANY indicates no
   * restriction
   * \param flags              One of OFPFF_*
   * \param actionsList        List of actions to be set in this header
   * \brief Create packet to modify switch's flow
   */
  Ptr<OpenflowHeader>CreateFlowModification(uint32_t                         wildcards,
                                            uint16_t                         inPort,
                                            const Mac48Address             & dlSrc,
                                            const Mac48Address             & dlDst,
                                            uint16_t                         dlVlan,
                                            uint8_t                          dlVlanPcp,
                                            uint16_t                         dlType,
                                            uint8_t                          nwTos,
                                            uint8_t                          nwProto,
                                            uint32_t                         nwSrc,
                                            uint32_t                         nwDst,
                                            uint16_t                         tpSrc,
                                            uint16_t                         tpDst,
                                            uint64_t                         cookie,
                                            uint16_t                         command,
                                            uint16_t                         idleTimeout,
                                            uint16_t                         hardTimeout,
                                            uint16_t                         priority,
                                            uint32_t                         bufferId,
                                            uint16_t                         outPort,
                                            uint16_t                         flags,
                                            const action_utils::ActionsList& actionsList);

  /**
   * \param bufferId           ID assigned by datapath
   * \param inPort             Packet’s input port (OFPP_NONE if none)
   * \param actionsLen         Size of action array in bytes
   * \param actionsList        List of actions to be set in this header
   * \param packet             Pointer to the packet being forwarded to the
   * controller
   * \brief Create packet out, used when controller wants to send a packet
   * through the datapath
   */
  Ptr<OpenflowHeader>CreatePacketOut(uint32_t                         bufferId,
                                     uint16_t                         inPort,
                                     uint16_t                         actionsLen,
                                     const action_utils::ActionsList& actionsList,
                                     Ptr<Packet>                      packet);

  /**
   * \param portNumber         Number of the port to be modified
   * \param hwAddr             The hardware address is not configurable. This is
   * used to
                               sanity-check the request, so it must be the same
   * as returned
                               in an ofp_phy_port struct
   * \param config             Bitmap of OFPPC_* flags
   * \param mask               Bitmap of OFPPC_* flags to be changed
   * \param advertise          Bitmap of "ofp_port_features"s. Zero all
                               bits to prevent any action taking place
   * \brief Create packet to modify switch's port
   */
  Ptr<OpenflowHeader>CreatePortModification(uint16_t portNumber,
                                            uint8_t  hwAddr[],
                                            uint32_t config,
                                            uint32_t mask,
                                            uint32_t advertise);

  /**
   * \param flags              OFPSF_REQ_* flags (none yet defined)
   * \param matchHeader        Pointer to match header to set in this header
   * \param tableId            ID of table to read (from ofp_table_stats), 0xff
   * for all
                               tables or 0xfe for emergency
   * \param outPort            Require matching entries to include this as an
   * output port.
                               A value of OFPP_NONE indicates no restriction
   * \brief Create packet to ask for information about individual flows
   */
  Ptr<OpenflowHeader>CreateFlowStatsRequest(uint16_t            flags,
                                            Ptr<FlowMatchHeader>matchHeader,
                                            uint8_t             tableId,
                                            uint16_t            outPort);

  /**
   * \param flags              OFPSF_REQ_* flags (none yet defined)
   * \param wildcards          Wildcard fields
   * \param inPort             Input switch port
   * \param dlSrc              Ethernet source address
   * \param dlDst              Ethernet destination address
   * \param dlVlan             Input VLAN
   * \param dlVlanPcp          Input VLAN priority
   * \param dlType             Ethernet frame type
   * \param nwTos              IP ToS (actually DSCP field, 6 bits)
   * \param nwProto            IP protocol or lower 8 bits of ARP opcode
   * \param nwSrc              IP source address
   * \param nwDst              IP destination address
   * \param tpSrc              TCP/UDP source port
   * \param tpDst              TCP/UDP destination port
   * \param tableId            ID of table to read (from ofp_table_stats), 0xff
   * for all
                               tables or 0xfe for emergency
   * \param outPort            Require matching entries to include this as an
   * output port.
                               A value of OFPP_NONE indicates no restriction
   * \brief Create packet to ask for information about individual flows
   */
  Ptr<OpenflowHeader>CreateFlowStatsRequest(uint16_t            flags,
                                            uint32_t            wildcards,
                                            uint16_t            inPort,
                                            const Mac48Address& dlSrc,
                                            const Mac48Address& dlDst,
                                            uint16_t            dlVlan,
                                            uint8_t             dlVlanPcp,
                                            uint16_t            dlType,
                                            uint8_t             nwTos,
                                            uint8_t             nwProto,
                                            uint32_t            nwSrc,
                                            uint32_t            nwDst,
                                            uint16_t            tpSrc,
                                            uint16_t            tpDst,
                                            uint8_t             tableId,
                                            uint16_t            outPort);

  /**
   * \param flags              OFPSF_REQ_* flags (none yet defined)
   * \param portNumber         OFPST_PORT message must request statistics either
   * for a
                               single port (specified in port_no) or for all
   * ports (if
                               port_no == OFPP_NONE)
   * \brief Create packet to ask for information about physical ports
   */
  Ptr<OpenflowHeader>CreatePortStatsRequest(uint16_t flags,
                                            uint16_t portNumber);

  /**
   * \param openflowHeader    A pointer to the openflow header ready to be sent
   * \param target            Target OF switch
   * \brief Send Openflow message to switch
   */
  void SendOpenflowMessage(Ptr<OfSwitch>      target,
                           Ptr<OpenflowHeader>openflowHeader,
                           bool               updateXid = false);

  SwitchList GetSwitchList();

  Ptr<OfSwitch>GetSwitch(const Mac48Address& chassisId);

  virtual void ForwardToApps(Ptr<OfSwitch>origin,
                             unsigned     portIn,
                             unsigned     bufferId,
                             Ptr<Packet>  packet) = 0;

  virtual void NewSwitch(Ptr<OfSwitch>origin) = 0;

  virtual void SwitchSyncCompleted(Ptr<OfSwitch>origin, int stateId) = 0;

protected:

  void InitializeNetwork(Ptr<Node>          node,
                         const Ipv4Address& myAddress);

private:

  void AcceptHandler(Ptr<Socket>    socket,
                     const Address& from);

  SwitchList  m_swList;
  Ptr<Socket> m_socketListener;
};
} // namespace ns3
#endif  /* CONTROLLER_SBI_H */
