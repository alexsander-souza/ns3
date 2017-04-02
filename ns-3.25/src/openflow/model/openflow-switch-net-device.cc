/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
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
#ifdef NS3_OPENFLOW

#include "ns3/breakpoint.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "openflow-switch-net-device.h"
#include "openflow-client.h"
#include "openflow-lib.h"
#include "headers/openflow-header.h"
#include "headers/packet-in-header.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("OpenFlowSwitchNetDevice");

NS_OBJECT_ENSURE_REGISTERED(OpenFlowSwitchNetDevice);

const char *
OpenFlowSwitchNetDevice::GetManufacturerDescription()
{
  return "The ns-3 team";
}

const char *
OpenFlowSwitchNetDevice::GetHardwareDescription()
{
  return "N/A";
}

const char *
OpenFlowSwitchNetDevice::GetSoftwareDescription()
{
  return "Simulated OpenFlow Switch";
}

const char *
OpenFlowSwitchNetDevice::GetSerialNumber()
{
  return "N/A";
}

static uint64_t
GenerateId()
{
  uint8_t ea[ETH_ADDR_LEN];

  eth_addr_random(ea);
  return eth_addr_to_uint64(ea);
}

TypeId
OpenFlowSwitchNetDevice::GetTypeId(void)
{
  static TypeId tid = TypeId("ns3::OpenFlowSwitchNetDevice")
                      .SetParent<NetDevice>()
                      .SetGroupName("Openflow")
                      .AddConstructor<OpenFlowSwitchNetDevice>()
                      .AddAttribute("ID",
                                    "The identification of the OpenFlowSwitchNetDevice/Datapath, needed for OpenFlow compatibility.",
                                    UintegerValue(GenerateId()),
                                    MakeUintegerAccessor(&OpenFlowSwitchNetDevice::m_id),
                                    MakeUintegerChecker<uint64_t>())
                      .AddAttribute("FlowTableLookupDelay",
                                    "A real switch will have an overhead for looking up in the flow table. For the default, we simulate a standard TCAM on an FPGA.",
                                    TimeValue(NanoSeconds(30)),
                                    MakeTimeAccessor(&OpenFlowSwitchNetDevice::m_lookupDelay),
                                    MakeTimeChecker())
                      .AddAttribute("Flags",
                                    "Flags to turn different functionality on/off, such as whether to inform the controller when a flow expires, or how to handle fragments.",
                                    UintegerValue(0),
                                    MakeUintegerAccessor(&OpenFlowSwitchNetDevice::m_flags),
                                    MakeUintegerChecker<uint16_t>())
                      .AddAttribute("PacketCacheSize",
                                    "Flags to turn different functionality on/off, such as whether to inform the controller when a flow expires, or how to handle fragments.",
                                    UintegerValue(2000),
                                    MakeUintegerAccessor(&OpenFlowSwitchNetDevice::m_packetDataLimit),
                                    MakeUintegerChecker<unsigned>())
                      .AddAttribute("FlowTableMissSendLength",
                                    "When forwarding a packet the switch didn't match up to the controller, it can be more efficient to forward only the first x bytes.",
                                    UintegerValue(OFP_DEFAULT_MISS_SEND_LEN),
                                    MakeUintegerAccessor(&OpenFlowSwitchNetDevice::m_missSendLen),
                                    MakeUintegerChecker<uint16_t>())
  ;

  return tid;
}

OpenFlowSwitchNetDevice::OpenFlowSwitchNetDevice()
  : m_node(0),
  m_ifIndex(0),
  m_mtu(DEFAULT_MTU),
  m_application(0)
{
  NS_LOG_FUNCTION(this);

  m_channel = CreateObject<BridgeChannel>();

  time_init(); // OFSI's clock; needed to use the buffer storage system.

  m_chain = chain_create();

  if (m_chain == 0)
  {
    NS_LOG_ERROR("Not enough memory to create the flow table.");
  }

  m_ports.reserve(DP_MAX_PORTS);

  Simulator::Schedule(Seconds(1),
                      &OpenFlowSwitchNetDevice::SweepExpiredFlows,
                      this);
}

OpenFlowSwitchNetDevice::~OpenFlowSwitchNetDevice()
{
  NS_LOG_FUNCTION_NOARGS();
}

Ptr<OpenflowClient>
OpenFlowSwitchNetDevice::GetOpenflowClient(void)
{
  NS_LOG_FUNCTION(this);
  return m_application;
}

void
OpenFlowSwitchNetDevice::SetOpenflowClient(Ptr<OpenflowClient>app)
{
  NS_LOG_FUNCTION(this << app);
  m_application = app;
}

