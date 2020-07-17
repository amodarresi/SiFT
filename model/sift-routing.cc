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

#define NS_LOG_APPEND_CONTEXT                                   \
  if (GetObject<Node> ()) { std::clog << "[node " << GetObject<Node> ()->GetId () << "] "; }
#define ICMP_PROTOCOL 1
#define TCP_PROTOCOL 6
#define UDP_PROTOCOL 17
#define GRE_PROTOCOL 47
#define MAX_SEQ_NO 65536
#define TTL 64
#define ALPHA 0.01
static const double TRANSMISSION_TIME=0.000047;

#include <list>
#include <ctime>
#include <map>
#include <limits>
#include <algorithm>
#include <iostream>
#include <float.h>
//#include "ns3/config.h"
#include "ns3/enum.h"
#include "ns3/string.h"
#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/fatal-error.h"
#include "ns3/assert.h"
#include "ns3/uinteger.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/boolean.h"
#include "ns3/node-list.h"
#include "ns3/double.h"
#include "ns3/pointer.h"
#include "ns3/object-vector.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv4-route.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/icmpv4-l4-protocol.h"
#include "ns3/wifi-net-device.h"
#include "ns3/inet-socket-address.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/llc-snap-header.h"
#include "ns3/arp-header.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/adhoc-wifi-mac.h"

#include "sift-routing.h"
#include "sift-header.h"
#include "sift-geo.h"

NS_LOG_COMPONENT_DEFINE ("SiftRouting");

namespace ns3 {
namespace sift {


NS_OBJECT_ENSURE_REGISTERED (SiftRouting);

/* see http://www.iana.org/assignments/protocol-numbers */
const uint8_t SiftRouting::PROT_NUMBER = 47;
/*
 * The extension header is the sift header, it is response for recognizing SIFT option types
 * and demux to right options to process the packet.
 *
 * The header format with neighboring layers is as follows:
 *
 +-+-+-+-+-+-+-+-+-+-+--
 |  Application Header |
 +-+-+-+-+-+-+-+-+-+-+-+
 |   Transport Header  |
 +-+-+-+-+-+-+-+-+-+-+-+
 |       SIFT Header   |
 +-+-+-+-+-+-+-+-+-+-+-+
 |      IP Header      |
 +-+-+-+-+-+-+-+-+-+-+-+
 */

TypeId SiftRouting::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::sift::SiftRouting")
    .SetParent<IpL4Protocol> ()
    .AddConstructor<SiftRouting> ()
    .AddTraceSource ("Tx", "Send SIFT packet.",
                     MakeTraceSourceAccessor (&SiftRouting::m_txPacketTrace),
					 "ns3::SiftRouting::TxTracedValueCallback")
    .AddTraceSource ("Drop", "Drop SIFT packet",
                     MakeTraceSourceAccessor (&SiftRouting::m_dropTrace),
					 "ns3::SiftRouting::DropTracedValueCallback")
  ;
  return tid;
}

SiftRouting::SiftRouting ()
  : m_seqNo (0)
{
  NS_LOG_FUNCTION (this);
  m_uniformRandomVariable = CreateObject<UniformRandomVariable> ();

}

SiftRouting::~SiftRouting ()
{
  NS_LOG_FUNCTION (this);
}

void
SiftRouting::NotifyNewAggregate ()
{
  NS_LOG_FUNCTION (this << "NotifyNewAggregate");
  if (m_node == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      if (node != 0)
        {
          m_ipv4 = this->GetObject<Ipv4L3Protocol> ();
          if (m_ipv4 != 0)
            {
              this->SetNode (node);
              m_ipv4->Insert (this);
              this->SetDownTarget (MakeCallback (&Ipv4L3Protocol::Send, m_ipv4));
            }

          m_ip = node->GetObject<Ipv4> ();
          if (m_ip != 0)
            {
              NS_LOG_DEBUG ("Ipv4 started");
            }
        }
    }
  Object::NotifyNewAggregate ();
  Simulator::ScheduleNow (&SiftRouting::Start, this);
}

void SiftRouting::Start ()
{
  NS_LOG_FUNCTION (this << "Start SIFT Routing protocol");

  if (m_mainAddress == Ipv4Address ())
    {

      //add the node as a neighbor of itself
      GeographicTuple geo_tuple;
      geo_tuple.nodeAddr = m_mainAddress;
      geo_tuple.nodeReceivedFrom = m_mainAddress;
      Ptr<MobilityModel> thisNodeMobility = m_ip->GetObject<Node> ()->GetObject<MobilityModel> ();
      geo_tuple.xcoord = (int) thisNodeMobility->GetPosition ().x;
      geo_tuple.ycoord = (int) thisNodeMobility->GetPosition ().y;
      geo_tuple.zcoord = (int) thisNodeMobility->GetPosition ().z;
      geo_tuple.xvelocity = (int) thisNodeMobility->GetVelocity ().x;
      geo_tuple.yvelocity = (int) thisNodeMobility->GetVelocity ().y;
      geo_tuple.zvelocity = (int) thisNodeMobility->GetVelocity ().z;

      Time now = Simulator::Now ();
      geo_tuple.recordedTime = now;
      geo_tuple.dataRecordedTime = now;
      geo_tuple.startTime = now;
      geo_tuple.expireTime = Simulator::GetMaximumSimulationTime ();
      AddGeographicTuple (geo_tuple);

      Ipv4Address loopback ("127.0.0.1");
      for (uint32_t i = 0; i < m_ipv4->GetNInterfaces (); i++)
        {
          // Use primary address, if multiple
          Ipv4Address addr = m_ipv4->GetAddress (i, 0).GetLocal ();
          m_broadcast = m_ipv4->GetAddress (i, 0).GetBroadcast ();

          if (addr != loopback)
            {
              // Set the main address as the current ip address
              m_mainAddress = addr;
              NS_LOG_LOGIC ("Starting SIFT on node " << m_mainAddress);

              NS_ASSERT (m_mainAddress != Ipv4Address () && m_broadcast != Ipv4Address ());
            }
        }
    }
}

void
SiftRouting::AddNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_nodePtrs.push_back (node);
  NS_LOG_INFO (node << "in AddNode");
}

