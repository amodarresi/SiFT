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

#ifndef SIFT_HEADER_H
#define SIFT_HEADER_H
#define UDP_PROTOCOL 17
#define GRE_PROTOCOL 47

#include <vector>
#include <list>
#include <ostream>

#include "ns3/header.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
namespace sift {
/**
 * \class SiftHeader
 * \brief Header for Sift Routing.
 */

/**
* \ingroup sift
* \brief Sift header Format
        0     |       1       |       2       |       3
0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
| Next Header | Option Length |  Message Type |   Segment Left  |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Source X Coordinate                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Source Y Coordinate                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Last Source X Coordinate                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Last Source Y Coordinate                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination X Coordinate                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Destination Y Coordinate                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                        Source IP Address                      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                      Destination IP Address                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         Seq No              |      TTL      |       Pad       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
class SiftHeader : public Header
{
public:
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId ();
  /**
   * \brief Get the instance type ID.
   * \return instance type ID
   */
  virtual TypeId GetInstanceTypeId () const;
  /**
   * \brief Constructor.
   */
  SiftHeader ();
  /**
   * \brief Destructor.
   */
  virtual ~SiftHeader ();
  /**
   * \brief Set the "Next header" field.
   * \param nextHeader the next header number
   */
  void SetNextHeader (uint8_t protocol);
  /**
   * \brief Get the next header.
   * \return the next header number
   */
  uint8_t GetNextHeader () const;
  /**
   * brief Set the message type of the header.
   * \param message type the message type of the header
   */
  void SetOptionLength (uint8_t optionLength);
  /**
   * brief Get the message type of the header.
   * \return message type the message type of the header
   */
  uint8_t GetOptionLength () const;
  /**
   * brief Set the source id of the header.
   * \param source id the source id of the header
   */


  void SetSourceAddress (Ipv4Address address);
  /**
   * brief Get the source id of the header.
   * \return source id the source id of the header
   */
  Ipv4Address GetSourceAddress () const;

  void SetDestAddress (Ipv4Address address);
  /**
   * brief Get the dest id of the header.
   * \return dest id the dest id of the header
   */
  Ipv4Address GetDestAddress () const;
  /**
   * brief Set the segments left of the header.
   * \param segmentsLeft segment left field of the header
   */
  void SetSegmentsLeft (uint8_t segmentsLeft);
  /**
   * brief Get the segments left field of the header.
   * \return segments left field of the header
   */
  uint8_t GetSegmentsLeft () const;
  /**
   * brief Set the message type of the header.
   * \param message type the message type of the header
   */
  void SetMessageType (uint8_t messageType);
  /**
   * brief Get the message type of the header.
   * \return message type the message type of the header
   */
  uint8_t GetMessageType () const;
  /**
   * \brief Set the number of ipv4 address.
   * \param n the number of ipv4 address
   */
  void SetSourceXLoc (int32_t x);
  int32_t GetSourceXLoc () const;
  void SetSourceYLoc (int32_t y);
  int32_t GetSourceYLoc () const;
  void SetLastSourceXLoc (int32_t x);
  int32_t GetLastSourceXLoc () const;
  void SetLastSourceYLoc (int32_t y);
  int32_t GetLastSourceYLoc () const;
  void SetDestXLoc (int32_t x);
  int32_t GetDestXLoc () const;
  void SetDestYLoc (int32_t y);
  int32_t GetDestYLoc () const;
  void SetHeaderSeqNo (uint16_t seqNo);
  uint16_t GetHeaderSeqNo () const;
  void SetHeaderTTL (uint8_t ttl);
  uint8_t GetHeaderTTL () const;
  void UpdateHeaderTTL ();


  void SetNumberAddress (uint8_t n);
  /**
   * \brief Set the vector of ipv4 address
   * \param ipv4Address the vector of ipv4 address
   */
  void SetNodesAddress (std::vector<Ipv4Address> ipv4Address);
  /**
   * \brief Get the vector of ipv4 address
   * \return the vector of ipv4 address
   */
  std::vector<Ipv4Address> GetNodesAddresses () const;
  /**
   * \brief Set a Node IPv4 Address.
   * \param index the index of the IPv4 Address
   * \param addr the new IPv4 Address
   */
  void SetNodeAddress (uint8_t index, Ipv4Address addr);
  /**
   * \brief Get a Node IPv4 Address.
   * \param index the index of the IPv4 Address
   * \return the router IPv4 Address
   */
  Ipv4Address GetNodeAddress (uint8_t index) const;
  /*
 * \brief Get the node list size which is the number of ip address of the route
 * \return the node list size
 */
  uint8_t GetNodeListSize () const;
  /**
   * \brief Print some informations about the packet.
   * \param os output stream
   * \return info about this packet
   */
  virtual void Print (std::ostream &os) const;
  /**
   * \brief Get the serialized size of the packet.
   * \return size
   */
  virtual void Print ();
  /*
   * Prints header
   */
  virtual uint32_t GetSerializedSize () const;
  /**
   * \brief Serialize the packet.
   * \param start Buffer iterator
   */
  virtual void Serialize (Buffer::Iterator start) const;
  /**
   * \brief Deserialize the packet.
   * \param start Buffer iterator
   * \return size of the packet
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  /**
   * \brief The "next header" field.
   */
  uint8_t m_nextHeader;
  /**
   * \brief The type of the message.
   */
  uint8_t m_optionLength;
  /**
   * \brief The type of the message.
   */
  uint8_t m_messageType;
  /**
   * \brief The segments left to keep track of how many hops to go.
   */
  uint8_t m_segmentsLeft;
  /**
   * \brief The data of the extension.
   */
  //Buffer m_data;
  /**
   * \brief The source node id
   */
  uint32_t m_sourceId;
  /**
   * \brief The destination node id
   */
  int32_t m_sourceXLoc;
  /**
   * \ brief source x location. it doesn't change until reaches to final destination
   */
  int32_t m_sourceYLoc;
  /** 
   * brief source Y location. It doesn't change until reaches to final destination
   */
  int32_t m_lastsourceXLoc;
  /**
   * brief last source X location. it changes in each hup
   */
  int32_t m_lastsourceYLoc;
  /**
   * brief last source Y location. it changes in each hop
   */
  int32_t m_destId;
  /**
   * \brief The source node id
   */
  int32_t m_destXLoc;
  /**
   * destination X location
   */
  int32_t m_destYLoc;
  /**
   * destination Y location
   */
  uint16_t m_headerSeqNo; // SIFT Seq number
  uint8_t m_headerTTL; // SIFT TTL
  uint8_t m_pad; // 1 Byte pad to fit the header to multiple of 4
  Ipv4Address m_sourceAddress;
  /**
   * \brief The destination node id
   */
  Ipv4Address m_destAddress;
  /**
   * \brief A vector of IPv4 Address.
   */

  typedef std::vector<Ipv4Address> VectorIpv4Address_t;
  /**
   * \brief The vector of Nodes' IPv4 Address.
   */
  VectorIpv4Address_t m_ipv4Address;
};

}  // namespace sift
}  // namespace ns3

#endif /* SIFT_HEADER_H */

