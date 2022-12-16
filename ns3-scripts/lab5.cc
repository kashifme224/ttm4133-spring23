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
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <array>
#include <utility>

#include "ns3/flow-monitor-module.h"
#include "ns3/rectangle.h"
#include <ns3/buildings-module.h>
#include <ns3/spectrum-helper.h>

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
uint16_t cellID = 1;

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


  std::cout << std::setw(5) << Simulator::Now ().GetSeconds ();

  for (int i = 0; i < counterRP; i++)
  {
      if(i==0)
        {
          std::cout << std::setw(13) << throughPutVector.at(i);

        }
      else
        {
          std::cout << std::setw(20) << throughPutVector.at(i);
        }
  }


  //for (double i : throughPutVector)
  //{
  //  std::cout << std::setw(11) << i;
  //};

  std::cout << std::endl;

  //cout statement

  Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEsNEW);

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

  std::cout << std::setw(9) << Simulator::Now ().GetSeconds ()<< std::setw(9) << cellID << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;
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
//#######################################################ENB##########################################################3
void EnbSinrFromUes(std::string context, uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10)<< std::setprecision(3) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << 10 * std::log10(sinrLinear) << std::endl;
}

void
NotifyHandoverEndOkEnb (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
{
  cellID = cellid;
}

int
main (int argc, char *argv[])
{
//  uint16_t numUe = 2;
  double simTime = 5; //Seconds
  double dis1 = 500;
//  double ueH = 0;
  Time interPacketInterval = MicroSeconds(500);
  double eNBTxPowerDbm = 10;
  double ueTxPowerDbm = 10;
  uint16_t eNBDlEarfcn = 100;
  uint16_t eNBBandwidth = 25;
  uint64_t runId =1 ;

  double hEnb = 30.0;
  double hUe = 1.0;

  bool disableDl = false;
  bool disableUl = false;
  bool enablersrp = false, enablesinrenb = false, enablesinrue = false;
  bool enableflowstats = false, enableRem = false;
  bool enableTraces = true, enableUePhyStates = false, enableUeRrcStates = false,
      enableUeNasStates = false, enableEnbRrcStates = false, enableInstTput = false,
      enableFading = false; bool enablebuilding = false;
  bool a2a4Ho = true, a3Ho = false, enableX2 = true;
  uint8_t a2a4SCellThreshold = 30, a2a4neighborCellOffset = 1;

  //Parameters from GUI with buildings and UE positions
//  std::string arrayPosUEsString = "[(-209.01456344505624,912.8082166034646),]";
//  std::string arrayPosMobUEsString = "[(-555.5387081039651,-852.3209251417888),]";
//  std::string arrayPosMobUEPathString = "[(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";


  //  std::string arrayPosUE= "[(-209.01456344505624,912.8082166034646),(-109.01456344505624,502.8082166034646),m(-555.5387081039651,-852.3209251417888),(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";

  //  std::string arrayPosUE= "[(16.498907646474493,1039.2816442051494),(2298.8477987421384,-1061.2770229184869),(3129.3277963728265,1017.2799035416064),"
  //      "m(-472.96868586560754,-742.344031575107),(-252.98325057927826,-808.3301677151171),(3998.2352863290293,-753.3417209317761),]";

//  std::string arrayPoseNBsString = "[(0,0),(2337.6628806354956,38.49191274833947),]";
//  std::string arrayPosBuildingsString = "[(0,1),]";
//  std::string arrayPosMobUE= "[(-555.5387081039651,-852.3209251417888),(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";
//  std::string arrayPosUE= "[m(-472.96868586560754,-742.344031575107),(-252.98325057927826,-808.3301677151171),(3998.2352863290293,-753.3417209317761),]";

//  LogLevel logLevel1 = (LogLevel)( LOG_PREFIX_TIME | LOG_LEVEL_INFO  );
//  LogComponentEnable ("LteEnbRrc", logLevel1);
//  LogComponentEnable ("LteUeRrc", logLevel1);


//  std::cout << arrayPosUEsString << std::endl;
//  std::cout << arrayPosMobUEsString << std::endl;
//  std::cout << arrayPosMobUEPathString << std::endl;

  std::string arrayPosUE, arrayPosMobUE, arrayPosBuildingsString, arrayPoseNBsString;



  // Command line arguments
  CommandLine cmd (__FILE__);

  cmd.AddValue ("enablersrp", "get rsrp rsrq stats", enablersrp);
  cmd.AddValue ("enablesinrue", "get ue sinr stats", enablesinrue);
  cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
  cmd.AddValue ("arrayPoseNBsString", "Positions of eNBs", arrayPoseNBsString);
  cmd.AddValue ("arrayPosUE", "Position and Path of UEs", arrayPosUE);
  cmd.AddValue ("arrayPosBuildingsString", "Positions of Buildings", arrayPosBuildingsString);
  cmd.AddValue ("eNBTxPowerDbm", "eNB transmit power", eNBTxPowerDbm);
  cmd.AddValue ("runId", "Randomization parameter", runId);
  cmd.AddValue ("enableX2", "Handover Enabler", enableX2);
  cmd.AddValue ("enablebuilding", "Building Part Enabler", enablebuilding);


  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse(argc, argv);



  int pos1 = arrayPosUE.find("m");


  std::string arrayPosUEsString =  arrayPosUE.substr(0,pos1) + "]";

  std::string arrayPosMobUEString = "[" + arrayPosUE.substr(pos1 + 1) + "]";

  int pos2 = arrayPosMobUEString.find(")");


  std::string arrayPosMobUEsString = arrayPosMobUEString.substr(0,pos2+ 2) + "]";

  std::string arrayPosMobUEPathString = "[" + arrayPosMobUEString.substr(pos2 + 2); 

  RngSeedManager::SetRun(runId);


  std::string newUEsString = cleanUpString(arrayPosUEsString);
  std::string neweNBsString = cleanUpString(arrayPoseNBsString);
  std::string newMobUesString = cleanUpString(arrayPosMobUEsString);
  std::string newMobUesPathString = cleanUpString(arrayPosMobUEPathString);


  std::vector<float>  UECords = stringToVector(newUEsString);
  std::vector<float>  eNBCords = stringToVector(neweNBsString);
  std::vector<float>  mobUECords = stringToVector(newMobUesString);
  std::vector<float>  mobUEPathCords = stringToVector(newMobUesPathString);

  Ptr<UniformRandomVariable> enbNoise = CreateObject<UniformRandomVariable> ();
  enbNoise->SetAttribute ("Min", DoubleValue (1));
  enbNoise->SetAttribute ("Max", DoubleValue (5));

  Ptr<UniformRandomVariable> ueNoise = CreateObject<UniformRandomVariable> ();
  ueNoise->SetAttribute ("Min", DoubleValue (1));
  ueNoise->SetAttribute ("Max", DoubleValue (5));

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNBTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoise->GetValue()));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (enbNoise->GetValue()));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));



  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
    
    
  Config::SetDefault("ns3::LteUePhy::UeMeasurementsFilterPeriod", TimeValue(MilliSeconds(100)));
  Config::SetDefault("ns3::LteUePhy::RsrpSinrSamplePeriod", UintegerValue(100));  
  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (eNBDlEarfcn));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (eNBDlEarfcn + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (eNBBandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (eNBBandwidth));
  lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");
    
  if(enableX2)
  {
    if(a2a4Ho)
    {
      lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
      lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (a2a4SCellThreshold));
      lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (a2a4neighborCellOffset));
    }
  if(a3Ho)
    {
      lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
      lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (0));
      lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (0)));
    }
  }
  //  epcHelper->SetAttribute("S1uLinkEnablePcap", BooleanValue(true));
