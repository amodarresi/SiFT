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

/*
 * The following script let the user to choose a mobility model and a routing
 * algorithm to be simulated. In addition, other general parameters such as
 * number of nodes, sinks, total simulation time and size of the simulation
 * area are adjustable. These parameters can be entered either through the
 * command line or changing their values at the beginning of these script.
 * The comments next to each parameter describes their purpose in the script.
 * This script can be used to compare various routing algorithms with different
 * mobility models.
 * The output is stored in pcap files.
 */
#include <string>
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/sift-module.h"
#include "ns3/sift-helper.h"
#include "ns3/sift-main-helper.h"
#include "ns3/aodv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/dsdv-helper.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/olsr-helper.h"
#include <sstream>
#include <iostream>


NS_LOG_COMPONENT_DEFINE ("SiftTest");

using namespace ns3;

class SiftTest
{
public:
  enum Model
  {
    CONSTANTPOSITION, RANDOMDIRECTION2D, RANDOMWALK2D, RANDOMWAYPOINT
  };
  enum Routing
  {
    SIFT, AODV, DSDV, DSR, OLSR
  };
  SiftTest ();                                                // Constructor
  bool Configure (int agrc, char **argv);                      //configure script parameters
  void Run ();                                                // Run simulation


private:
  uint32_t nWifis;                            // number of wireless nodes
  uint32_t nSinks;                            // number of Sink nodes
  double totalTime;                           // Total simulation time
  double dataTime;                            //ns was 10000.0
  double ppers;
  uint32_t packetSize;                        //Packet size
  double dataStart;                           // time to start sending data
  double pauseTime;                           // Pause time between nodes movement
  double nodeSpeed;                           // Node speed
  double txpDistance;                         //Wireless range
  std::string rate;
  std::string dataMode;
  std::string phyMode;
  Routing algo;                               // Routing algorithm used in the simulation
  Model mobilityModel;                        // Mobility model used in the simulation
  uint32_t xLength;                           // x length of Mobility area
  uint32_t yLength;                           // y length of Mobility area
  int numberRun;
  int xDistance;                              //x distance between two consecutive nodes in grid topology
  int yDistance;                                      //y distance between two consecutive nodes in grid topology
  double gridWidth;                           //Number of nodes on each line of grid
  bool pcap;                                          //enable disable pcap report files
  NodeContainer adhocNodes;
  NetDeviceContainer allDevices;
  Ipv4InterfaceContainer allInterfaces;
  // DSDV Parameter
  uint32_t periodicUpdateInterval;
  uint32_t settlingTime;


  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();


};



