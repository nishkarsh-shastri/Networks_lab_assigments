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
 */

// Network topology
//
//       n0    n1   n2   n3   n4  n5  n6  n7     
//       |     |    |    |    |   |   |   |
//       ==================================  
//                      LAN
//
// - UDP flows from n0 to n4 and back
// - UDP flows from n1 to n5 and back
// - UDP flows from n6 to n2 and back
// - UDP flows from n7 to n3 and back
// - DropTail queues 
// - Tracing of queues and packet receptions to file "assignment3.tr"
#include <time.h>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/rng-seed-manager.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ass3UdpEchoExample");

int 
main (int argc, char *argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
  RngSeedManager::SetSeed((unsigned int)time(NULL));
#if 0
  LogComponentEnable ("ass3UdpEchoExample", LOG_LEVEL_INFO);
  LogComponentEnable ("ass3UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("ass3UdpEchoServerApplication", LOG_LEVEL_INFO);
#endif
//
// Allow the user to override any of the defaults and the above Bind() at
// run-time, via command-line arguments
//
  Address serverAddress1, serverAddress2, serverAddress3,serverAddress4;
  uint32_t maxPacketCount = 1000;
  uint32_t packetSize = 1024;
  Time interPacketInterval = Seconds (0.01);

  CommandLine cmd;
  cmd.AddValue ("n","maxPacketCount",maxPacketCount);
  cmd.AddValue ("s","packetSize",packetSize);
  cmd.AddValue ("i","interPacketInterval",interPacketInterval);
  cmd.Parse (argc, argv);
//
// Explicitly create the nodes required by the topology (shown above).
//
//  NS_LOG_INFO ("Create nodes.");
  NodeContainer n;
  n.Create (8);

  InternetStackHelper internet;
  internet.Install (n);

//  NS_LOG_INFO ("Create channels.");
//
// Explicitly create the channels required by the topology (shown above).
//
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (1024000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1400));
  NetDeviceContainer d = csma.Install (n);

//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
//  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (d);
  serverAddress1 = Address(i.GetAddress (4));
  serverAddress2 = Address(i.GetAddress (5));
  serverAddress3 = Address(i.GetAddress (2));
  serverAddress4 = Address(i.GetAddress (3));
//  NS_LOG_INFO ("Create Applications.");
//
// Create a UdpEchoServer application on node one.
//
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server1 (port);
  ApplicationContainer apps1 = server1.Install (n.Get (4));
  apps1.Start (Seconds (1.0));
  apps1.Stop (Seconds (10.0));
  UdpEchoServerHelper server2 (port);
  ApplicationContainer apps2 = server2.Install (n.Get (5));
  apps2.Start (Seconds (1.0));
  apps2.Stop (Seconds (10.0));
  UdpEchoServerHelper server3 (port);
  ApplicationContainer apps3 = server3.Install (n.Get (2));
  apps3.Start (Seconds (1.0));
  apps3.Stop (Seconds (10.0));
  UdpEchoServerHelper server4 (port);
  ApplicationContainer apps4 = server4.Install (n.Get (3));
  apps4.Start (Seconds (1.0));
  apps4.Stop (Seconds (10.0));

//
// Create a UdpEchoClient application to send UDP datagrams from node zero to
// node one.
//
  UdpEchoClientHelper client1 (serverAddress1, port);
  client1.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client1.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client1.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps1 = client1.Install (n.Get (0));
  apps1.Start (Seconds (2.0));
  apps1.Stop (Seconds (10.0));

  UdpEchoClientHelper client2 (serverAddress2, port);
  client2.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client2.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client2.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps2 = client2.Install (n.Get (1));
  apps2.Start (Seconds (2.0));
  apps2.Stop (Seconds (10.0));

  UdpEchoClientHelper client3 (serverAddress3, port);
  client3.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client3.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client3.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps3 = client3.Install (n.Get (6));
  apps3.Start (Seconds (2.0));
  apps3.Stop (Seconds (10.0));

  UdpEchoClientHelper client4 (serverAddress4, port);
  client4.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client4.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client4.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps4 = client4.Install (n.Get (7));
  apps4.Start (Seconds (2.0));
  apps4.Stop (Seconds (10.0));

#if 0
//
// Users may find it convenient to initialize echo packets with actual data;
// the below lines suggest how to do this
//
  client.SetFill (apps.Get (0), "Hello World");

  client.SetFill (apps.Get (0), 0xa5, 1024);

  uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  client.SetFill (apps.Get (0), fill, sizeof(fill), 1024);
#endif

  AsciiTraceHelper ascii;
  csma.EnableAsciiAll (ascii.CreateFileStream ("assignment3.tr"));
  csma.EnablePcapAll ("assignment3", false);

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