int
OpenFlowSwitchNetDevice::AddSwitchPort(Ptr<NetDevice>switchPort)
{
  NS_LOG_FUNCTION(this << switchPort->GetAddress());
  NS_ASSERT(switchPort != this);

  if (!Mac48Address::IsMatchingType(switchPort->GetAddress()))
  {
    NS_FATAL_ERROR("Device does not support eui 48 addresses: cannot be added to switch.");
  }

  if (!switchPort->SupportsSendFrom())
  {
    NS_FATAL_ERROR("Device does not support SendFrom: cannot be added to switch.");
  }

  if (m_ports.size() < DP_MAX_PORTS)
  {
    ofi::Port p;
    p.config = 0;
    p.netdev = switchPort;
    m_ports.push_back(p);

    // Notify the controller that this port has been added
    SendPortStatus(p, OFPPR_ADD);

    m_node->RegisterProtocolHandler(MakeCallback(&OpenFlowSwitchNetDevice::ReceiveFromDevice, this),
                                    0, switchPort, true);
    m_channel->AddChannel(switchPort->GetChannel());
  }
  else
  {
    return EXFULL;
  }

  return 0;
}

int
OpenFlowSwitchNetDevice::StatsDump(ofi::StatsDumpCallback *cb)
{
  ofp_stats_reply *osr;
  ofpbuf *buffer;
  int     err;

  if (cb->done)
  {
    return 0;
  }

  osr        = (ofp_stats_reply *)MakeOpenflowReply(sizeof *osr, OFPT_STATS_REPLY, &buffer);
  osr->type  = htons(cb->s->type);
  osr->flags = 0;

  err = cb->s->DoDump(this, cb->state, buffer);

  if (err >= 0)
  {
    if (err == 0)
    {
      cb->done = true;
    }
    else
    {
      // Buffer might have been reallocated, so find our data again.
      osr        = (ofp_stats_reply *)ofpbuf_at_assert(buffer, 0, sizeof *osr);
      osr->flags = ntohs(OFPSF_REPLY_MORE);
    }

    int err2 = SendOpenflowBuffer(buffer);

    if (err2)
    {
      err = err2;
    }
  }

  return err;
}

void
OpenFlowSwitchNetDevice::StatsDone(ofi::StatsDumpCallback *cb)
{
  if (cb)
  {
    cb->s->DoCleanup(cb->state);
    free(cb->s);
    free(cb);
  }
}

void
OpenFlowSwitchNetDevice::DoOutput(uint32_t packet_uid, int in_port, size_t max_len, int out_port, bool ignore_no_fwd)
{
  NS_LOG_FUNCTION(this << packet_uid << in_port << out_port);

  if (out_port != OFPP_CONTROLLER)
  {
    OutputPort(packet_uid, in_port, out_port, ignore_no_fwd);
  }
  else
  {
    OutputControl(packet_uid, in_port, max_len, OFPR_ACTION);
  }
}

int
OpenFlowSwitchNetDevice::ForwardControlInput(const void *msg, size_t length)
{
  NS_LOG_FUNCTION(this << msg);

  // Check encapsulated length.
  ofp_header *oh = (ofp_header *)msg;

  if (oh->length > length)
  {
    NS_LOG_WARN(this << " Invalid message " << int(oh->type) << " length, expected "
                     << length << " got " << oh->length);
    return -EINVAL;
  }

  NS_ASSERT(oh->version == OFP_VERSION);

  int error = 0;

  // Figure out how to handle it.
  switch (oh->type)
  {
  case OFPT_FEATURES_REQUEST:
    error = length < sizeof(ofp_header) ? -EFAULT : ReceiveFeaturesRequest(msg);
    break;

  case OFPT_GET_CONFIG_REQUEST:
    error = length < sizeof(ofp_header) ? -EFAULT : ReceiveGetConfigRequest(msg);
    break;

  case OFPT_SET_CONFIG:
    error = length < sizeof(ofp_switch_config) ? -EFAULT : ReceiveSetConfig(msg);
    break;

  case OFPT_PACKET_OUT:
    error = length < sizeof(ofp_packet_out) ? -EFAULT : ReceivePacketOut(msg);
    break;

  case OFPT_FLOW_MOD:
    error = length < sizeof(ofp_flow_mod) ? -EFAULT : ReceiveFlow(msg);
    break;

  case OFPT_PORT_MOD:
    error = length < sizeof(ofp_port_mod) ? -EFAULT : ReceivePortMod(msg);
    break;

  case OFPT_STATS_REQUEST:
    error = length < sizeof(ofp_stats_request) ? -EFAULT : ReceiveStatsRequest(msg);
    break;

  default:
    SendErrorMsg((ofp_error_type)OFPET_BAD_REQUEST, (ofp_bad_request_code)OFPBRC_BAD_TYPE, msg, length);
    error = -EINVAL;
  }

  if (error != 0)
    NS_LOG_WARN("Failed to process Controller request " << error);

  return error;
}

sw_chain *
OpenFlowSwitchNetDevice::GetChain()
{
  return m_chain;
}

uint32_t
OpenFlowSwitchNetDevice::GetNSwitchPorts(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_ports.size();
}

ofi::Port
OpenFlowSwitchNetDevice::GetSwitchPort(uint32_t n) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_ports[n];
}

int
OpenFlowSwitchNetDevice::GetSwitchPortIndex(const ofi::Port& p)
{
  for (size_t i = 0; i < m_ports.size(); i++)
  {
    if (m_ports[i].netdev == p.netdev)
    {
      return i;
    }
  }
  return -1;
}

void
OpenFlowSwitchNetDevice::SetIfIndex(const uint32_t index)
{
  NS_LOG_FUNCTION_NOARGS();
  m_ifIndex = index;
}