SiftTest::SiftTest ()
{
  nWifis = 10;
  nSinks = 1;
  totalTime = 10.0;
  dataTime = 10000.0;
  ppers = 1;
  packetSize = 64;
  dataStart = 2.0;           // start sending data at 100s
  pauseTime = 1.0;
  nodeSpeed = 20.0;
  txpDistance = 250.0;
  algo = SIFT;
  mobilityModel = RANDOMWALK2D;
  xLength = 1500;
  yLength = 300;
  rate = ".512kbps";
  dataMode = "DsssRate11Mbps";
  phyMode = "DsssRate11Mbps";
  numberRun = 1;
  xDistance = 200;
  yDistance = 200;
  periodicUpdateInterval = 15;
  settlingTime = 6;
  pcap = true;
  gridWidth = xLength / xDistance;
}
bool
SiftTest::Configure (int argc, char **argv)
{
  // Enable Sift logs by default. Comment this if too noisy
  // LogComponentEnable("SiftRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", numberRun);
  cmd.AddValue ("nWifis", "Number of wifi nodes", nWifis);
  cmd.AddValue ("nSinks", "Number of SINK traffic nodes", nSinks);
  cmd.AddValue ("rate", "CBR traffic rate(in kbps), Default:8", rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model, Default:20", nodeSpeed);
  cmd.AddValue ("packetSize", "The packet size", packetSize);
  cmd.AddValue ("txpDistance", "Specify node's transmit range, Default:250", txpDistance);
  cmd.AddValue ("pauseTime", "pauseTime for mobility model, Default: 100", pauseTime);

  cmd.Parse (argc, argv);


  return true;
}

void
SiftTest::Run ()
{
  //  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "Starting simulation for " << totalTime << " s ...\n";

  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
  std::cout << "Simulation finished.\n\n";
}

void
SiftTest::CreateNodes ()
{

  std::cout << "Creating " << (unsigned)nWifis << " adhocNodes \n";
  adhocNodes.Create (nWifis);
  // Name adhocNodes

  for (uint32_t i = 0; i < nWifis; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), adhocNodes.Get (i));

    }

  // Initializing variables to setup mobility and position allicator
  MobilityHelper adhocMobility;
  ObjectFactory pos;
  std::stringstream convert;
  std::string speed;
  std::string pause;
  std::string xPos,yPos;
  convert.str ("");
  convert.clear ();
  convert << xLength;
  xPos="ns3::UniformRandomVariable[Min=0.0|Max=" + convert.str () + "]";
  convert.str ("");
  convert.clear ();
  convert << yLength;
  yPos="ns3::UniformRandomVariable[Min=0.0|Max=" + convert.str () + "]";
  convert.str ("");
  convert.clear ();
  convert << nodeSpeed;
  speed = "ns3::UniformRandomVariable[Min=1.0|Max=" + convert.str () + "]";
  convert.str ("");
  convert.clear ();
  convert << pauseTime;
  pause = "ns3::ConstantRandomVariable[Constant=" + convert.str () + "s]";


  switch (mobilityModel)
    {
    case CONSTANTPOSITION:
      NS_LOG_INFO ("Mobility model is constant position. Static nodes are "
                   << xDistance << " m apart and " << gridWidth << " nodes in each line.\n");
      adhocMobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                          "MinX", DoubleValue (0.0),
                                          "MinY", DoubleValue (0.0),
                                          "DeltaX", DoubleValue (xDistance),
                                          "DeltaY", DoubleValue (yDistance),
                                          "GridWidth", UintegerValue (gridWidth),
                                          "LayoutType", StringValue ("RowFirst"));
      adhocMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      break;

    case RANDOMDIRECTION2D:
      NS_LOG_INFO ("Mobility model is RANDOMDIRECTION2D with Rectangle Position allocator.\nNode speed =  "
                   << speed <<"\nNode pause time = " << pause <<"\n");
      adhocMobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                          "X", StringValue (xPos),
                                          "Y", StringValue (yPos));

      adhocMobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
                                      "Pause", StringValue (pause),
                                      "Speed", StringValue (speed),
                                      "Bounds", RectangleValue (Rectangle (0.0, xLength, 0.0, yLength)));
      break;

    case RANDOMWALK2D:
      pause="1s";
      NS_LOG_INFO ("Mobility model is RANDOMDWALK2D with Rectangle Position allocator.\nNode speed =  "
                   << speed <<"\nNode pause time = " << pause <<"\n");
      adhocMobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                          "X", StringValue (xPos),
                                          "Y", StringValue (yPos));

      adhocMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                      "Mode", StringValue ("Time"),
                                      "Time", StringValue (pause),
                                      "Speed", StringValue (speed),
                                      "Bounds", RectangleValue (Rectangle (0.0, xLength, 0.0, yLength)));
      break;

    case RANDOMWAYPOINT:
      NS_LOG_INFO ("Mobility model is RANDOMWAYPOINT with Rectangle Position allocator.\nNode speed =  "
                   << speed <<"\nNode pause time = " << pause <<"\n");
      pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
      pos.Set ("X", StringValue (xPos));
      pos.Set ("Y", StringValue (yPos));

      Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
      adhocMobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                      "Speed", StringValue (speed),
                                      "Pause", StringValue (pause),
                                      "PositionAllocator", PointerValue (taPositionAlloc));

      break;
    }

  adhocMobility.Install (adhocNodes);


}

