// /* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
// /*
//  * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
//  *
//  * This program is free software; you can redistribute it and/or modify
//  * it under the terms of the GNU General Public License version 2 as
//  * published by the Free Software Foundation;
//  *
//  * This program is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied warranty of
//  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  * GNU General Public License for more details.
//  *
//  * You should have received a copy of the GNU General Public License
//  * along with this program; if not, write to the Free Software
//  * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  *
//  * Author: Jaume Nin <jnin@cttc.es>
//  */

// #include "ns3/core-module.h"
// #include "ns3/network-module.h"
// #include "ns3/mobility-module.h"
// #include "ns3/lte-module.h"
// #include "ns3/config-store.h"
// #include <ns3/buildings-propagation-loss-model.h>
// #include <ns3/buildings-helper.h>
// #include <ns3/radio-environment-map-helper.h>
// #include <iomanip>
// #include <string>
// #include <vector>
// #include "ns3/point-to-point-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/applications-module.h"
// #include "ns3/config-store-module.h"
// #include "ns3/lte-module.h"

// #include "ns3/radio-bearer-stats-calculator.h"
// #include "ns3/rectangle.h"
// #include <ns3/buildings-module.h>
// #include <ns3/spectrum-helper.h>
// #include <iostream>
// #include <algorithm>
// #include <sstream>
// #include <array>
// #include <utility>

// //#include "ns3/gtk-config-store.h"

// //put in header file
// // a type to wrap another function type with `operator==` that says they're never equal
// template <typename F>
// struct func_with_equality {
//   // the function we're wrapping
//   F f;

//   // these functions are never equal to each other
//   friend bool operator==(const func_with_equality& lhs, const func_with_equality& rhs) {
//     return false;
//   }
//   friend bool operator!=(const func_with_equality& lhs, const func_with_equality& rhs) {
//     return true;
//   }


//   template <typename... Args>
//   auto operator()(Args&&... args)
//     -> decltype(f(std::forward<Args>(args)...))
//   {
//    return f(std::forward<Args>(args)...);
//   }
// };

// // a function that creates a callback function from an arbitrary lambda
// // R => the return type of the callback we want to create
// // Args... => the argument types of the callback
// // F => the type of the functor that was passed here, deduced on call
// template <typename R, typename... Args, typename F>
// ns3::Callback<R, Args...> make_nice_callback(const F& f) {
//   static_assert(sizeof...(Args) <= 9, "ns3::Callback only supports up to 9 arguments");
//   return ns3::Callback<R, Args...>(func_with_equality<F>{ f }, true, true);
// }

// using namespace ns3;
// using std::vector;

// uint32_t ByteCounter1 = 0, ByteCounter2 = 0, ByteCounter3 = 0;
// uint32_t oldByteCounter1 = 0, oldByteCounter2 = 0, oldByteCounter3 = 0;
// uint16_t cellID = 0;

// std::vector<uint32_t> byteCounterVector(20, 0);
// std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
// std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
// std::vector<double> throughPutVector(20, 0);
// std::vector<uint32_t> cellid(20, 0);
// int counterRP = 0;



// void FindCurrentCell1(std::string context, uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
// {
//   cellID = cellId;
// //  std::cout << std::setw(9) << cellId;

// }

// void FindCurrentCell(std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
// {
//   cellID = cellId;
// //  if(isServingCell)
// //    {
// //      std::cout << std::setw(9) << Simulator::Now ().GetSeconds () << std::setw(9) << cellId;
// //    }

// }



// void
// ReceivePacket1 (Ptr<const Packet> packet, const Address &)
// {
//   ByteCounter1 += packet->GetSize ();
// }

// void
// ReceivePacket2 (Ptr<const Packet> packet, const Address &)
// {
//   ByteCounter2 += packet->GetSize ();
// }

// void
// ReceivePacket3 (Ptr<const Packet> packet, const Address &)
// {
//   ByteCounter3 += packet->GetSize ();
// }

// void
// Throughput (Time binSize)
// {
//   double  throughput1 = (ByteCounter1 - oldByteCounter1) * 8 / binSize.GetSeconds () / 1024 / 1024;
//   double  throughput2 = (ByteCounter2 - oldByteCounter2) * 8 / binSize.GetSeconds () / 1024 / 1024;
//   double  throughput3 = (ByteCounter3 - oldByteCounter3) * 8 / binSize.GetSeconds () / 1024 / 1024;

// //  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ();
// //
// //  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ();
// //
// //  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements"
// //      , MakeCallback (&FindCurrentCell));

// //    Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
// //        , MakeCallback (&FindCurrentCell1));

//   std::cout << std::setw(9) << Simulator::Now ().GetSeconds ()<< std::setw(9) << cellID << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;

// //

// //  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
// //      , MakeCallback (&FindCurrentCell1));

// //  std::cout << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;


//   oldByteCounter1 = ByteCounter1;
//   oldByteCounter2 = ByteCounter2;
//   oldByteCounter3 = ByteCounter3;



//   Simulator::Schedule (binSize, &Throughput, binSize);
// }




// void RsrpRsrqFromUes(std::string context, uint16_t rnti, uint16_t cellId, double rsrp, double rsrq, bool isServingCell, uint8_t componentCarrierId)
// {



//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(15) << rsrp;
//   std::cout << std::setw(15) << rsrq << std::endl;
// }
// void RsrpSinrFromUes (std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
// {
//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(15) << rsrp;
//   std::cout << std::setw(15) << 10 * std::log10(sinr) << std::endl;
// }


// void
// Throughput1 (Time binSize, std::vector<double> throughPutVectors, std::vector<uint32_t> byteCounterVectors, std::vector<uint32_t> oldByteCounterVectors, int numUEs)
// {


//   for (int i = 0; i < counterRP; i++)
//   {



//     throughPutVector.at(i) = (byteCounterVector.at(i) - oldByteCounterVector.at(i)) * 8 / binSize.GetSeconds () / 1024 / 1024;
//     oldByteCounterVector.at(i) = byteCounterVector.at(i);
//   };


