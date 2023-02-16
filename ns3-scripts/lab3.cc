/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Jaume Nin <jaume.nin@cttc.cat>
 *          Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/lte-module.h"
#include "ns3/flow-monitor-module.h"
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <array>
#include <utility>

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

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeBs,
 * attaches one UE per eNodeB starts a flow for each UE to and from a remote host.
 * It also starts another flow between each UE pair.
 */

NS_LOG_COMPONENT_DEFINE ("Task-2-1-Int");

std::vector<uint32_t> byteCounterVector(20, 0);
std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
std::vector<double> throughPutVector(20, 0);

uint32_t ByteCounter1 = 0, ByteCounter2 = 0, ByteCounter3 = 0;
uint32_t oldByteCounter1 = 0, oldByteCounter2 = 0, oldByteCounter3 = 0;

///#############################################LTE States From Different Prespectives#####################
/// Map each of UE RRC states to its string representation.
static const std::string ueRrcStateName[LteUeRrc::NUM_STATES] =
{
  "IDLE_START",
  "IDLE_CELL_SEARCH",
  "IDLE_WAIT_MIB_SIB1",
  "IDLE_WAIT_MIB",
  "IDLE_WAIT_SIB1",
  "IDLE_CAMPED_NORMALLY",
  "IDLE_WAIT_SIB2",
  "IDLE_RANDOM_ACCESS",
  "IDLE_CONNECTING",
  "CONNECTED_NORMALLY",
  "CONNECTED_HANDOVER",
  "CONNECTED_PHY_PROBLEM",
  "CONNECTED_REESTABLISHING"
};

static const std::string enbRrcStateName[UeManager::NUM_STATES] =
{
  "INITIAL_RANDOM_ACCESS",
  "CONNECTION_SETUP",
  "CONNECTION_REJECTED",
  "ATTACH_REQUEST",
  "CONNECTED_NORMALLY",
  "CONNECTION_RECONFIGURATION",
  "CONNECTION_REESTABLISHMENT",
  "HANDOVER_PREPARATION",
  "HANDOVER_JOINING",
  "HANDOVER_PATH_SWITCH",
  "HANDOVER_LEAVING",
};

static const std::string uePhyStateName[LteUePhy::NUM_STATES] =
{
  "CELL_SEARCH",
  "SYNCHRONIZED"
};

static const std::string ueNasStateName[EpcUeNas::NUM_STATES] =
{
  "OFF",
  "ATTACHING",
  "IDLE_REGISTERED",
  "CONNECTING_TO_EPC",
  "ACTIVE"
};
//#################################################################################################################


void UePhyStateChange(std::string context, uint16_t cellId, uint16_t rnti, LteUePhy::State oldState, LteUePhy::State newState)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(20) << uePhyStateName[oldState];
  std::cout << std::setw(20) << uePhyStateName[newState] << std::endl;
}
void UeRrcStateChange(std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, ns3::LteUeRrc::State  oldState, ns3::LteUeRrc::State newState)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << imsi;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(25) << ueRrcStateName[oldState];
  std::cout << std::setw(25) << ueRrcStateName[newState] << std::endl;
}
void EpcUeNasStateChange(std::string context, const ns3::EpcUeNas::State oldState, const ns3::EpcUeNas::State newState)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(20) << ueNasStateName[oldState];
  std::cout << std::setw(20) << ueNasStateName[newState] << std::endl;
}
void EnbRrcStateChange(std::string context, const uint64_t imsi, const uint16_t cellId, const uint16_t rnti, const ns3::UeManager::State  oldState, const UeManager::State newState)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << imsi;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(25) << enbRrcStateName[oldState];
  std::cout << std::setw(25) << enbRrcStateName[newState] << std::endl;
}