uint32_t
OpenFlowSwitchNetDevice::GetIfIndex(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_ifIndex;
}

Ptr<Channel>
OpenFlowSwitchNetDevice::GetChannel(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_channel;
}

void
OpenFlowSwitchNetDevice::SetAddress(Address address)
{
  NS_LOG_FUNCTION(address);
  m_myMacAddress = Mac48Address::ConvertFrom(address);

  uint8_t ea[ETH_ADDR_LEN];

  m_myMacAddress.CopyTo(ea);

  m_id = eth_addr_to_uint64(ea);
}

Address
OpenFlowSwitchNetDevice::GetAddress(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_myMacAddress;
}

bool
OpenFlowSwitchNetDevice::SetMtu(const uint16_t mtu)
{
  NS_LOG_FUNCTION_NOARGS();
  m_mtu = mtu;
  return true;
}

uint16_t
OpenFlowSwitchNetDevice::GetMtu(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_mtu;
}

bool
OpenFlowSwitchNetDevice::IsLinkUp(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

void
OpenFlowSwitchNetDevice::AddLinkChangeCallback(Callback<void>callback)
{}

bool
OpenFlowSwitchNetDevice::IsBroadcast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address
OpenFlowSwitchNetDevice::GetBroadcast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return Mac48Address("ff:ff:ff:ff:ff:ff");
}

bool
OpenFlowSwitchNetDevice::IsMulticast(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address
OpenFlowSwitchNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION(this << multicastGroup);
  Mac48Address multicast = Mac48Address::GetMulticast(multicastGroup);
  return multicast;
}

bool
OpenFlowSwitchNetDevice::IsPointToPoint(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return false;
}

bool
OpenFlowSwitchNetDevice::IsBridge(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

bool
OpenFlowSwitchNetDevice::Send(Ptr<Packet>packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS();
  return SendFrom(packet, m_myMacAddress, dest, protocolNumber);
}

bool
OpenFlowSwitchNetDevice::SendFrom(Ptr<Packet>packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS();

  ofpbuf *buffer = ofi::BufferFromPacket(packet, src, dest, GetMtu(), protocolNumber);

  ofi::SwitchPacketMetadata& data = GetPacketBuffer();
  data.packet         = packet;
  data.buffer         = buffer;
  data.protocolNumber = protocolNumber;
  data.src            = Address(src);
  data.dst            = Address(dest);

  RunThroughFlowTable(data.cookie, -1);

  return true;
}

Ptr<Node>
OpenFlowSwitchNetDevice::GetNode(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return m_node;
}

void
OpenFlowSwitchNetDevice::SetNode(Ptr<Node>node)
{
  NS_LOG_FUNCTION_NOARGS();
  m_node = node;
}

bool
OpenFlowSwitchNetDevice::NeedsArp(void) const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

void
OpenFlowSwitchNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS();
  m_rxCallback = cb;
}

void
OpenFlowSwitchNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS();
  m_promiscRxCallback = cb;
}

bool
OpenFlowSwitchNetDevice::SupportsSendFrom() const
{
  NS_LOG_FUNCTION_NOARGS();
  return true;
}

Address
OpenFlowSwitchNetDevice::GetMulticast(Ipv6Address addr) const
{
  NS_LOG_FUNCTION(this << addr);
  return Mac48Address::GetMulticast(addr);
}

void
OpenFlowSwitchNetDevice::DoDispose()
{
  NS_LOG_FUNCTION_NOARGS();

  for (Ports_t::iterator b = m_ports.begin(), e = m_ports.end(); b != e; b++)
  {
    SendPortStatus(*b, OFPPR_DELETE);
    b->netdev = 0;
  }
  m_ports.clear();

  chain_destroy(m_chain);
  m_channel     = 0;
  m_node        = 0;
  m_application = 0;
  NetDevice::DoDispose();
}

void
OpenFlowSwitchNetDevice::FreePacketBuffer(uint32_t cookie)
{
  auto data = m_packetData.find(cookie);

  if (data != m_packetData.end()) {
    if (data->second.buffer != nullptr)
      ofpbuf_delete(data->second.buffer);

    m_packetData.erase(data);
  }
}

ofi::SwitchPacketMetadata&
OpenFlowSwitchNetDevice::GetPacketBuffer()
{
  static Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();

  // Do cleanup
  while (m_packetData.size() > m_packetDataLimit) {
    FreePacketBuffer(m_packetData.begin()->first);
  }

  // Create new buffer
  uint32_t cookie = x->GetInteger(0, std::numeric_limits<uint32_t>::max() - 1);

  m_packetData[cookie].ttl    = Simulator::Now() + Seconds(1);
  m_packetData[cookie].cookie = cookie;

  return m_packetData[cookie];
}