//   //std::cout << "this: " <<throughPutVector.at(1) << std::endl;
//   //std::cout << "this: "  <<throughPutVector.at(2) << std::endl;

//   std::cout << std::setw(9) << Simulator::Now ().GetSeconds () << std::setw(9);

//   for (int i = 0; i < counterRP; i++)
//   {
//     std::cout << std::setw(11) << throughPutVector.at(i);
//   }


//   //for (double i : throughPutVector)
//   //{
//   //  std::cout << std::setw(11) << i;
//   //};

//   std::cout << std::endl;

//   //cout statement

//   Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr"
//       , MakeCallback (&FindCurrentCell));


//   Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEs);

// }
// //#######################################################ENB##########################################################3

// void
// PrintGnuplottableUeListToFile (std::string filename)
// {
//   std::ofstream outFile;
//   outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
//   if (!outFile.is_open ())
//     {
// //      NS_LOG_ERROR ("Can't open file " << filename);
//       return;
//     }
//   for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
//     {
//       Ptr<Node> node = *it;
//       int nDevs = node->GetNDevices ();
//       for (int j = 0; j < nDevs; j++)
//         {
//           Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
//           if (uedev)
//             {
//               Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
//               outFile << "set label \"UE#" << uedev->GetImsi ()
//                       << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,10\" textcolor rgb \"black\" front point pt 5 ps 1 lc rgb \"black\" offset 0,0"
//                       << std::endl;
//             }
//         }
//     }
// }

// void
// PrintGnuplottableEnbListToFile (std::string filename)
// {
//   std::ofstream outFile;
//   outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
//   if (!outFile.is_open ())
//     {
// //      NS_LOG_ERROR ("Can't open file " << filename);
//       return;
//     }
//   for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
//     {
//       Ptr<Node> node = *it;
//       int nDevs = node->GetNDevices ();
//       for (int j = 0; j < nDevs; j++)
//         {
//           Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
//           if (enbdev)
//             {
//               Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
//               outFile << "set label \"Cell#" << enbdev->GetCellId ()
//                       << "\" at "<< pos.x << "," << pos.y
//                       << " left font \"Helvetica,4\" textcolor rgb \"black\" front  point pt 3 ps 1 lc rgb \"cyan\" offset 0,0"
//                       << std::endl;
//             }
//         }
//     }
// }

// void
// SaveUePosition (NodeContainer nodes, std::ostream *os, double interval)
// {
//   for (NodeContainer::Iterator node = nodes.Begin (); node != nodes.End (); ++node)
//     {
//       uint32_t nodeId = (*node)->GetId();
//       uint64_t nodeImsi = (Ptr<NetDevice>((*node)->GetDevice(0)))->GetObject<LteUeNetDevice> ()->GetImsi ();
//       Ptr<MobilityModel> mobModel = (*node)->GetObject<MobilityModel>();
//       Vector3D pos = mobModel->GetPosition();

//       // Prints position and velocities
//       *os << Simulator::Now().GetSeconds() << " "
//           << nodeId << " "
//           << nodeImsi << " "
//           << pos.x << " " << pos.y << " " << std::endl;
//     }
//   Simulator::Schedule(Seconds(interval), &SaveUePosition, nodes, os, interval);
// }





// //##########################################################################################################
// //##########################################################################################################
// //#########################################MAIN#############################################################
// //##########################################################################################################
// //##########################################################################################################



// int
// main (int argc, char *argv[])
// {
//   int nUEs = 1;
//   int neNBs = 1;
//   Time::SetResolution (Time::NS);
//   // Create one eNodeB per room + one 3 sector eNodeB (i.e. 3 eNodeB) + one regular eNodeB
//   uint16_t d = 500;
//   double bsTxPower = 25;
// //  bool enableFading = true;
//   Time interPacketInterval = MilliSeconds (1);
//   bool disableDl = false, disableUl = false;
//   double enbHeight = 5;
//   double ueHeight = 1.5;
//   bool enableSectors = false;
//   bool enableRem = false;
//   bool enablesignalpower = false, enablesinr = false, enableInstTput = false;
//   bool enableTput = false;

//   uint32_t dlEarfcn = 100;
//   uint16_t enbBw = 6;

//   uint64_t runId =1 ;


//   // Command line arguments
//   CommandLine cmd (__FILE__);
//   cmd.AddValue ("enableSectors", "Sectors in the cell", enableSectors);
//   cmd.AddValue ("enableRem", "Sectors in the cell", enableRem);
//   cmd.AddValue ("enablesignalpower", "get rsrp rsrq stats", enablesignalpower);
//   cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
//   cmd.AddValue ("enableTput", "Enable throughput stats", enableTput);
//   cmd.AddValue ("bsTxPower", "Base station transmit power", bsTxPower);
//   cmd.AddValue ("enablesinr", "Enable sinr", enablesinr);
//   cmd.AddValue ("runId", "Randomization parameter", runId);




//   cmd.Parse (argc, argv);

//   ConfigStore inputConfig;
//   inputConfig.ConfigureDefaults ();

//   // parse again so you can override default values from the command line
//   cmd.Parse(argc, argv);

//   RngSeedManager::SetRun(runId);

//   Ptr < LteHelper > lteHelper = CreateObject<LteHelper> ();
//   Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
//   lteHelper->SetEpcHelper (epcHelper);
//   //lteHelper->EnableLogComponents ();


// //  RngSeedManager::SetSeed (seed);  // Changes seed from default of 1 to 3