uint32_t
SiftRouting::GetNodesSize (void)
{
  NS_LOG_FUNCTION  (this);
  NS_LOG_DEBUG ("m_nodePtrs.size (): " << m_nodePtrs.size ());
  return m_nodePtrs.size ();
}

void
SiftRouting::DoDispose (void)
{
  NS_LOG_FUNCTION  (this);
  m_node = 0;
  IpL4Protocol::DoDispose ();
}

void
SiftRouting::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}

Ptr<Node>
SiftRouting::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

Ptr<Node>
SiftRouting::GetNodeWithAddress (Ipv4Address ipv4Address)
{
  NS_LOG_FUNCTION (this << ipv4Address);
  int32_t nNodes = NodeList::GetNNodes ();
  for (int32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      int32_t ifIndex = ipv4->GetInterfaceForAddress (ipv4Address);
      if (ifIndex != -1)
        {
          return node;
        }
    }
  return 0;
}

void SiftRouting::PrintVector (std::vector<Ipv4Address>& vec)
{
  NS_LOG_FUNCTION (this);
  /*
   * Check elements in a route vector
   */
  if (!vec.size ())
    {
      NS_LOG_DEBUG ("The vector is empty");
    }
  else
    {
      NS_LOG_DEBUG ("Print all the elements in a vector");
      for (std::vector<Ipv4Address>::const_iterator i = vec.begin (); i != vec.end (); ++i)
        {
          NS_LOG_DEBUG ("The ip address " << *i);
        }
    }
}

Ptr<Ipv4Route>
SiftRouting::SetRoute (Ipv4Address nextHop, Ipv4Address srcAddress)
{
  NS_LOG_FUNCTION (this << nextHop << srcAddress);
  m_ipv4Route = Create<Ipv4Route> ();
  m_ipv4Route->SetDestination (nextHop);
  m_ipv4Route->SetGateway (nextHop);
  m_ipv4Route->SetSource (srcAddress);
  return m_ipv4Route;
}

int
SiftRouting::GetProtocolNumber (void) const
{
  NS_LOG_FUNCTION (this);
  // / This is the protocol number for SIFT which is 48
  return PROT_NUMBER;
}
void SiftRouting::SetSeqNo (uint16_t seqNo)
{
  m_seqNo = seqNo;
}

uint16_t SiftRouting::GetSeqNo ()
{
  m_seqNo = (m_seqNo + 1) % MAX_SEQ_NO;
  return m_seqNo;
}