void
OpenFlowSwitchNetDevice::ReceiveFromDevice(Ptr<NetDevice>netdev, Ptr<const Packet>packet, uint16_t protocol,
                                           const Address& src, const Address& dst, PacketType packetType)
{
  NS_LOG_FUNCTION(this << packet << src << dst);

  Mac48Address dst48 = Mac48Address::ConvertFrom(dst);

  if (!m_promiscRxCallback.IsNull())
  {
    m_promiscRxCallback(this, packet, protocol, src, dst, packetType);
  }

  NS_LOG_LOGIC(this << " Received packet from " << Mac48Address::ConvertFrom(src) << " looking for " << dst48);

  for (size_t i = 0; i < m_ports.size(); i++)
  {
    if (m_ports[i].netdev == netdev)
    {
      if ((packetType == PACKET_HOST) && (dst48 == m_myMacAddress))
      {
        m_rxCallback(this, packet, protocol, src);
      }
      else if ((packetType == PACKET_BROADCAST) || (packetType == PACKET_MULTICAST) || (packetType == PACKET_OTHERHOST))
      {
        if ((packetType == PACKET_OTHERHOST) && (dst48 == m_myMacAddress))
        {
          m_rxCallback(this, packet, protocol, src);
        }
        else
        {
          if (packetType != PACKET_OTHERHOST)
          {
            m_rxCallback(this, packet, protocol, src);
          }

          ofi::SwitchPacketMetadata& data = GetPacketBuffer();
          data.packet = packet->Copy();

          ofpbuf *buffer = ofi::BufferFromPacket(data.packet, src, dst, netdev->GetMtu(), protocol);
          data.buffer         = buffer;
          data.protocolNumber = protocol;
          data.src            = Address(src);
          data.dst            = Address(dst);

          m_ports[i].rx_packets++;
          m_ports[i].rx_bytes += buffer->size;

          RunThroughFlowTable(data.cookie, i);
        }
      }

      break;
    }
  }
}

void
OpenFlowSwitchNetDevice::SweepExpiredFlows()
{
  // If port status is modified in any way, notify the controller.
  for (size_t i = 0; i < m_ports.size(); i++)
  {
    if (UpdatePortStatus(m_ports[i]))
    {
      SendPortStatus(m_ports[i], OFPPR_MODIFY);
    }
  }

  // If any flows have expired, delete them and notify the controller.
  List deleted = LIST_INITIALIZER(&deleted);
  sw_flow *f, *n;
  chain_timeout(m_chain, &deleted);
  LIST_FOR_EACH_SAFE(f, n, sw_flow, node, &deleted)
  {
    SendFlowExpired(f, (ofp_flow_expired_reason)f->reason);
    list_remove(&f->node);
    flow_free(f);
  }

  Simulator::Schedule(Seconds(1),
                      &OpenFlowSwitchNetDevice::SweepExpiredFlows,
                      this);
}

int
OpenFlowSwitchNetDevice::AddFlow(const ofp_flow_mod *ofm)
{
  NS_LOG_FUNCTION(this << ofm);

  size_t actions_len = ntohs(ofm->header.length) - sizeof *ofm;

  // Allocate memory.
  sw_flow *flow = flow_alloc(actions_len);

  if (flow == 0)
  {
    FreePacketBuffer(ntohl(ofm->buffer_id));
    return -ENOMEM;
  }

  flow_extract_match(&flow->key, &ofm->match);

  // flow->key.wildcards   = ofm->match.wildcards;
  // flow->key.flow.nw_src = ofm->match.nw_src;
  // flow->key.flow.nw_dst = ofm->match.nw_dst;

  uint16_t v_code = ofi::ValidateActions(&flow->key, ofm->actions, actions_len);

  if (v_code != ACT_VALIDATION_OK)
  {
    NS_LOG_WARN("Action validation failed");

    SendErrorMsg(OFPET_BAD_ACTION, v_code, ofm, ntohs(ofm->header.length));
    flow_free(flow);

    FreePacketBuffer(ntohl(ofm->buffer_id));
    return -ENOMEM;
  }

  // Fill out flow.
  flow->priority             = flow->key.wildcards ? ntohs(ofm->priority) : -1;
  flow->idle_timeout         = ntohs(ofm->idle_timeout);
  flow->hard_timeout         = ntohs(ofm->hard_timeout);
  flow->used                 = flow->created = time_now();
  flow->sf_acts->actions_len = actions_len;
  flow->byte_count           = 0;
  flow->packet_count         = 0;
  memcpy(flow->sf_acts->actions, ofm->actions, actions_len);

  // Act.
  int error = chain_insert(m_chain, flow);

  if (error)
  {
    if (error == -ENOBUFS)
    {
      SendErrorMsg(OFPET_FLOW_MOD_FAILED, OFPFMFC_ALL_TABLES_FULL, ofm, ntohs(ofm->header.length));
    }
    flow_free(flow);

    FreePacketBuffer(ntohl(ofm->buffer_id));
    return error;
  }

  if (ntohl(ofm->buffer_id) != (uint32_t)-1) {
    auto b = m_packetData.find(ntohl(ofm->buffer_id));

    if (b == m_packetData.end())
      return -ESRCH;

    ofpbuf *buffer = b->second.buffer;

    sw_flow_key key;
    flow_used(flow, buffer);
    flow_extract(buffer, ofm->match.in_port, &key.flow); // ntohs(ofm->match.in_port);
    ofi::ExecuteActions(this, ntohl(ofm->buffer_id), buffer, &key, ofm->actions, actions_len, false);

    FreePacketBuffer(b->second.cookie);
  }
  return 0;
}