void
enbRrcTracing()
{
  std::cout << "#";
  std::cout << std::setw(9) << "time";
  std::cout << std::setw(8) << "cellID";
  std::cout << std::setw(8) << "imsi";
  std::cout << std::setw(8) << "rnti";
  std::cout << std::setw(20) << "OldState" ;
  std::cout << std::setw(20) << "NewState" << std::endl;
  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/UeMap/*/StateTransition"
      , MakeCallback (&EnbRrcStateChange));
}

//Clean input parameter by removing ( ) etc
std::string cleanUpString(std::string stringOfPositions)
{
    stringOfPositions.erase(
        std::remove_if(stringOfPositions.begin(),stringOfPositions.end(),
        [](char chr){ return chr == '(' || chr == ')' || chr == '[' || chr == ']';}),
        stringOfPositions.end()
        );
        return stringOfPositions;
}

//Convert clean string to vector
std::vector<float> stringToVector(std::string cleanString)
{
    std::vector<float> numbers;
    std::stringstream text_stream(cleanString);
    std::string item;

    while (std::getline(text_stream, item, ','))
    {
        numbers.push_back(std::stof(item));
    }
    return numbers;
}


std::vector<std::vector<float>> guiCoordsToNS3Coords(std::vector<float> vectorAllBuildingCords)
{


  std::vector<std::vector<float>> vectorVectorWithAllCords;

  int amountOfBuildingsTimesTwo = vectorAllBuildingCords.size();


  for(int i = 0; i < amountOfBuildingsTimesTwo; i+=2)
  {

    //why doesnt this loop twice when we have two buildings, amountofbuildings is 4 thereofre we should have one loop at i = 0 and one at i = 2
    float xmin = vectorAllBuildingCords.at(i)-25;
    float xmax = vectorAllBuildingCords.at(i)+25;
    float ymin = vectorAllBuildingCords.at(i+1)-25;
    float ymax = vectorAllBuildingCords.at(i+1)+25;

    std::vector<float> tempVector;

    tempVector.push_back(xmin);
    tempVector.push_back(xmax);
    tempVector.push_back(ymin);
    tempVector.push_back(ymax);


    vectorVectorWithAllCords.push_back(tempVector);


  }


  return vectorVectorWithAllCords;
}

int counterRP = 0;      // Counter for the receieve packet

void
Throughput1 (Time binSize, std::vector<double> throughPutVectors, std::vector<uint32_t> byteCounterVectors, std::vector<uint32_t> oldByteCounterVectors, int numUEsNEW)
{


  for (int i = 0; i < counterRP; i++)
  {



    throughPutVector.at(i) = (byteCounterVector.at(i) - oldByteCounterVector.at(i)) * 8 / binSize.GetSeconds () / 1024 / 1024;
    oldByteCounterVector.at(i) = byteCounterVector.at(i);
  };


  //std::cout << "this: " <<throughPutVector.at(1) << std::endl;
  //std::cout << "this: "  <<throughPutVector.at(2) << std::endl;


  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ();

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

  Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEsNEW);

}

//void cqiReport(std::string context, uint16_t rnti, uint8_t cqi)
//{
//  std::cout << "Received CQI Report From RNTI: " << rnti << "With CQI: " << cqi << std::endl;
//}

//void
//ulPhyTrx(std::string context , const PhyTransmissionStatParameters a)
//{
//  std::cout << std::setw(7) << Simulator::Now ().GetSeconds ();
//  std::cout << std::setw(7) << a.m_cellId;
//  std::cout << std::setw(7) << a.m_imsi;
//  std::cout << std::setw(7) << a.m_rnti;
//  std::cout << std::setw(7) << std::to_string(a.m_txMode);
//  std::cout << std::setw(7) << std::to_string(a.m_mcs) ;
//  std::cout << std::setw(7) << std::to_string(a.m_ndi) ;
//  std::cout << std::setw(7) << std::to_string(a.m_layer) ;
//  std::cout << std::setw(7) << std::to_string(a.m_rv) << std::endl;
//}

int
main (int argc, char *argv[])
{
  double simTime = 3; //Seconds
  double d1 = 50;
  Time interPacketInterval = MilliSeconds (1);
  double eNBTxPowerDbm = 40;
  double ueTxPowerDbm = 20;
  uint16_t eNBDlEarfcn = 100;
  uint16_t eNBBandwidth = 25;

  double hEnb = 30.0;
  double hUe = 1.0;

  bool disableDl = false;
  bool disableUl = false;
  bool enableflowstats = false, enableRem = false;
  bool enableTraces = true, enableUePhyStates = false, enableUeRrcStates = false,
      enableUeNasStates = false, enableEnbRrcStates = false, enableInstTput = false,
      enableFading = false;
  bool enableEpcLogs = false, enableRanLogs=false;
  uint64_t runId =1 ;

  //Parameters from GUI with buildings and UE positions
  std::string arrayPosUEsString = "[(483.9872330323306,-143.0411255890087),(-2337.4383413493247,-2288.6580094241517),]";
  std::string arrayPoseNBsString = "[(0,0),]";


  // Command line arguments
  CommandLine cmd (__FILE__);
//  cmd.AddValue ("simTime", "Total duration of the simulation", simTime);
//  cmd.AddValue ("interPacketInterval", "Inter packet interval", interPacketInterval);
  cmd.AddValue ("eNBTxPowerDbm", "eNB transmit power", eNBTxPowerDbm);
  cmd.AddValue ("disableDl", "Disable downlink data flows", disableDl);
  cmd.AddValue ("disableUl", "Disable uplink data flows", disableUl);
  cmd.AddValue ("enableUePhyStates", "Enable PHY level UE states", enableUePhyStates);
  cmd.AddValue ("enableUeRrcStates", "Enable RRC level UE states", enableUeRrcStates);
  cmd.AddValue ("enableUeNasStates", "Enable NAS level UE states", enableUeNasStates);
  cmd.AddValue ("enableEnbRrcStates", "Enable RRC level ENB states", enableEnbRrcStates);
  cmd.AddValue ("enableflowstats", "Enable flow level stats", enableflowstats);
  cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
  cmd.AddValue ("enableRem", "Enable radio environment map", enableRem);
  cmd.AddValue ("enableEpcLogs", "Enable EPC logs", enableEpcLogs);
  cmd.AddValue ("enableRanLogs", "Enable RAN logs", enableRanLogs);
  cmd.AddValue ("runId", "Randomization parameter", runId);

  //Pass parameters into variables
  cmd.AddValue ("arrayPosUEsString", "Positions of UEs", arrayPosUEsString);
  cmd.AddValue ("arrayPoseNBsString", "Positions of eNBs", arrayPoseNBsString);

  cmd.Parse (argc, argv);

  std::string newUEsString = cleanUpString(arrayPosUEsString);
  std::string neweNBsString = cleanUpString(arrayPoseNBsString);

  std::vector<float>  UECords = stringToVector(newUEsString);
  std::vector<float>  eNBCords = stringToVector(neweNBsString);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);
    
  RngSeedManager::SetRun(runId);


  if(enableEpcLogs)
    {
      LogLevel logLevel2 = (LogLevel)( LOG_PREFIX_TIME |
          LOG_PREFIX_NODE | LOG_INFO);
      LogComponentEnable ("EpcMmeApplication", logLevel2);
      LogComponentEnable ("EpcPgwApplication", logLevel2);
      LogComponentEnable ("EpcPgwApplication", logLevel2);

    }
  if(enableRanLogs)
    {
      LogLevel logLevel1 = (LogLevel)( LOG_PREFIX_TIME |
        LOG_PREFIX_NODE | LOG_INFO);
      LogComponentEnable ("LteEnbMac", logLevel1);
      LogComponentEnable ("LteEnbRrc", logLevel1);
      LogComponentEnable ("LteRrcProtocolIdeal", logLevel1); //info
      LogComponentEnable ("LteUeRrc", logLevel1); //info
    }
  //      LogComponentEnable ("LteUeMac", logLevel1); //info






  Ptr<UniformRandomVariable> enbNoise = CreateObject<UniformRandomVariable> ();
  enbNoise->SetAttribute ("Min", DoubleValue (1));
  enbNoise->SetAttribute ("Max", DoubleValue (1));

  Ptr<UniformRandomVariable> ueNoise = CreateObject<UniformRandomVariable> ();
  ueNoise->SetAttribute ("Min", DoubleValue (1));
  ueNoise->SetAttribute ("Max", DoubleValue (1));

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNBTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoise->GetValue()));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (enbNoise->GetValue()));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.01));




  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (eNBDlEarfcn));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (eNBDlEarfcn + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (eNBBandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (eNBBandwidth));


  Ptr<UniformRandomVariable> d0 = CreateObject<UniformRandomVariable> ();
  d0->SetAttribute ("Min", DoubleValue (1));
  d0->SetAttribute ("Max", DoubleValue (5));
  Ptr<UniformRandomVariable> d11 = CreateObject<UniformRandomVariable> ();
  d11->SetAttribute ("Min", DoubleValue (150));
  d11->SetAttribute ("Max", DoubleValue (160));
  Ptr<UniformRandomVariable> d2 = CreateObject<UniformRandomVariable> ();
  d2->SetAttribute ("Min", DoubleValue (300));
  d2->SetAttribute ("Max", DoubleValue (310));
  Ptr<UniformRandomVariable> e0 = CreateObject<UniformRandomVariable> ();
  e0->SetAttribute ("Min", DoubleValue (2.95));
  e0->SetAttribute ("Max", DoubleValue (3.95));
  Ptr<UniformRandomVariable> e1 = CreateObject<UniformRandomVariable> ();
  e1->SetAttribute ("Min", DoubleValue (4.4));
  e1->SetAttribute ("Max", DoubleValue (5.0));
  Ptr<UniformRandomVariable> e2 = CreateObject<UniformRandomVariable> ();
  e2->SetAttribute ("Min", DoubleValue (5.8));
  e2->SetAttribute ("Max", DoubleValue (6.2));


//  std::cout<< "d0: " << d0->GetValue() << ", d1: " << d1->GetValue() << ", d2: " << d2->GetValue() << ", e0: " << e0->GetValue() << ", e1: " << e1->GetValue() << ", e2: " << e2->GetValue() << std::endl;

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));

  lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue(d0->GetValue()));
  lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue(d11->GetValue()));
  lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue(d2->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue(e0->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue(e1->GetValue()));
  lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue(e2->GetValue()));

  if(enableFading)
    {
      lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));

      std::ifstream ifTraceFile;
      ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
      if (ifTraceFile.good ())
        {
          // script launched by test.py
          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
        }
      else
        {
          // script launched as an example
          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
        }

      // these parameters have to be set only in case of the trace format
      // differs from the standard one, that is
      // - 10 seconds length trace
      // - 10,000 samples
      // - 0.5 seconds for window size
      // - 100 RB
      lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
      lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
      lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
      lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
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

  // Create Nodes: eNodeB and UE
  NodeContainer eNBNodesArg;
  NodeContainer ueNodesArg;

  //NodeContainer movingNodes;
  //movingueNodes.Create(1);


  int numUEsNEW = UECords.size()/2;  //number of UEs defined by user
  int numeNBsNEW = eNBCords.size()/2;  //number of UEs defined by user


  ueNodesArg.Create (numUEsNEW);
  eNBNodesArg.Create (numeNBsNEW);

  // Position of UEs attached to eNB 1 // attached to eNB1 ???
  Ptr<ListPositionAllocator> positionAllocue = CreateObject<ListPositionAllocator> ();

  //Allocate cordinates to object
  for(int i = 0; i < numUEsNEW*2; i+=2)
    {
        float x = UECords.at(i);
        float y = UECords.at(i+1);
        positionAllocue->Add (Vector (x, y, hUe)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
    }

  //Pass object with cordinates into the mobilityobject of the UEs
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  uemobility.SetPositionAllocator (positionAllocue); //why? // this doesnt seem nessecary //this has been fixed !
  uemobility.Install (ueNodesArg);

  // Position of UEs attached to eNB 1 // attached to eNB1 ???
  Ptr<ListPositionAllocator> positionAlloceNB = CreateObject<ListPositionAllocator> ();

  //Allocate cordinates to object
  for(int i = 0; i < numeNBsNEW*2; i+=2)
    {
        float x = eNBCords.at(i);
        float y = eNBCords.at(i+1);
        positionAlloceNB->Add (Vector (x, y, hEnb)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
    }

  //Pass object with cordinates into the mobilityobject of the UEs
  MobilityHelper eNBmobility;
  eNBmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  eNBmobility.SetPositionAllocator (positionAlloceNB); //why? // this doesnt seem nessecary //this has been fixed !
  eNBmobility.Install (eNBNodesArg);


  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (eNBNodesArg);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodesArg);

  // Install the IP stack on the UEs
  internet.Install (ueNodesArg);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodesArg.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodesArg.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

//  // Attach one UE per eNodeB
//  for (uint16_t i = 0; i < numNodePairs; i++)
//    {
//      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
//      // side effect: the default EPS bearer will be activated
//    }

  lteHelper->Attach(ueLteDevs);

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0.2602));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.2603));
  Time startTime = Seconds (startTimeSeconds->GetValue ());
  for (uint32_t u = 0; u < ueNodesArg.GetN (); ++u)
    {
      if (!disableDl)
        {
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ueNodesArg.Get (u)));

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
          clientApps.Add (ulClient.Install (ueNodesArg.Get(u)));
        }

    }

  serverApps.Start (startTime);
  clientApps.Start (startTime);


  if(enableTraces)
    {
      if(enableUePhyStates)
        {
          std::cout << "#";
          std::cout << std::setw(9) << "time";
          std::cout << std::setw(8) << "cellID";
          std::cout << std::setw(8) << "rnti";
          std::cout << std::setw(20) << "OldState" ;
          std::cout << std::setw(20) << "NewState" << std::endl;
          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/StateTransition"
              , MakeCallback (&UePhyStateChange));
        }
      if(enableUeRrcStates)
        {
          std::cout << "#";
          std::cout << std::setw(9) << "time";
          std::cout << std::setw(8) << "cellID";
          std::cout << std::setw(8) << "imsi";
          std::cout << std::setw(8) << "rnti";
          std::cout << std::setw(20) << "OldState" ;
          std::cout << std::setw(20) << "NewState" << std::endl;
          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/LteUeRrc/StateTransition"
              , MakeCallback (&UeRrcStateChange));
        }
      if(enableUeNasStates)
        {
          std::cout << "#";
          std::cout << std::setw(9) << "time";
          std::cout << std::setw(20) << "OldState" ;
          std::cout << std::setw(20) << "NewState" << std::endl;
          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/EpcUeNas/StateTransition"
              , MakeCallback (&EpcUeNasStateChange));
        }
    }
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-simple-epc");

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if(enableRem)
    {
      //    PrintGnuplottableBuildingListToFile ("buildings.txt");
      //    PrintGnuplottableEnbListToFile ("enbs.txt");
      //    PrintGnuplottableUeListToFile ("ues.txt");
      // Configure Radio Environment Map (REM) output
      // for LTE-only simulations always use /ChannelList/0 which is the downlink channel
      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      //  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("Channel", PointerValue (lteHelper->GetDownlinkSpectrumChannel ()));
      remHelper->SetAttribute ("OutputFile", StringValue ("task2-1-rem.out"));
      remHelper->SetAttribute ("XMin", DoubleValue (-2*numeNBsNEW*d1));
      remHelper->SetAttribute ("XMax", DoubleValue (2*numeNBsNEW*d1));
      remHelper->SetAttribute ("YMin", DoubleValue (-2*numeNBsNEW*d1));
      remHelper->SetAttribute ("YMax", DoubleValue (2*numeNBsNEW*d1));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
      remHelper->Install ();
    }

  FlowMonitorHelper flowmonHelper;
  Ptr<ns3::FlowMonitor> monitor;
  if(enableflowstats)
    {

      NodeContainer endpointNodes;
      endpointNodes.Add (ueNodesArg);
      endpointNodes.Add(remoteHost);

      monitor = flowmonHelper.Install (endpointNodes);
      monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
      monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
      monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));
      //
      //      monitor->SerializeToXmlFile("flowmon.xml", true, true);
      //
      //      std::string anim_filename = outputDir + "/" + "anim.xml";
      //      AnimationInterface anim (anim_filename);
      //      anim.SetMobilityPollInterval (MilliSeconds(5));
      //      anim.EnablePacketMetadata (true);
      //      anim.EnableIpv4L3ProtocolCounters(startTime, simTime, MilliSeconds(1));
      //      anim.UpdateNodeSize (6, 1.5, 1.5);
    }


  if(enableTraces)
    {
      if(enableEnbRrcStates)
        {
          Simulator::Schedule(Seconds(0.26), &enbRrcTracing);
        }
//      lteHelper->EnableTraces ();

    }
//  p2ph.EnablePcapAll("w_results/pcaps/task2-1", true);

  if(enableInstTput)            // todo make flexible
    {

      //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
      //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
      //std::vector<double> throughputVector(numUEsNEW, 0);

      for (int i = 0; i < numUEsNEW; i++)
      {


        std::cout << "Number of numUEsNEW: " << numUEsNEW << std::endl;

        std::cout << "Counter is the value: " << counterRP << std::endl;

        auto function = [i](Ptr<const Packet> packet, const Address&) {
            byteCounterVector.at(i) += packet->GetSize();
          };


        ossGroup.at(i) << "/NodeList/" << ueNodesArg.Get (counterRP)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

        Config::ConnectWithoutContext(
               ossGroup.at(i).str(),
            make_nice_callback<void, Ptr<const Packet>, const Address&>(function)
          );

        std::cout << "Connected to the UE device " << counterRP << std::endl;

        counterRP = counterRP + 1;



      };

      std::cout << "#";
      std::cout << std::setw(9) << "time";
      for (int i = 0; i < numUEsNEW; i++)
      {
          std::cout << std::setw(11) << "Throughput-UE" << i << "(Mbps)";
      }
      std::cout << std::endl;
      Time binSize = Seconds (0.04);
      Simulator::Schedule (Seconds (0.1), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, numUEsNEW);

    }



//  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/FfMacScheduler/RrFfMacScheduler/WidebandCqiReport"
//      , MakeCallback (&cqiReport));


//  std::cout << "#";
//  std::cout << std::setw(7) << "Time";
//  std::cout << std::setw(7) << "CellID";
//  std::cout << std::setw(7) << "IMSI";
//  std::cout << std::setw(7) << "RNTI";
//  std::cout << std::setw(7) << "TxMode";
//  std::cout << std::setw(7) << "MCS" ;
//  std::cout << std::setw(7) << "NDI" ;
//  std::cout << std::setw(7) << "Layer" ;
//  std::cout << std::setw(7) << "RV" << std::endl;
//  Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/UlPhyTransmission"
//      , MakeCallback (&ulPhyTrx));


  Simulator::Stop (Seconds(simTime));
  Simulator::Run ();



  if(enableflowstats)
    {
      // Print per-flow statistics
      monitor->CheckForLostPackets ();
      Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
      FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

      double averageFlowThroughput = 0.0;
      double averageFlowDelay = 0.0;


      for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
        {
          Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
          std::stringstream protoStream;
          std::string channel;
          protoStream << (uint16_t) t.protocol;
          if (t.protocol == 6)
            {
              protoStream.str ("TCP");
            }
          if (t.protocol == 17)
            {
              protoStream.str ("UDP");
            }

          if(t.sourceAddress.GetSubnetDirectedBroadcast("255.0.0.0") == "7.255.255.255")
            {
              channel = "UL";
            }
          else
            {
              channel = "DL";
            }
          std::cout  << "\n<" << channel << "> " << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n\n";
          //    std::cout  << "  Tx Packets: " << i->second.txPackets << "\n";
          //    std::cout  << "  Tx Bytes:   " << i->second.txBytes << "\n";
          //    std::cout  << "  TxOffered:  " << (((i->second.txBytes * 8.0) / (simTime.GetSeconds() - startTime.GetSeconds())) / 1000) / 1000  << " Mbps\n";
          //    std::cout  << "  Rx Bytes:   " << i->second.rxBytes << "\n";
                std::cout << "   Packet Lost: " << i->second.lostPackets << "\n";

          if (i->second.rxPackets > 0)
            {
              // Measure the duration of the flow from receiver's perspective
              //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
              double rxDuration = simTime - startTime.GetSeconds();

              averageFlowThroughput += (((i->second.rxBytes * 8.0) / rxDuration) / 1024) / 1024;
              averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

              std::cout  << "  Throughput: " << (((i->second.rxBytes * 8.0) / rxDuration) / 1024) / 1024  << " Mbps\n";
              std::cout  << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
              //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
              std::cout  << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
            }
          else
            {
              std::cout << "  Throughput:  0 Mbps\n";
              std::cout  << "  Mean delay:  0 ms\n";
              std::cout  << "  Mean jitter: 0 ms\n";
            }
        }
    }

  /*GtkConfigStore config;
  config.ConfigureAttributes();*/

  Simulator::Destroy ();
  return 0;
}
