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

#include <ns3/action-header.h>
#include <ns3/action-utils.h>
#include <ns3/log.h>

namespace ns3
{
namespace action_utils
{
NS_LOG_COMPONENT_DEFINE("ActionUtils");

Ptr<ActionHeader> CreateOutputAction(uint16_t port, uint16_t maxLen)
{
  NS_LOG_INFO("Creating output action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_OUTPUT, sizeof(ofp_action_output));
  Ptr<ActionOutputHeader> outputAHeader = Create<ActionOutputHeader>(port, maxLen);

  actionHeader->SetOutput(outputAHeader);

  return actionHeader;
}

void CreateOutputAction(ActionsList* actionsList, uint16_t port, uint16_t maxLen)
{
  actionsList->push_back(CreateOutputAction(port, maxLen));
}

Ptr<ActionHeader> CreateSetVlanVidAction(uint16_t vlanVid)
{
  NS_LOG_INFO("Creating set VLAN VID action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_VLAN_VID, sizeof(ofp_action_vlan_vid));
  Ptr<ActionVlanVidHeader> vlanVidAHeader = Create<ActionVlanVidHeader>(vlanVid);

  actionHeader->SetVlanVid(vlanVidAHeader);

  return actionHeader;
}

void CreateSetVlanVidAction(ActionsList* actionsList, uint16_t vlanVid)
{
  actionsList->push_back(CreateSetVlanVidAction(vlanVid));
}

Ptr<ActionHeader> CreateSetVlanPcpAction(uint8_t vlanPcp)
{
  NS_LOG_INFO("Creating set VLAN PCP action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_VLAN_PCP, sizeof(ofp_action_vlan_pcp));
  Ptr<ActionVlanPcpHeader> vlanPcpAHeader = Create<ActionVlanPcpHeader>(vlanPcp);

  actionHeader->SetVlanPcp(vlanPcpAHeader);

  return actionHeader;
}

void CreateSetVlanPcpAction(ActionsList* actionsList, uint8_t vlanPcp)
{
  actionsList->push_back(CreateSetVlanPcpAction(vlanPcp));
}

Ptr<ActionHeader> CreateStripVlanAction()
{
  NS_LOG_INFO("Creating strip VLAN action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_STRIP_VLAN, sizeof(ofp_action_vlan_vid));

  return actionHeader;
}

void CreateStripVlanAction(ActionsList* actionsList)
{
  actionsList->push_back(CreateStripVlanAction());
}

Ptr<ActionHeader> CreateSetDlSrcAction(const Mac48Address &dlAddress)
{
  NS_LOG_INFO("Creating set DL source action");

  uint8_t srcAddress[OFP_ETH_ALEN];
  dlAddress.CopyTo(srcAddress);

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_DL_SRC, sizeof(ofp_action_dl_addr));
  Ptr<ActionDlAddressHeader> dlAHeader = Create<ActionDlAddressHeader>(srcAddress);

  actionHeader->SetDlSrcAddress(dlAHeader);

  return actionHeader;
}

Ptr<ActionHeader> CreateSetDlSrcAction(uint8_t dlAddress[])
{
  NS_LOG_INFO("Creating set DL source action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_DL_SRC, sizeof(ofp_action_dl_addr));
  Ptr<ActionDlAddressHeader> dlAHeader = Create<ActionDlAddressHeader>(dlAddress);

  actionHeader->SetDlSrcAddress(dlAHeader);

  return actionHeader;
}

void CreateSetDlSrcAction(ActionsList* actionsList, const Mac48Address &dlAddress)
{
  actionsList->push_back(CreateSetDlSrcAction(dlAddress));
}

void CreateSetDlSrcAction(ActionsList* actionsList, uint8_t dlAddress[])
{
  actionsList->push_back(CreateSetDlSrcAction(dlAddress));
}

Ptr<ActionHeader> CreateSetDlDstAction(const Mac48Address &dlAddress)
{
  NS_LOG_INFO("Creating set DL destination action");

  uint8_t dstAddress[OFP_ETH_ALEN];
  dlAddress.CopyTo(dstAddress);

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_DL_DST, sizeof(ofp_action_dl_addr));
  Ptr<ActionDlAddressHeader> dlAHeader = Create<ActionDlAddressHeader>(dstAddress);

  actionHeader->SetDlDstAddress(dlAHeader);

  return actionHeader;
}

Ptr<ActionHeader> CreateSetDlDstAction(uint8_t dlAddress[])
{
  NS_LOG_INFO("Creating set DL destination action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_DL_DST, sizeof(ofp_action_dl_addr));
  Ptr<ActionDlAddressHeader> dlAHeader = Create<ActionDlAddressHeader>(dlAddress);

  actionHeader->SetDlDstAddress(dlAHeader);

  return actionHeader;
}

void CreateSetDlDstAction(ActionsList* actionsList, const Mac48Address &dlAddress)
{
  actionsList->push_back(CreateSetDlDstAction(dlAddress));
}

void CreateSetDlDstAction(ActionsList* actionsList, uint8_t dlAddress[])
{
  actionsList->push_back(CreateSetDlDstAction(dlAddress));
}

Ptr<ActionHeader> CreateSetNwSrcAction(const Ipv4Address &nwAddress)
{
  NS_LOG_INFO("Creating set NW source action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_NW_SRC, sizeof(ofp_action_nw_addr));
  Ptr<ActionNwAddressHeader> nwAHeader = Create<ActionNwAddressHeader>(nwAddress.Get());

  actionHeader->SetNwSrcAddress(nwAHeader);

  return actionHeader;
}

Ptr<ActionHeader> CreateSetNwSrcAction(uint32_t nwAddress)
{
  NS_LOG_INFO("Creating set NW source action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_NW_SRC, sizeof(ofp_action_nw_addr));
  Ptr<ActionNwAddressHeader> nwAHeader = Create<ActionNwAddressHeader>(nwAddress);

  actionHeader->SetNwSrcAddress(nwAHeader);

  return actionHeader;
}

void CreateSetNwSrcAction(ActionsList* actionsList, const Ipv4Address &nwAddress)
{
  actionsList->push_back(CreateSetNwSrcAction(nwAddress));
}

void CreateSetNwSrcAction(ActionsList* actionsList, uint32_t nwAddress)
{
  actionsList->push_back(CreateSetNwSrcAction(nwAddress));
}

Ptr<ActionHeader> CreateSetNwDstAction(const Ipv4Address &nwAddress)
{
  NS_LOG_INFO("Creating set NW destination action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_NW_DST, sizeof(ofp_action_nw_addr));
  Ptr<ActionNwAddressHeader> nwAHeader = Create<ActionNwAddressHeader>(nwAddress.Get());

  actionHeader->SetNwDstAddress(nwAHeader);

  return actionHeader;
}

Ptr<ActionHeader> CreateSetNwDstAction(uint32_t nwAddress)
{
  NS_LOG_INFO("Creating set NW destination action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_NW_DST, sizeof(ofp_action_nw_addr));
  Ptr<ActionNwAddressHeader> nwAHeader = Create<ActionNwAddressHeader>(nwAddress);

  actionHeader->SetNwDstAddress(nwAHeader);

  return actionHeader;
}

void CreateSetNwDstAction(ActionsList* actionsList, const Ipv4Address &nwAddress)
{
  actionsList->push_back(CreateSetNwDstAction(nwAddress));
}

void CreateSetNwDstAction(ActionsList* actionsList, uint32_t nwAddress)
{
  actionsList->push_back(CreateSetNwDstAction(nwAddress));
}

Ptr<ActionHeader> CreateSetTpSrcAction(uint16_t tpPort)
{
  NS_LOG_INFO("Creating set TCP/UDP source port action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_TP_SRC, sizeof(ofp_action_tp_port));
  Ptr<ActionTpPortHeader> tpAHeader = Create<ActionTpPortHeader>(tpPort);

  actionHeader->SetTpSrc(tpAHeader);

  return actionHeader;
}

void CreateSetTpSrcAction(ActionsList* actionsList, uint16_t tpPort)
{
  actionsList->push_back(CreateSetTpSrcAction(tpPort));
}

Ptr<ActionHeader> CreateSetTpDstAction(uint16_t tpPort)
{
  NS_LOG_INFO("Creating set TCP/UDP destination port action");

  Ptr<ActionHeader> actionHeader = Create<ActionHeader>(OFPAT_SET_TP_DST, sizeof(ofp_action_tp_port));
  Ptr<ActionTpPortHeader> tpAHeader = Create<ActionTpPortHeader>(tpPort);

  actionHeader->SetTpDst(tpAHeader);

  return actionHeader;
}

void CreateSetTpDstAction(ActionsList* actionsList, uint16_t tpPort)
{
  actionsList->push_back(CreateSetTpDstAction(tpPort));
}

} // namespace action-utils
} // namespace ns3