//   Ptr<UniformRandomVariable> d0 = CreateObject<UniformRandomVariable> ();
//   d0->SetAttribute ("Min", DoubleValue (50));
//   d0->SetAttribute ("Max", DoubleValue (100));
//   Ptr<UniformRandomVariable> d1 = CreateObject<UniformRandomVariable> ();
//   d1->SetAttribute ("Min", DoubleValue (300));
//   d1->SetAttribute ("Max", DoubleValue (500));
//   Ptr<UniformRandomVariable> d2 = CreateObject<UniformRandomVariable> ();
//   d2->SetAttribute ("Min", DoubleValue (600));
//   d2->SetAttribute ("Max", DoubleValue (800));
//   Ptr<UniformRandomVariable> e0 = CreateObject<UniformRandomVariable> ();
//   e0->SetAttribute ("Min", DoubleValue (1.75));
//   e0->SetAttribute ("Max", DoubleValue (2));
//   Ptr<UniformRandomVariable> e1 = CreateObject<UniformRandomVariable> ();
//   e1->SetAttribute ("Min", DoubleValue (3.4));
//   e1->SetAttribute ("Max", DoubleValue (4.4));
//   Ptr<UniformRandomVariable> e2 = CreateObject<UniformRandomVariable> ();
//   e2->SetAttribute ("Min", DoubleValue (4.8));
//   e2->SetAttribute ("Max", DoubleValue (5.8));


//   // std::cout<< "d0: " << d0->GetValue() << ", d1: " << d1->GetValue() << ", d2: " << d2->GetValue() << ", e0: " << e0->GetValue() << ", e1: " << e1->GetValue() << ", e2: " << e2->GetValue() << std::endl;

//   lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));

//   lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue(d0->GetValue()));
//   lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue(d1->GetValue()));
//   lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue(d2->GetValue()));
//   lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue(e0->GetValue()));
//   lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue(e1->GetValue()));
//   lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue(e2->GetValue()));



// //  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
// //  lteHelper->SetAttribute("UeMeasurementsFilterPeriod", TimeValue(MilliSeconds(20)));

//   Config::SetDefault("ns3::LteUePhy::UeMeasurementsFilterPeriod", TimeValue(MilliSeconds(100)));
//   Config::SetDefault("ns3::LteUePhy::RsrpSinrSamplePeriod", UintegerValue(100));

// //  if(enableFading)
// //    {
// //      lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
// //
// //      std::ifstream ifTraceFile;
// //      ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
// //      if (ifTraceFile.good ())
// //        {
// //          // script launched by test.py
// //          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
// //        }
// //      else
// //        {
// //          // script launched as an example
// //          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
// //        }
// //
// //      // these parameters have to be set only in case of the trace format
// //      // differs from the standard one, that is
// //      // - 10 seconds length trace
// //      // - 10,000 samples
// //      // - 0.5 seconds for window size
// //      // - 100 RB
// //      lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
// //      lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
// //      lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
// //      lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
// //    }

//   // Create Nodes: eNodeB and UE
//   NodeContainer enbNodes, oneSectorNodes, threeSectorNodes, ueNodes, oneSectorueNodes,threeSectorueNodes;

//   MobilityHelper mobilityEnb, mobilityUe;
//   Ptr < ListPositionAllocator > enbPa = CreateObject<ListPositionAllocator> ();
//   Ptr < ListPositionAllocator > uePa = CreateObject<ListPositionAllocator> ();

//   // Create Devices and install them in the Nodes (eNB and UE)
//   NetDeviceContainer enbDevs, ueDevs;

//   lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (dlEarfcn));
//   lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (dlEarfcn + 18000));
//   lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (enbBw));
//   lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (enbBw));

//   if(enableSectors)
//     {
//       nUEs = 3;
//       neNBs = nUEs * neNBs;
//       threeSectorNodes.Create (neNBs);
//       threeSectorueNodes.Create(nUEs);
//       enbNodes.Add (threeSectorNodes);
//       ueNodes.Add(threeSectorueNodes);
//       enbPa->Add (Vector (0, 0, enbHeight));


// //      uePa->Add (Vector (-0.75*d*3 , 1.75*d*3, ueHeight));
// //      uePa->Add (Vector (-0.8*d*3 , -1.35*d*3, ueHeight));
//       uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));
//       uePa->Add (Vector (3.35*d*3 , 1.00*d*3, ueHeight));
//       uePa->Add (Vector (0.5*d*3 , 0.2*d*3, ueHeight));


//       mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//       mobilityEnb.SetPositionAllocator(enbPa);
//       mobilityEnb.Install(enbNodes);

//       mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//       mobilityUe.SetPositionAllocator(uePa);
//       mobilityUe.Install(ueNodes);

//       // power setting for three-sector macrocell
//       Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (bsTxPower));

//       // Beam width is made quite narrow so sectors can be noticed in the REM

//       lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
//       lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (0));
//       lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
//       lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
//       enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (0)));


//       lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
//       lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (360/3));
//       lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
//       lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
//       enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (1)));

//       Ptr<LteEnbNetDevice> enbPtr1 = enbDevs.Get(1)->GetObject<LteEnbNetDevice>();
// //      std::cout<<enbPtr1->GetDlEarfcn()<<std::endl;
// //      std::cout<<enbPtr1->GetUlEarfcn()<<std::endl;
//       enbPtr1->SetDlEarfcn(dlEarfcn + 100);
//       enbPtr1->SetUlEarfcn(dlEarfcn + 100 + 18000);
// //      std::cout<<enbPtr1->GetDlEarfcn()<<std::endl;
// //      std::cout<<enbPtr1->GetUlEarfcn()<<std::endl;

//       lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
//       lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (2*360/3));
//       lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
//       lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
//       enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (2)));

//       Ptr<LteEnbNetDevice> enbPtr2 = enbDevs.Get(2)->GetObject<LteEnbNetDevice>();
// //      std::cout<<enbPtr2->GetDlEarfcn()<<std::endl;
// //      std::cout<<enbPtr2->GetUlEarfcn()<<std::endl;
//       enbPtr2->SetDlEarfcn(dlEarfcn + 200);
//       enbPtr2->SetUlEarfcn(dlEarfcn + 200 + 18000);
// //      std::cout<<enbPtr2->GetDlEarfcn()<<std::endl;
// //      std::cout<<enbPtr2->GetUlEarfcn()<<std::endl;

//       ueDevs = lteHelper->InstallUeDevice (ueNodes);

