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

#include "openflow-lib.h"
#include "ns3/openflow-switch-net-device.h"
#include "ns3/tcp-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/log.h"
#include "ns3/breakpoint.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("OpenFlowInterface");

namespace ofi {
Stats::Stats(ofp_stats_types _type, size_t body_len)
{
  type = _type;
  size_t min_body = 0, max_body = 0;

  switch (type)
  {
  case OFPST_DESC:
    break;

  case OFPST_FLOW:
    min_body = max_body = sizeof(ofp_flow_stats_request);
    break;

  case OFPST_AGGREGATE:
    min_body = max_body = sizeof(ofp_aggregate_stats_request);
    break;

  case OFPST_TABLE:
    break;

  case OFPST_PORT:
    min_body = 0;
    max_body = std::numeric_limits<size_t>::max(); // Not sure about this one.
                                                   // This would guarantee that
                                                   // the body_len is always
                                                   // acceptable.
    break;

  default:
    NS_LOG_ERROR("received stats request of unknown type " << type);
    return; // -EINVAL;
  }

  if (((min_body != 0) || (max_body != 0)) && ((body_len < min_body) || (body_len > max_body)))
  {
    NS_LOG_ERROR("stats request type " << type << " with bad body length " << body_len);
    return; // -EINVAL;
  }
}

int
Stats::DoInit(const void *body, int body_len, void **state)
{
  switch (type)
  {
  case OFPST_DESC:
    return 0;

  case OFPST_FLOW:
    return FlowStatsInit(body, body_len, state);

  case OFPST_AGGREGATE:
    return AggregateStatsInit(body, body_len, state);

  case OFPST_TABLE:
    return 0;

  case OFPST_PORT:
    return PortStatsInit(body, body_len, state);

  case OFPST_VENDOR:
    return 0;
  }

  return 0;
}

int
Stats::DoDump(Ptr<OpenFlowSwitchNetDevice>swtch, void *state, ofpbuf *buffer)
{
  switch (type)
  {
  case OFPST_DESC:
    return DescStatsDump(state, buffer);

  case OFPST_FLOW:
    return FlowStatsDump(swtch, (FlowStatsState *)state, buffer);

  case OFPST_AGGREGATE:
    return AggregateStatsDump(swtch, (ofp_aggregate_stats_request *)state, buffer);

  case OFPST_TABLE:
    return TableStatsDump(swtch, state, buffer);

  case OFPST_PORT:
    return PortStatsDump(swtch, (PortStatsState *)state, buffer);

  case OFPST_VENDOR:
    return 0;
  }

  return 0;
}

void
Stats::DoCleanup(void *state)
{
  switch (type)
  {
  case OFPST_DESC:
    break;

  case OFPST_FLOW:
    free((FlowStatsState *)state);
    break;

  case OFPST_AGGREGATE:
    free((ofp_aggregate_stats_request *)state);
    break;

  case OFPST_TABLE:
    break;

  case OFPST_PORT:
    free(((PortStatsState *)state)->ports);
    free((PortStatsState *)state);
    break;

  case OFPST_VENDOR:
    break;
  }
}

int
Stats::DescStatsDump(void *state, ofpbuf *buffer)
{
  ofp_desc_stats *ods = (ofp_desc_stats *)ofpbuf_put_zeros(buffer, sizeof *ods);

  strncpy(ods->mfr_desc,   OpenFlowSwitchNetDevice::GetManufacturerDescription(), sizeof ods->mfr_desc);
  strncpy(ods->hw_desc,    OpenFlowSwitchNetDevice::GetHardwareDescription(),     sizeof ods->hw_desc);
  strncpy(ods->sw_desc,    OpenFlowSwitchNetDevice::GetSoftwareDescription(),     sizeof ods->sw_desc);
  strncpy(ods->serial_num, OpenFlowSwitchNetDevice::GetSerialNumber(),            sizeof ods->serial_num);
  return 0;
}

#define MAX_FLOW_STATS_BYTES 4096

int
Stats::FlowStatsInit(const void *body, int body_len, void **state)
{
  const ofp_flow_stats_request *fsr = (ofp_flow_stats_request *)body;
  FlowStatsState *s                 = (FlowStatsState *)xmalloc(sizeof *s);

  s->table_idx = fsr->table_id == 0xff ? 0 : fsr->table_id;
  memset(&s->position, 0, sizeof s->position);
  s->rq  = *fsr;
  *state = s;
  return 0;
}

int
Stats_FlowDumpCallback(sw_flow *flow, void *state)
{
  Stats::FlowStatsState *s = (Stats::FlowStatsState *)state;

  // Fill Flow Stats
  ofp_flow_stats *ofs;
  int length = sizeof *ofs + flow->sf_acts->actions_len;

  ofs                  = (ofp_flow_stats *)ofpbuf_put_zeros(s->buffer, length);
  ofs->length          = htons(length);
  ofs->table_id        = s->table_idx;
  ofs->match.wildcards = htonl(flow->key.wildcards);
  ofs->match.in_port   = flow->key.flow.in_port;
  memcpy(ofs->match.dl_src, flow->key.flow.dl_src, ETH_ADDR_LEN);
  memcpy(ofs->match.dl_dst, flow->key.flow.dl_dst, ETH_ADDR_LEN);
  ofs->match.dl_vlan  = flow->key.flow.dl_vlan;
  ofs->match.dl_type  = flow->key.flow.dl_type;
  ofs->match.nw_src   = flow->key.flow.nw_src;
  ofs->match.nw_dst   = flow->key.flow.nw_dst;
  ofs->match.nw_proto = flow->key.flow.nw_proto;
  ofs->match.tp_src   = flow->key.flow.tp_src;
  ofs->match.tp_dst   = flow->key.flow.tp_dst;
  ofs->duration       = htonl(s->now - flow->created);
  ofs->priority       = htons(flow->priority);
  ofs->idle_timeout   = htons(flow->idle_timeout);
  ofs->hard_timeout   = htons(flow->hard_timeout);
  ofs->packet_count   = htonll(flow->packet_count);
  ofs->byte_count     = htonll(flow->byte_count);
  memcpy(ofs->actions, flow->sf_acts->actions, flow->sf_acts->actions_len);

  return s->buffer->size >= MAX_FLOW_STATS_BYTES;
}

int
Stats::FlowStatsDump(Ptr<OpenFlowSwitchNetDevice>swtch, FlowStatsState *s, ofpbuf *buffer)
{
  sw_flow_key match_key;

  flow_extract_match(&match_key, &s->rq.match);

  s->buffer = buffer;
  s->now    = time_now();

  while (s->table_idx < swtch->GetChain()->n_tables
         && (s->rq.table_id == 0xff || s->rq.table_id == s->table_idx))
  {
    sw_table *table = swtch->GetChain()->tables[s->table_idx];

    if (table->iterate(table, &match_key, s->rq.out_port, &s->position, Stats::FlowDumpCallback, s))
    {
      break;
    }

    s->table_idx++;
    memset(&s->position, 0, sizeof s->position);
  }
  return s->buffer->size >= MAX_FLOW_STATS_BYTES;
}

int
Stats::AggregateStatsInit(const void *body, int body_len, void **state)
{
  // ofp_aggregate_stats_request *s = (ofp_aggregate_stats_request*)body;
  *state = (ofp_aggregate_stats_request *)body;
  return 0;
}

int
Stats_AggregateDumpCallback(sw_flow *flow, void *state)
{
  ofp_aggregate_stats_reply *s = (ofp_aggregate_stats_reply *)state;

  s->packet_count += flow->packet_count;
  s->byte_count   += flow->byte_count;
  s->flow_count++;
  return 0;
}

int
Stats::AggregateStatsDump(Ptr<OpenFlowSwitchNetDevice>swtch, ofp_aggregate_stats_request *s, ofpbuf *buffer)
{
  ofp_aggregate_stats_request *rq  = s;
  ofp_aggregate_stats_reply   *rpy = (ofp_aggregate_stats_reply *)ofpbuf_put_zeros(buffer, sizeof *rpy);
  sw_flow_key match_key;

  flow_extract_match(&match_key, &rq->match);
  int table_idx = rq->table_id == 0xff ? 0 : rq->table_id;

  sw_table_position position;
  memset(&position, 0, sizeof position);

  while (table_idx < swtch->GetChain()->n_tables
         && (rq->table_id == 0xff || rq->table_id == table_idx))
  {
    sw_table *table = swtch->GetChain()->tables[table_idx];
    int error       = table->iterate(table, &match_key, rq->out_port, &position, Stats::AggregateDumpCallback, rpy);

    if (error)
    {
      return error;
    }

    table_idx++;
    memset(&position, 0, sizeof position);
  }

  rpy->packet_count = htonll(rpy->packet_count);
  rpy->byte_count   = htonll(rpy->byte_count);
  rpy->flow_count   = htonl(rpy->flow_count);
  return 0;
}

int
Stats::TableStatsDump(Ptr<OpenFlowSwitchNetDevice>swtch, void *state, ofpbuf *buffer)
{
  sw_chain *ft = swtch->GetChain();

  for (int i = 0; i < ft->n_tables; i++)
  {
    ofp_table_stats *ots = (ofp_table_stats *)ofpbuf_put_zeros(buffer, sizeof *ots);
    sw_table_stats   stats;
    ft->tables[i]->stats(ft->tables[i], &stats);
    strncpy(ots->name, stats.name, sizeof ots->name);
    ots->table_id      = i;
    ots->wildcards     = htonl(stats.wildcards);
    ots->max_entries   = htonl(stats.max_flows);
    ots->active_count  = htonl(stats.n_flows);
    ots->lookup_count  = htonll(stats.n_lookup);
    ots->matched_count = htonll(stats.n_matched);
  }
  return 0;
}

int
Stats::PortStatsInit(const void *body, int body_len, void **state)
{
  PortStatsState *s = (PortStatsState *)xmalloc(sizeof *s);

  // the body contains a list of port numbers
  s->ports = (uint32_t *)xmalloc(body_len);
  memcpy(s->ports, body, body_len);
  s->num_ports = body_len / sizeof(uint32_t);

  *state = s;
  return 0;
}

int
Stats::PortStatsDump(Ptr<OpenFlowSwitchNetDevice>swtch, PortStatsState *s, ofpbuf *buffer)
{
  ofp_port_stats *ops;
  uint32_t port;

  // port stats are different depending on whether port is physical or virtual
  for (size_t i = 0; i < s->num_ports; i++)
  {
    port = ntohl(s->ports[i]);

    // physical port?
    if (port <= OFPP_MAX)
    {
      Port p = swtch->GetSwitchPort(port);

      if (p.netdev == 0)
      {
        continue;
      }

      ops               = (ofp_port_stats *)ofpbuf_put_zeros(buffer, sizeof *ops);
      ops->port_no      = htonl(swtch->GetSwitchPortIndex(p));
      ops->rx_packets   = htonll(p.rx_packets);
      ops->tx_packets   = htonll(p.tx_packets);
      ops->rx_bytes     = htonll(p.rx_bytes);
      ops->tx_bytes     = htonll(p.tx_bytes);
      ops->rx_dropped   = htonll(-1);
      ops->tx_dropped   = htonll(p.tx_dropped);
      ops->rx_errors    = htonll(-1);
      ops->tx_errors    = htonll(-1);
      ops->rx_frame_err = htonll(-1);
      ops->rx_over_err  = htonll(-1);
      ops->rx_crc_err   = htonll(-1);
      ops->collisions   = htonll(-1);
      ops++;
    }
  }
  return 0;
}

bool
Action::IsValidType(ofp_action_type type)
{
  switch (type)
  {
  case OFPAT_OUTPUT:
  case OFPAT_SET_VLAN_VID:
  case OFPAT_SET_VLAN_PCP:
  case OFPAT_STRIP_VLAN:
  case OFPAT_SET_DL_SRC:
  case OFPAT_SET_DL_DST:
  case OFPAT_SET_NW_SRC:
  case OFPAT_SET_NW_DST:
  case OFPAT_SET_TP_SRC:
  case OFPAT_SET_TP_DST:
    return true;

  default:
    return false;
  }
}

uint16_t
Action::Validate(ofp_action_type type, size_t len, const sw_flow_key *key, const ofp_action_header *ah)
{
  size_t size = 0;

  switch (type)
  {
  case OFPAT_OUTPUT:
  {
    if (len != sizeof(ofp_action_output))
    {
      NS_LOG_WARN("Action::Validate OFPBAC_BAD_LEN");
      return OFPBAC_BAD_LEN;
    }

    ofp_action_output *oa = (ofp_action_output *)ah;

    if ((oa->port == htons(OFPP_NONE)) ||
        (!(key->wildcards & OFPFW_IN_PORT) && (oa->port == key->flow.in_port)))
      return OFPBAC_BAD_OUT_PORT;

    return ACT_VALIDATION_OK;
  }

  case OFPAT_SET_VLAN_VID:
    size = sizeof(ofp_action_vlan_vid);
    break;

  case OFPAT_SET_VLAN_PCP:
    size = sizeof(ofp_action_vlan_pcp);
    break;

  case OFPAT_STRIP_VLAN:
    size = sizeof(ofp_action_header);
    break;

  case OFPAT_SET_DL_SRC:
  case OFPAT_SET_DL_DST:
    size = sizeof(ofp_action_dl_addr);
    break;

  case OFPAT_SET_NW_SRC:
  case OFPAT_SET_NW_DST:
    size = sizeof(ofp_action_nw_addr);
    break;

  case OFPAT_SET_TP_SRC:
  case OFPAT_SET_TP_DST:
    size = sizeof(ofp_action_tp_port);
    break;

  default:
    break;
  }

  if (len != size)
  {
    return OFPBAC_BAD_LEN;
  }
  return ACT_VALIDATION_OK;
}

void
Action::Execute(ofp_action_type type, ofpbuf *buffer, sw_flow_key *key, const ofp_action_header *ah)
{
  switch (type)
  {
  case OFPAT_OUTPUT:
    break;

  case OFPAT_SET_VLAN_VID:
    set_vlan_vid(buffer, key, ah);
    break;

  case OFPAT_SET_VLAN_PCP:
    set_vlan_pcp(buffer, key, ah);
    break;

  case OFPAT_STRIP_VLAN:
    strip_vlan(buffer, key, ah);
    break;

  case OFPAT_SET_DL_SRC:
  case OFPAT_SET_DL_DST:
    set_dl_addr(buffer, key, ah);
    break;

  case OFPAT_SET_NW_SRC:
  case OFPAT_SET_NW_DST:
    set_nw_addr(buffer, key, ah);
    break;

  case OFPAT_SET_TP_SRC:
  case OFPAT_SET_TP_DST:
    set_tp_port(buffer, key, ah);
    break;

  default:
    break;
  }
}

void
ExecuteActions(Ptr<OpenFlowSwitchNetDevice>swtch,
               uint64_t                    packet_uid,
               ofpbuf                     *buffer,
               sw_flow_key                *key,
               const ofp_action_header    *actions,
               size_t                      actions_len,
               int                         ignore_no_fwd)
{
  NS_LOG_FUNCTION(packet_uid);

  /* Every output action needs a separate clone of 'buffer', but the common
   * case is just a single output action, so that doing a clone and then
   * freeing the original buffer is wasteful.  So the following code is
   * slightly obscure just to avoid that. */
  int prev_port;
  size_t   max_len = 0;                 // Initialze to make compiler happy
  uint16_t in_port = key->flow.in_port; // ntohs(key->flow.in_port);
  uint8_t *p       = (uint8_t *)actions;

  prev_port = -1;

  if (actions_len == 0)
  {
    NS_LOG_INFO("No actions set to this flow. Dropping packet.");
    return;
  }

  /* The action list was already validated, so we can be a bit looser
   * in our sanity-checking. */
  while (actions_len > 0)
  {
    ofp_action_header *ah = (ofp_action_header *)p;
    size_t len            = htons(ah->len);

    if (prev_port != -1)
    {
      swtch->DoOutput(packet_uid, in_port, max_len, prev_port, ignore_no_fwd);
      prev_port = -1;
    }

    if (ah->type == htons(OFPAT_OUTPUT))
    {
      ofp_action_output *oa = (ofp_action_output *)p;

      // port is now 32-bits
      prev_port = oa->port; // ntohl(oa->port);
      // prev_port = ntohs(oa->port);
      max_len = ntohs(oa->max_len);
    }
    else
    {
      uint16_t type = ntohs(ah->type);

      if (Action::IsValidType((ofp_action_type)type)) // Execute a built-in
                                                      // OpenFlow action against
                                                      // 'buffer'.
      {
        Action::Execute((ofp_action_type)type, buffer, key, ah);
      }
    }

    p           += len;
    actions_len -= len;
  }

  if (prev_port != -1)
  {
    swtch->DoOutput(packet_uid, in_port, max_len, prev_port, ignore_no_fwd);
  }
}

uint16_t
ValidateActions(const sw_flow_key *key, const ofp_action_header *actions, size_t actions_len)
{
  uint8_t *p = (uint8_t *)actions;
  int err;

  while (actions_len >= sizeof(ofp_action_header))
  {
    ofp_action_header *ah = (ofp_action_header *)p;
    size_t   len          = ntohs(ah->len);
    uint16_t type;

    /* Make there's enough remaining data for the specified length
     * and that the action length is a multiple of 64 bits. */
    if ((actions_len < len) || ((len % 8) != 0))
    {
      return OFPBAC_BAD_LEN;
    }

    type = ntohs(ah->type);

    if (Action::IsValidType((ofp_action_type)type)) // Validate built-in
                                                    // OpenFlow actions.
    {
      err = Action::Validate((ofp_action_type)type, len, key, ah);

      if (err != ACT_VALIDATION_OK)
      {
        return err;
      }
    }
    else
    {
      return OFPBAC_BAD_TYPE;
    }

    p           += len;
    actions_len -= len;
  }

  // Check if there's any trailing garbage.
  if (actions_len != 0)
  {
    return OFPBAC_BAD_LEN;
  }

  return ACT_VALIDATION_OK;
}

uint8_t
GetPacketType(ofpbuf *buffer)
{
  ofp_header *hdr  = (ofp_header *)ofpbuf_try_pull(buffer, sizeof(ofp_header));
  uint8_t     type = hdr->type;

  ofpbuf_push_uninit(buffer, sizeof(ofp_header));
  return type;
}

ofpbuf *
BufferFromPacket(Ptr<Packet>packet, Address src, Address dst,
                 int mtu, uint16_t protocol)
{
  /*
   * Allocate buffer with some headroom to add headers in forwarding
   * to the controller or adding a vlan tag, plus an extra 2 bytes to
   * allow IP headers to be aligned on a 4-byte boundary.
   */
  Ptr<Packet> copyPacket  = packet->Copy();
  const int   headroom    = 128 + 2;
  const int   hard_header = VLAN_ETH_HEADER_LEN;
  uint8_t     l4_protocol;
  ofpbuf     *buffer = ofpbuf_new(headroom + hard_header + mtu);
  assert(buffer != 0);

  buffer->data = (char *)buffer->data + headroom + hard_header;

  int l2_length = 0, l3_length = 0, l4_length = 0;

  // Load headers
  l2_length = LoadEthernetHeader(copyPacket, buffer, src, dst, protocol);

  // We have to wrap this because PeekHeader has an assert fail if we check
  // for an Ipv4Header that isn't there.
  if (protocol == Ipv4L3Protocol::PROT_NUMBER)
  {
    uint32_t packetLength = packet->GetSize();
    l3_length = LoadIpv4Header(copyPacket, buffer, packetLength);

    if (l3_length != -1)
    {
      l4_protocol = ((ip_header *)buffer->l3)->ip_proto;

      if (l4_protocol == TcpL4Protocol::PROT_NUMBER)
        l4_length = LoadTcpHeader(copyPacket, buffer);
      else if (l4_protocol == UdpL4Protocol::PROT_NUMBER)
        l4_length = LoadUdpHeader(copyPacket, buffer);
    }
  }
  else
    l3_length = LoadArpHeader(copyPacket, buffer);

  // Load Packet data into buffer data

  copyPacket->CopyData((uint8_t *)buffer->data, copyPacket->GetSize());
  buffer->size += copyPacket->GetSize();

  if (buffer->l4)
  {
    ofpbuf_push(buffer, buffer->l4, l4_length);

    if (l4_protocol == TcpL4Protocol::PROT_NUMBER)
      delete (tcp_header *)buffer->l4;
    else if (l4_protocol == UdpL4Protocol::PROT_NUMBER)
      delete (udp_header *)buffer->l4;
  }

  if (buffer->l3)
  {
    ofpbuf_push(buffer, buffer->l3, l3_length);
    delete (ip_header *)buffer->l3;
  }

  if (buffer->l2)
  {
    ofpbuf_push(buffer, buffer->l2, l2_length);
    delete (eth_header *)buffer->l2;
  }

  return buffer;
}

int
LoadEthernetHeader(Ptr<Packet>packet, ofpbuf *buffer, Address src,
                   Address dst, uint16_t protocol)
{
  EthernetHeader eth_hd;

  if (packet->PeekHeader(eth_hd))
  {
    buffer->l2 = new eth_header;
    eth_header *eth_h = (eth_header *)buffer->l2;
    dst.CopyTo(eth_h->eth_dst);        // Destination Mac Address
    src.CopyTo(eth_h->eth_src);        // Source Mac Address
    eth_h->eth_type = htons(protocol); // Ether Type

    NS_LOG_INFO("Parsed EthernetHeader");
    return ETH_HEADER_LEN;
  }

  NS_LOG_ERROR("EthernetHeader could not be parsed!");
  return -1;
}

int
LoadIpv4Header(Ptr<Packet>packet, ofpbuf *buffer, uint32_t packetLength)
{
  Ipv4Header ip_hd;

  if (packet->PeekHeader(ip_hd))
  {
    packet->RemoveHeader(ip_hd);

    buffer->l3 = new ip_header;
    ip_header *ip_h = (ip_header *)buffer->l3;
    ip_h->ip_ihl_ver  = IP_IHL_VER(5, IP_VERSION);
    ip_h->ip_tos      = ip_hd.GetTos();
    ip_h->ip_tot_len  = htons(packetLength);
    ip_h->ip_id       = ip_hd.GetIdentification();
    ip_h->ip_frag_off = ip_hd.GetFragmentOffset();
    ip_h->ip_ttl      = ip_hd.GetTtl();
    ip_h->ip_proto    = ip_hd.GetProtocol();
    ip_h->ip_src      = htonl(ip_hd.GetSource().Get());
    ip_h->ip_dst      = htonl(ip_hd.GetDestination().Get());
    ip_h->ip_csum     = csum(&ip_h, sizeof ip_h);

    NS_LOG_INFO("Parsed Ipv4Header");
    return IP_HEADER_LEN;
  }

  return -1;
}

int
LoadTcpHeader(Ptr<Packet>packet, ofpbuf *buffer)
{
  TcpHeader tcp_hd;

  if (packet->PeekHeader(tcp_hd))
  {
    packet->RemoveHeader(tcp_hd);

    buffer->l4 = new tcp_header;
    tcp_header *tcp_h = (tcp_header *)buffer->l4;
    tcp_h->tcp_src   = tcp_hd.GetSourcePort();
    tcp_h->tcp_dst   = tcp_hd.GetDestinationPort();
    tcp_h->tcp_seq   = tcp_hd.GetSequenceNumber().GetValue();
    tcp_h->tcp_ack   = tcp_hd.GetAckNumber().GetValue();
    tcp_h->tcp_ctl   = ((TCP_HEADER_LEN/4) << 12 | TCP_FLAGS(tcp_hd.GetFlags()));
    tcp_h->tcp_winsz = tcp_hd.GetWindowSize();
    tcp_h->tcp_urg   = tcp_hd.GetUrgentPointer();
    tcp_h->tcp_csum  = csum(&tcp_h, sizeof tcp_h);

    NS_LOG_INFO("Parsed TcpHeader");
    return TCP_HEADER_LEN;
  }

  return -1;
}

int
LoadUdpHeader(Ptr<Packet>packet, ofpbuf *buffer)
{
  UdpHeader udp_hd;

  if (packet->PeekHeader(udp_hd))
  {
    packet->RemoveHeader(udp_hd);

    buffer->l4 = new udp_header;
    udp_header *udp_h = (udp_header *)buffer->l4;
    udp_h->udp_src = htons(udp_hd.GetSourcePort());      // Source Port
    udp_h->udp_dst = htons(udp_hd.GetDestinationPort()); // Destination Port
    udp_h->udp_len = htons(UDP_HEADER_LEN + packet->GetSize());

    ip_header *ip_h     = (ip_header *)buffer->l3;
    uint32_t   udp_csum = csum_add32(0, ip_h->ip_src);
    udp_csum        = csum_add32(udp_csum, ip_h->ip_dst);
    udp_csum        = csum_add16(udp_csum, IP_TYPE_UDP << 8);
    udp_csum        = csum_add16(udp_csum, udp_h->udp_len);
    udp_csum        = csum_continue(udp_csum, udp_h, sizeof udp_h);
    udp_h->udp_csum = csum_finish(csum_continue(udp_csum, buffer->data, buffer->size));

    NS_LOG_INFO("Parsed UdpHeader");
    return UDP_HEADER_LEN;
  }

  return -1;
}

int
LoadArpHeader(Ptr<Packet>packet, ofpbuf *buffer)
{
  // ARP Packet; the underlying OpenFlow header isn't used to match,
  // so this is probably superfluous.
  ArpHeader arp_hd;

  if (packet->PeekHeader(arp_hd))
  {
    packet->RemoveHeader(arp_hd);

    buffer->l3 = new arp_eth_header;
    arp_eth_header *arp_h = (arp_eth_header *)buffer->l3;
    arp_h->ar_hrd = htons(ARP_HRD_ETHERNET);
    arp_h->ar_pro = htons(ARP_PRO_IP);
    arp_h->ar_op  = htons(arp_hd.m_type);
    arp_hd.GetDestinationHardwareAddress().CopyTo(arp_h->ar_tha);
    arp_hd.GetSourceHardwareAddress().CopyTo(arp_h->ar_sha);
    arp_h->ar_tpa = htonl(arp_hd.GetDestinationIpv4Address().Get());
    arp_h->ar_spa = htonl(arp_hd.GetSourceIpv4Address().Get());
    arp_h->ar_hln = sizeof arp_h->ar_tha;
    arp_h->ar_pln = sizeof arp_h->ar_tpa;

    NS_LOG_INFO("Parsed ArpHeader");
    return ARP_ETH_HEADER_LEN;
  }

  return -1;
}

ofp_flow_mod *
BuildFlow(sw_flow_key key, uint32_t buffer_id, uint16_t command,
          void *acts, size_t actions_len, int idle_timeout, int hard_timeout)
{
  ofp_flow_mod *ofm = (ofp_flow_mod *)malloc(sizeof(ofp_flow_mod) + actions_len);

  ofm->header.version = OFP_VERSION;
  ofm->header.type    = OFPT_FLOW_MOD;
  ofm->header.length  = htons(sizeof(ofp_flow_mod) + actions_len);
  ofm->command        = htons(command);
  ofm->idle_timeout   = htons(idle_timeout);
  ofm->hard_timeout   = htons(hard_timeout);
  ofm->buffer_id      = htonl(buffer_id);
  ofm->priority       = OFP_DEFAULT_PRIORITY;
  memcpy(ofm->actions, acts, actions_len);

  ofm->match.wildcards = key.wildcards;
  ofm->match.in_port   = key.flow.in_port;

  memcpy(ofm->match.dl_src, key.flow.dl_src, sizeof ofm->match.dl_src);
  memcpy(ofm->match.dl_dst, key.flow.dl_dst, sizeof ofm->match.dl_dst);

  ofm->match.dl_vlan  = key.flow.dl_vlan;
  ofm->match.dl_type  = key.flow.dl_type;
  ofm->match.nw_proto = key.flow.nw_proto;
  ofm->match.nw_src   = key.flow.nw_src;
  ofm->match.nw_dst   = key.flow.nw_dst;
  ofm->match.tp_src   = key.flow.tp_src;
  ofm->match.tp_dst   = key.flow.tp_dst;

  return ofm;
}

bool
operator==(const SwitchPacketMetadata& a, const SwitchPacketMetadata& b)
{
  return a.cookie == b.cookie;
}

bool
operator<(const SwitchPacketMetadata& a, const SwitchPacketMetadata& b)
{
  return a.ttl < b.ttl;
}
}
}
#endif // NS3_OPENFLOW
