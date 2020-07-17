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

#define TTL 64
#define HEADER_SIZE 48

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "sift-header.h"
#include "ns3/address-utils.h"

namespace ns3 {
namespace sift {

NS_LOG_COMPONENT_DEFINE ("SiftHeader");

NS_OBJECT_ENSURE_REGISTERED (SiftHeader);

TypeId SiftHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::sift::SiftHeader")
    .AddConstructor<SiftHeader> ()
    .SetParent<Header> ()
  ;
  return tid;
}

TypeId SiftHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

SiftHeader::SiftHeader ()
  : m_nextHeader (17),
    // Protocol number in layer 4 (TCP=6, UDP=17, ICMP=1)
    m_optionLength (0),
    m_messageType (47),
    // Generic Routing Encapsulation GRE
    m_segmentsLeft (48),
    // Total Sift Header size
    //m_data (0),
    m_sourceId (0),
    m_sourceXLoc (0),
    // Source x Location, it doesn't change until reaches to final destination
    m_sourceYLoc (0),
    // Source y Location, it doesn't change until reaches to final destination
    m_lastsourceXLoc (0),
    // Last source X Location, this field changes in each hop
    m_lastsourceYLoc (0),
    // Last source Y Location, this field changes in each hop
    m_destId (0),
    m_destXLoc (0),
    m_destYLoc (0),
    m_headerSeqNo (0),
    m_headerTTL (TTL),
    m_sourceAddress ((Ipv4Address)"127.0.0.1"),
    m_destAddress ((Ipv4Address)"127.0.0.1")
    //m_ipv4Address (0)
{


}

SiftHeader::~SiftHeader ()
{
}

void SiftHeader::SetNextHeader (uint8_t protocol)
{
  m_nextHeader = protocol;
}

uint8_t SiftHeader::GetNextHeader () const
{
  return m_nextHeader;
}

void SiftHeader::SetOptionLength (uint8_t optionLength)
{
  m_optionLength = optionLength;
}

uint8_t SiftHeader::GetOptionLength () const
{
  return m_optionLength;
}

void SiftHeader::SetSourceAddress (Ipv4Address address)
{
  m_sourceAddress = address;
}

Ipv4Address SiftHeader::GetSourceAddress () const
{
  return m_sourceAddress;
}

void SiftHeader::SetDestAddress (Ipv4Address address)
{
  m_destAddress = address;
}

Ipv4Address SiftHeader::GetDestAddress () const
{
  return m_destAddress;
}

void SiftHeader::SetSegmentsLeft (uint8_t segmentsLeft)
{
  m_segmentsLeft = segmentsLeft;
}

uint8_t SiftHeader::GetSegmentsLeft () const
{
  return m_segmentsLeft;
}

void SiftHeader::SetMessageType (uint8_t messageType)
{
  m_messageType = messageType;
}

uint8_t SiftHeader::GetMessageType () const
{
  return m_messageType;
}
// New get and set functions for location
void SiftHeader::SetSourceXLoc (int32_t x)
{
  m_sourceXLoc = x;
}

int32_t SiftHeader::GetSourceXLoc () const
{
  return m_sourceXLoc;
}

void SiftHeader::SetSourceYLoc (int32_t y)
{
  m_sourceYLoc = y;
}

int32_t SiftHeader::GetSourceYLoc () const
{
  return m_sourceYLoc;
}

void SiftHeader::SetLastSourceXLoc (int32_t x)
{
  m_lastsourceXLoc = x;
}

int32_t SiftHeader::GetLastSourceXLoc () const
{
  return m_lastsourceXLoc;
}

void SiftHeader::SetLastSourceYLoc (int32_t y)
{
  m_lastsourceYLoc = y;
}

int32_t SiftHeader::GetLastSourceYLoc () const
{
  return m_lastsourceYLoc;
}

void SiftHeader::SetDestXLoc (int32_t x)
{
  m_destXLoc = x;
}

int32_t SiftHeader::GetDestXLoc () const
{
  return m_destXLoc;
}

void SiftHeader::SetDestYLoc (int32_t y)
{
  m_destYLoc = y;
}

int32_t SiftHeader::GetDestYLoc () const
{
  return m_destYLoc;
}
// end of set and get functions for locations
void SiftHeader::SetHeaderSeqNo (uint16_t seqNo)
{
  m_headerSeqNo = seqNo;
}

uint16_t SiftHeader::GetHeaderSeqNo () const
{
  return m_headerSeqNo;
}

void SiftHeader::SetHeaderTTL (uint8_t ttl)
{
  m_headerTTL = ttl;
}

uint8_t SiftHeader::GetHeaderTTL () const
{
  return m_headerTTL;
}

void SiftHeader::UpdateHeaderTTL ()
{
  m_headerTTL = m_headerTTL - 1;
}


void SiftHeader::SetNumberAddress (uint8_t n)
{
  m_ipv4Address.clear ();
  m_ipv4Address.assign (n, Ipv4Address (""));
}

void SiftHeader::SetNodesAddress (std::vector<Ipv4Address> ipv4Address)
{
  m_ipv4Address = ipv4Address;
  SetOptionLength (10);
}

std::vector<Ipv4Address> SiftHeader::GetNodesAddresses () const
{
  return m_ipv4Address;
}

void SiftHeader::SetNodeAddress (uint8_t index, Ipv4Address addr)
{
  m_ipv4Address.at (index) = addr;
}

Ipv4Address SiftHeader::GetNodeAddress (uint8_t index) const
{
  return m_ipv4Address.at (index);
}

uint8_t SiftHeader::GetNodeListSize () const
{
  return m_ipv4Address.size ();
}

void SiftHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << "SiftHeader::Print");
  os
    << " Next Protocol Header: " << (uint32_t)GetNextHeader ()
    << " optionLength: " << (uint32_t)GetOptionLength ()
    << " messageType: " << (uint32_t)GetMessageType ()
    << " Source IP Address: " << GetSourceAddress ()
    << " SourceXLoc= " << (uint32_t)GetSourceXLoc ()
    << " SourceYLoc= " << (uint32_t)GetSourceYLoc ()
    << " Last Source Xloc= " << (uint32_t)GetLastSourceXLoc ()
    << " Last source Yloc= " << (uint32_t)GetLastSourceYLoc ()
    << " Destination Ip Address: " << GetDestAddress ()
    << " Dest Xloc= " << (uint32_t)GetDestXLoc ()
    << " Dest YLoc= " << (uint32_t)GetDestYLoc ()
    << " Seq No= " << (uint32_t) GetHeaderSeqNo ()
    << " TTL= " << (uint32_t)GetHeaderTTL ();



}
void SiftHeader::Print ()
{

  std::cout << "Next header= " << (uint32_t)GetNextHeader () << std::endl;
  std::cout << "Option Length= " << (uint32_t)GetOptionLength () << std::endl;
  std::cout << "MessageType= " << (uint32_t)GetMessageType () << std::endl;
  std::cout << "SegmenstLeft= " << (uint32_t)GetSegmentsLeft () << std::endl;
  std::cout << "Source Address= " << GetSourceAddress () << std::endl;
  std::cout << "Destination Address= " << GetDestAddress () << std::endl;
  std::cout << "SourceXLoc= " << GetSourceXLoc () << std::endl;
  std::cout << "SourceYLoc= " << GetSourceYLoc () << std::endl;
  std::cout << "last Source Xloc= " << GetLastSourceXLoc () << std::endl;
  std::cout << "last source Yloc= " << GetLastSourceYLoc () << std::endl;
  std::cout << "dest Xloc= " << GetDestXLoc () << std::endl;
  std::cout << "dest YLoc= " << GetDestYLoc () << std::endl;
  std::cout << " Seq No= " << (uint32_t) GetHeaderSeqNo () << "\n";
  std::cout << " TTL= " << (uint32_t)GetHeaderTTL () << "\n";
}

