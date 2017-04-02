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
 *
 */
#ifdef NS3_OPENFLOW

#include <ns3/abort.h>
#include <ns3/log.h>
#include "ns3/packet-socket-address.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/nstime.h"
#include "ns3/socket-factory.h"
#include "ns3/arp-header.h"
#include "ns3/string.h"
#include "ns3/ipv4.h"
#include "headers/openflow-header.h"
#include "headers/error-msg-header.h"
#include "headers/switch-features-header.h"
#include "headers/switch-configuration-header.h"
#include "headers/flow-removed-header.h"
#include "headers/port-status-header.h"
#include "headers/stats-reply-header.h"
#include "actions/action-header.h"
#include "controller-applications/controller-application.h"
// #include "controller-applications/of-dhcp.h"
#include "openflow-lib.h"
#include "vertex-host.h"
#include "vertex-switch.h"
#include "vertex-server.h"
#include "controller-ofswitch.h"
#include "controller.h"

NS_LOG_COMPONENT_DEFINE("Controller");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(Controller);

TypeId
Controller::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::Controller")
                      .SetParent<Application>()
                      .AddConstructor<Controller>()
                      .AddAttribute("LocalAddress",
                                    "Local IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&Controller::m_myAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("InbandAddress",
                                    "Inband mgmt IPv4 address",
                                    Ipv4AddressValue(),
                                    MakeIpv4AddressAccessor(&Controller::m_inbandAddress),
                                    MakeIpv4AddressChecker())
                      .AddAttribute("DftAccessProfile",
                                    "Default access profile name",
                                    StringValue("InternetProf"),
                                    MakeStringAccessor(&Controller::m_dftAccessProfile),
                                    MakeStringChecker())
                      .AddAttribute("DftMcastProfile",
                                    "Default multicast profile name",
                                    StringValue("McastProf"),
                                    MakeStringAccessor(&Controller::m_dftMcastProfile),
                                    MakeStringChecker())
  ;

  return tid;
}

Controller::Controller() :
  m_myAddress(Ipv4Address::GetAny()),
  m_stateId(0),
  m_inband(nullptr),
  m_dftAccessProfile(""),
  m_dftMcastProfile("")

{
  NS_LOG_FUNCTION_NOARGS();
}

Controller::~Controller()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
Controller::AddApplication(Ptr<ControllerApplication>newApp)
{
  NS_LOG_INFO("Controller adding new application");

  newApp->SetController(this);

  m_applications.push_back(newApp);

  m_applications.sort([](const Ptr<ControllerApplication>& app1, const Ptr<ControllerApplication>& app2) {
      return *app1 < *app2;
    });
}

Ptr<ControllerApplication>
Controller::GetAppByTypeId(TypeId appTypeId)
{
  for (auto it = m_applications.begin(); it != m_applications.end(); it++)
    if (appTypeId == (*it)->GetInstanceTypeId())
      return *it;

  return NULL;
}

void
Controller::Print(std::ostream& os) const
{
  os << "[Node " << GetNode()->GetId() << "] ";
}

void
Controller::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS();
  Application::DoDispose();
}

void
Controller::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  NS_ABORT_IF(m_myAddress == Ipv4Address::GetAny());

  NS_LOG_INFO("Controller Node " << GetNode()->GetId() << " IP: " << m_myAddress);

  /* Get netdev */
  Ptr<Ipv4> ipv4    = GetNode()->GetObject<Ipv4>();
  int32_t   ifIndex = ipv4->GetInterfaceForAddress(m_myAddress);

  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to local interface");
  Ptr<NetDevice> m_myNetDev = ipv4->GetNetDevice(ifIndex);

  InitializeNetwork(GetNode(), m_myAddress);

  NS_ABORT_IF(m_inbandAddress == Ipv4Address::GetAny());

  ifIndex = ipv4->GetInterfaceForAddress(m_inbandAddress);
  NS_ABORT_MSG_IF(ifIndex < 0, "Unable to bind to inband interface");
  m_inband = ipv4->GetNetDevice(ifIndex);

  for (auto& app : m_applications) {
    app->InitMe();
  }
}

void
Controller::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();
}

Ptr<NetDevice>
Controller::GetInbandNetDevice()
{
  return m_inband;
}

Ptr<ServerVertex>
Controller::GetInbandVertex()
{
  Mac48Address mac = Mac48Address::ConvertFrom(m_inband->GetAddress());

  return DynamicCast<ServerVertex, Vertex>(m_nbi.LookupVertex(mac));
}

void
Controller::ForwardToApps(Ptr<OfSwitch>origin, unsigned portIn,
                          unsigned bufferId, Ptr<Packet>packet)
{
  NS_LOG_FUNCTION(this << origin << *packet);

  for (auto& app : m_applications) {
    if (app->ReceiveFromSwitch(origin, portIn, bufferId, packet->Copy()) == true)
      break;
  }
}

void
Controller::NewSwitch(Ptr<OfSwitch>origin)
{
  NS_LOG_FUNCTION(this << origin);

  if (auto v = m_nbi.LookupVertex(origin->GetChassisId())) {
    NS_LOG_WARN("Duplicated OF-Switch anouncement, ignoring");
    return;
  }

  m_nbi.AddSwitch(origin);

  for (auto& app : m_applications) {
    app->InitSwitch(origin);
  }
}