//     }
//   else if(!enableSectors)
//     {
//       nUEs = 3;
//       oneSectorNodes.Create (neNBs);
//       oneSectorueNodes.Create(nUEs);
//       enbNodes.Add (oneSectorNodes);
//       ueNodes.Add(oneSectorueNodes);
//       enbPa->Add (Vector (0 , 0, enbHeight));

// //      uePa->Add (Vector (-0.75*d*3 , 1.75*d*3, ueHeight));
// //      uePa->Add (Vector (-0.8*d*3 , -1.35*d*3, ueHeight));
// //      uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));

//       uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));
//       uePa->Add (Vector (3.35*d*3 , 1.00*d*3, ueHeight));
//       uePa->Add (Vector (0.5*d*3 , 0.2*d*3, ueHeight));

//       mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//       mobilityEnb.SetPositionAllocator(enbPa);
//       mobilityEnb.Install(enbNodes);

//       mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
//       mobilityUe.SetPositionAllocator(uePa);
//       mobilityUe.Install(ueNodes);


//       // power setting for three-sector macrocell
//       Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (bsTxPower));
//       lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (dlEarfcn));
//       lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (dlEarfcn+ 18000));
//       enbDevs.Add(lteHelper->InstallEnbDevice (oneSectorNodes.Get(0)));
//       ueDevs = lteHelper->InstallUeDevice (ueNodes);

//     }

//   Ptr<Node> pgw = epcHelper->GetPgwNode ();
//   // Create a single RemoteHost
//   NodeContainer remoteHostContainer;
//   remoteHostContainer.Create (1);
//   Ptr<Node> remoteHost = remoteHostContainer.Get (0);
//   InternetStackHelper internet;
//   internet.Install (remoteHostContainer);

//   // Create the Internet
//   PointToPointHelper p2ph;
//   p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
//   p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
//   p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
//   NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
//   Ipv4AddressHelper ipv4h;
//   ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
//   Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
//   // interface 0 is localhost, 1 is the p2p device
//   Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

//   Ipv4StaticRoutingHelper ipv4RoutingHelper;
//   Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
//   remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


//   // Install the IP stack on the UEs
//   internet.Install (ueNodes);
//   Ipv4InterfaceContainer ueIpIface;
//   ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
//   // Assign IP address to UEs, and install applications
//   for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
//     {
//       Ptr<Node> ueNode = ueNodes.Get (u);
//       // Set the default gateway for the UE
//       Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
//       ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
//     }


//     lteHelper->Attach(ueDevs);

// //  for (uint32_t i = 0; i < enbNodes.GetN(); i++)
// //    {
// //      for (uint32_t j = 0; j < ueNodes.GetN(); j++)
// //        {
// //          lteHelper->Attach (ueDevs.Get(j), enbDevs.Get (i));
// //        }
// //    }


//   // Install and start applications on UEs and remote host
//   uint16_t dlPort = 1100;
//   uint16_t ulPort = 2000;
//   ApplicationContainer clientApps;
//   ApplicationContainer serverApps;

//   Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
//   startTimeSeconds->SetAttribute ("Min", DoubleValue (0.2602));
//   startTimeSeconds->SetAttribute ("Max", DoubleValue (0.2603));
//   Time startTime = Seconds (startTimeSeconds->GetValue ());
//   for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
//     {
//       if (!disableDl)
//         {
//           PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
//           serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

//           UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
//           dlClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
//           dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
//           clientApps.Add (dlClient.Install (remoteHost));
//         }

//       if (!disableUl)
//         {
//           ++ulPort;
//           PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
//           serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

//           UdpClientHelper ulClient (remoteHostAddr, ulPort);
//           ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
//           ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
//           clientApps.Add (ulClient.Install (ueNodes.Get(u)));
//         }

//     }

//   serverApps.Start (startTime);
//   clientApps.Start (startTime);


//   // by default, simulation will anyway stop right after the REM has been generated

// //  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
// //      , MakeCallback (&FindCurrentCell));

//   Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();


//   if(enableRem)
//     {

//       PrintGnuplottableEnbListToFile ("enbs.txt");
//       PrintGnuplottableUeListToFile ("ues.txt");
//       remHelper->SetAttribute ("Channel", PointerValue (lteHelper->GetDownlinkSpectrumChannel ()));
//       remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
//       remHelper->SetAttribute ("XMin", DoubleValue (-12*d));
//       remHelper->SetAttribute ("XMax", DoubleValue (12*d));
//       remHelper->SetAttribute ("YMin", DoubleValue (-12*d));
//       remHelper->SetAttribute ("YMax", DoubleValue (12*d));
//       remHelper->SetAttribute ("Z", DoubleValue (1.5));
// //      remHelper->SetAttribute ("Earfcn", IntegerValue (100));
// //      remHelper->SetAttribute ("Bandwidth", IntegerValue (enbBw));
// //      remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));

//       remHelper->Install ();
//     }

//   if(enablesignalpower)
//     {
//       std::cout << "#";
//       std::cout << std::setw(9) << "time";
//       std::cout << std::setw(8) << "cellID";
//       std::cout << std::setw(8) << "rnti" ;
//       std::cout << std::setw(12) << "rsrp" ;
//       std::cout << std::setw(12) << "rsrq" << std::endl;

//       Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements"
//           , MakeCallback (&RsrpRsrqFromUes));
//     }

//   if(enablesinr)
//     {
//       std::cout << "#";
//       std::cout << std::setw(9) << "time";
//       std::cout << std::setw(8) << "cellID";
//       std::cout << std::setw(8) << "rnti" ;
//       std::cout << std::setw(12) << "rsrp" ;
//       std::cout << std::setw(12) << "sinr" << std::endl;

//       Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr"
//           , MakeCallback (&RsrpSinrFromUes));
//     }

//   if(enableInstTput)
//     {
//       std::ostringstream oss1, oss2, oss3;
//       oss1 << "/NodeList/" << ueNodes.Get (0)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
//       oss2<< "/NodeList/" << ueNodes.Get (1)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
//       oss3 << "/NodeList/" << ueNodes.Get (2)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