uint32_t SiftHeader::GetSerializedSize () const
{
  return HEADER_SIZE;
}

void SiftHeader::Serialize (Buffer::Iterator start) const
{

  Buffer::Iterator i = start;

  i.WriteU8 (m_nextHeader);
  i.WriteU8 (m_optionLength);
  i.WriteU8 (m_messageType);
  i.WriteU8 (m_segmentsLeft);
  i.WriteU32 (m_sourceId);
  i.WriteU32 (m_sourceXLoc);
  i.WriteU32 (m_sourceYLoc);
  i.WriteU32 (m_lastsourceXLoc);
  i.WriteU32 (m_lastsourceYLoc);
  i.WriteU32 (m_destId);
  i.WriteU32 (m_destXLoc);
  i.WriteU32 (m_destYLoc);
  i.WriteU16 (m_headerSeqNo);
  i.WriteU8  (m_headerTTL);
  i.WriteU8  (m_pad);
  WriteTo (i, m_sourceAddress);
  WriteTo (i, m_destAddress);
}

uint32_t SiftHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_nextHeader = i.ReadU8 ();
  m_optionLength = i.ReadU8 ();
  m_messageType = i.ReadU8 ();
  m_segmentsLeft = i.ReadU8 ();
  m_sourceId = i.ReadU32 ();
  m_sourceXLoc = i.ReadU32 ();
  m_sourceYLoc = i.ReadU32 ();
  m_lastsourceXLoc = i.ReadU32 ();
  m_lastsourceYLoc = i.ReadU32 ();
  m_destId = i.ReadU32 ();
  m_destXLoc = i.ReadU32 ();
  m_destYLoc = i.ReadU32 ();
  m_headerSeqNo = i.ReadU16 ();
  m_headerTTL = i.ReadU8 ();
  m_pad = i.ReadU8 ();

  ReadFrom (i, m_sourceAddress);
  ReadFrom (i, m_destAddress);

  return GetSerializedSize ();
}

}  /* namespace sift */
}  /* namespace ns3 */