void
SiftRouting::Send (Ptr<Packet> packet,
                   Ipv4Address source,
                   Ipv4Address destination,
                   uint8_t protocol,
                   Ptr<Ipv4Route> route)
{
  NS_LOG_FUNCTION (this << "Send module");
  NS_LOG_FUNCTION (this << packet << source << destination << (uint32_t)protocol << route);

  // "packet" parameter is UDP header

  if (source == destination)
    {
      std::cout << "\nSimulator created similar source and destination! Source: " << source << " destination: " << destination << "\n";
      return;
    }
  NS_ASSERT_MSG (!m_downTarget.IsNull (), "Error, SiftRouting cannot send downward");

  if (1)
    {
      Time now = Simulator::Now ();
      NS_LOG_INFO ("\nSend from node (" << this->GetNode ()->GetId () << ") with IP address: "
                                        << this->GetIPfromID (this->GetNode ()->GetId ()) << " at time: " << now << "\n");

    }
  if (protocol == 1)
    {
      NS_LOG_INFO ("Drop packet. Not handling ICMP packet for now");
    }
  else
    {

      NS_LOG_INFO ("Protocol number if protocol<>1: " << (uint32_t)protocol);
      Ptr<Packet> cleanP = packet->Copy ();
      SiftHeader siftHeader;
      siftHeader.SetNextHeader (protocol);
      siftHeader.SetMessageType (47);
      siftHeader.SetSourceAddress (source);
      siftHeader.SetDestAddress (destination);

      //  take this node's location
      Ptr<MobilityModel> mobility = GetObject<Node> ()->GetObject<MobilityModel> ();
      Ptr<Node> destNode = GetNodeWithAddress (destination);
      Ptr<Node> sourceNode = GetNodeWithAddress (source);
      Ptr<MobilityModel> destMobility = destNode->GetObject<MobilityModel> ();
      Ptr<MobilityModel> sourceMobility = sourceNode->GetObject<MobilityModel> ();
      int32_t x_ = sourceMobility->GetPosition ().x;
      int32_t y_ = sourceMobility->GetPosition ().y;
      int32_t x = mobility->GetPosition ().x;
      int32_t y = mobility->GetPosition ().y;
      int32_t x1 = destMobility->GetPosition ().x;
      int32_t y1 = destMobility->GetPosition ().y;
      siftHeader.SetSourceXLoc ((int32_t)x_);
      siftHeader.SetSourceYLoc ((int32_t)y_);
      siftHeader.SetLastSourceXLoc ((int32_t)x);
      siftHeader.SetLastSourceYLoc ((int32_t)y);
      siftHeader.SetDestXLoc ((int32_t)x1);
      siftHeader.SetDestYLoc ((int32_t)y1);
      siftHeader.SetHeaderSeqNo (GetSeqNo ());
      siftHeader.SetHeaderTTL (TTL);
      NS_LOG_DEBUG (this << "Node positions in Send module of node: " << this->GetNode ()->GetId () <<
                    " (source X, Source Y)= ("
                         << x_ << ", " << y_ << ") (Current X, Current Y)= ("
                         << x << ", " << y << ") (Destination X, Destination Y)= ("
                         << x1 << ", " << y1 << ")");



      cleanP->AddHeader (siftHeader);
      Ptr<Packet> mtP = cleanP->Copy ();

      // Save a copy of packet in the buffer to check duplicate packets
      m_buffer.insert (std::pair<Ptr<Packet>,Time> (mtP,Simulator::Now ()));
      // Send the packet out with header
      SendPacket (mtP, source, m_broadcast, protocol);

    }
}

void
SiftRouting::SendPacket (Ptr<Packet> packet, Ipv4Address source, Ipv4Address nextHop, uint8_t protocol)
{
  NS_LOG_FUNCTION (this << "SiftRouting::SendPacket module");
  NS_LOG_FUNCTION (this << packet << source << nextHop << (uint32_t)protocol);
  // Send out the data packet

  m_ipv4Route = SetRoute (nextHop, m_mainAddress);
  Ptr<NetDevice> dev = m_ip->GetNetDevice (m_ip->GetInterfaceForAddress (m_mainAddress));
  m_ipv4Route->SetOutputDevice (dev);

  if (protocol == UDP_PROTOCOL || protocol == TCP_PROTOCOL || protocol == ICMP_PROTOCOL)
    {

      //m_downTarget (packet, source, nextHop, protocol, m_ipv4Route);
      m_downTarget (packet, source, nextHop, GetProtocolNumber (), m_ipv4Route);
    }
  else
    {
      m_downTarget (packet, source, nextHop, GetProtocolNumber (), m_ipv4Route);
    }
}

int64_t
SiftRouting::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformRandomVariable->SetStream (stream);
  return 1;
}

void
SiftRouting::ForwardPacket (Ptr<Packet> packet,
                            Ipv4Address source,
                            uint8_t protocol)
{
  NS_LOG_FUNCTION (this << "SiftRouting::ForwardPacket module");
  NS_LOG_FUNCTION (this << packet << source << (uint32_t)protocol);
  NS_ASSERT_MSG (!m_downTarget.IsNull (), "Error, SiftRouting cannot send downward");


  // Forward the packet here, we will not use any coding and multipath here
  SendPacket (packet, source, m_broadcast, protocol);
}