int
OpenFlowSwitchNetDevice::ModFlow(const ofp_flow_mod *ofm)
{
  NS_LOG_FUNCTION(this << ofm);

  sw_flow_key key;

  flow_extract_match(&key, &ofm->match);

  size_t actions_len = ntohs(ofm->header.length) - sizeof *ofm;

  uint16_t v_code = ofi::ValidateActions(&key, ofm->actions, actions_len);

  if (v_code != ACT_VALIDATION_OK)
  {
    SendErrorMsg((ofp_error_type)OFPET_BAD_ACTION, v_code, ofm, ntohs(ofm->header.length));

    FreePacketBuffer(ntohl(ofm->buffer_id));
    return -ENOMEM;
  }

  uint16_t priority = key.wildcards ? ntohs(ofm->priority) : -1;
  int strict        = (ofm->command == htons(OFPFC_MODIFY_STRICT)) ? 1 : 0;
  chain_modify(m_chain, &key, priority, strict, ofm->actions, actions_len);

  if (ntohl(ofm->buffer_id) != (uint32_t)-1) {
    auto b = m_packetData.find(ntohl(ofm->buffer_id));

    if (b == m_packetData.end())
      return -ESRCH;

    ofpbuf *buffer = b->second.buffer;

    sw_flow_key skb_key;
    flow_extract(buffer, ofm->match.in_port, &skb_key.flow); // ntohs(ofm->match.in_port);
    ofi::ExecuteActions(this, ntohl(ofm->buffer_id), buffer, &skb_key, ofm->actions, actions_len, false);

    FreePacketBuffer(b->second.cookie);
  }
  return 0;
}

int
OpenFlowSwitchNetDevice::OutputAll(uint32_t packet_uid, int in_port, bool flood)
{
  NS_LOG_FUNCTION(this << packet_uid << in_port);

  int prev_port = -1;

  for (size_t i = 0; i < m_ports.size(); i++)
  {
    if (i == (unsigned)in_port) // Originating port
    {
      continue;
    }

    if (flood && m_ports[i].config & OFPPC_NO_FLOOD) // Port configured to not
                                                     // allow flooding
    {
      continue;
    }

    if (prev_port != -1)
    {
      OutputPort(packet_uid, in_port, prev_port, false);
    }
    prev_port = i;
  }

  if (prev_port != -1)
  {
    OutputPort(packet_uid, in_port, prev_port, false);
  }

  return 0;
}

void
OpenFlowSwitchNetDevice::OutputPacket(uint32_t packet_uid, int out_port)
{
  NS_LOG_FUNCTION(this << packet_uid << out_port);

  if ((out_port >= 0) && (out_port < DP_MAX_PORTS))
  {
    ofi::Port& p = m_ports[out_port];

    if ((p.netdev != 0) && !(p.config & OFPPC_PORT_DOWN))
    {
      auto pktPair = m_packetData.find(packet_uid);

      if (pktPair != m_packetData.end())
      {
        ofi::SwitchPacketMetadata data = pktPair->second;
        size_t bufsize                 = data.buffer->size;
        NS_LOG_LOGIC("Sending packet " << data.packet->GetUid() << " over port " << out_port);

        if (p.netdev->SendFrom(data.packet->Copy(), data.src, data.dst, data.protocolNumber))
        {
          p.tx_packets++;
          p.tx_bytes += bufsize;
        }
        else
        {
          p.tx_dropped++;
        }
      } else {
        NS_LOG_WARN("Packet id not found: ID " << packet_uid);
      }
      return;
    }
  }

  NS_LOG_WARN("can't forward to bad port " << out_port);
}

void
OpenFlowSwitchNetDevice::OutputPort(uint32_t packet_uid, int in_port, int out_port, bool ignore_no_fwd)
{
  NS_LOG_FUNCTION(this << packet_uid << in_port << out_port);

  if (out_port == OFPP_FLOOD)
  {
    OutputAll(packet_uid, in_port, true);
  }
  else if (out_port == OFPP_ALL)
  {
    OutputAll(packet_uid, in_port, false);
  }
  else if (out_port == OFPP_CONTROLLER)
  {
    OutputControl(packet_uid, in_port, 0, OFPR_ACTION);
  }
  else if (out_port == OFPP_IN_PORT)
  {
    OutputPacket(packet_uid, in_port);
  }
  else if (out_port == OFPP_TABLE)
  {
    RunThroughFlowTable(packet_uid, in_port < DP_MAX_PORTS ? in_port : -1, false);
  }
  else if (in_port == out_port)
  {
    NS_LOG_WARN("can't directly forward to input port, IN=" << in_port << " OUT=" << out_port);
  }
  else
  {
    OutputPacket(packet_uid, out_port);
  }
}

