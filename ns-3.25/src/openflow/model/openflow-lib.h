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
#ifndef OPENFLOW_LIB_H
#define OPENFLOW_LIB_H

#include <assert.h>
#include <errno.h>

// Include OFSI code
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/nstime.h"
#include "ns3/mac48-address.h"

#include <set>
#include <map>
#include <limits>

// Include main header and Vendor Extension files
#include "openflow/openflow.h"
#include "openflow/nicira-ext.h"

extern "C"
{
// Inexplicably, the OpenFlow implementation uses these two reserved words as
// member names.
#define private _private
#define delete _delete
#define list List

// Include OFSI Library files
#include "openflow/private/csum.h"
#include "openflow/private/poll-loop.h"
#include "openflow/private/rconn.h"
#include "openflow/private/stp.h"
#include "openflow/private/vconn.h"
#include "openflow/private/xtoxll.h"

// Include OFSI Switch files
#include "openflow/private/chain.h"
#include "openflow/private/table.h"
#include "openflow/private/dp_act.h"

#undef list
#undef private
#undef delete
}

namespace ns3 {
class OpenFlowSwitchNetDevice;

namespace ofi {
/**
 * \brief Port and its metadata.
 *
 * We need to store port metadata, because OpenFlow dictates that there
 * exists a type of request where the Controller asks for data about a
 * port, or multiple ports. Otherwise, we'd refer to it via Ptr<NetDevice>
 * everywhere.
 */
struct Port {
  Port() : config(0),
    state(0),
    netdev(0),
    rx_packets(0),
    tx_packets(0),
    rx_bytes(0),
    tx_bytes(0),
    tx_dropped(0)
  {}

  uint32_t               config; ///< Some subset of OFPPC_* flags.
  uint32_t               state;  ///< Some subset of OFPPS_* flags.
  Ptr<NetDevice>         netdev;
  unsigned long long int rx_packets, tx_packets;
  unsigned long long int rx_bytes, tx_bytes;
  unsigned long long int tx_dropped;
};

class Stats {
public:

  Stats(ofp_stats_types _type,
        size_t          body_len);

  /**
   * \brief Prepares to dump some kind of statistics on the connected
   * OpenFlowSwitchNetDevice.
   *
   * \param body Body member of the struct ofp_stats_request.
   * \param body_len Length of the body member.
   * \param state State information.
   * \return 0 if successful, otherwise a negative error code.
   */
  int DoInit(const void *body,
             int         body_len,
             void      **state);

  /**
   * \brief Appends statistics for OpenFlowSwitchNetDevice to 'buffer'.
   *
   * \param swtch The OpenFlowSwitchNetDevice this callback is associated with.
   * \param state State information.
   * \param buffer Buffer to append stats reply to.
   * \return 1 if it should be called again later with another buffer, 0 if it
   * is done, or a negative errno value on failure.
   */
  int DoDump(Ptr<OpenFlowSwitchNetDevice>swtch,
             void                       *state,
             ofpbuf                     *buffer);

  /**
   * \brief Cleans any state created by the init or dump functions.
   *
   * May not be implemented if no cleanup is required.
   *
   * \param state State information to clear.
   */
  void DoCleanup(void *state);

  /**
   * \brief State of the FlowStats request/reply.
   */
  struct FlowStatsState {
    int                    table_idx;
    sw_table_position      position;
    ofp_flow_stats_request rq;
    time_t                 now;

    ofpbuf *buffer;
  };

  /**
   * \brief State of the PortStats request/reply.
   */
  struct PortStatsState {
    uint32_t  num_ports; ///< Number of ports in host byte order
    uint32_t *ports;     ///< Array of ports in network byte order
  };

  ofp_stats_types type;

private:

  int DescStatsDump(void   *state,
                    ofpbuf *buffer);

  int FlowStatsInit(const void *body,
                    int         body_len,
                    void      **state);
  int (*FlowDumpCallback)(sw_flow *flow,
                          void    *state);
  int FlowStatsDump(Ptr<OpenFlowSwitchNetDevice>dp,
                    FlowStatsState             *s,
                    ofpbuf                     *buffer);