//       Config::ConnectWithoutContext (oss1.str (), MakeCallback (&ReceivePacket1));
//       Config::ConnectWithoutContext (oss2.str (), MakeCallback (&ReceivePacket2));
//       Config::ConnectWithoutContext (oss3.str (), MakeCallback (&ReceivePacket3));

//       std::cout << "#";
//       std::cout << std::setw(9) << "time";
//       std::cout << std::setw(9) << "cellID";
//       std::cout << std::setw(11) << "Throughput-UE1 (Mbps)" << std::setw(11) << "Throughput-UE2 (Mbps)" << std::setw(11) << "Throughput-UE3 (Mbps)" << std::endl;
//       Time binSize = Seconds (0.02);
//       Simulator::Schedule (Seconds (0.1), &Throughput, binSize);

//     }

//   if(enableTput)
//     {

//       //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
//       //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
//       //std::vector<double> throughputVector(numUEsNEW, 0);

//       for (int i = 0; i < nUEs; i++)
//       {


// //        std::cout << "Number of numUEsNEW: " << nUEs << std::endl;
// //
// //        std::cout << "Counter is the value: " << counterRP << std::endl;

//         auto function = [i](Ptr<const Packet> packet, const Address&) {
//             byteCounterVector.at(i) += packet->GetSize();
//           };


//         ossGroup.at(i) << "/NodeList/" << ueNodes.Get (counterRP)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

//         Config::ConnectWithoutContext(
//                ossGroup.at(i).str(),
//             make_nice_callback<void, Ptr<const Packet>, const Address&>(function)
//           );

// //        std::cout << "Connected to the UE device " << counterRP << std::endl;

//         counterRP = counterRP + 1;



//       };

//       std::cout << "#";
//       std::cout << std::setw(9) << "time" << std::setw(9) << "cellID";
//       for (int i = 0; i < nUEs; i++)
//       {
//           std::cout << std::setw(11) << "Throughput-UE" << i << "(Mbps)";
//       }

//       std::cout << std::endl;
//       Time binSize = Seconds (0.1);
//       Simulator::Schedule (Seconds (0.1), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, nUEs);
//     }

//   Simulator::Stop (Seconds (2));
//   Simulator::Run ();


// //  GtkConfigStore config;
// //  config.ConfigureAttributes ();

//   lteHelper = 0;
//   Simulator::Destroy ();
//   return 0;
// }


/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-propagation-loss-model.h>
#include <ns3/buildings-helper.h>
#include <ns3/radio-environment-map-helper.h>
#include <iomanip>
#include <string>
#include <vector>
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/lte-module.h"

#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/rectangle.h"
#include <ns3/buildings-module.h>
#include <ns3/spectrum-helper.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <array>
#include <utility>

//#include "ns3/gtk-config-store.h"

//put in header file
// a type to wrap another function type with `operator==` that says they're never equal
template <typename F>
struct func_with_equality {
  // the function we're wrapping
  F f;

  // these functions are never equal to each other
  friend bool operator==(const func_with_equality& lhs, const func_with_equality& rhs) {
    return false;
  }
  friend bool operator!=(const func_with_equality& lhs, const func_with_equality& rhs) {
    return true;
  }


  template <typename... Args>
  auto operator()(Args&&... args)
    -> decltype(f(std::forward<Args>(args)...))
  {
   return f(std::forward<Args>(args)...);
  }
};

// a function that creates a callback function from an arbitrary lambda
// R => the return type of the callback we want to create
// Args... => the argument types of the callback
// F => the type of the functor that was passed here, deduced on call
template <typename R, typename... Args, typename F>
ns3::Callback<R, Args...> make_nice_callback(const F& f) {
  static_assert(sizeof...(Args) <= 9, "ns3::Callback only supports up to 9 arguments");
  return ns3::Callback<R, Args...>(func_with_equality<F>{ f }, true, true);
}

using namespace ns3;
using std::vector;

uint32_t ByteCounter1 = 0, ByteCounter2 = 0, ByteCounter3 = 0;
uint32_t oldByteCounter1 = 0, oldByteCounter2 = 0, oldByteCounter3 = 0;
uint16_t cellID = 0;

std::vector<uint32_t> byteCounterVector(20, 0);
std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
std::vector<double> throughPutVector(20, 0);
std::vector<uint32_t> cellid(20, 0);
int counterRP = 0;