void
OpenFlowSwitchNetDevice::OutputControl(uint32_t packet_uid, int in_port, size_t max_len, int reason)
{
  NS_LOG_FUNCTION(this << packet_uid << in_port << reason);

  ofpbuf *buffer    = m_packetData.find(packet_uid)->second.buffer;
  size_t  total_len = buffer->size;

  if ((packet_uid != std::numeric_limits<uint32_t>::max()) && (max_len != 0) && (buffer->size > max_len))
  {
    NS_LOG_ERROR("Invalid buffer size, some data will be lost!");
    buffer->size = max_len;
  }

  ofp_packet_in *opi = (ofp_packet_in *)ofpbuf_push_uninit(buffer, offsetof(ofp_packet_in, data));
  opi->header.version = OFP_VERSION;
  opi->header.type    = OFPT_PACKET_IN;
  opi->header.length  = htons(buffer->size);
  opi->header.xid     = htonl(0);
  opi->buffer_id      = htonl(packet_uid);
  opi->total_len      = htons(total_len);
  opi->in_port        = htons(in_port);
  opi->reason         = reason;
  opi->pad            = 0;
  SendOpenflowBuffer(buffer);
}

void
OpenFlowSwitchNetDevice::SendErrorMsg(uint16_t type, uint16_t code, const void *data, size_t len)
{
  ofpbuf *buffer;
  ofp_error_msg *oem = (ofp_error_msg *)MakeOpenflowReply(sizeof(*oem) + len, OFPT_ERROR, &buffer);

  oem->type = htons(type);
  oem->code = htons(code);
  memcpy(oem->data, data, len);
  SendOpenflowBuffer(buffer);
}

void
OpenFlowSwitchNetDevice::SendFeaturesReply(uint32_t xid)
{
  NS_LOG_FUNCTION(this << xid);

  if (m_application)
    m_application->SendFeaturesReply(m_ports.size(), xid, m_id, (uint8_t)m_chain->n_tables,
                                     m_packetDataLimit,
                                     m_ports);
}

void
OpenFlowSwitchNetDevice::SendFlowExpired(sw_flow *flow, enum ofp_flow_expired_reason reason)
{
  NS_LOG_FUNCTION(this);
  ofpbuf *buffer;
  ofp_flow_expired *ofe = (ofp_flow_expired *)MakeOpenflowReply(sizeof *ofe, OFPT_FLOW_EXPIRED, &buffer);

  flow_fill_match(&ofe->match, &flow->key);

  ofe->priority = htons(flow->priority);
  ofe->reason   = reason;
  memset(ofe->pad,  0, sizeof ofe->pad);

  ofe->duration = htonl(time_now() - flow->created);
  memset(ofe->pad2, 0, sizeof ofe->pad2);
  ofe->packet_count = htonll(flow->packet_count);
  ofe->byte_count   = htonll(flow->byte_count);
  SendOpenflowBuffer(buffer);
}

void
OpenFlowSwitchNetDevice::SendPortStatus(ofi::Port p, uint8_t status)
{
  NS_LOG_FUNCTION(this);

  ofpbuf *buffer;
  ofp_port_status *ops = (ofp_port_status *)MakeOpenflowReply(sizeof *ops, OFPT_PORT_STATUS, &buffer);

  ops->reason = status;
  memset(ops->pad, 0, sizeof ops->pad);
  FillPortDesc(p, &ops->desc);

  SendOpenflowBuffer(buffer);
  ofpbuf_delete(buffer);
}

int
OpenFlowSwitchNetDevice::SendOpenflowBuffer(ofpbuf *buffer)
{
  NS_LOG_FUNCTION(this);

  if (!m_application)
    return -1;

  update_openflow_length(buffer);
  m_application->ReceiveFromNetDevice(buffer);
  return 0;
}

void
OpenFlowSwitchNetDevice::RunThroughFlowTable(uint32_t packet_uid, int port, bool send_to_controller)
{
  NS_LOG_FUNCTION(this << packet_uid);

  if (m_packetData.find(packet_uid) == m_packetData.end()) {
    NS_LOG_WARN(this << "packet not found");
    return;
  }

  ofi::SwitchPacketMetadata& data = m_packetData.find(packet_uid)->second;
  ofpbuf *buffer                  = data.buffer;

  sw_flow_key key;

  key.wildcards = 0; // Lookup cannot take wildcards.

  // Extract the matching key's flow data from the packet's headers; if the
  // policy is to drop fragments and the message is a fragment, drop it.
  if (flow_extract(buffer, (port != -1) ? port : OFPP_NONE,
                   &key.flow) && ((m_flags & OFPC_FRAG_MASK) == OFPC_FRAG_DROP))
  {
    FreePacketBuffer(packet_uid);
    return;
  }

  // If we received the packet on a port, and opted not to receive any messages
  // from it...
  if (port != -1)
  {
    uint32_t config = m_ports[port].config;

    if (config & (OFPPC_NO_RECV | OFPPC_NO_RECV_STP)
        && config & (!eth_addr_equals(key.flow.dl_dst, stp_eth_addr) ? OFPPC_NO_RECV : OFPPC_NO_RECV_STP))
    {
      return;
    }
  }

  Simulator::Schedule(m_lookupDelay,
                      &OpenFlowSwitchNetDevice::FlowTableLookup,
                      this,
                      key,
                      buffer,
                      packet_uid,
                      port,
                      send_to_controller);
}

