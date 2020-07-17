/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Amir Modarresi
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
 * Author: Amir Modarresi   <amodarresi@ittc.ku.edu>
 *
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */

#ifndef SIFT_ROUTING_H
#define SIFT_ROUTING_H

#include <map>
#include <list>
#include <vector>
#include <stdint.h>
#include <cassert>
#include <sys/types.h>

#include "sift-repository.h"
#include "ns3/callback.h"
#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/buffer.h"
#include "ns3/packet.h"
#include "ns3/ipv4.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/random-variable-stream.h"
#include "ns3/ipv4-route.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/socket.h"
#include "ns3/event-garbage-collector.h"
#include "ns3/test.h"
#include "sift-header.h"
#include "sift-geo.h"

namespace ns3 {

class Packet;
class Node;
class Ipv4;
class Ipv4Address;
class Ipv4Header;
class Ipv4Interface;
class Ipv4L3Protocol;
class Time;

namespace sift {

/**
 * \class SiftRouting
 * \brief This header file defines SiftRouting class that implements sift routing
 */
class SiftRouting : public IpL4Protocol
{
public:
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId ();
  /**
    * \brief Define the sift protocol number.
    */
  static const uint8_t PROT_NUMBER;
  /**
   * \brief Constructor.
   */
  SiftRouting ();
  /**
   * \brief Destructor.
   */
  virtual ~SiftRouting ();
  /**
   * \brief Get the node.
   * \return the node
   */
  void  PrintSiftHeader (SiftHeader, GeographicTuple, Ptr<OutputStreamWrapper>) const;
  Ptr<Node> GetNode () const;
  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  /**
    * \brief Get the node with give ip address.
    * \return the node associated with the ip address
    */
  Ptr<Node> GetNodeWithAddress (Ipv4Address ipv4Address);
  /**
    * \brief Print the route vector.
    */
  void PrintVector (std::vector<Ipv4Address>& vec);
  /**
    * \brief Get the sift protocol number.
    * \return protocol number
    */
  int GetProtocolNumber (void) const;
  /**
   * \brief Set the route to use for data packets
   * \return the route
   * \used by the option headers when sending data/control packets
   */
  Ptr<Ipv4Route> SetRoute (Ipv4Address nextHop, Ipv4Address srcAddress);
  /**
   * \brief This function is called by higher layer protocol when sending packets
   */
  void Send (Ptr<Packet> packet, Ipv4Address source,
             Ipv4Address destination, uint8_t protocol, Ptr<Ipv4Route> route);
  /**
   * \brief This function is called by when really sending out the packet
   */
  void SendPacket (Ptr<Packet> packet, Ipv4Address source, Ipv4Address nextHop, uint8_t protocol);
  /**
   * \brief Forward the packet using the route saved in the source route option header
   */
  void ForwardPacket (Ptr<Packet> packet,
                      Ipv4Address source,
                      uint8_t protocol);
  void ScheduleTimer (Ptr<Packet> packet, Time timer, Ipv4Address source, uint8_t protocol);
  /**
   * \param p packet to forward up
   * \param header IPv4 Header information
   * \param incomingInterface the Ipv4Interface on which the packet arrived
   *
   * Called from lower-level layers to send the packet up
   * in the stack.
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                               Ipv4Header const &header,
                                               Ptr<Ipv4Interface> incomingInterface);

  /**
   * \param p packet to forward up
   * \param header IPv6 Header information
   * \param incomingInterface the Ipv6Interface on which the packet arrived
   *
   * Called from lower-level layers to send the packet up
   * in the stack.  Not implemented (IPv6).
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                               Ipv6Header const &header,
                                               Ptr<Ipv6Interface> incomingInterface);

  void SetDownTarget (IpL4Protocol::DownTargetCallback callback);
  void SetDownTarget6 (IpL4Protocol::DownTargetCallback6 callback);
  IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
  IpL4Protocol::DownTargetCallback6 GetDownTarget6 (void) const;
  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);
  void AddGeographicTuple (const GeographicTuple &tuple);
  SiftGeo m_state;

  void AddNode (Ptr<Node> node);

  uint32_t GetNodesSize (void);

  void  CancelTimer (Ptr<Packet> p);

  //void SiftRouting::CheckParameters (Ipv4Header const ip, Ipv4Header ipHeader);

  void PrintReceiveBuffer (void);
  std::map<Ptr<Packet>, Time> GetReceiveBuffer (void);

  uint16_t GetSeqNo ();
  void SetSeqNo (uint16_t sno);



protected:
  /*
 *    * This function will notify other components connected to the node that a new stack member is now connected
 *       * This will be used to notify Layer 3 protocol of layer 4 protocol stack to connect them together.
 *          */
  virtual void NotifyNewAggregate ();
  /**
   * \brief Drop trace callback.
   */
  virtual void DoDispose (void);
  /**
   * The trace for drop, receive and send data packets
   */
  TracedCallback<Ptr<const Packet> > m_dropTrace;
  TracedCallback <const SiftHeader &> m_txPacketTrace;

private:
  void Start ();

  Ptr<Ipv4L3Protocol> m_ipv4;                            ///< Ipv4l3Protocol

  Ptr<Ipv4Route> m_ipv4Route;                            ///< Ipv4 Route

  Ptr<Ipv4> m_ip;                                        ///< The ip ptr

  Ptr<Node> m_node;                                      ///< The node ptr

  Ipv4Address m_mainAddress;                             ///< Our own Ip address

  Ipv4Address m_broadcast;                                                               ///< The broadcast address

  IpL4Protocol::DownTargetCallback m_downTarget;         ///< The callback for down layer

  Ptr<UniformRandomVariable> m_uniformRandomVariable;    ///< Provides uniform random variables.
  uint16_t m_seqNo;


  std::map<Ptr<Packet>, Timer> m_timer; ///< The timer to provide timing for each packet, the key is the packet itself TODO
  // std::map<Ptr<Packet>, Timer> m_sentPacket;
  std::map<Ptr<Packet>, Time> m_buffer;                                 // Keeps received packets to control duplicate ones

private:
  uint16_t GetIDfromIP (Ipv4Address address);
  Ipv4Address GetIPfromID (const uint32_t id);                ///< node id -> IP
  GeographicSet m_geoSet;                                     ///< the geographical set
  std::vector<Ptr<Node> > m_nodePtrs;
};
}  /* namespace sift */
}  /* namespace ns3 */

#endif /* SIFT_ROUTING_H */
