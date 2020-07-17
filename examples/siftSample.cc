//* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/sift-module.h"
#include "ns3/sift-helper.h"
#include "ns3/sift-main-helper.h"
#include <sstream>
#include <iostream>
#include <string>
#include <cmath>

using namespace ns3;

/**
 * \brief Test script.
 * 
 * This script creates 1-dimensional grid topology and then sends flows with onOffApplication between them:
 * 
 * 
 */
class SiftExample
{
public:
  SiftExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();


private:
  // number of wireless nodes
  uint32_t nWifis;
  // number of Sink nodes
  uint32_t nSinks;
  // Total simulation time
  double totalTime;
  double dataTime;   //ns was 10000.0
  double ppers;
  //Packet size
  uint32_t packetSize;
  // time to start sending data
  double dataStart;
  // Pause time between nodes movement
  double pauseTime;
  // Node speed
  double nodeSpeed;
  //Wireless range
  double txpDistance;
  std::string rate;
  std::string dataMode;
  std::string phyMode;
  int run;
  int step;    //distance between nodes in grid topology
  bool pcap;   //enable disable pcap report files


  NodeContainer adhocNodes;
  NetDeviceContainer allDevices;
  Ipv4InterfaceContainer allInterfaces;


private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
};

int main (int argc, char **argv)
{
  //NS_LOG_FUNCTION ("Main" << argc << "!");
  SiftExample test;
  if (!test.Configure (argc, argv))
    {
      // NS_FATAL_ERROR ("Configuration failed. Aborted.");
      std::cout << "Configuration failed. /n";
      exit (1);
    }

  test.Run ();
  return 0;
}

//-----------------------------------------------------------------------------
SiftExample::SiftExample ()
  : nWifis (10),
    nSinks (1),
    totalTime (10.0),
    dataTime (10000.0),
    ppers (1),
    packetSize (64),
    dataStart (2.0),
    // start sending data 
    pauseTime (1.0),
    nodeSpeed (20.0),
    txpDistance (250.0),
    rate (".512kbps"),
    dataMode ("DsssRate11Mbps"),
    phyMode ("DsssRate11Mbps"),
    run (1),
    step (200),
    pcap (true)
{
}

bool
SiftExample::Configure (int argc, char **argv)
{
  // Enable Sift logs by default. Comment this if too noisy
  // LogComponentEnable("SiftRoutingProtocol", LOG_LEVEL_ALL);

  SeedManager::SetSeed (12345);
  CommandLine cmd;

  cmd.AddValue ("run", "Run index (for setting repeatable seeds)", run);
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
SiftExample::Run ()
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
}



void
SiftExample::CreateNodes ()
{

  std::cout << "Creating " << (unsigned)nWifis << " adhocNodes " << step << " m apart.\n";
  adhocNodes.Create (nWifis);
  // Name adhocNodes

  for (uint32_t i = 0; i < nWifis; ++i)
    {
      std::ostringstream os;
      os << "node-" << i;
      Names::Add (os.str (), adhocNodes.Get (i));

    }

  // Create static grid

  MobilityHelper adhocMobility;

  adhocMobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                      "MinX", DoubleValue (0.0),
                                      "MinY", DoubleValue (0.0),
                                      "DeltaX", DoubleValue (step),
                                      "DeltaY", DoubleValue (0),
                                      "GridWidth", UintegerValue (nWifis),
                                      "LayoutType", StringValue ("RowFirst"));
  


  adhocMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  adhocMobility.Install (adhocNodes);

}

void
SiftExample::CreateDevices ()
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



  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("Siftpcap"));
    }

  //NS_LOG_INFO ("Configure Tracing.");

  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("siftTrace.tr");
  wifiPhy.EnableAsciiAll (stream);

}

void
SiftExample::InstallInternetStack ()
{
  InternetStackHelper internet;
  // SIFT --------
  SiftMainHelper siftMain;
  SiftHelper sift;
  internet.Install (adhocNodes);
  sift.SetNodes (adhocNodes);
  siftMain.Install (sift, adhocNodes);

  //NS_LOG_INFO ("assigning ip address");
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  //Ipv4InterfaceContainer allInterfaces;
  allInterfaces = address.Assign (allDevices);


}

void
SiftExample::InstallApplications ()
{
  uint16_t port = 9;
  double randomStartTime = (1 / ppers) / nSinks;         //distributed btw 1s evenly as we are sending 4pkt/s


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