void
OpenFlowSwitchNetDevice::FlowTableLookup(sw_flow_key key,
                                         ofpbuf     *buffer,
                                         uint32_t    packet_uid,
                                         int         port,
                                         bool        send_to_controller)
{
  NS_LOG_FUNCTION(this << packet_uid << port);
  sw_flow *flow = chain_lookup(m_chain, &key);

  if (flow != 0)
  {
    NS_LOG_LOGIC(this << " UID " << packet_uid << " Flow matched: " << flow);

    key.flow.in_port = ntohs(key.flow.in_port);
    flow_used(flow, buffer);
    ofi::ExecuteActions(this, packet_uid, buffer, &key, flow->sf_acts->actions, flow->sf_acts->actions_len, false);
  }
  else
  {
    NS_LOG_LOGIC(this << " UID " << packet_uid << " Flow not matched.");

    if (send_to_controller)
    {
      OutputControl(packet_uid, port, m_missSendLen, OFPR_NO_MATCH);
    }
  }
}

int
OpenFlowSwitchNetDevice::UpdatePortStatus(ofi::Port& p)
{
  uint32_t orig_config = p.config;
  uint32_t orig_state  = p.state;

  // Port is always enabled because the Net Device is always enabled.
  p.config &= ~OFPPC_PORT_DOWN;

  if (p.netdev->IsLinkUp())
  {
    p.state &= ~OFPPS_LINK_DOWN;
  }
  else
  {
    p.state |= OFPPS_LINK_DOWN;
  }

  return (orig_config != p.config) || (orig_state != p.state);
}

void
OpenFlowSwitchNetDevice::FillPortDesc(ofi::Port p, ofp_phy_port *desc)
{
  desc->port_no = htons(GetSwitchPortIndex(p));

  std::string portname = "eth" + std::to_string(GetSwitchPortIndex(p));
  strncpy((char *)desc->name, portname.c_str(), portname.size());

  p.netdev->GetAddress().CopyTo(desc->hw_addr);
  desc->config = htonl(p.config);
  desc->state  = htonl(p.state);

  /// \todo This should probably be fixed eventually to specify different
  // available features.
  desc->curr = 0;       // htonl(netdev_get_features(p->netdev,
                        // NETDEV_FEAT_CURRENT));
  desc->supported = 0;  // htonl(netdev_get_features(p->netdev,
                        // NETDEV_FEAT_SUPPORTED));
  desc->advertised = 0; // htonl(netdev_get_features(p->netdev,
                        // NETDEV_FEAT_ADVERTISED));
  desc->peer = 0;       // htonl(netdev_get_features(p->netdev,
                        // NETDEV_FEAT_PEER));
}

void *
OpenFlowSwitchNetDevice::MakeOpenflowReply(size_t openflow_len, uint8_t type, ofpbuf **bufferp)
{
  return make_openflow_xid(openflow_len, type, 0, bufferp);
}

int
OpenFlowSwitchNetDevice::ReceiveFeaturesRequest(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);

  // Check encapsulated xid.
  ofp_header *oh = (ofp_header *)msg;

  SendFeaturesReply(oh->xid);
  return 0;
}

int
OpenFlowSwitchNetDevice::ReceiveGetConfigRequest(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);

  ofp_switch_config *osc = (ofp_switch_config *)msg;

  if (!m_application)
    return -1;

  m_application->SendConfigurationReply(osc->header.xid, m_flags, m_missSendLen);
  return 0;
}

int
OpenFlowSwitchNetDevice::ReceiveSetConfig(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);

  const ofp_switch_config *osc = (ofp_switch_config *)msg;

  int n_flags = osc->flags & (OFPC_SEND_FLOW_EXP | OFPC_FRAG_MASK);

  if (((n_flags & OFPC_FRAG_MASK) != OFPC_FRAG_NORMAL) && ((n_flags & OFPC_FRAG_MASK) != OFPC_FRAG_DROP))
  {
    n_flags = (n_flags & ~OFPC_FRAG_MASK) | OFPC_FRAG_DROP;
  }

  m_flags       = n_flags;
  m_missSendLen = osc->miss_send_len;

  return 0;
}

int
OpenFlowSwitchNetDevice::ReceivePacketOut(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);

  const ofp_packet_out *opo = (ofp_packet_out *)msg;
  ofpbuf  *buffer;
  size_t   actions_len = ntohs(opo->actions_len);
  uint32_t packetId    = ntohl(opo->buffer_id);

  if (actions_len > (ntohs(opo->header.length) - sizeof *opo))
  {
    NS_LOG_DEBUG("message too short for number of actions");
    return -EINVAL;
  }

  if (packetId == (uint32_t)-1)
  {
    // FIXME: can we avoid copying data here?
    int data_len  = ntohs(opo->header.length) - sizeof *opo - actions_len;
    uint8_t *data = new u_int8_t[data_len];

    memcpy(data, (uint8_t *)opo->actions + actions_len, data_len);

    EthernetHeader ethHeader;

    Ptr<Packet> packet = new Packet(data, data_len);
    packet->RemoveHeader(ethHeader);

    ofi::SwitchPacketMetadata& metaData = GetPacketBuffer();
    metaData.src            = m_myMacAddress;
    metaData.dst            = ethHeader.GetDestination();
    metaData.protocolNumber = ethHeader.GetLengthType();

    buffer = ofi::BufferFromPacket(packet, metaData.src, metaData.dst,
                                   GetMtu(), metaData.protocolNumber);
    metaData.buffer = buffer;
    metaData.packet = packet;

    packetId = metaData.cookie;
  }
  else
  {
    auto pair = m_packetData.find(packetId);

    if (pair == m_packetData.end())
      return -ESRCH;

    ofi::SwitchPacketMetadata& metaData = pair->second;
    buffer = metaData.buffer;
  }

  sw_flow_key key;
  flow_extract(buffer, opo->in_port, &key.flow); // ntohs(opo->in_port)

  uint16_t v_code = ofi::ValidateActions(&key, opo->actions, actions_len);

  if (v_code != ACT_VALIDATION_OK)
  {
    SendErrorMsg(OFPET_BAD_ACTION, v_code, msg, ntohs(opo->header.length));
    FreePacketBuffer(packetId);
    return -EINVAL;
  }

  ofi::ExecuteActions(this, (uint64_t)packetId, buffer, &key, opo->actions, actions_len, true);
  return 0;
}