void
SiftRouting::ScheduleTimer (Ptr<Packet> packet, Time delay, Ipv4Address source, uint8_t protocol)
{
  NS_LOG_FUNCTION ("SiftRouting::ScheduleTimer" << this << packet << delay << source << (uint32_t)protocol);
  NS_LOG_INFO (this << packet << delay << source << protocol);



  SiftHeader tempSift, timerSift;
  packet->PeekHeader (tempSift);
  bool matchFound = false;
  if (m_timer.size () == 0)     // Timer buffer is empty. This is the first packet being scheduled
    {
      NS_LOG_DEBUG ("In ScheduleTimer of node (" << this->GetNode ()->GetId () << ") with IP address: "
                                                 << this->GetIPfromID (this->GetNode ()->GetId ())
                                                 << "Timer buffer is empty. New packet is scheduling. \nThis is the packet: \n"
                                                 << " Current time: " << Seconds (Simulator::Now ()) );

      Timer timer (Timer::CANCEL_ON_DESTROY);
      m_timer[packet] = timer;
    }
  else                                          // Timer buffer has some content. Duplicate packets are checking and if there is no duplication, the packet is scheduled.
    {
      for (std::map<Ptr<Packet>,Timer >::iterator it = m_timer.begin (); it != m_timer.end (); ++it)
        {
          (it->first)->PeekHeader (timerSift);
          NS_LOG_DEBUG (*(it->first) << " State in ScheduleTimer: " << (it->second).GetState () << " Current Time (Sec): " << Seconds (Simulator::Now ()) );

          if (timerSift.GetSourceAddress () == tempSift.GetSourceAddress ()
              && timerSift.GetDestAddress () == tempSift.GetDestAddress ()
              && timerSift.GetHeaderSeqNo () == tempSift.GetHeaderSeqNo ()
              && matchFound == false)           // Checking duplicate packets
            {
              NS_LOG_DEBUG ("\nMatch was found in timer buffer (duplicate packet). The packet is unscheduling and erasing\n");

              // A duplicate packet was found for a new arrival packet.
              (it->second).Cancel ();
              //sum_timer.erase(it);
              matchFound = true;

            }
          if ((it->second).GetState () == Timer::EXPIRED)           //Expired event
            {
              NS_LOG_DEBUG ("One expired event was removed at node (" << this->GetNode ()->GetId () << ") with IP address: "
                                                                      << this->GetIPfromID (this->GetNode ()->GetId ())
                                                                      << "Current time (sec): " << Seconds (Simulator::Now ()));

              m_timer.erase (it);
            }

        }
      if (matchFound == false)
        {
          NS_LOG_DEBUG ("No duplication was found. New packet is scheduling ....Current time: " << Seconds (Simulator::Now ()));

          Timer timer (Timer::CANCEL_ON_DESTROY);
          m_timer[packet] = timer;
        }
    }


  // When the timer expires, it will call this function here, which is send for this case
  m_timer[packet].SetFunction (&SiftRouting::ForwardPacket, this);
  m_timer[packet].Remove ();
  m_timer[packet].SetArguments (packet, source, protocol);
  m_timer[packet].Schedule (delay);
}

void 
SiftRouting::CancelTimer (Ptr<Packet> p)
{
  NS_LOG_FUNCTION ("SiftRouting::CancelTimer");
  NS_LOG_INFO (this << p );

  // We erase the timer if we have received it before
  NS_LOG_DEBUG ("\nTimer Content: \n");
  SiftHeader tempSift, timerSift;
  p->PeekHeader (tempSift);
  for (std::map<Ptr<Packet>,Timer >::iterator it = m_timer.begin (); it != m_timer.end (); ++it)
    {
      (it->first)->PeekHeader (timerSift);
      NS_LOG_DEBUG (*(it->first) << " State: " << (it->second).GetState () << " Current Time: " << Simulator::Now ());

      if (timerSift.GetSourceAddress () == tempSift.GetSourceAddress ()
          && timerSift.GetDestAddress () == tempSift.GetDestAddress ()
          && timerSift.GetHeaderSeqNo () == tempSift.GetHeaderSeqNo ())
        {
          NS_LOG_DEBUG ("\nMatch was found in timer buffer (Cancel Timer). The record is deleting \n");
          (it->second).Cancel ();
          m_timer.erase (it);
          break;
        }

    }

}

