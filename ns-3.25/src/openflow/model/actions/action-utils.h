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
 *
 */

#ifndef ACTION_UTILS_H
#define ACTION_UTILS_H

#include <ns3/ptr.h>
#include <list>
#include <stdint.h>

namespace ns3
{
class ActionHeader;
class Mac48Address;
class Ipv4Address;

namespace action_utils
{
typedef std::list<Ptr<ActionHeader> > ActionsList;

/**
 * \param port             Output port
 * \param maxLen           Max length to send to controller
 * \return action          The action header with output action header aggregated.
 * \brief Create output action, which sends packets out ’port’.
 */
Ptr<ActionHeader> CreateOutputAction(uint16_t port, uint16_t maxLen);

/**
 * \param actionsList      ActionsList to be added to the message, an output action header will be
 *                         added to the end of this list.
 * \param port             Output port
 * \param maxLen           Max length to send to controller
 * \brief Create output action, which sends packets out ’port’.
 */
void CreateOutputAction(ActionsList* actionsList, uint16_t port, uint16_t maxLen);

/**
 * \param vlanVid          VLAN id
 * \return action          The action header with vlan vid action header aggregated.
 * \brief Create an set VLAN VID action.
 */
Ptr<ActionHeader> CreateSetVlanVidAction(uint16_t vlanVid);

/**
 * \param actionsList      ActionsList to be added to the message, a vlan vid action header will be
 *                         added to the end of this list.
 * \param vlanVid          VLAN id
 * \brief Create an set VLAN VID action.
 */
void CreateSetVlanVidAction(ActionsList* actionsList, uint16_t vlanVid);

/**
 * \param vlanPcp          VLAN PCP
 * \return action          The action header with vlan pcp action header aggregated.
 * \brief Create an set VLAN PCP action.
 */
Ptr<ActionHeader> CreateSetVlanPcpAction(uint8_t vlanPcp);

/**
 * \param actionsList      ActionsList to be added to the message, a vlan pcp action header will be
 *                         added to the end of this list.
 * \param vlanPcp          VLAN PCP
 * \brief Create an set VLAN PCP action.
 */
void CreateSetVlanPcpAction(ActionsList* actionsList, uint8_t vlanPcp);

/**
 * \return action          The action header with strip vlan action header aggregated.
 * \brief Create an strip VLAN action.
 */
Ptr<ActionHeader> CreateStripVlanAction(void);

/**
 * \param actionsList      ActionsList to be added to the message, a strip vlan action header will be
 *                         added to the end of this list.
 * \brief Create an strip VLAN action.
 */
void CreateStripVlanAction(ActionsList* actionsList);

/**
 * \param dlAddress        Ethernet address of type Mac48Address
 * \return action          The action header with dl address action header aggregated.
 * \brief Create an set DL source action.
 */
Ptr<ActionHeader> CreateSetDlSrcAction(const Mac48Address &dlAddress);

/**
 * \param dlAddress        Ethernet address of type uint8_t*
 * \return action          The action header with dl address action header aggregated.
 * \brief Create an set DL source action.
 */
Ptr<ActionHeader> CreateSetDlSrcAction(uint8_t dlAddress[]);

/**
 * \param actionsList      ActionsList to be added to the message, a dl address action header will be
 *                         added to the end of this list.
 * \param dlAddress        Ethernet address of type Mac48Address
 * \brief Create an set DL source action.
 */
void CreateSetDlSrcAction(ActionsList* actionsList, const Mac48Address &dlAddress);

/**
 * \param dlAddress        Ethernet address of type uint8_t*
 * \param actionsList      ActionsList to be added to the message, a dl address action header will be
 *                         added to the end of this list.
 * \brief Create an set DL source action.
 */
void CreateSetDlSrcAction(ActionsList* actionsList, uint8_t dlAddress[]);

/**
 * \param dlAddress        Ethernet address of type Mac48Address
 * \return action          The action header with dl address action header aggregated.
 * \brief Create an set DL destination action.
 */
Ptr<ActionHeader> CreateSetDlDstAction(const Mac48Address &dlAddress);

/**
 * \param dlAddress        Ethernet address of type uint8_t*
 * \return action          The action header with dl address action header aggregated.
 * \brief Create an set DL destination action.
 */
Ptr<ActionHeader> CreateSetDlDstAction(uint8_t dlAddress[]);

/**
 * \param actionsList      ActionsList to be added to the message, a dl address action header will be
 *                         added to the end of this list.
 * \param dlAddress        Ethernet address of type Mac48Address
 * \brief Create an set DL destination action.
 */
void CreateSetDlDstAction(ActionsList* actionsList, const Mac48Address &dlAddress);

/**
 * \param actionsList      ActionsList to be added to the message, a dl address action header will be
 *                         added to the end of this list.
 * \param dlAddress        Ethernet address of type uint8_t*
 * \brief Create an set DL destination action.
 */
void CreateSetDlDstAction(ActionsList* actionsList, uint8_t dlAddress[]);

/**
 * \param nwAddress        Network address of type Ipv4Address
 * \return action          The action header with nw address action header aggregated.
 * \brief Create an set NW source action.
 */
Ptr<ActionHeader> CreateSetNwSrcAction(const Ipv4Address& nwAddress);

/**
 * \param nwAddress        Network address of type uint32_t
 * \return action          The action header with nw address action header aggregated.
 * \brief Create an set NW source action.
 */
Ptr<ActionHeader> CreateSetNwSrcAction(uint32_t nwAddress);

/**
 * \param actionsList      ActionsList to be added to the message, a nw address action header will be
 *                         added to the end of this list.
 * \param nwAddress        Network address of type Ipv4Address
 * \brief Create an set NW source action.
 */
void CreateSetNwSrcAction(ActionsList* actionsList, const Ipv4Address& nwAddress);

/**
 * \param actionsList      ActionsList to be added to the message, a nw address action header will be
 *                         added to the end of this list.
 * \param nwAddress        Network address of type uint32_t
 * \brief Create an set NW source action.
 */
void CreateSetNwSrcAction(ActionsList* actionsList, uint32_t nwAddress);

/**
 * \param nwAddress        Network address of type Ipv4Address
 * \return action          The action header with nw address action header aggregated.
 * \brief Create an set NW destination action.
 */
Ptr<ActionHeader> CreateSetNwDstAction(const Ipv4Address& nwAddress);

/**
 * \param nwAddress        Network address of type uint32_t
 * \return action          The action header with nw address action header aggregated.
 * \brief Create an set NW destination action.
 */
Ptr<ActionHeader> CreateSetNwDstAction(uint32_t nwAddress);

/**
 * \param actionsList      ActionsList to be added to the message, a nw address action header will be
 *                         added to the end of this list.
 * \param nwAddress        Network address of type Ipv4Address
 * \brief Create an set NW destination action.
 */
void CreateSetNwDstAction(ActionsList* actionsList, const Ipv4Address& nwAddress);

/**
 * \param actionsList      ActionsList to be added to the message, a nw address action header will be
 *                         added to the end of this list.
 * \param nwAddress        Network address of type uint32_t
 * \brief Create an set NW destination action.
 */
void CreateSetNwDstAction(ActionsList* actionsList, uint32_t nwAddress);

/**
 * \param tpPort           TCP/UDP port
 * \return action          The action header with tp port action header aggregated.
 * \brief Create an set TCP/UDP source port action.
 */
Ptr<ActionHeader> CreateSetTpSrcAction(uint16_t tpPort);

/**
 * \param actionsList      ActionsList to be added to the message, a tp port action header will be
 *                         added to the end of this list.
 * \param tpPort           TCP/UDP port
 * \brief Create an set TCP/UDP source port action.
 */
void CreateSetTpSrcAction(ActionsList* actionsList, uint16_t tpPort);

/**
 * \param tpPort           TCP/UDP port
 * \return action          The action header with tp port action header aggregated.
 * \brief Create an set TCP/UDP destination port action.
 */
Ptr<ActionHeader> CreateSetTpDstAction(uint16_t tpPort);

/**
 * \param actionsList      ActionsList to be added to the message, a tp port action header will be
 *                         added to the end of this list.
 * \param tpPort           TCP/UDP port
 * \brief Create an set TCP/UDP destination port action.
 */
void CreateSetTpDstAction(ActionsList* actionsList, uint16_t tpPort);

} // namespace action_utils
} // namespace ns3
#endif  /* ACTION_UTILS_H */