int
OpenFlowSwitchNetDevice::ReceiveFlow(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);

  const ofp_flow_mod *ofm = (ofp_flow_mod *)msg;
  uint16_t command        = ntohs(ofm->command);

  NS_LOG_FUNCTION(this << command);

  if (command == OFPFC_ADD)
  {
    return AddFlow(ofm);
  }
  else if ((command == OFPFC_MODIFY) || (command == OFPFC_MODIFY_STRICT))
  {
    return ModFlow(ofm);
  }
  else if (command == OFPFC_DELETE)
  {
    sw_flow_key key;
    flow_extract_match(&key, &ofm->match);
    return chain_delete(m_chain, &key, ofm->out_port, 0, 0) ? 0 : -ESRCH;
  }
  else if (command == OFPFC_DELETE_STRICT)
  {
    sw_flow_key key;
    uint16_t    priority;
    flow_extract_match(&key, &ofm->match);
    priority = key.wildcards ? ntohs(ofm->priority) : -1;
    return chain_delete(m_chain, &key, ofm->out_port, priority, 1) ? 0 : -ESRCH;
  }
  else
  {
    return -ENODEV;
  }
}

int
OpenFlowSwitchNetDevice::ReceivePortMod(const void *msg)
{
  NS_LOG_FUNCTION(this << msg);
  ofp_port_mod *opm = (ofp_port_mod *)msg;

  int port = opm->port_no; // ntohs(opm->port_no);

  if (port < DP_MAX_PORTS)
  {
    ofi::Port& p = m_ports[port];

    // Make sure the port id hasn't changed since this was sent
    Mac48Address hw_addr = Mac48Address();
    hw_addr.CopyFrom(opm->hw_addr);

    if (p.netdev->GetAddress() != hw_addr)
    {
      return 0;
    }

    if (opm->mask)
    {
      uint32_t config_mask = ntohl(opm->mask);
      p.config &= ~config_mask;
      p.config |= ntohl(opm->config) & config_mask;
    }

    if (opm->mask & htonl(OFPPC_PORT_DOWN))
    {
      if ((opm->config & htonl(OFPPC_PORT_DOWN)) && ((p.config & OFPPC_PORT_DOWN) == 0))
      {
        p.config |= OFPPC_PORT_DOWN;
        p.netdev->SetLineProtocolStatus(false);
      }
      else if (((opm->config & htonl(OFPPC_PORT_DOWN)) == 0) && (p.config & OFPPC_PORT_DOWN))
      {
        p.config &= ~OFPPC_PORT_DOWN;
        p.netdev->SetLineProtocolStatus(true);
      }
    }
  }

  return 0;
}

int
OpenFlowSwitchNetDevice::ReceiveStatsRequest(const void *oh)
{
  NS_LOG_FUNCTION(this);
  const ofp_stats_request *rq = (ofp_stats_request *)oh;
  size_t rq_len               = ntohs(rq->header.length);
  int    type                 = ntohs(rq->type);
  int    body_len             = rq_len - offsetof(ofp_stats_request, body);
  ofi::Stats *st              = new ofi::Stats((ofp_stats_types)type, (unsigned)body_len);

  if (st == 0)
  {
    return -EINVAL;
  }

  ofi::StatsDumpCallback cb;
  cb.done  = false;
  cb.rq    = (ofp_stats_request *)xmemdup(rq, rq_len);
  cb.s     = st;
  cb.state = 0;
  cb.swtch = this;

  if (cb.s)
  {
    int err = cb.s->DoInit(rq->body, body_len, &cb.state);

    if (err)
    {
      NS_LOG_WARN("failed initialization of stats request type " << type << ": " << strerror(-err));
      free(cb.rq);
      return err;
    }
  }

  // if (m_controller != 0)
  //   {
  //     m_controller->StartDump(&cb);
  //   }
  // else
  //   {
  //     NS_LOG_ERROR("Switch needs to be registered to a controller in order to
  // start the stats reply.");
  //   }

  return 0;
}
} // namespace ns3
#endif // NS3_OPENFLOW
