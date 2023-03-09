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


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ Other functions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
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
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$

using namespace ns3;
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Global Variables $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
std::vector<uint32_t> byteCounterVector(20, 0);
std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
std::vector<double> throughPutVector(20, 0);


//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ string clean ip functions $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$4
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
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Tracing functions for output logs$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
void RsrpRsrqFromUes(std::string context, uint16_t rnti, uint16_t cellId, double rsrp, double rsrq, bool isServingCell, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds () << std::setw(7) << cellId << std::setw(7) << rnti << std::setw(15) << rsrp << std::setw(15) << rsrq << std::endl;
}
void RsrpSinrFromUes (std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds () << std::setw(7) << cellId << std::setw(7) << rnti << std::setw(15) << rsrp << std::setw(15) << 10 * std::log10(sinr) << std::endl;
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

  std::cout << std::endl;
  Simulator::Schedule (binSize, Throughput1, binSize, throughPutVectors, byteCounterVectors, oldByteCounterVectors, numUEsNEW);
}
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$s$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


int
main (int argc, char *argv[])
{

  double simTime = 5; //Seconds
  Time interPacketInterval = MicroSeconds(500);
  double eNBTxPowerDbm = 45, ueTxPowerDbm = 25;
  uint16_t eNBDlEarfcn = 100, eNBBandwidth = 25, bw1 = 25, bw2 = 25;
  uint64_t runId =1 ;
  double hEnb = 30.0, hUe = 1.0;

  bool disableDl = false, disableUl = false, enablersrp = false, enablesinrue = false, enableTraces = true, enableInstTput = false;
  bool enableFading = false; bool enablebuilding = false, enableX2 = false, a2a4Ho = true, a3Ho = false, enable_diff_freq = false;
  uint8_t a2a4SCellThreshold = 30, a2a4neighborCellOffset = 1;

  std::string testPos = "es(0,0),(2287.9396470619276,5.501581753424205),ee_bs(0,0),be_usue_ms(-637.9831708153451,-847.24359002721),(-456.4879584282212,-797.7293542463997),(2859.924558827409,-935.2688980819853),me_";

  // Command line arguments
  CommandLine cmd (__FILE__);
  cmd.AddValue ("enablersrp", "get rsrp rsrq stats", enablersrp);
  cmd.AddValue ("enablesinrue", "get ue sinr stats", enablesinrue);
  cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);
  cmd.AddValue ("enableX2", "Enable radio environment map", enableX2);
  cmd.AddValue ("testPos", "Location information for nodes and buildings", testPos);
  cmd.AddValue ("runId", "Randomization parameter", runId);
  cmd.AddValue ("eNBTxPowerDbm", "eNB transmit power", eNBTxPowerDbm);
  cmd.AddValue ("bw1", "eNB transmit power", bw1);
  cmd.AddValue ("bw2", "eNB transmit power", bw2);
  cmd.AddValue ("enable_diff_freq", "eNB transmit power", enable_diff_freq);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  cmd.Parse(argc, argv);
  RngSeedManager::SetRun(runId);

  Ptr<UniformRandomVariable> enbNoise = CreateObject<UniformRandomVariable> ();
  enbNoise->SetAttribute ("Min", DoubleValue (1));
  enbNoise->SetAttribute ("Max", DoubleValue (2));

  Ptr<UniformRandomVariable> ueNoise = CreateObject<UniformRandomVariable> ();
  ueNoise->SetAttribute ("Min", DoubleValue (1));
  ueNoise->SetAttribute ("Max", DoubleValue (2));

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNBTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (ueTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoise->GetValue()));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue (enbNoise->GetValue()));
  Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
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
          lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue (3));
          lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (256)));
        }
    }

  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Post Processing of Node Locations from CMD$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  int poseNBstart = testPos.find("es");
  int poseNBend = testPos.find("ee");
  int posbuildstart = testPos.find("bs");
  int posbuildend = testPos.find("be");
  int posUestart = testPos.find("us");
  int posUeend = testPos.find("ue");
  int posmUestart = testPos.find("ms");
  int posmUeend = testPos.find("me");

  std::string arrayPoseNBsString = "[" + testPos.substr(poseNBstart+2, poseNBend-poseNBstart-2) + "]"; /// seems correct
  std::string arrayPosBuildingsString = "[" + testPos.substr(posbuildstart+2, posbuildend-posbuildstart-2) + "]"; //wierd output
  std::string arrayPosUEsString = "[" + testPos.substr(posUestart+2, posUeend-posUestart-2) + "]"; //wierd output
  std::string mUEPosString = "[" + testPos.substr(posmUestart+2, posmUeend-posmUestart-2) + "]"; //seems correct

  //  std::cout<< eNBPosString << std::endl;
  //  std::cout<< buildPosString << std::endl;
  //  std::cout<< sUEPosString << std::endl;
  //  std::cout<< mUEPosString << std::endl;

  int pos2 = mUEPosString.find(")");
  std::string arrayPosMobUEsString = mUEPosString.substr(0,pos2+ 2) + "]";
  std::string arrayPosMobUEPathString = "[" + mUEPosString.substr(pos2 + 2);
  std::string newUEsString = cleanUpString(arrayPosUEsString);
  std::string neweNBsString = cleanUpString(arrayPoseNBsString);
  std::string newMobUesString = cleanUpString(arrayPosMobUEsString);
  std::string newMobUesPathString = cleanUpString(arrayPosMobUEPathString);

  std::vector<float>  UECords = stringToVector(newUEsString);
  std::vector<float>  eNBCords = stringToVector(neweNBsString);
  std::vector<float>  mobUECords = stringToVector(newMobUesString);
  std::vector<float>  mobUEPathCords = stringToVector(newMobUesPathString);

  //  std::cout<< newUEsString << std::endl;
  //  std::cout<< neweNBsString << std::endl;
  //  std::cout<< newMobUesString <<std::endl;
  //  std::cout<< newMobUesPathString <<std::endl;
  // std::cout<< arrayPosBuildingsString ;

  if(arrayPosBuildingsString == "(0,0),")
    {
      enablebuilding = false;
    }
  else
    {
      enablebuilding = true;
    }

   // std::cout<< enablebuilding <<std::endl;
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Channel Propagation Setup and Config$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
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

  if(enablebuilding)
    {
      Ptr<UniformRandomVariable> iwl = CreateObject<UniformRandomVariable> ();
      iwl->SetAttribute ("Min", DoubleValue (1));
      iwl->SetAttribute ("Max", DoubleValue (3));
      Ptr<UniformRandomVariable> ssew = CreateObject<UniformRandomVariable> ();
      ssew->SetAttribute ("Min", DoubleValue (1));
      ssew->SetAttribute ("Max", DoubleValue (3));
      Ptr<UniformRandomVariable> sso = CreateObject<UniformRandomVariable> ();
      sso->SetAttribute ("Min", DoubleValue (1));
      sso->SetAttribute ("Max", DoubleValue (3));
      Ptr<UniformRandomVariable> ssi = CreateObject<UniformRandomVariable> ();
      ssi->SetAttribute ("Min", DoubleValue (1));
      ssi->SetAttribute ("Max", DoubleValue (3));

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
      d1->SetAttribute ("Min", DoubleValue (150));
      d1->SetAttribute ("Max", DoubleValue (160));
      Ptr<UniformRandomVariable> d2 = CreateObject<UniformRandomVariable> ();
      d2->SetAttribute ("Min", DoubleValue (300));
      d2->SetAttribute ("Max", DoubleValue (310));
      Ptr<UniformRandomVariable> e0 = CreateObject<UniformRandomVariable> ();
      e0->SetAttribute ("Min", DoubleValue (2.95));
      e0->SetAttribute ("Max", DoubleValue (3.5));
      Ptr<UniformRandomVariable> e1 = CreateObject<UniformRandomVariable> ();
      e1->SetAttribute ("Min", DoubleValue (4.4));
      e1->SetAttribute ("Max", DoubleValue (5));
      Ptr<UniformRandomVariable> e2 = CreateObject<UniformRandomVariable> ();
      e2->SetAttribute ("Min", DoubleValue (5.8));
      e2->SetAttribute ("Max", DoubleValue (6.2));

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
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Create the EPC connection with the external networks$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
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
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Creation, Placement, and Mobility  of UE, eNBS in the RAN$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  // Create Nodes: eNodeB and UE
  NodeContainer eNBNodesArg;
  NodeContainer ueNodesArg;
  NodeContainer mobileueNodesArg;

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

  Ptr<LteEnbNetDevice> enb1, enb2;

  for(int i = 0; i <numeNBsNEW; i+=1)
    {
      if(i%2 == 0)
        {
          enb1 = enbLteDevs.Get(0)->GetObject<LteEnbNetDevice>();
          enb1->SetDlBandwidth(bw1);
          enb1->SetUlBandwidth(bw1);
          if(enable_diff_freq)
          {
              enb1->SetDlEarfcn(eNBDlEarfcn);
              enb1->SetUlEarfcn(eNBDlEarfcn + 18000);
          }
        }
      if(i%2 !=0)
        {
          enb2 = enbLteDevs.Get(1)->GetObject<LteEnbNetDevice>();
          enb2->SetDlBandwidth(bw2);
          enb2->SetUlBandwidth(bw2);
          if(enable_diff_freq)
          {
              enb2->SetDlEarfcn(eNBDlEarfcn * 6);
              enb2->SetUlEarfcn(eNBDlEarfcn * 6 + 18000);
          }

        }
    }

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

  // UE attachement with eNB
  lteHelper->Attach(ueDevs);
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Applications running on UEs and remote nodes$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
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
  //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$Ouput formating and processing for results$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  if(enableTraces)
    {
      if(enablersrp)
        {
          std::cout << "#" << std::setw(9) << "time" << std::setw(8) << "cellID" << std::setw(8) << "rnti" << std::setw(12) << "rsrp" << std::setw(12) << "rsrq" << std::endl;
          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportUeMeasurements", MakeCallback (&RsrpRsrqFromUes));
        }
      if(enablesinrue)
        {
          std::cout << "#" << std::setw(9) << "time" << std::setw(8) << "cellID" << std::setw(8) << "rnti" << std::setw(12) << "rsrp" << std::setw(12) << "sinr" << std::endl;
          Config::Connect("/NodeList/*/DeviceList/*/$ns3::LteUeNetDevice/ComponentCarrierMapUe/*/LteUePhy/ReportCurrentCellRsrpSinr", MakeCallback (&RsrpSinrFromUes));
        }
      int total_ues = numUEsNEW + numMobUeNEW;
      if(enableInstTput)
        {
          for (int i = 0; i < total_ues; i++)
            {
              //        std::cout << "Number of UEs: " << total_ues << std::endl;
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
          Time binSize = Seconds (0.05);
          Simulator::Schedule (Seconds (0), &Throughput1, binSize, throughPutVector, byteCounterVector, oldByteCounterVector, total_ues);
        }
    }


  Simulator::Stop (Seconds(simTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}