//  epcHelper->SetAttribute("X2LinkEnablePcap", BooleanValue(true));

  if(enablebuilding)
    {
      Ptr<UniformRandomVariable> iwl = CreateObject<UniformRandomVariable> ();
      iwl->SetAttribute ("Min", DoubleValue (1));
      iwl->SetAttribute ("Max", DoubleValue (5));
      Ptr<UniformRandomVariable> ssew = CreateObject<UniformRandomVariable> ();
      ssew->SetAttribute ("Min", DoubleValue (1));
      ssew->SetAttribute ("Max", DoubleValue (5));
      Ptr<UniformRandomVariable> sso = CreateObject<UniformRandomVariable> ();
      sso->SetAttribute ("Min", DoubleValue (1));
      sso->SetAttribute ("Max", DoubleValue (7));
      Ptr<UniformRandomVariable> ssi = CreateObject<UniformRandomVariable> ();
      ssi->SetAttribute ("Min", DoubleValue (1));
      ssi->SetAttribute ("Max", DoubleValue (8));


      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (ssew->GetValue()));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (sso->GetValue()));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (ssi->GetValue()));
      lteHelper->SetPathlossModelAttribute ("InternalWallLoss", DoubleValue (iwl->GetValue()));

    }
  else
    {

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


    lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));
    lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue(d0->GetValue()));
    lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue(d1->GetValue()));
    lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue(d2->GetValue()));
    lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue(e0->GetValue()));
    lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue(e1->GetValue()));
    lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue(e2->GetValue()));
    }

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

   if(enablebuilding)
     {

       std::string newBuildingsString = cleanUpString(arrayPosBuildingsString);
       std::vector<float>  BUCords = stringToVector(newBuildingsString);


       uint8_t floors = 10;
       uint8_t Xrooms = 5;
       uint8_t Yrooms = 5;


      // std::cout << "New Buildings String: " << newBuildingsString << std::endl;


       std::vector<std::vector<float>> vectorVectorBuildingsBounds = guiCoordsToNS3Coords(BUCords);


       //...
       std::vector<Ptr<Building>>  objects;

       int amountOfBuildings = BUCords.size()/2;

       objects.resize(amountOfBuildings);

       //std::cout <<"Amount Of Buildings: " << amountOfBuildings << std::endl;

       int temp = 0;




       //std::cout << "Vector Vector with Building Bounds size: " << vectorVectorBuildingsBounds.size() << std::endl;




       for (Ptr<Building> i : objects)
       {
         std::vector<float> tempVector = vectorVectorBuildingsBounds.at(temp);


         i = CreateObject <Building> ();
         i->SetBoundaries (Box (tempVector.at(0),tempVector.at(1),tempVector.at(2),tempVector.at(3),0,7.5));
         i->SetBuildingType (Building::Residential);
         i->SetExtWallsType (Building::ConcreteWithWindows);
         i->SetNFloors (floors);
         i->SetNRoomsX (Xrooms);
         i->SetNRoomsY (Yrooms);

         temp++;
       }

     }

  // Create Nodes: eNodeB and UE
  NodeContainer eNBNodesArg;
  NodeContainer ueNodesArg;
  NodeContainer mobileueNodesArg;

  //NodeContainer movingNodes;
  //movingueNodes.Create(1);


  int numUEsNEW = UECords.size()/2;  //number of UEs defined by user
  int numeNBsNEW = eNBCords.size()/2;  //number of UEs defined by user
  int numMobUePathNEW = mobUEPathCords.size()/2;  //number of UEs defined by user
  int numMobUeNEW = mobUECords.size()/2;  //number of UEs defined by user



  ueNodesArg.Create (numUEsNEW);
  eNBNodesArg.Create (numeNBsNEW);
  mobileueNodesArg.Create (numMobUeNEW);

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
  if(enablebuilding)
    {
      BuildingsHelper::Install(ueNodesArg);

    }

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

  if(enablebuilding)
    {
      BuildingsHelper::Install(eNBNodesArg);

    }
  // Position of UEs attached to eNB 1 // attached to eNB1 ???
  Ptr<ListPositionAllocator> positionAllocmobileUE = CreateObject<ListPositionAllocator> ();

  //Allocate cordinates to object
  for(int i = 0; i < numMobUeNEW*2; i+=2)
    {
        float x = mobUECords.at(i);
        float y = mobUECords.at(i+1);
        positionAllocmobileUE->Add (Vector (x, y, hUe)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
    }



  MobilityHelper movingueMobility;

  movingueMobility.SetPositionAllocator(positionAllocmobileUE);
  movingueMobility.SetMobilityModel("ns3::WaypointMobilityModel");
  movingueMobility.Install (mobileueNodesArg);


  if(enablebuilding)
    {
      BuildingsHelper::Install(mobileueNodesArg);

    }

  Ptr<WaypointMobilityModel> wayMobility;

  wayMobility = mobileueNodesArg.Get(0)->GetObject<WaypointMobilityModel>();

  for(int i = 0; i < numMobUePathNEW*2; i+=2)
    {
        float x = mobUEPathCords.at(i);
        float y = mobUEPathCords.at(i+1);
        wayMobility->AddWaypoint(Waypoint(Seconds(simTime - (simTime/(i + 1))), Vector3D(x, y, hUe)));
    }


  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (eNBNodesArg);
  NetDeviceContainer movingueDevs = lteHelper->InstallUeDevice (mobileueNodesArg);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodesArg);


  NodeContainer ueNodes;
  ueNodes.Add(ueNodesArg);
  ueNodes.Add(mobileueNodesArg);

  NetDeviceContainer ueDevs;
  ueDevs.Add(ueLteDevs);
  ueDevs.Add(movingueDevs);

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