  int AggregateStatsInit(const void *body,
                         int         body_len,
                         void      **state);
  int (*AggregateDumpCallback)(sw_flow *flow,
                               void    *state);
  int AggregateStatsDump(Ptr<OpenFlowSwitchNetDevice> dp,
                         ofp_aggregate_stats_request *s,
                         ofpbuf                      *buffer);

  int TableStatsDump(Ptr<OpenFlowSwitchNetDevice>dp,
                     void                       *state,
                     ofpbuf                     *buffer);

  int PortStatsInit(const void *body,
                    int         body_len,
                    void      **state);
  int PortStatsDump(Ptr<OpenFlowSwitchNetDevice>dp,
                    PortStatsState             *s,
                    ofpbuf                     *buffer);
};

/**
 * \brief Class for handling flow table actions.
 */
struct Action {
  /**
   * \param type Type of Flow Table Action.
   * \return true if the provided type is a type of flow table action.
   */
  static bool IsValidType(ofp_action_type type);

  /**
   * \brief Validates the action on whether its data is valid or not.
   *
   * \param type Type of action to validate.
   * \param len Length of the action data.
   * \param key Matching key for the flow that is tied to this action.
   * \param ah Action's data header.
   * \return ACT_VALIDATION_OK if the action checks out, otherwise an error
   * type.
   */
  static uint16_t Validate(ofp_action_type          type,
                           size_t                   len,
                           const sw_flow_key       *key,
                           const ofp_action_header *ah);