enum IpL4Protocol::RxStatus
SiftRouting::Receive (Ptr<Packet> p,
                      Ipv4Header const &ip,
                      Ptr<Ipv4Interface> incomingInterface)
{

  NS_LOG_FUNCTION (this << *p << ip  << incomingInterface);


  NS_LOG_INFO ("Our own IP address " << m_mainAddress << " The incoming interface address " << incomingInterface);
  m_node = GetNode ();                        // Get the node
  NS_LOG_FUNCTION ("\n\nEntering Receive module of node: " << m_node->GetId () << " with IP address: " << this->GetIPfromID (this->GetNode ()->GetId ()));

  Ptr<Packet> packet = p->Copy ();            // Save a copy of the received packet


  SiftHeader siftHeader;
  packet->RemoveHeader (siftHeader);          // Remove the SIFT header in whole
  uint8_t protocol = 0;
  protocol = GetProtocolNumber ();

  Ipv4Address sourceAddress = siftHeader.GetSourceAddress ();
  Ipv4Address destAddress = siftHeader.GetDestAddress ();
  int32_t sourceXLoc = siftHeader.GetSourceXLoc ();
  int32_t sourceYLoc = siftHeader.GetSourceYLoc ();
  int32_t lastsourceXLoc = siftHeader.GetLastSourceXLoc ();
  int32_t lastsourceYLoc = siftHeader.GetLastSourceYLoc ();
  int32_t destXLoc = siftHeader.GetDestXLoc ();
  int32_t destYLoc = siftHeader.GetDestYLoc ();

  uint8_t segmentsLeft = siftHeader.GetSegmentsLeft ();
  uint16_t headerSeqNo = siftHeader.GetHeaderSeqNo ();
  uint8_t headerTTL = siftHeader.GetHeaderTTL ();

  NS_LOG_FUNCTION ("\tCurrent Address= " << m_mainAddress << " with position ("
                                         << GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().x << ", "
                                         << GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().y << ")\n"
                                         << "\tSource Address= " << sourceAddress << " with position (" << sourceXLoc << ", " << sourceYLoc << ")\n"
                                         << "\tDestination Address= " << destAddress << " with position (" << destXLoc << ", " << destYLoc << ")\n"
                                         << "\tLast Sourece Position (" << lastsourceXLoc << ", " << lastsourceYLoc << ")\n");



  if (destAddress.IsEqual ("0.0.0.0"))
    {
      NS_LOG_DEBUG ("A 0.0.0.0 received in Receive module at time: " << Seconds (Simulator::Now ())
                                                                     << "\nReturning from the Receive module with code RX_ENDPOINT_UNREACH\n");

      return RX_ENDPOINT_UNREACH;
    }
  if ((int)GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().x == sourceXLoc
      && (int)GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().y == sourceYLoc  )
    {
      NS_LOG_DEBUG ("\nCurrent node position has overlap with Source. Discarding the packet.\n");
      return RX_ENDPOINT_UNREACH;
    }

  NS_LOG_DEBUG ("The segments left in source route header " << (uint32_t)segmentsLeft);
  Ptr<Packet> tempPacket;
  SiftHeader tempSift;
  bool duplicate = false;
  NS_LOG_DEBUG ("\nChecking Receive Buffer in node: " << this->GetIPfromID (this->GetNode ()->GetId ()) << " ... \n");


  NS_LOG_DEBUG ("\n Comparing Source Address: " << sourceAddress << " Destination Address: " << destAddress << " Seq No: " << headerSeqNo << " with the content of the buffer ... \n");

  for (std::map<Ptr<Packet>,Time >::iterator it = m_buffer.begin (); it != m_buffer.end (); ++it)
    {
      if (Simulator::Now () - it->second >= Seconds (3))
        {
          m_buffer.erase (it);
          NS_LOG_DEBUG ("One expired packet was removed from buffer of node: " << this->GetIPfromID (this->GetNode ()->GetId ()));

        }
      else
        {
          tempPacket = it->first;
          tempPacket->PeekHeader (tempSift);

          if (tempSift.GetSourceAddress () == sourceAddress
              && tempSift.GetDestAddress () == destAddress
              && tempSift.GetHeaderSeqNo () == headerSeqNo)
            {
              NS_LOG_INFO (" Duplicate packet received " << m_node->GetId () << p);
              CancelTimer (tempPacket);
              duplicate = true;
              return RX_ENDPOINT_UNREACH;
            }


        } // end of else (Simulator::Now()-it->second >= Seconds(2)
    } // end of for

  if (duplicate == false)
    {
      m_buffer.insert (std::pair<Ptr<Packet>,Time> (p,Simulator::Now ()));
      NS_LOG_DEBUG ("One packet was added to node " << this->GetIPfromID (this->GetNode ()->GetId ())
                                                    << "Current time: " << Seconds (Simulator::Now ()) << "\n");

    }

  Ipv4Address zero;
  zero.Set ("0.0.0.0");
  if (destAddress.IsEqual (zero))
    {
	  NS_LOG_DEBUG( "Unexpected IP address 0.0.0.0 is detected"
    		  	  << "Source Address= " << sourceAddress << ", "
                  << "Destination Address= " << destAddress << ", "
                  << "SourceXLoc= " << sourceXLoc << ", "
                  << "SourceYLoc= " << sourceYLoc << ", "
                  << "last Source Xlox= " << lastsourceXLoc << ", "
                  << "last source Yloc= " << lastsourceYLoc << ", "
                  << "dest Xloc= " << destXLoc << ", "
                  << "dest YLoc= " << destYLoc);
      std::cout << "Content of PACKET:\n";
      packet->Print (std::cout);
      std::cout << "\nContent of P:\n";
      p->Print (std::cout);
      std::cout << "\nContent of Sift Headre:\n";
      siftHeader.Print ();

    }


  if (m_mainAddress == destAddress)   // This is the final destination of the packet
    {

      // / Get the next header, should be one transport protocol
      NS_LOG_INFO ("*** This is FINAL destination ***");
      NS_LOG_INFO ("Sift Header details: "
                   << "\n\t Source geographic location (x,y): (" << sourceXLoc << ", " << sourceYLoc << ")"
                   << "\n\t Source IP Address: " << sourceAddress
                   << "\n\t Destination geographic location (x,y): (" << destXLoc << ", " << destYLoc << ")"
                   << "\n\t Destination IP Address: " << destAddress);

      uint8_t nextHeader = siftHeader.GetNextHeader ();


      Ptr<Ipv4L3Protocol> l3proto = m_node->GetObject<Ipv4L3Protocol> ();
      Ptr<IpL4Protocol> nextProto = l3proto->GetProtocol (nextHeader);


      if (nextProto != 0)
        {
          // we need to make a copy in the unlikely event we hit the
          // RX_ENDPOINT_UNREACH code path
          // Here we can use the packet that has been get off whole SIFT header, which is packet p

          Ipv4Header ipHeader;
          ipHeader.SetDestination (m_mainAddress);
          ipHeader.SetSource(sourceAddress);
          //ipHeader.SetSource ((ip.GetSource ()));
          ipHeader.SetProtocol (ip.GetProtocol ());
          ipHeader.SetPayloadSize (ip.GetPayloadSize ());
          ipHeader.SetTos (ip.GetTos ());
          ipHeader.SetTtl (ip.GetTtl ());
          ipHeader.SetEcn (ip.GetEcn ());
          ipHeader.SetDscp (ip.GetDscp ());



          // Packet is L4 header, ipHeader is L3 header
          enum IpL4Protocol::RxStatus status =
            nextProto->Receive (packet, ipHeader, incomingInterface);

          NS_LOG_DEBUG ("The receive status " << status);

          switch (status)
            {
            case IpL4Protocol::RX_OK:

              NS_LOG_DEBUG ("\n*** RX_Ok for packet in node " << GetNode ()->GetId () << "at time: " << Simulator::Now () << "\n");
              break;
            // fall through
            case IpL4Protocol::RX_ENDPOINT_CLOSED:
              NS_LOG_DEBUG ("\n*** RX_ENDPOINT_CLOSED for packet in node \n");
              break;
            // fall through
            case IpL4Protocol::RX_CSUM_FAILED:
              NS_LOG_DEBUG ("\n*** RX_CSUM_FAILED for packet in node \n");
              break;
            case IpL4Protocol::RX_ENDPOINT_UNREACH:
              NS_LOG_DEBUG ("\n*** RX_ENDPOINT_UNREACH for packet in node " << m_node->GetId () << GetTypeId () << GetNode () << "\n");

              if (ip.GetDestination ().IsBroadcast () == true
                  || ip.GetDestination ().IsMulticast () == true)
                {

                  break;               // Do not reply to broadcast or multicast
                }
              // Another case to suppress ICMP is a subnet-directed broadcast
            }
          return status;
        }
      else
        {
          NS_FATAL_ERROR ("Should not have 0 next protocol value");
        }
    }
  else   // This is not final destination, forward the packet to next hop
    {

      GeographicTuple geoTuple;
      NS_LOG_INFO ("This is not the final destination, forward the packet to next hop");
      siftHeader.SetSegmentsLeft (segmentsLeft - 1);  // decrement the segments left field
      siftHeader.SetNextHeader (protocol);
      siftHeader.SetMessageType (2);
      siftHeader.SetSourceAddress (sourceAddress);
      siftHeader.SetSourceXLoc (sourceXLoc);            // New code
      siftHeader.SetSourceYLoc (sourceYLoc);
      siftHeader.SetLastSourceXLoc (GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().x);
      siftHeader.SetLastSourceYLoc (GetObject<Node> ()->GetObject<MobilityModel> ()->GetPosition ().y);
      siftHeader.SetDestAddress (destAddress);
      siftHeader.SetDestXLoc (destXLoc);
      siftHeader.SetDestYLoc (destYLoc);                                // New code
      siftHeader.SetHeaderSeqNo (headerSeqNo);
      siftHeader.SetHeaderTTL (headerTTL - 1);
      packet->AddHeader (siftHeader);
      NS_LOG_DEBUG ("Current node IP: " << this->GetIPfromID (this->GetNode ()->GetId ()) <<
                    ", Current Node Id:" << this->GetNode ()->GetId () <<
                    ", This is not final destination for this packet in Receive module at time: " << Seconds (Simulator::Now ()) << "\n");


      for (uint32_t nodeNum = 0; nodeNum < GetNodesSize (); nodeNum++)
        {
          //GeographicTuple geoTuple;
          Ptr<Node> node = m_nodePtrs[nodeNum];
          NS_ASSERT (node != 0);
          geoTuple.nodeAddr = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
          Ptr<MobilityModel> mobility = node->GetObject<MobilityModel> ();


          Time now = Simulator::Now ();
          geoTuple.nodeReceivedFrom = m_mainAddress;
          geoTuple.recordedTime = now;
          geoTuple.dataRecordedTime = now;
          geoTuple.expireTime = Simulator::GetMaximumSimulationTime ();
          geoTuple.xcoord = (int32_t) mobility->GetPosition ().x;
          geoTuple.ycoord = (int32_t) mobility->GetPosition ().y;
          geoTuple.zcoord = (int32_t) mobility->GetPosition ().z;
          geoTuple.xvelocity = (uint16_t) mobility->GetVelocity ().x;
          geoTuple.yvelocity = (uint16_t) mobility->GetVelocity ().y;
          geoTuple.zvelocity = (uint16_t) mobility->GetVelocity ().z;
          bool needTriggerUpdate = false;
          m_state.InsertGeographicTuple (geoTuple, needTriggerUpdate);
          NS_LOG_INFO ("Current Location details: " << geoTuple.nodeAddr
                                                    << " X:" << (int)geoTuple.xcoord
                                                    << " Y:" << (int)geoTuple.ycoord
                                                    << " Z:" << (int)geoTuple.zcoord
                                                    << " Vx:" << (int)geoTuple.xvelocity
                                                    << " Vy:" << (int)geoTuple.yvelocity
                                                    << " Vz:" << (int)geoTuple.zvelocity);
          NS_LOG_INFO ("Sift Header details: "
                       << "\n\t Source geographic location (x,y): (" << sourceXLoc << ", " << sourceYLoc << ")"
                       << "\n\t Source IP Address: " << sourceAddress
                       << "\n\t Current geographic location (x,y): (" << (int) mobility->GetPosition ().x << ", " << (int) mobility->GetPosition ().y << ")"
                       << "\n\t Current IP Address: " << m_mainAddress
                       << "\n\t Destination geographic location (x,y): (" << destXLoc << ", " << destYLoc << ")"
                       << "\n\t Destination IP Address: " << destAddress);

        }


      // calculation of dist.
      double currentX = siftHeader.GetLastSourceXLoc ();
      double currentY = siftHeader.GetLastSourceYLoc ();
      double slope = 0; // slope= (destYLoc-sourceYLoc)/(destXLoc-sourceXLoc); //y=mx+b
      double Y = (destYLoc - sourceYLoc);
      double X = (destXLoc - sourceXLoc);
      double dTrajectory = 0.0; // Distance of the current node from the trajectory
      double dLastSource = 0.0; // Distance of the current node from the last sender
      double b = 0.0;
      double xIntersect = 0.0;
      double yIntersect = 0.0;
      double delayTimer = (double)TRANSMISSION_TIME;

      if (Y == 0)
        {
          slope = 0;
          b = sourceYLoc - (slope * sourceXLoc);

          xIntersect = (currentX + slope * currentY - slope * b) / (pow (slope,2) + 1);
          yIntersect = slope * xIntersect + b;
          dTrajectory = sqrt (pow ((xIntersect - currentX),2) + pow ((yIntersect - currentY),2));
          dLastSource = sqrt (pow ((lastsourceXLoc - currentX),2) +
                              pow ((lastsourceYLoc - currentY),2));


        }
      else if (X == 0)
        {
          dLastSource = sqrt (pow ((lastsourceXLoc - currentX),2) +
                              pow ((lastsourceYLoc - currentY),2));
          dTrajectory = sqrt (pow (dLastSource,2) - pow ((currentY - lastsourceYLoc),2)); // They have the same x value
        }
      else
        {
          slope = Y / X;
          b = sourceYLoc - (slope * sourceXLoc);
          xIntersect = (currentX + slope * currentY - slope * b) / (pow (slope,2) + 1);
          yIntersect = slope * xIntersect + b;
          dTrajectory = sqrt (pow ((xIntersect - currentX),2) + pow ((yIntersect - currentY),2));
          dLastSource = sqrt (pow ((lastsourceXLoc - currentX),2) +
                              pow ((lastsourceYLoc - currentY),2));

        }
      /// calculate the delay for each packet and pass it in
      /// as a time value, ns3 has one class called *Time*
      /// if the timer expires, we will forward the packet

      if (dTrajectory == 0)
        {
          dTrajectory = 1;
        }
      try
        {
          delayTimer += ALPHA * ((double)(dTrajectory / dLastSource));
        }
      catch (int e)
        {
          std::cout << "\n\n Unable to calculate delay. dTrajectory=" << dTrajectory << " dLastSource= " << dLastSource << "\n";
          Simulator::Stop ();
        }

      /// calculate the delay time here
      /// The default time is in Second
      Time delay = Seconds (delayTimer);

      NS_LOG_DEBUG (" SIFT routing calculation"
                    << "\n\t destYLoc= "  << destYLoc
                    << ", SourceYLoc: " << sourceYLoc
                    << ", destXLoc: "  << destXLoc
                    << ", sourceXLoc: " << sourceXLoc
                    << "\n\t Slope= x1/x2 = "     << slope
                    << "\n\t b in y=m+b is sourceYLoc-(slope*sourceXLoc)= "  << b
                    << "\n\t dTrajectory=  |y1-mx1-b|/Sqrt(m*m+1)= " << dTrajectory
                    << "\n\t dLastSource: " << dLastSource
                    << "\n\t Y= (destYLoc-sourceYLoc)= "         << Y
                    << ", X= (destXLoc-sourceXLoc)= "         << X
                    << "\n\t Delay: " << delay
                    << "\n\t Current time: " << Seconds (Simulator::Now ()));



      //*** Changed p to packet in the below command
      ScheduleTimer (packet, delay, sourceAddress, protocol);

    }

  return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus
SiftRouting::Receive (Ptr<Packet> p,
                      Ipv6Header const &ip,
                      Ptr<Ipv6Interface> incomingInterface)
{
  //NS_LOG_FUNCTION (this << p << ip.GetSourceAddress () << ip.GetDestinationAddress () << incomingInterface);
  return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void
SiftRouting::SetDownTarget (DownTargetCallback callback)
{
  m_downTarget = callback;
}

void
SiftRouting::SetDownTarget6 (DownTargetCallback6 callback)
{
  NS_FATAL_ERROR ("Unimplemented");
}


IpL4Protocol::DownTargetCallback
SiftRouting::GetDownTarget (void) const
{
  return m_downTarget;
}

IpL4Protocol::DownTargetCallback6
SiftRouting::GetDownTarget6 (void) const
{
  NS_FATAL_ERROR ("Unimplemented");
  return MakeNullCallback<void,Ptr<Packet>, Ipv6Address, Ipv6Address, uint8_t, Ptr<Ipv6Route> > ();
}

uint16_t
SiftRouting::GetIDfromIP (Ipv4Address address)
{
  int32_t nNodes = NodeList::GetNNodes ();
  for (int32_t i = 0; i < nNodes; ++i)
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();

      int tempint;
      tempint = ipv4->GetNInterfaces ();
      for (int j = 1; j < tempint; j++) 
        {
          Ipv4InterfaceAddress iaddr = ipv4->GetAddress (j, 0);
          if (iaddr.GetLocal () == address)
            {
              NS_LOG_DEBUG ("Test address " << i << "," << iaddr.GetLocal ());
              return uint16_t (i);
            }
        }
    }
  return 256;
}

Ipv4Address
SiftRouting::GetIPfromID (const uint32_t id)
{
  NS_LOG_FUNCTION (this);

  if (id >= 256)
    {
      NS_LOG_DEBUG ("Exceed the node range");
      return "0.0.0.0";
    }
  else
    {
      Ptr<Node> node = NodeList::GetNode (id);
      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      return ipv4->GetAddress (1, 0).GetLocal ();
    }
}

void SiftRouting::AddGeographicTuple (const GeographicTuple &tuple)
{
  bool needTriggerUpdate = false;
  m_state.InsertGeographicTuple (tuple, needTriggerUpdate);
}
void SiftRouting::PrintSiftHeader (SiftHeader siftHeader, GeographicTuple geoTuple, Ptr<OutputStreamWrapper> stream) const
{
  Ptr<Node> node = SiftRouting::GetNode ();
  Ipv4Address sourceAddress = siftHeader.GetSourceAddress ();
  Ipv4Address destAddress = siftHeader.GetDestAddress ();
  int32_t sourceXLoc = siftHeader.GetSourceXLoc ();
  int32_t sourceYLoc = siftHeader.GetSourceYLoc ();
  int32_t lastsourceXLoc = siftHeader.GetLastSourceXLoc ();
  int32_t lastsourceYLoc = siftHeader.GetLastSourceYLoc ();
  int32_t destXLoc = siftHeader.GetDestXLoc ();
  int32_t destYLoc = siftHeader.GetDestYLoc ();
  Ipv4Address currentAddress = node->GetObject<Ipv4> ()->GetAddress (1,0).GetLocal ();
  *stream->GetStream () << "Current Node IP address= " << currentAddress << std::endl
   << "Source Address= " << sourceAddress << std::endl
   << "Destination Address= " << destAddress << std::endl
   << "SourceXLoc= " << sourceXLoc << std::endl
   << "SourceYLoc= " << sourceYLoc << std::endl
   << "CurrentXLoc= " << (int)geoTuple.xcoord << std::endl
   << "CurrentYLoc= " << (int)geoTuple.ycoord << std::endl
   << "last Source Xlox= " << lastsourceXLoc << std::endl
   << "last source Yloc= " << lastsourceYLoc << std::endl
   << "dest Xloc= " << destXLoc << std::endl
   << "dest YLoc= " << destYLoc << std::endl;
}

void SiftRouting::PrintReceiveBuffer ()
{
  std::map<Ptr<Packet>, Time> receiveBuffer;
  receiveBuffer = GetReceiveBuffer ();
  for (std::map<Ptr<Packet>,Time >::iterator it = receiveBuffer.begin (); it != receiveBuffer.end (); ++it)
    {
      std::cout << *it->first << "=>" << it->second << '\n';
    }


}

std::map<Ptr<Packet>, Time> SiftRouting::GetReceiveBuffer ()
{
  return m_buffer;
}


}  /* namespace sift */
}  /* namespace ns3 */