//  // Attach one UE per eNodeB
//  for (uint16_t i = 0; i < numNodePairs; i++)
//    {
//      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
//      // side effect: the default EPS bearer will be activated
//    }

  lteHelper->Attach(ueDevs);

  // Install and start applications on UEs and remote host
  uint16_t dlPort = 1100;
  uint16_t ulPort = 2000;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.10));
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

  if(enableX2)
 {
      // Add X2 interface
      lteHelper->AddX2Interface (eNBNodesArg);
 }

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

      if(enablersrp)
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

      if(enablesinrenb)
        {
          std::cout << "#";
          std::cout << std::setw(9) << "time";
          std::cout << std::setw(8) << "cellID";
          std::cout << std::setw(8) << "rnti" ;
          std::cout << std::setw(12) << "sinr" << std::endl;

          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
              , MakeCallback (&EnbSinrFromUes));
        }

      if(enablesinrue)
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
    }
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-simple-epc");

  Ptr<RadioEnvironmentMapHelper> remHelper;
  if(enableRem)
    {

      // Configure Radio Environment Map (REM) output
      // for LTE-only simulations always use /ChannelList/0 which is the downlink channel
      remHelper = CreateObject<RadioEnvironmentMapHelper> ();
      //  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
      remHelper->SetAttribute ("Channel", PointerValue (lteHelper->GetDownlinkSpectrumChannel ()));
      remHelper->SetAttribute ("OutputFile", StringValue ("lab5-rem.out"));
      remHelper->SetAttribute ("XMin", DoubleValue (-2*numeNBsNEW*dis1));
      remHelper->SetAttribute ("XMax", DoubleValue (2*numeNBsNEW*dis1));
      remHelper->SetAttribute ("YMin", DoubleValue (-2*numeNBsNEW*dis1));
      remHelper->SetAttribute ("YMax", DoubleValue (2*numeNBsNEW*dis1));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
      remHelper->Install ();
    }

  FlowMonitorHelper flowmonHelper;
  Ptr<ns3::FlowMonitor> monitor;
  if(enableflowstats)
    {

      NodeContainer endpointNodes;
      endpointNodes.Add (ueNodes);
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

  if(enableInstTput)
    {

      //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
      //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
      //std::vector<double> throughputVector(numUEsNEW, 0);

      int total_ues = numUEsNEW + numMobUeNEW;

      for (int i = 0; i < total_ues; i++)
      {


//        std::cout << "Number of UEs: " << total_ues << std::endl;
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
      std::cout << std::setw(5) << "time";
      for (int i = 0; i < total_ues; i++)
      {
          std::cout << std::setw(13) << "Tput-UE" << i << "(Mbps)";
      }
      std::cout << std::endl;
      Time binSize = Seconds (0.06);
      Simulator::Schedule (Seconds (0), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, total_ues);

    }



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



// #include "ns3/core-module.h"
// #include "ns3/point-to-point-module.h"
// #include "ns3/internet-module.h"
// #include "ns3/applications-module.h"
// #include "ns3/mobility-module.h"
// #include "ns3/config-store-module.h"
// #include "ns3/lte-module.h"
// #include <iomanip>
// #include <iostream>
// #include <vector>
// #include <algorithm>
// #include <string>
// #include <sstream>
// #include <array>
// #include <utility>

// #include "ns3/flow-monitor-module.h"
// #include "ns3/rectangle.h"
// #include <ns3/buildings-module.h>
// #include <ns3/spectrum-helper.h>


// //////////////////////////////////////////////////////////////////////////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%///////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////Required Supplementary Functions Being Here/////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%///////////////////////////////////////////////////////////////////////////////
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

// /**
//  * Sample simulation script for LTE+EPC. It instantiates several eNodeBs,
//  * attaches one UE per eNodeB starts a flow for each UE to and from a remote host.
//  * It also starts another flow between each UE pair.
//  */

// NS_LOG_COMPONENT_DEFINE ("Task-2-1-Int");

// std::vector<uint32_t> byteCounterVector(20, 0);
// std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
// std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
// std::vector<double> throughPutVector(20, 0);


// uint32_t ByteCounter1 = 0, ByteCounter2 = 0, ByteCounter3 = 0;
// uint32_t oldByteCounter1 = 0, oldByteCounter2 = 0, oldByteCounter3 = 0;
// uint16_t cellID = 1;

// ///#############################################LTE States From Different Prespectives#####################
// /// Map each of UE RRC states to its string representation.
// static const std::string ueRrcStateName[LteUeRrc::NUM_STATES] =
// {
//   "IDLE_START",
//   "IDLE_CELL_SEARCH",
//   "IDLE_WAIT_MIB_SIB1",
//   "IDLE_WAIT_MIB",
//   "IDLE_WAIT_SIB1",
//   "IDLE_CAMPED_NORMALLY",
//   "IDLE_WAIT_SIB2",
//   "IDLE_RANDOM_ACCESS",
//   "IDLE_CONNECTING",
//   "CONNECTED_NORMALLY",
//   "CONNECTED_HANDOVER",
//   "CONNECTED_PHY_PROBLEM",
//   "CONNECTED_REESTABLISHING"
// };

// static const std::string enbRrcStateName[UeManager::NUM_STATES] =
// {
//   "INITIAL_RANDOM_ACCESS",
//   "CONNECTION_SETUP",
//   "CONNECTION_REJECTED",
//   "ATTACH_REQUEST",
//   "CONNECTED_NORMALLY",
//   "CONNECTION_RECONFIGURATION",
//   "CONNECTION_REESTABLISHMENT",
//   "HANDOVER_PREPARATION",
//   "HANDOVER_JOINING",
//   "HANDOVER_PATH_SWITCH",
//   "HANDOVER_LEAVING",
// };

// static const std::string uePhyStateName[LteUePhy::NUM_STATES] =
// {
//   "CELL_SEARCH",
//   "SYNCHRONIZED"
// };

// static const std::string ueNasStateName[EpcUeNas::NUM_STATES] =
// {
//   "OFF",
//   "ATTACHING",
//   "IDLE_REGISTERED",
//   "CONNECTING_TO_EPC",
//   "ACTIVE"
// };
// //#################################################################################################################


// void UePhyStateChange(std::string context, uint16_t cellId, uint16_t rnti, LteUePhy::State oldState, LteUePhy::State newState)
// {
//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(20) << uePhyStateName[oldState];
//   std::cout << std::setw(20) << uePhyStateName[newState] << std::endl;
// }
// void UeRrcStateChange(std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, ns3::LteUeRrc::State  oldState, ns3::LteUeRrc::State newState)
// {
//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << imsi;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(25) << ueRrcStateName[oldState];
//   std::cout << std::setw(25) << ueRrcStateName[newState] << std::endl;
// }
// void EpcUeNasStateChange(std::string context, const ns3::EpcUeNas::State oldState, const ns3::EpcUeNas::State newState)
// {
//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(20) << ueNasStateName[oldState];
//   std::cout << std::setw(20) << ueNasStateName[newState] << std::endl;
// }
// void EnbRrcStateChange(std::string context, const uint64_t imsi, const uint16_t cellId, const uint16_t rnti, const ns3::UeManager::State  oldState, const UeManager::State newState)
// {
//   std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << imsi;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(25) << enbRrcStateName[oldState];
//   std::cout << std::setw(25) << enbRrcStateName[newState] << std::endl;
// }

// void
// enbRrcTracing()
// {
//   std::cout << "#";
//   std::cout << std::setw(9) << "time";
//   std::cout << std::setw(8) << "cellID";
//   std::cout << std::setw(8) << "imsi";
//   std::cout << std::setw(8) << "rnti";
//   std::cout << std::setw(20) << "OldState" ;
//   std::cout << std::setw(20) << "NewState" << std::endl;
//   Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/LteEnbRrc/UeMap/*/StateTransition"
//       , MakeCallback (&EnbRrcStateChange));
// }

// //Clean input parameter by removing ( ) etc
// std::string cleanUpString(std::string stringOfPositions)
// {
//     stringOfPositions.erase(
//         std::remove_if(stringOfPositions.begin(),stringOfPositions.end(),
//         [](char chr){ return chr == '(' || chr == ')' || chr == '[' || chr == ']';}),
//         stringOfPositions.end()
//         );
//         return stringOfPositions;
// }

// //Convert clean string to vector
// std::vector<float> stringToVector(std::string cleanString)
// {
//     std::vector<float> numbers;
//     std::stringstream text_stream(cleanString);
//     std::string item;

//     while (std::getline(text_stream, item, ','))
//     {
//         numbers.push_back(std::stof(item));
//     }
//     return numbers;
// }


// std::vector<std::vector<float>> guiCoordsToNS3Coords(std::vector<float> vectorAllBuildingCords)
// {


//   std::vector<std::vector<float>> vectorVectorWithAllCords;

//   int amountOfBuildingsTimesTwo = vectorAllBuildingCords.size();


//   for(int i = 0; i < amountOfBuildingsTimesTwo; i+=2)
//   {

//     //why doesnt this loop twice when we have two buildings, amountofbuildings is 4 thereofre we should have one loop at i = 0 and one at i = 2
//     float xmin = vectorAllBuildingCords.at(i)-25;
//     float xmax = vectorAllBuildingCords.at(i)+25;
//     float ymin = vectorAllBuildingCords.at(i+1)-25;
//     float ymax = vectorAllBuildingCords.at(i+1)+25;

//     std::vector<float> tempVector;

//     tempVector.push_back(xmin);
//     tempVector.push_back(xmax);
//     tempVector.push_back(ymin);
//     tempVector.push_back(ymax);


//     vectorVectorWithAllCords.push_back(tempVector);


//   }


//   return vectorVectorWithAllCords;
// }

// int counterRP = 0;      // Counter for the receieve packet

// void
// Throughput1 (Time binSize, std::vector<double> throughPutVectors, std::vector<uint32_t> byteCounterVectors, std::vector<uint32_t> oldByteCounterVectors, int numUEsNEW)
// {


//   for (int i = 0; i < counterRP; i++)
//   {



//     throughPutVector.at(i) = (byteCounterVector.at(i) - oldByteCounterVector.at(i)) * 8 / binSize.GetSeconds () / 1024 / 1024;
//     oldByteCounterVector.at(i) = byteCounterVector.at(i);
//   };


//   //std::cout << "this: " <<throughPutVector.at(1) << std::endl;
//   //std::cout << "this: "  <<throughPutVector.at(2) << std::endl;


//   std::cout << std::setw(5) << Simulator::Now ().GetSeconds ();

//   for (int i = 0; i < counterRP; i++)
//   {
//       if(i==0)
//         {
//           std::cout << std::setw(13) << throughPutVector.at(i);

//         }
//       else
//         {
//           std::cout << std::setw(20) << throughPutVector.at(i);
//         }
//   }


//   //for (double i : throughPutVector)
//   //{
//   //  std::cout << std::setw(11) << i;
//   //};

//   std::cout << std::endl;

//   //cout statement

//   Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEsNEW);

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

//   std::cout << std::setw(9) << Simulator::Now ().GetSeconds ()<< std::setw(9) << cellID << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;
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
// //#######################################################ENB##########################################################3
// void EnbSinrFromUes(std::string context, uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
// {
//   std::cout << std::fixed << std::setw (10)<< std::setprecision(3) << Simulator::Now ().GetSeconds ();
//   std::cout << std::setw(7) << cellId;
//   std::cout << std::setw(7) << rnti;
//   std::cout << std::setw(15) << 10 * std::log10(sinrLinear) << std::endl;
// }

// void
// NotifyHandoverEndOkEnb (std::string context, uint64_t imsi, uint16_t cellid, uint16_t rnti)
// {
//   cellID = cellid;
// }



















// //////////////////////////////////////////////////////////////////////////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%///////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////Main Function Begins Here///////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////%%%%%%%%%%%%%%%%%%%%%%%%%%%%%///////////////////////////////////////////////////////////////////////////////

// int
// main (int argc, char *argv[])
// {
// //  uint16_t numUe = 2;
//   double simTime = 5; //Seconds
//   double dis1 = 500;
// //  double ueH = 0;
//   Time interPacketInterval = MicroSeconds(500);
//   double eNBTxPowerDbm = 30;
//   double ueTxPowerDbm = 25;
//   uint16_t eNBDlEarfcn = 100;
//   uint16_t eNBBandwidth = 6;
//   uint64_t runId =1 ;

//   double hEnb = 30.0;
//   double hUe = 1.0;

//   bool disableDl = false;
//   bool disableUl = false;
//   bool enablersrp = false;
//   bool enablesinrenb = false;
//   bool enablesinrue = false;
//   bool enableflowstats = false; 
//   bool enableRem = false;
//   bool enableUePhyStates = false; 
//   bool enableUeRrcStates = false;
//   bool enableUeNasStates = false; 
//   bool enableEnbRrcStates = false;
//   bool enableInstTput = false;
//   bool enablebuilding = false;
//   bool a3Ho = false;
    
//   bool a2a4Ho = true; 
//   bool enableX2 = true;
//   bool enableTraces = true; 
//   bool enableFading = true; 

//   uint8_t a2a4SCellThreshold = 30;
//   uint8_t a2a4neighborCellOffset = 1;
    
    

//   //Parameters from GUI with buildings and UE positions
// //  std::string arrayPosUEsString = "[(-209.01456344505624,912.8082166034646),]";
// //  std::string arrayPosMobUEsString = "[(-555.5387081039651,-852.3209251417888),]";
// //  std::string arrayPosMobUEPathString = "[(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";


//   //  std::string arrayPosUE= "[(-209.01456344505624,912.8082166034646),(-109.01456344505624,502.8082166034646),m(-555.5387081039651,-852.3209251417888),(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";

//   //  std::string arrayPosUE= "[(16.498907646474493,1039.2816442051494),(2298.8477987421384,-1061.2770229184869),(3129.3277963728265,1017.2799035416064),"
//   //      "m(-472.96868586560754,-742.344031575107),(-252.98325057927826,-808.3301677151171),(3998.2352863290293,-753.3417209317761),]";

// //  std::string arrayPoseNBsString = "[(0,0),(2337.6628806354956,38.49191274833947),]";
// //  std::string arrayPosBuildingsString = "[(0,1),]";
// //    std::string arrayPosMobUE= "[(-555.5387081039651,-852.3209251417888),(55.003832485540556,-874.3163038551255),(3844.767890739321,-830.3255464284525),]";

// //  LogLevel logLevel1 = (LogLevel)( LOG_PREFIX_TIME | LOG_LEVEL_INFO  );
// //  LogComponentEnable ("LteEnbRrc", logLevel1);
// //  LogComponentEnable ("LteUeRrc", logLevel1);


// //  std::cout << arrayPosUEsString << std::endl;
// //  std::cout << arrayPosMobUEsString << std::endl;
// //  std::cout << arrayPosMobUEPathString << std::endl;


//     // std::string arrayPosUE= "[m(-451.06896459986405,-467.4741209713585),(-214.53280023652053,-571.968336247309),(1864.7851097482176,-610.4662050331863),]";
//     // std::string arrayPosBuildingsString = "[(0,0),]";
//     // std::string arrayPoseNBsString = "[(0,0),(1501.729601655645,-32.998173245036924),]";

//   std::string arrayPosUE, arrayPosBuildingsString, arrayPoseNBsString;



//   // Command line arguments
//   CommandLine cmd (__FILE__);

//   cmd.AddValue ("enablersrp", "get rsrp rsrq stats", enablersrp);
//   cmd.AddValue ("enablesinrue", "get ue sinr stats", enablesinrue);
//   cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
//   cmd.AddValue ("arrayPoseNBsString", "Positions of eNBs", arrayPoseNBsString);
//   cmd.AddValue ("arrayPosUE", "Position and Path of UEs", arrayPosUE);
//   cmd.AddValue ("arrayPosBuildingsString", "Positions of Buildings", arrayPosBuildingsString);
//   cmd.AddValue ("eNBTxPowerDbm", "eNB transmit power", eNBTxPowerDbm);
//   cmd.AddValue ("runId", "Randomization parameter", runId);
//   cmd.AddValue ("enableX2", "Handover Enabler", enableX2);
//   cmd.AddValue ("enablebuilding", "Building Part Enabler", enablebuilding);


//   ConfigStore inputConfig;
//   inputConfig.ConfigureDefaults ();

//   // parse again so you can override default values from the command line
//   cmd.Parse(argc, argv);


//   int pos1 = arrayPosUE.find("m");


//   std::string arrayPosUEsString =  arrayPosUE.substr(0,pos1) + "]";

//   std::string arrayPosMobUEString = "[" + arrayPosUE.substr(pos1 + 1) + "]";

//   int pos2 = arrayPosMobUEString.find(")");


//   std::string arrayPosMobUEsString = arrayPosMobUEString.substr(0,pos2+ 2) + "]";

//   std::string arrayPosMobUEPathString = "[" + arrayPosMobUEString.substr(pos2 + 2);

//   RngSeedManager::SetRun(runId);
    
//   // std::cout<< arrayPosUEsString <<std::endl << arrayPosMobUEString <<std::endl << arrayPosMobUEsString <<std::endl << arrayPosMobUEPathString <<std::endl;



//   std::string newUEsString = cleanUpString(arrayPosUEsString);
//   std::string neweNBsString = cleanUpString(arrayPoseNBsString);
//   std::string newMobUesString = cleanUpString(arrayPosMobUEsString);
//   std::string newMobUesPathString = cleanUpString(arrayPosMobUEPathString);


//   std::vector<float>  UECords = stringToVector(newUEsString);
//   std::vector<float>  eNBCords = stringToVector(neweNBsString);
//   std::vector<float>  mobUECords = stringToVector(newMobUesString);
//   std::vector<float>  mobUEPathCords = stringToVector(newMobUesPathString);
    
//   // std::cout<< newUEsString <<std::endl << neweNBsString <<std::endl <<newMobUesString <<std::endl <<newMobUesPathString <<std::endl;  



// //
// //  std::cout<< newUEsString << std::endl;
// //  std::cout<< neweNBsString << std::endl;
// //  std::cout<< newMobUesString <<std::endl;
// //  std::cout<< newMobUesPathString <<std::endl;
    
//     // std::cout<< arrayPosBuildingsString ;


// //   if(arrayPosBuildingsString == "(0,0),")
// //      {
// //        enablebuilding = false;
// //      }
// //    else
// //      {
// //        enablebuilding = true;
// //      }

//  // std::cout<< enablebuilding <<std::endl;

    
    
    
//   Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
//   Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
//   lteHelper->SetEpcHelper (epcHelper);
    
    
//   lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (eNBDlEarfcn));
//   lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (eNBDlEarfcn + 18000));
//   lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (eNBBandwidth));
//   lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (eNBBandwidth));
//   lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");


//   Ptr<UniformRandomVariable> enbNoise = CreateObject<UniformRandomVariable> ();
//   enbNoise->SetAttribute ("Min", DoubleValue (1));
//   enbNoise->SetAttribute ("Max", DoubleValue (5));

//   Ptr<UniformRandomVariable> ueNoise = CreateObject<UniformRandomVariable> ();
//   ueNoise->SetAttribute ("Min", DoubleValue (1));
//   ueNoise->SetAttribute ("Max", DoubleValue (5));

//   Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNBTxPowerDbm));
//   Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPowerDbm));
//   Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoise->GetValue()));
//   Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (enbNoise->GetValue()));
//   Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
    
    
//   if(enableX2)
//   {
//     if(a2a4Ho)
//     {
//       lteHelper->SetHandoverAlgorithmType ("ns3::A2A4RsrqHandoverAlgorithm");
//       lteHelper->SetHandoverAlgorithmAttribute ("ServingCellThreshold", UintegerValue (a2a4SCellThreshold));
//       lteHelper->SetHandoverAlgorithmAttribute ("NeighbourCellOffset", UintegerValue (a2a4neighborCellOffset));
//       // std::cout<< "The HO Algorithms is: A2A4" <<std::endl;
//     }
//   if(a3Ho)
//     {
//       lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
//       lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (0));
//       lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (0)));
//       // std::cout<< "The HO Algorithm is: A3" <<std::endl;

//     }
//   }
    
// if(enablebuilding)
//     {
//       Ptr<UniformRandomVariable> iwl = CreateObject<UniformRandomVariable> ();
//       iwl->SetAttribute ("Min", DoubleValue (1));
//       iwl->SetAttribute ("Max", DoubleValue (5));
//       Ptr<UniformRandomVariable> ssew = CreateObject<UniformRandomVariable> ();
//       ssew->SetAttribute ("Min", DoubleValue (1));
//       ssew->SetAttribute ("Max", DoubleValue (5));
//       Ptr<UniformRandomVariable> sso = CreateObject<UniformRandomVariable> ();
//       sso->SetAttribute ("Min", DoubleValue (1));
//       sso->SetAttribute ("Max", DoubleValue (7));
//       Ptr<UniformRandomVariable> ssi = CreateObject<UniformRandomVariable> ();
//       ssi->SetAttribute ("Min", DoubleValue (1));
//       ssi->SetAttribute ("Max", DoubleValue (8));


//       lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (ssew->GetValue()));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (sso->GetValue()));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (ssi->GetValue()));
//       lteHelper->SetPathlossModelAttribute ("InternalWallLoss", DoubleValue (iwl->GetValue()));

//     }
//   else
//     {

//       Ptr<UniformRandomVariable> d0 = CreateObject<UniformRandomVariable> ();
//       d0->SetAttribute ("Min", DoubleValue (1));
//       d0->SetAttribute ("Max", DoubleValue (5));
//       Ptr<UniformRandomVariable> d1 = CreateObject<UniformRandomVariable> ();
//       d1->SetAttribute ("Min", DoubleValue (200));
//       d1->SetAttribute ("Max", DoubleValue (250));
//       Ptr<UniformRandomVariable> d2 = CreateObject<UniformRandomVariable> ();
//       d2->SetAttribute ("Min", DoubleValue (500));
//       d2->SetAttribute ("Max", DoubleValue (600));
//       Ptr<UniformRandomVariable> e0 = CreateObject<UniformRandomVariable> ();
//       e0->SetAttribute ("Min", DoubleValue (1.75));
//       e0->SetAttribute ("Max", DoubleValue (2));
//       Ptr<UniformRandomVariable> e1 = CreateObject<UniformRandomVariable> ();
//       e1->SetAttribute ("Min", DoubleValue (3.4));
//       e1->SetAttribute ("Max", DoubleValue (4.4));
//       Ptr<UniformRandomVariable> e2 = CreateObject<UniformRandomVariable> ();
//       e2->SetAttribute ("Min", DoubleValue (4.8));
//       e2->SetAttribute ("Max", DoubleValue (5.8));


//     lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));
//     lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue(d0->GetValue()));
//     lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue(d1->GetValue()));
//     lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue(d2->GetValue()));
//     lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue(e0->GetValue()));
//     lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue(e1->GetValue()));
//     lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue(e2->GetValue()));
//     }


    

//   if(enableFading)
//     {
//       lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));

//       std::ifstream ifTraceFile;
//       ifTraceFile.open ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
//       if (ifTraceFile.good ())
//         {
//           // script launched by test.py
//           lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("../../src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
//         }
//       else
//         {
//           // script launched as an example
//           lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("src/lte/model/fading-traces/fading_trace_EPA_3kmph.fad"));
//         }

//       // these parameters have to be set only in case of the trace format
//       // differs from the standard one, that is
//       // - 10 seconds length trace
//       // - 10,000 samples
//       // - 0.5 seconds for window size
//       // - 100 RB
//       lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
//       lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
//       lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
//       lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
//     }

//   //  epcHelper->SetAttribute("S1uLinkEnablePcap", BooleanValue(true));
// //  epcHelper->SetAttribute("X2LinkEnablePcap", BooleanValue(true));
    
  

//    if(enablebuilding)
//      {

//        std::string newBuildingsString = cleanUpString(arrayPosBuildingsString);
//        std::vector<float>  BUCords = stringToVector(newBuildingsString);


//        uint8_t floors = 10;
//        uint8_t Xrooms = 5;
//        uint8_t Yrooms = 5;


//       // std::cout << "New Buildings String: " << newBuildingsString << std::endl;


//        std::vector<std::vector<float>> vectorVectorBuildingsBounds = guiCoordsToNS3Coords(BUCords);


//        //...
//        std::vector<Ptr<Building>>  objects;

//        int amountOfBuildings = BUCords.size()/2;

//        objects.resize(amountOfBuildings);

//        //std::cout <<"Amount Of Buildings: " << amountOfBuildings << std::endl;

//        int temp = 0;




//        //std::cout << "Vector Vector with Building Bounds size: " << vectorVectorBuildingsBounds.size() << std::endl;




//        for (Ptr<Building> i : objects)
//        {
//          std::vector<float> tempVector = vectorVectorBuildingsBounds.at(temp);


//          i = CreateObject <Building> ();
//          i->SetBoundaries (Box (tempVector.at(0),tempVector.at(1),tempVector.at(2),tempVector.at(3),0,7.5));
//          i->SetBuildingType (Building::Residential);
//          i->SetExtWallsType (Building::ConcreteWithWindows);
//          i->SetNFloors (floors);
//          i->SetNRoomsX (Xrooms);
//          i->SetNRoomsY (Yrooms);

//          temp++;
//        }

//      }

  
//   Ptr<Node> pgw = epcHelper->GetPgwNode ();

//    // Create a single RemoteHost
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


//   // Create Nodes: eNodeB and UE
//   NodeContainer eNBNodesArg;
//   NodeContainer ueNodesArg;
//   NodeContainer mobileueNodesArg;

//   //NodeContainer movingNodes;
//   //movingueNodes.Create(1);


//   int numUEsNEW = UECords.size()/2;  //number of UEs defined by user
//   int numeNBsNEW = eNBCords.size()/2;  //number of UEs defined by user
//   int numMobUePathNEW = mobUEPathCords.size()/2;  //number of UEs defined by user
//   int numMobUeNEW = mobUECords.size()/2;  //number of UEs defined by user
    
//   // std::cout<< numUEsNEW <<std::endl << numeNBsNEW <<std::endl << numMobUePathNEW <<std::endl << numMobUeNEW <<std::endl;



//   ueNodesArg.Create (numUEsNEW);
//   eNBNodesArg.Create (numeNBsNEW);
//   mobileueNodesArg.Create (numMobUeNEW);

//   // Position of UEs attached to eNB 1 // attached to eNB1 ???
//   Ptr<ListPositionAllocator> positionAllocue = CreateObject<ListPositionAllocator> ();

//   //Allocate cordinates to object
//   for(int i = 0; i < numUEsNEW*2; i+=2)
//     {
//         float x = UECords.at(i);
//         float y = UECords.at(i+1);
//         positionAllocue->Add (Vector (x, y, hUe)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
//     }

//   //Pass object with cordinates into the mobilityobject of the UEs
//   MobilityHelper uemobility;
//   uemobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//   uemobility.SetPositionAllocator (positionAllocue); //why? // this doesnt seem nessecary //this has been fixed !
//   uemobility.Install (ueNodesArg);
//   if(enablebuilding)
//     {
//       BuildingsHelper::Install(ueNodesArg);

//     }

//   // Position of UEs attached to eNB 1 // attached to eNB1 ???
//   Ptr<ListPositionAllocator> positionAlloceNB = CreateObject<ListPositionAllocator> ();

//   //Allocate cordinates to object
//   for(int i = 0; i < numeNBsNEW*2; i+=2)
//     {
//         float x = eNBCords.at(i);
//         float y = eNBCords.at(i+1);
//         positionAlloceNB->Add (Vector (x, y, hEnb)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
//     }

//   //Pass object with cordinates into the mobilityobject of the UEs
//   MobilityHelper eNBmobility;
//   eNBmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//   eNBmobility.SetPositionAllocator (positionAlloceNB); //why? // this doesnt seem nessecary //this has been fixed !
//   eNBmobility.Install (eNBNodesArg);

//   if(enablebuilding)
//     {
//       BuildingsHelper::Install(eNBNodesArg);

//     }
//   // Position of UEs attached to eNB 1 // attached to eNB1 ???
//   Ptr<ListPositionAllocator> positionAllocmobileUE = CreateObject<ListPositionAllocator> ();

//   //Allocate cordinates to object
//   for(int i = 0; i < numMobUeNEW*2; i+=2)
//     {
//         float x = mobUECords.at(i);
//         float y = mobUECords.at(i+1);
//         positionAllocmobileUE->Add (Vector (x, y, hUe)); //this doesnt seem right, what if we are not adding a vector to an UE for eNB 1? //fixed issue
//     }



//   MobilityHelper movingueMobility;

//   movingueMobility.SetPositionAllocator(positionAllocmobileUE);
//   movingueMobility.SetMobilityModel("ns3::WaypointMobilityModel");
//   movingueMobility.Install (mobileueNodesArg);


//   Ptr<WaypointMobilityModel> wayMobility;

//   wayMobility = mobileueNodesArg.Get(0)->GetObject<WaypointMobilityModel>();

//   for(int i = 0; i < numMobUePathNEW*2; i+=2)
//     {
//         float x = mobUEPathCords.at(i);
//         float y = mobUEPathCords.at(i+1);
//         wayMobility->AddWaypoint(Waypoint(Seconds(simTime - (simTime/(i + 1))), Vector3D(x, y, hUe)));
//     }



//   if(enablebuilding)
//     {
//       BuildingsHelper::Install(mobileueNodesArg);

//     }
    
//   // Install LTE Devices to the nodes
//   NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (eNBNodesArg);
//   NetDeviceContainer movingueDevs = lteHelper->InstallUeDevice (mobileueNodesArg);
//   NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodesArg);


//   NodeContainer ueNodes;
//   ueNodes.Add(ueNodesArg);
//   ueNodes.Add(mobileueNodesArg);

//   NetDeviceContainer ueDevs;
//   ueDevs.Add(ueLteDevs);
//   ueDevs.Add(movingueDevs);

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

// //  // Attach one UE per eNodeB
// //  for (uint16_t i = 0; i < numNodePairs; i++)
// //    {
// //      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
// //      // side effect: the default EPS bearer will be activated
// //    }

//   lteHelper->Attach(ueDevs);

//   // Install and start applications on UEs and remote host
//   uint16_t dlPort = 1100;
//   uint16_t ulPort = 2000;
//   ApplicationContainer clientApps;
//   ApplicationContainer serverApps;

//   Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
//   startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
//   startTimeSeconds->SetAttribute ("Max", DoubleValue (0.10));
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

//     if(enableX2)
//     {
//         // Add X2 interface
//         lteHelper->AddX2Interface (eNBNodesArg);
//     }


//   if(enableTraces)
//     {
//       if(enableUePhyStates)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(8) << "cellID";
//           std::cout << std::setw(8) << "rnti";
//           std::cout << std::setw(20) << "OldState" ;
//           std::cout << std::setw(20) << "NewState" << std::endl;
//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/StateTransition"
//               , MakeCallback (&UePhyStateChange));
//         }
//       if(enableUeRrcStates)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(8) << "cellID";
//           std::cout << std::setw(8) << "imsi";
//           std::cout << std::setw(8) << "rnti";
//           std::cout << std::setw(20) << "OldState" ;
//           std::cout << std::setw(20) << "NewState" << std::endl;
//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/LteUeRrc/StateTransition"
//               , MakeCallback (&UeRrcStateChange));
//         }
//       if(enableUeNasStates)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(20) << "OldState" ;
//           std::cout << std::setw(20) << "NewState" << std::endl;
//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/EpcUeNas/StateTransition"
//               , MakeCallback (&EpcUeNasStateChange));
//         }

//       if(enablersrp)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(8) << "cellID";
//           std::cout << std::setw(8) << "rnti" ;
//           std::cout << std::setw(12) << "rsrp" ;
//           std::cout << std::setw(12) << "rsrq" << std::endl;

//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements"
//               , MakeCallback (&RsrpRsrqFromUes));
//         }

//       if(enablesinrenb)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(8) << "cellID";
//           std::cout << std::setw(8) << "rnti" ;
//           std::cout << std::setw(12) << "sinr" << std::endl;

//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteEnbNetDevice/ComponentCarrierMap/*/LteEnbPhy/ReportUeSinr"
//               , MakeCallback (&EnbSinrFromUes));
//         }

//       if(enablesinrue)
//         {
//           std::cout << "#";
//           std::cout << std::setw(9) << "time";
//           std::cout << std::setw(8) << "cellID";
//           std::cout << std::setw(8) << "rnti" ;
//           std::cout << std::setw(12) << "rsrp" ;
//           std::cout << std::setw(12) << "sinr" << std::endl;

//           Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr"
//               , MakeCallback (&RsrpSinrFromUes));
//         }
//     }
//   // Uncomment to enable PCAP tracing
//   //p2ph.EnablePcapAll("lena-simple-epc");

//   Ptr<RadioEnvironmentMapHelper> remHelper;
//   if(enableRem)
//     {

//       // Configure Radio Environment Map (REM) output
//       // for LTE-only simulations always use /ChannelList/0 which is the downlink channel
//       remHelper = CreateObject<RadioEnvironmentMapHelper> ();
//       //  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
//       remHelper->SetAttribute ("Channel", PointerValue (lteHelper->GetDownlinkSpectrumChannel ()));
//       remHelper->SetAttribute ("OutputFile", StringValue ("lab5-rem.out"));
//       remHelper->SetAttribute ("XMin", DoubleValue (-2*numeNBsNEW*dis1));
//       remHelper->SetAttribute ("XMax", DoubleValue (2*numeNBsNEW*dis1));
//       remHelper->SetAttribute ("YMin", DoubleValue (-2*numeNBsNEW*dis1));
//       remHelper->SetAttribute ("YMax", DoubleValue (2*numeNBsNEW*dis1));
//       remHelper->SetAttribute ("Z", DoubleValue (1.5));
//       remHelper->Install ();
//     }

//   FlowMonitorHelper flowmonHelper;
//   Ptr<ns3::FlowMonitor> monitor;
//   if(enableflowstats)
//     {

//       NodeContainer endpointNodes;
//       endpointNodes.Add (ueNodes);
//       endpointNodes.Add(remoteHost);

//       monitor = flowmonHelper.Install (endpointNodes);
//       monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
//       monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
//       monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));
//       //
//       //      monitor->SerializeToXmlFile("flowmon.xml", true, true);
//       //
//       //      std::string anim_filename = outputDir + "/" + "anim.xml";
//       //      AnimationInterface anim (anim_filename);
//       //      anim.SetMobilityPollInterval (MilliSeconds(5));
//       //      anim.EnablePacketMetadata (true);
//       //      anim.EnableIpv4L3ProtocolCounters(startTime, simTime, MilliSeconds(1));
//       //      anim.UpdateNodeSize (6, 1.5, 1.5);
//     }


//   if(enableTraces)
//     {
//       if(enableEnbRrcStates)
//         {
//           Simulator::Schedule(Seconds(0.26), &enbRrcTracing);
//         }
// //      lteHelper->EnableTraces ();

//     }
// //  p2ph.EnablePcapAll("w_results/pcaps/task2-1", true);

//   if(enableInstTput)
//     {

//       //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
//       //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
//       //std::vector<double> throughputVector(numUEsNEW, 0);

//       int total_ues = numUEsNEW + numMobUeNEW;

//       for (int i = 0; i < total_ues; i++)
//       {


// //        std::cout << "Number of UEs: " << total_ues << std::endl;
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
//       std::cout << std::setw(5) << "time";
//       for (int i = 0; i < total_ues; i++)
//       {
//           std::cout << std::setw(13) << "Tput-UE" << i << "(Mbps)";
//       }
//       std::cout << std::endl;
//       Time binSize = Seconds (0.01);
//       Simulator::Schedule (Seconds (0), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, total_ues);

//     }



//   Simulator::Stop (Seconds(simTime));
//   Simulator::Run ();



//   if(enableflowstats)
//     {
//       // Print per-flow statistics
//       monitor->CheckForLostPackets ();
//       Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
//       FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

//       double averageFlowThroughput = 0.0;
//       double averageFlowDelay = 0.0;


//       for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
//         {
//           Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
//           std::stringstream protoStream;
//           std::string channel;
//           protoStream << (uint16_t) t.protocol;
//           if (t.protocol == 6)
//             {
//               protoStream.str ("TCP");
//             }
//           if (t.protocol == 17)
//             {
//               protoStream.str ("UDP");
//             }

//           if(t.sourceAddress.GetSubnetDirectedBroadcast("255.0.0.0") == "7.255.255.255")
//             {
//               channel = "UL";
//             }
//           else
//             {
//               channel = "DL";
//             }
//           std::cout  << "\n<" << channel << "> " << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n\n";
//           //    std::cout  << "  Tx Packets: " << i->second.txPackets << "\n";
//           //    std::cout  << "  Tx Bytes:   " << i->second.txBytes << "\n";
//           //    std::cout  << "  TxOffered:  " << (((i->second.txBytes * 8.0) / (simTime.GetSeconds() - startTime.GetSeconds())) / 1000) / 1000  << " Mbps\n";
//           //    std::cout  << "  Rx Bytes:   " << i->second.rxBytes << "\n";
//                 std::cout << "   Packet Lost: " << i->second.lostPackets << "\n";

//           if (i->second.rxPackets > 0)
//             {
//               // Measure the duration of the flow from receiver's perspective
//               //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
//               double rxDuration = simTime - startTime.GetSeconds();

//               averageFlowThroughput += (((i->second.rxBytes * 8.0) / rxDuration) / 1024) / 1024;
//               averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

//               std::cout  << "  Throughput: " << (((i->second.rxBytes * 8.0) / rxDuration) / 1024) / 1024  << " Mbps\n";
//               std::cout  << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
//               //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
//               std::cout  << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
//             }
//           else
//             {
//               std::cout << "  Throughput:  0 Mbps\n";
//               std::cout  << "  Mean delay:  0 ms\n";
//               std::cout  << "  Mean jitter: 0 ms\n";
//             }
//         }
//     }


//   Simulator::Destroy ();
//   return 0;
// }