int
Controller::SwitchSync()
{
  ++m_stateId;

  NS_LOG_FUNCTION(this << m_stateId);

  for (auto sw: GetSwitchList()) {
    m_syncMap.emplace(m_stateId, sw);
    sw->SyncSwitch(m_stateId);
  }

  return m_stateId;
}

void
Controller::SwitchSyncCompleted(Ptr<OfSwitch>origin, int stateId)
{
  NS_LOG_FUNCTION(this << origin);

  auto iters = m_syncMap.equal_range(stateId);

  for (auto it = iters.first; it != iters.second; ++it) {
    if (it->second == origin) {
      m_syncMap.erase(it);
      break;
    }
  }

  if (m_syncMap.count(stateId) == 0) {
    for (auto& app : m_applications) app->SyncCompleted(stateId);
  }
}

Ptr<HostVertex>
Controller::AddHost(const Mac48Address& mac)
{
  if (auto host = m_nbi.LookupVertex(mac))
    return DynamicCast<HostVertex, Vertex>(host);

  Ptr<HostVertex> h = m_nbi.AddHost(mac);

  if (h) {
    h->SetAccessProfileName(m_dftAccessProfile);
    h->SetMCastProfileName(m_dftMcastProfile);

    for (auto& app : m_applications) app->HostInstalled(h);
  }
  return h;
}

void
Controller::RemoveHost(const Mac48Address& mac)
{
  if (auto host = m_nbi.LookupVertex(mac)) {
    Ptr<HostVertex> h = DynamicCast<HostVertex, Vertex>(host);

    for (auto& app : m_applications) app->HostUninstalled(h);
  }

  m_nbi.RemoveVertex(mac);
}

void
Controller::AddHostMapping(const Mac48Address& mac,
                           const Address     & addr)
{
  Ptr<HostVertex> host = AddHost(mac);
  AddHostMapping(host, addr);
}

void
Controller::AddHostMapping(Ptr<HostVertex>host,
                           const Address& addr)
{
  host->AddAddress(addr);
}

void
Controller::RemoveHostMapping(const Mac48Address& mac, const Address& addr)
{
  if (auto host = m_nbi.LookupVertex(mac))
    host->RemoveAddress(addr);
}

void
Controller::RemoveHostMapping(const Address& addr)
{
  if (auto host = m_nbi.LookupVertex(addr))
    host->RemoveAddress(addr);
}

Ptr<Adjacency>
Controller::AddEdge(Ptr<Vertex>from,
                    Ptr<Vertex>to,
                    uint16_t   portNum)
{
  Ptr<Adjacency> adj = m_nbi.AddEdge(from, to);

  if (adj)
    adj->SetPortNumber(portNum);

  return adj;
}

Ptr<Adjacency>
Controller::AddEdge(Ptr<Adjacency>adj)
{
  return m_nbi.AddEdge(adj);
}

Ptr<Adjacency>
Controller::AddEdge(Ptr<Vertex>from,
                    Ptr<Vertex>to)
{
  return m_nbi.AddEdge(from, to);
}

Ptr<Adjacency>
Controller::AddEdge(const Mac48Address& from,
                    const Mac48Address& to)
{
  return m_nbi.AddEdge(from, to);
}

Ptr<SwitchVertex>
Controller::LookupSwitch(const Mac48Address& mac) const
{
  return DynamicCast<SwitchVertex, Vertex>(m_nbi.LookupVertex(mac));
}

Ptr<HostVertex>
Controller::LookupHost(const Mac48Address& mac) const
{
  return DynamicCast<HostVertex, Vertex>(m_nbi.LookupVertex(mac));
}

Ptr<HostVertex>
Controller::LookupHost(const Address& addr) const
{
  return DynamicCast<HostVertex, Vertex>(m_nbi.LookupVertex(addr));
}

UndirectedGraph::GraphPath
Controller::FindShortestPath(const Mac48Address& from,
                             const Mac48Address& to) const
{
  return m_nbi.FindShortestPath(from, to);
}

UndirectedGraph::GraphPath
Controller::FindShortestPath(Ptr<Vertex>from,
                             Ptr<Vertex>to) const
{
  return m_nbi.FindShortestPath(from, to);
}

Ipv4Address
Controller::GetLocalAddress(Ptr<NetDevice>device) const
{
  NS_LOG_FUNCTION(device);

  Ptr<Ipv4> ipv4 = GetNode()->GetObject<Ipv4>();

  unsigned int i = ipv4->GetInterfaceForDevice(device);

  for (unsigned int j = 0; j < ipv4->GetNAddresses(i); ++j)
  {
    const Ipv4InterfaceAddress& iaddr = ipv4->GetAddress(i, j);

    if (iaddr.GetLocal() != Ipv4Address::GetLoopback())
    {
      return iaddr.GetLocal();
    }
  }

  return Ipv4Address::GetAny();
}

void
Controller::SetAccessProfile(const std::string& profName)
{
  m_dftAccessProfile = profName;
}
} // namespace ns3
#endif // NS3_OPENFLOW