void FindCurrentCell1(std::string context, uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
{
  cellID = cellId;
//  std::cout << std::setw(9) << cellId;

}

void FindCurrentCell(std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{
  cellID = cellId;
//  if(isServingCell)
//    {
//      std::cout << std::setw(9) << Simulator::Now ().GetSeconds () << std::setw(9) << cellId;
//    }

}



void
ReceivePacket1 (Ptr<const Packet> packet, const Address &)
{
  ByteCounter1 += packet->GetSize ();
}

void
ReceivePacket2 (Ptr<const Packet> packet, const Address &)
{
  ByteCounter2 += packet->GetSize ();
}

void
ReceivePacket3 (Ptr<const Packet> packet, const Address &)
{
  ByteCounter3 += packet->GetSize ();
}

void
Throughput (Time binSize)
{
  double  throughput1 = (ByteCounter1 - oldByteCounter1) * 8 / binSize.GetSeconds () / 1024 / 1024;
  double  throughput2 = (ByteCounter2 - oldByteCounter2) * 8 / binSize.GetSeconds () / 1024 / 1024;
  double  throughput3 = (ByteCounter3 - oldByteCounter3) * 8 / binSize.GetSeconds () / 1024 / 1024;

//  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ();
//
//  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ();
//
//  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements"
//      , MakeCallback (&FindCurrentCell));

//    Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
//        , MakeCallback (&FindCurrentCell1));

  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ()<< std::setw(9) << cellID << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;

//

//  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
//      , MakeCallback (&FindCurrentCell1));

//  std::cout << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;


  oldByteCounter1 = ByteCounter1;
  oldByteCounter2 = ByteCounter2;
  oldByteCounter3 = ByteCounter3;



  Simulator::Schedule (binSize, &Throughput, binSize);
}




void RsrpRsrqFromUes(std::string context, uint16_t rnti, uint16_t cellId, double rsrp, double rsrq, bool isServingCell, uint8_t componentCarrierId)
{



  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << rsrp;
  std::cout << std::setw(15) << rsrq << std::endl;
}
void RsrpSinrFromUes (std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << rsrp;
  std::cout << std::setw(15) << 10 * std::log10(sinr) << std::endl;
}


void
Throughput1 (Time binSize, std::vector<double> throughPutVectors, std::vector<uint32_t> byteCounterVectors, std::vector<uint32_t> oldByteCounterVectors, int numUEs)
{


  for (int i = 0; i < counterRP; i++)
  {



    throughPutVector.at(i) = (byteCounterVector.at(i) - oldByteCounterVector.at(i)) * 8 / binSize.GetSeconds () / 1024 / 1024;
    oldByteCounterVector.at(i) = byteCounterVector.at(i);
  };


  //std::cout << "this: " <<throughPutVector.at(1) << std::endl;
  //std::cout << "this: "  <<throughPutVector.at(2) << std::endl;

  std::cout << std::setw(9) << Simulator::Now ().GetSeconds () << std::setw(9);

  for (int i = 0; i < counterRP; i++)
  {
    std::cout << std::setw(11) << throughPutVector.at(i);
  }


  //for (double i : throughPutVector)
  //{
  //  std::cout << std::setw(11) << i;
  //};

  std::cout << std::endl;

  //cout statement

  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr"
      , MakeCallback (&FindCurrentCell));


  Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEs);

}
//#######################################################ENB##########################################################3

void
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
//      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"UE#" << uedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,10\" textcolor rgb \"black\" front point pt 5 ps 1 lc rgb \"black\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

void
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
//      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"Cell#" << enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left font \"Helvetica,4\" textcolor rgb \"black\" front  point pt 3 ps 1 lc rgb \"cyan\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

void
SaveUePosition (NodeContainer nodes, std::ostream *os, double interval)
{
  for (NodeContainer::Iterator node = nodes.Begin (); node != nodes.End (); ++node)
    {
      uint32_t nodeId = (*node)->GetId();
      uint64_t nodeImsi = (Ptr<NetDevice>((*node)->GetDevice(0)))->GetObject<LteUeNetDevice> ()->GetImsi ();
      Ptr<MobilityModel> mobModel = (*node)->GetObject<MobilityModel>();
      Vector3D pos = mobModel->GetPosition();

      // Prints position and velocities
      *os << Simulator::Now().GetSeconds() << " "
          << nodeId << " "
          << nodeImsi << " "
          << pos.x << " " << pos.y << " " << std::endl;
    }
  Simulator::Schedule(Seconds(interval), &SaveUePosition, nodes, os, interval);
}





//##########################################################################################################
//##########################################################################################################
//#########################################MAIN#############################################################
//##########################################################################################################
//##########################################################################################################



int
main (int argc, char *argv[])
{
  int nUEs = 1;
  int neNBs = 1;
  Time::SetResolution (Time::NS);
  // Create one eNodeB per room + one 3 sector eNodeB (i.e. 3 eNodeB) + one regular eNodeB
  uint16_t d = 500;
  double bsTxPower = 25;
//  bool enableFading = true;
  Time interPacketInterval = MilliSeconds (1);
  bool disableDl = false, disableUl = false;
  double enbHeight = 5;
  double ueHeight = 1.5;
  bool enableSectors = false;
  bool enableRem = false;
  bool enablesignalpower = false, enablesinr = false, enableInstTput = false;
  bool enableTput = false;

  uint32_t dlEarfcn = 100;
  uint16_t enbBw = 6;

  uint64_t runId =1 ;


  // Command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("enableSectors", "Sectors in the cell", enableSectors);
  cmd.AddValue ("enableRem", "Sectors in the cell", enableRem);
  cmd.AddValue ("enablesignalpower", "get rsrp rsrq stats", enablesignalpower);
  cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
  cmd.AddValue ("enableTput", "Enable throughput stats", enableTput);
  cmd.AddValue ("bsTxPower", "Base station transmit power", bsTxPower);
  cmd.AddValue ("enbBw", "Base station bandwidth", enbBw);
  cmd.AddValue ("enablesinr", "Enable sinr", enablesinr);
  cmd.AddValue ("runId", "Randomization parameter", runId);





  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);

  RngSeedManager::SetRun(runId);

  Ptr < LteHelper > lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  //lteHelper->EnableLogComponents ();


//  RngSeedManager::SetSeed (seed);  // Changes seed from default of 1 to 3

  Ptr<UniformRandomVariable> d0 = CreateObject<UniformRandomVariable> ();
  d0->SetAttribute ("Min", DoubleValue (1));
  d0->SetAttribute ("Max", DoubleValue (5));
  Ptr<UniformRandomVariable> d1 = CreateObject<UniformRandomVariable> ();
  d1->SetAttribute ("Min", DoubleValue (200));
  d1->SetAttribute ("Max", DoubleValue (250));
  Ptr<UniformRandomVariable> d2 = CreateObject<UniformRandomVariable> ();
  d2->SetAttribute ("Min", DoubleValue (500));
  d2->SetAttribute ("Max", DoubleValue (600));
  Ptr<UniformRandomVariable> e0 = CreateObject<UniformRandomVariable> ();
  e0->SetAttribute ("Min", DoubleValue (1.75));
  e0->SetAttribute ("Max", DoubleValue (2));
  Ptr<UniformRandomVariable> e1 = CreateObject<UniformRandomVariable> ();
  e1->SetAttribute ("Min", DoubleValue (3.4));
  e1->SetAttribute ("Max", DoubleValue (4.4));
  Ptr<UniformRandomVariable> e2 = CreateObject<UniformRandomVariable> ();
  e2->SetAttribute ("Min", DoubleValue (4.8));
  e2->SetAttribute ("Max", DoubleValue (5.8));