void
SiftTest::CreateDevices ()
{
  //NS_LOG_INFO ("setting the default phy and channel parameters");
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));

  //NS_LOG_INFO ("setting the default phy and channel parameters ");
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (txpDistance));
  wifiPhy.SetChannel (wifiChannel.Create ());
  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (dataMode), "ControlMode",
                                StringValue (phyMode));

  wifiMac.SetType ("ns3::AdhocWifiMac");
  allDevices = wifi.Install (wifiPhy, wifiMac, adhocNodes);




  //NS_LOG_INFO ("Configure Tracing.");

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream;
  switch (algo)
    {
    case SIFT:
      stream = ascii.CreateFileStream ("siftTrace.tr");
      if (pcap)
        {
          wifiPhy.EnablePcapAll (std::string ("siftpcap"));
        }
      break;

    case AODV:
      stream = ascii.CreateFileStream ("aodvTrace.tr");
      if (pcap)
        {
          wifiPhy.EnablePcapAll (std::string ("aodvpcap"));
        }
      break;
    case DSDV:
      stream = ascii.CreateFileStream ("dsdvTrace.tr");
      if (pcap)
        {
          wifiPhy.EnablePcapAll (std::string ("dsdvpcap"));
        }
      break;
    case DSR:
      stream = ascii.CreateFileStream ("dsrTrace.tr");
      if (pcap)
        {
          wifiPhy.EnablePcapAll (std::string ("dsrpcap"));
        }
      break;
    case OLSR:
      stream = ascii.CreateFileStream ("olsrTrace.tr");
      if (pcap)
        {
          wifiPhy.EnablePcapAll (std::string ("olsrpcap"));
        }
      break;
    }
  wifiPhy.EnableAsciiAll (stream);

}

void
SiftTest::InstallInternetStack ()
{
  InternetStackHelper internet;
  // SIFT --------
  SiftMainHelper siftMain;
  SiftHelper sift;

  // AODV --------
  AodvHelper aodv;
  // DSR ---------
  DsrMainHelper dsrMain;
  DsrHelper dsr;
  // DSDV --------
  DsdvHelper dsdv;
  // OLSR --------
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;
  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (list,10);

  switch (algo)
    {
    case SIFT:
      internet.Install (adhocNodes);
      sift.SetNodes (adhocNodes);
      siftMain.Install (sift, adhocNodes);
      break;

    case AODV:
      internet.SetRoutingHelper (aodv);
      internet.Install (adhocNodes);
      break;
    case DSDV:
      dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (periodicUpdateInterval)));
      dsdv.Set ("SettlingTime", TimeValue (Seconds (settlingTime)));

      internet.SetRoutingHelper (dsdv);               // has effect on the next Install ()
      internet.Install (adhocNodes);
      break;
    case DSR:
      internet.Install (adhocNodes);
      dsrMain.Install (dsr, adhocNodes);
      break;
    case OLSR:
      internet.SetRoutingHelper (list);
      internet.Install (adhocNodes);

      break;
    }
  //NS_LOG_INFO ("assigning ip address");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  //Ipv4InterfaceContainer allInterfaces;
  allInterfaces = address.Assign (allDevices);


}

void
SiftTest::InstallApplications ()
{
  uint16_t port = 9;
  double randomStartTime = (1 / ppers) / nSinks;               //distributed btw 1s evenly as we are sending 4pkt/s


  for (uint32_t i = 0; i < nSinks; ++i)
    {

      PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
      ApplicationContainer apps_sink = sink.Install (adhocNodes.Get (i));
      apps_sink.Start (Seconds (0.0));
      apps_sink.Stop (Seconds (totalTime - 1));

      OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (allInterfaces.GetAddress (i), port)));
      onoff1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
      onoff1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));
      onoff1.SetAttribute ("PacketSize", UintegerValue (packetSize));
      onoff1.SetAttribute ("DataRate", DataRateValue (DataRate (rate)));

      ApplicationContainer apps1 = onoff1.Install (adhocNodes.Get (i + nWifis - nSinks));
      apps1.Start (Seconds (dataStart + i * randomStartTime));
      apps1.Stop (Seconds (dataTime + i * randomStartTime));
    }
}

int main (int argc, char ** argv)
{

  SiftTest test;
  test.Run ();
  return 0;
}