  /**
   * \brief Executes the action.
   *
   * \param type Type of action to execute.
   * \param buffer Buffer of the Packet if it's needed for the action.
   * \param key Matching key for the flow that is tied to this action.
   * \param ah Action's data header.
   */
  static void Execute(ofp_action_type          type,
                      ofpbuf                  *buffer,
                      sw_flow_key             *key,
                      const ofp_action_header *ah);
};

/**
 * \brief Callback for a stats dump request.
 */
struct StatsDumpCallback {
  bool                        done;  ///< Whether we are done requesting stats.
  ofp_stats_request          *rq;    ///< Current stats request.
  Stats                      *s;     ///< Handler of the stats request.
  void                       *state; ///< Stats request state data.
  Ptr<OpenFlowSwitchNetDevice>swtch; ///< The switch that we're requesting data
                                     // from.
};

/**
 * \brief Packet Metadata, allows us to track the packet's metadata as it passes
 * through the switch.
 */
struct SwitchPacketMetadata {
  uint32_t   cookie;       ///< Packet cookie
  Ptr<Packet>packet;       ///< The Packet itself.
  ofpbuf    *buffer;       ///< The OpenFlow buffer as created from the
                           // Packet, with its data and headers.
  uint16_t protocolNumber; ///< Protocol type of the Packet when the Packet is
                           // received
  Address src;             ///< Source Address of the Packet when the Packet
                           // is received
  Address dst;             ///< Destination Address of the Packet when the
                           // Packet is received.
  Time ttl;                ///< Packet Buffer TTL
};

bool operator==(const SwitchPacketMetadata &a, const SwitchPacketMetadata &b);

bool operator<(const SwitchPacketMetadata &a, const SwitchPacketMetadata &b);

/**
 * \brief Executes a list of flow table actions.
 *
 * \param swtch OpenFlowSwitchNetDevice these actions are being executed on.
 * \param packet_uid Packet UID; used to fetch the packet and its metadata.
 * \param buffer The Packet OpenFlow buffer.
 * \param key The matching key for the flow tied to this list of actions.
 * \param actions A buffer of actions.
 * \param actions_len Length of actions buffer.
 * \param ignore_no_fwd If true, during port forwarding actions, ports that are
 * set to not forward are forced to forward.
 */
void ExecuteActions(Ptr<OpenFlowSwitchNetDevice>swtch,
                    uint64_t                    packet_uid,
                    ofpbuf                     *buffer,
                    sw_flow_key                *key,
                    const ofp_action_header    *actions,
                    size_t                      actions_len,
                    int                         ignore_no_fwd);

/**
 * \brief Validates a list of flow table actions.
 *
 * \param key The matching key for the flow tied to this list of actions.
 * \param actions A buffer of actions.
 * \param actions_len Length of actions buffer.
 * \return If the action list validates, ACT_VALIDATION_OK is returned.
 * Otherwise, a code for the OFPET_BAD_ACTION error type is returned.
 */
uint16_t ValidateActions(const sw_flow_key       *key,
                         const ofp_action_header *actions,
                         size_t                   actions_len);

/**
 * \internal
 *
 * Get the packet type on the buffer, which can then be used
 * to determine how to handle the buffer.
 *
 * \param buffer The packet in OpenFlow buffer format.
 * \return The packet type, as defined in the ofp_type struct.
 */
uint8_t GetPacketType(ofpbuf *buffer);

/**
 * \internal
 *
 * Takes a packet and generates an OpenFlow buffer from it, loading the
 * packet data as well as its headers.
 *
 * \param packet The packet.
 * \param src The source address.
 * \param dst The destination address.
 * \param mtu The Maximum Transmission Unit.
 * \param protocol The protocol defining the packet.
 * \return The OpenFlow Buffer created from the packet.
 */
ofpbuf* BufferFromPacket(Ptr<Packet>packet,
                         Address    src,
                         Address    dst,
                         int        mtu,
                         uint16_t   protocol);

/**
 * \internal
 *
 * Loads ethernet header info to the OpenFlow buffer.
 *
 * \param packet    The Packet itself.
 * \param buffer    Pointer to the OpenFlow buffer to be loaded.
 * \param src       Source address of the Packet.
 * \param dst       Destination address of the Packet.
 * \param protocol  Protocol type of this header.
 * \return The length of the header loaded.
 */
int LoadEthernetHeader(Ptr<Packet>packet,
                       ofpbuf    *buffer,
                       Address    src,
                       Address    dst,
                       uint16_t   protocol);

/**
 * \internal
 *
 * Loads ipv4 header info to the OpenFlow buffer.
 *
 * \param packet        The Packet itself without lower layer headers.
 * \param buffer        Pointer to the OpenFlow buffer to be loaded.
 * \param packetLength  Length of the packet received.
 * \return The length of the header loaded.
 */
int LoadIpv4Header(Ptr<Packet>packet,
                   ofpbuf    *buffer,
                   uint32_t   packetLength);

/**
 * \internal
 *
 * Loads tcp header info to the OpenFlow buffer.
 *
 * \param packet  The Packet itself without lower layer headers.
 * \param buffer  Pointer to the OpenFlow buffer to be loaded.
 * \return The length of the header loaded.
 */
int LoadTcpHeader(Ptr<Packet>packet,
                  ofpbuf    *buffer);

/**
 * \internal
 *
 * Loads udp header info to the OpenFlow buffer.
 *
 * \param packet  The Packet itself without lower layer headers.
 * \param buffer  Pointer to the OpenFlow buffer to be loaded.
 * \return The length of the header loaded.
 */
int LoadUdpHeader(Ptr<Packet>packet,
                  ofpbuf    *buffer);

/**
 * \internal
 *
 * Loads arp header info to the OpenFlow buffer.
 *
 * \param packet  The Packet itself without lower layer headers.
 * \param buffer  Pointer to the OpenFlow buffer to be loaded.
 * \return The length of the header loaded.
 */
int LoadArpHeader(Ptr<Packet>packet,
                  ofpbuf    *buffer);

/**
 * \internal
 *
 * Construct flow data from a matching key to build a flow
 * entry for adding, modifying, or deleting a flow.
 *
 * \param key           The matching key data; used to create a flow that
 * matches the packet.
 * \param buffer_id     The OpenFlow Buffer ID; used to run the actions on the
 * packet if we add or modify the flow.
 * \param command       Whether to add, modify, or delete this flow.
 * \param acts          List of actions to execute.
 * \param actions_len   Length of the actions buffer.
 * \param idle_timeout  Flow expires if left inactive for this amount of time
 *(specify OFP_FLOW_PERMANENT to disable feature).
 * \param hard_timeout  Flow expires after this amount of time (specify
 * OFP_FLOW_PERMANENT to disable feature).
 * \return Flow data that when passed to SetFlow will add, modify, or delete a
 * flow it defines.
 */
ofp_flow_mod* BuildFlow(sw_flow_key key,
                        uint32_t    buffer_id,
                        uint16_t    command,
                        void       *acts,
                        size_t      actions_len,
                        int         idle_timeout,
                        int         hard_timeout);
}
}
#endif /* OPENFLOW_LIB_H */