//  std::cout<< "d0: " << d0->GetValue() << ", d1: " << d1->GetValue() << ", d2: " << d2->GetValue() << ", e0: " << e0->GetValue() << ", e1: " << e1->GetValue() << ", e2: " << e2->GetValue() << std::endl;

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));

  lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue(d0->GetValue()));
  lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue(d1->GetValue()));
  lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue(d2->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue(e0->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue(e1->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue(e2->GetValue()));



//  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisPropagationLossModel"));
//  lteHelper->SetAttribute("UeMeasurementsFilterPeriod", TimeValue(MilliSeconds(20)));

  Config::SetDefault("ns3::LteUePhy::UeMeasurementsFilterPeriod", TimeValue(MilliSeconds(100)));
  Config::SetDefault("ns3::LteUePhy::RsrpSinrSamplePeriod", UintegerValue(100));

//  if(enableFading)
//    {
//      lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
//
//      std::ifstream ifTraceFile;
//      ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
//      if (ifTraceFile.good ())
//        {
//          // script launched by test.py
//          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
//        }
//      else
//        {
//          // script launched as an example
//          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
//        }
//
//      // these parameters have to be set only in case of the trace format
//      // differs from the standard one, that is
//      // - 10 seconds length trace
//      // - 10,000 samples
//      // - 0.5 seconds for window size
//      // - 100 RB
//      lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
//      lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
//      lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
//      lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
//    }

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes, oneSectorNodes, threeSectorNodes, ueNodes, oneSectorueNodes,threeSectorueNodes;

  MobilityHelper mobilityEnb, mobilityUe;
  Ptr < ListPositionAllocator > enbPa = CreateObject<ListPositionAllocator> ();
  Ptr < ListPositionAllocator > uePa = CreateObject<ListPositionAllocator> ();

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs, ueDevs;

  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (dlEarfcn));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (dlEarfcn + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (enbBw));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (enbBw));

  if(enableSectors)
    {
      nUEs = 3;
      neNBs = nUEs * neNBs;
      threeSectorNodes.Create (neNBs);
      threeSectorueNodes.Create(nUEs);
      enbNodes.Add (threeSectorNodes);
      ueNodes.Add(threeSectorueNodes);
      enbPa->Add (Vector (0, 0, enbHeight));


//      uePa->Add (Vector (-0.75*d*3 , 1.75*d*3, ueHeight));
//      uePa->Add (Vector (-0.8*d*3 , -1.35*d*3, ueHeight));
      uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));
      uePa->Add (Vector (3.35*d*3 , 1.00*d*3, ueHeight));
      uePa->Add (Vector (0.5*d*3 , 0.2*d*3, ueHeight));


      mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobilityEnb.SetPositionAllocator(enbPa);
      mobilityEnb.Install(enbNodes);

      mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobilityUe.SetPositionAllocator(uePa);
      mobilityUe.Install(ueNodes);

      // power setting for three-sector macrocell
      Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (bsTxPower));

      // Beam width is made quite narrow so sectors can be noticed in the REM

      lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
      lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (0));
      lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
      lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
      enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (0)));


      lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
      lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (360/3));
      lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
      lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
      enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (1)));

      Ptr<LteEnbNetDevice> enbPtr1 = enbDevs.Get(1)->GetObject<LteEnbNetDevice>();
//      std::cout<<enbPtr1->GetDlEarfcn()<<std::endl;
//      std::cout<<enbPtr1->GetUlEarfcn()<<std::endl;
      enbPtr1->SetDlEarfcn(dlEarfcn + 100);
      enbPtr1->SetUlEarfcn(dlEarfcn + 100 + 18000);
//      std::cout<<enbPtr1->GetDlEarfcn()<<std::endl;
//      std::cout<<enbPtr1->GetUlEarfcn()<<std::endl;

      lteHelper->SetEnbAntennaModelType ("ns3::CosineAntennaModel");
      lteHelper->SetEnbAntennaModelAttribute ("Orientation", DoubleValue (2*360/3));
      lteHelper->SetEnbAntennaModelAttribute ("HorizontalBeamwidth", DoubleValue (100));
      lteHelper->SetEnbAntennaModelAttribute ("MaxGain", DoubleValue (0.0));
      enbDevs.Add ( lteHelper->InstallEnbDevice (threeSectorNodes.Get (2)));

      Ptr<LteEnbNetDevice> enbPtr2 = enbDevs.Get(2)->GetObject<LteEnbNetDevice>();
//      std::cout<<enbPtr2->GetDlEarfcn()<<std::endl;
//      std::cout<<enbPtr2->GetUlEarfcn()<<std::endl;
      enbPtr2->SetDlEarfcn(dlEarfcn + 200);
      enbPtr2->SetUlEarfcn(dlEarfcn + 200 + 18000);
//      std::cout<<enbPtr2->GetDlEarfcn()<<std::endl;
//      std::cout<<enbPtr2->GetUlEarfcn()<<std::endl;

      ueDevs = lteHelper->InstallUeDevice (ueNodes);

    }
  else if(!enableSectors)
    {
      nUEs = 3;
      oneSectorNodes.Create (neNBs);
      oneSectorueNodes.Create(nUEs);
      enbNodes.Add (oneSectorNodes);
      ueNodes.Add(oneSectorueNodes);
      enbPa->Add (Vector (0 , 0, enbHeight));

//      uePa->Add (Vector (-0.75*d*3 , 1.75*d*3, ueHeight));
//      uePa->Add (Vector (-0.8*d*3 , -1.35*d*3, ueHeight));
//      uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));

      uePa->Add (Vector (2.35*d*3 , -1.00*d*3, ueHeight));
      uePa->Add (Vector (3.35*d*3 , 1.00*d*3, ueHeight));
      uePa->Add (Vector (0.5*d*3 , 0.2*d*3, ueHeight));

      mobilityEnb.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobilityEnb.SetPositionAllocator(enbPa);
      mobilityEnb.Install(enbNodes);

      mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
      mobilityUe.SetPositionAllocator(uePa);
      mobilityUe.Install(ueNodes);


      // power setting for three-sector macrocell
      Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (bsTxPower));
      lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (dlEarfcn));
      lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (dlEarfcn+ 18000));
      enbDevs.Add(lteHelper->InstallEnbDevice (oneSectorNodes.Get(0)));
      ueDevs = lteHelper->InstallUeDevice (ueNodes);

    }

  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (10)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }


    lteHelper->Attach(ueDevs);

//  for (uint32_t i = 0; i < enbNodes.GetN(); i++)
//    {
//      for (uint32_t j = 0; j < ueNodes.GetN(); j++)
//        {
//          lteHelper->Attach (ueDevs.Get(j), enbDevs.Get (i));
//        }
//    }


  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0.2602));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.2603));
  Time startTime = Seconds (startTimeSeconds->GetValue ());
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      if (!disableDl)
        {
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

          UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
          dlClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
          dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
          clientApps.Add (dlClient.Install (remoteHost));
        }

      if (!disableUl)
        {
          ++ulPort;
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("Interval", TimeValue (interPacketInterval));
          ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
          clientApps.Add (ulClient.Install (ueNodes.Get(u)));
        }

    }

  serverApps.Start (startTime);
  clientApps.Start (startTime);


  // by default, simulation will anyway stop right after the REM has been generated

//  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
//      , MakeCallback (&FindCurrentCell));

  Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();


  if(enableRem)
    {

      PrintGnuplottableEnbListToFile ("enbs.txt");
      PrintGnuplottableUeListToFile ("ues.txt");
      remHelper->SetAttribute ("Channel", PointerValue (lteHelper->GetDownlinkSpectrumChannel ()));
      remHelper->SetAttribute ("OutputFile", StringValue ("rem.out"));
      remHelper->SetAttribute ("XMin", DoubleValue (-12*d));
      remHelper->SetAttribute ("XMax", DoubleValue (12*d));
      remHelper->SetAttribute ("YMin", DoubleValue (-12*d));
      remHelper->SetAttribute ("YMax", DoubleValue (12*d));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
//      remHelper->SetAttribute ("Earfcn", IntegerValue (100));
//      remHelper->SetAttribute ("Bandwidth", IntegerValue (enbBw));
//      remHelper->SetAttribute ("UseDataChannel", BooleanValue (true));

      remHelper->Install ();
    }

  if(enablesignalpower)
    {
      std::cout << "#";
      std::cout << std::setw(9) << "time";
      std::cout << std::setw(8) << "cellID";
      std::cout << std::setw(8) << "rnti" ;
      std::cout << std::setw(12) << "rsrp" ;
      std::cout << std::setw(12) << "rsrq" << std::endl;

      Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements"
          , MakeCallback (&RsrpRsrqFromUes));
    }

  if(enablesinr)
    {
      std::cout << "#";
      std::cout << std::setw(9) << "time";
      std::cout << std::setw(8) << "cellID";
      std::cout << std::setw(8) << "rnti" ;
      std::cout << std::setw(12) << "rsrp" ;
      std::cout << std::setw(12) << "sinr" << std::endl;

      Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr"
          , MakeCallback (&RsrpSinrFromUes));
    }

  if(enableInstTput)
    {
      std::ostringstream oss1, oss2, oss3;
      oss1 << "/NodeList/" << ueNodes.Get (0)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
      oss2<< "/NodeList/" << ueNodes.Get (1)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";
      oss3 << "/NodeList/" << ueNodes.Get (2)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

      Config::ConnectWithoutContext (oss1.str (), MakeCallback (&ReceivePacket1));
      Config::ConnectWithoutContext (oss2.str (), MakeCallback (&ReceivePacket2));
      Config::ConnectWithoutContext (oss3.str (), MakeCallback (&ReceivePacket3));

      std::cout << "#";
      std::cout << std::setw(9) << "time";
      std::cout << std::setw(9) << "cellID";
      std::cout << std::setw(11) << "Throughput-UE1 (Mbps)" << std::setw(11) << "Throughput-UE2 (Mbps)" << std::setw(11) << "Throughput-UE3 (Mbps)" << std::endl;
      Time binSize = Seconds (0.02);
      Simulator::Schedule (Seconds (0.1), &Throughput, binSize);

    }

  if(enableTput)
    {

      //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
      //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
      //std::vector<double> throughputVector(numUEsNEW, 0);

      for (int i = 0; i < nUEs; i++)
      {


//        std::cout << "Number of numUEsNEW: " << nUEs << std::endl;
//
//        std::cout << "Counter is the value: " << counterRP << std::endl;

        auto function = [i](Ptr<const Packet> packet, const Address&) {
            byteCounterVector.at(i) += packet->GetSize();
          };


        ossGroup.at(i) << "/NodeList/" << ueNodes.Get (counterRP)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

        Config::ConnectWithoutContext(
               ossGroup.at(i).str(),
            make_nice_callback<void, Ptr<const Packet>, const Address&>(function)
          );

//        std::cout << "Connected to the UE device " << counterRP << std::endl;

        counterRP = counterRP + 1;



      };

      std::cout << "#";
      std::cout << std::setw(9) << "time" << std::setw(9) << "cellID";
      for (int i = 0; i < nUEs; i++)
      {
          std::cout << std::setw(11) << "Throughput-UE" << i << "(Mbps)";
      }

      std::cout << std::endl;
      Time binSize = Seconds (0.1);
      Simulator::Schedule (Seconds (0.1), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, nUEs);
    }

  Simulator::Stop (Seconds (2));
  Simulator::Run ();


//  GtkConfigStore config;
//  config.ConfigureAttributes ();

  lteHelper = 0;
  Simulator::Destroy ();
  return 0;
}
