#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include "ns3/radio-bearer-stats-calculator.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include <iomanip>
#include <string>
#include "ns3/flow-monitor-module.h"
#include "ns3/rectangle.h"
#include <ns3/buildings-module.h>
#include <ns3/spectrum-helper.h>
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

NS_LOG_COMPONENT_DEFINE ("Task-2");


std::vector<uint32_t> byteCounterVector(20, 0);
std::vector<std::ostringstream> ossGroup(20);                //clean up move to top
std::vector<uint32_t> oldByteCounterVector(20, 0);           //todo create byte vector of size n
std::vector<double> throughPutVector(20, 0);

uint32_t ByteCounter1 = 0, ByteCounter2 = 0, ByteCounter3 = 0;
uint32_t oldByteCounter1 = 0, oldByteCounter2 = 0, oldByteCounter3 = 0;

//RSRP stream out
void RsrpRsrqFromUes(std::string context, uint16_t rnti, uint16_t cellId, double rsrp, double rsrq, bool isServingCell, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << rsrp;
  std::cout << std::setw(15) << rsrq << std::endl;
}

//RSRP stream out
void RsrpSinrFromUes (std::string context, uint16_t cellId, uint16_t rnti, double rsrp, double sinr, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << rsrp;
  std::cout << std::setw(15) << 10 * std::log10(sinr) << std::endl;
}

//ENb SINR stream out
void EnbSinrFromUes(std::string context, uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId)
{
  std::cout << std::fixed << std::setw (10) << Simulator::Now ().GetSeconds ();
  std::cout << std::setw(7) << cellId;
  std::cout << std::setw(7) << rnti;
  std::cout << std::setw(15) << 10 * std::log10(sinrLinear) << std::endl;
}

//Print GnuPlot list to file
void
PrintGnuplottableBuildingListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      return;
    }
  uint32_t index = 0;
  for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
    {
      ++index;
      Box box = (*it)->GetBoundaries ();
      outFile << "set object " << index
              << " rect from " << box.xMin  << "," << box.yMin
              << " to "   << box.xMax  << "," << box.yMax
              << std::endl;
    }
}

//Save the position of the UE
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

void
Throughput (Time binSize)
{

  //TODO Rewrite
  double  throughput1 = (ByteCounter1 - oldByteCounter1) * 8 / binSize.GetSeconds () / 1024 / 1024;
  double  throughput2 = (ByteCounter2 - oldByteCounter2) * 8 / binSize.GetSeconds () / 1024 / 1024;
  double  throughput3 = (ByteCounter3 - oldByteCounter3) * 8 / binSize.GetSeconds () / 1024 / 1024;

  std::cout << std::setw(9) << Simulator::Now ().GetSeconds () << std::setw(11) << throughput1 << std::setw(11) << throughput2 << std::setw(11) << throughput3 << std::endl;
  oldByteCounter1 = ByteCounter1;
  oldByteCounter2 = ByteCounter2;
  oldByteCounter3 = ByteCounter3;

  Simulator::Schedule (binSize, &Throughput, binSize);
}





int main (int argc, char *argv[])
{



  double enbDist = 300.0;

  uint32_t numUes = 2;
  double simTime = 7; //  Time timeRes = MilliSeconds (10); // time resolution
  //  double speed = 500;       // m/s
  uint8_t numBearersPerUe = 1;
  double eNBTxPowerDbm = 40;  // range()
  uint16_t eNBDlEarfcn = 100;
  uint16_t eNBBandwidth = 25;
  double hEnb = 30.0;
  double hUe = 1.0;


  bool enablersrp = false, enablesinrenb = false, enablesinrue = false;
  bool enableRem = false, enableflowstats = false, enableTraces = true;
  bool enablebuilding = false, enableInstTput= false;

  //Parameters from GUI with buildings and UE positions
  std::string arrayPosUEsString = "[(66.03,-940.34),(-148.56,27.50),]";
//  std::string arrayPosBuildingsString = "[(0,0),]";
  std::string arrayPoseNBsString = "[(0,0),]";
  std::string arrayPosBuildingsString = "[]";


  CommandLine cmd (__FILE__);
  cmd.AddValue ("enbDist", "distance between the two eNBs", enbDist);
  cmd.AddValue ("numUes", "how many UEs are attached to each eNB", numUes);
  cmd.AddValue ("eNBTxPowerDbm", "base station power level", eNBTxPowerDbm);
  cmd.AddValue ("enableflowstats", "get flow stats", enableflowstats);
  cmd.AddValue ("enablersrp", "get rsrp rsrq stats", enablersrp);
  cmd.AddValue ("enablesinrue", "get ue sinr stats", enablesinrue);
  cmd.AddValue ("enablesinrenb", "get enb sinr stats", enablesinrenb);
  cmd.AddValue ("enableInstTput", "Enable instantenous throughput stats", enableInstTput);

  //Pass parameters into variables
  cmd.AddValue ("arrayPosUEsString", "Positions of UEs", arrayPosUEsString);
  cmd.AddValue ("arrayPosBuildingsString", "Positions of UEs", arrayPosBuildingsString);
  cmd.AddValue ("arrayPoseNBsString", "Positions of eNBs", arrayPoseNBsString);

  cmd.Parse (argc, argv);

  std::string newUEsString = cleanUpString(arrayPosUEsString);
  std::string neweNBsString = cleanUpString(arrayPoseNBsString);

  std::vector<float>  UECords = stringToVector(newUEsString);
  std::vector<float>  eNBCords = stringToVector(neweNBsString);


  //The cordinates of the user input in a vector with floats.



  //TODO building creation in loop with cords.

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  //double radius = enbDist/1;    DISABLE

  Ptr<UniformRandomVariable> enbNoise = CreateObject<UniformRandomVariable> ();
  enbNoise->SetAttribute ("Min", DoubleValue (1));
  enbNoise->SetAttribute ("Max", DoubleValue (5));

  Ptr<UniformRandomVariable> ueNoise = CreateObject<UniformRandomVariable> ();
  ueNoise->SetAttribute ("Min", DoubleValue (1));
  ueNoise->SetAttribute ("Max", DoubleValue (5));

  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (eNBTxPowerDbm));
  Config::SetDefault ("ns3::LteUePhy::NoiseFigure", DoubleValue (ueNoise->GetValue()));
  Config::SetDefault ("ns3::LteEnbPhy::NoiseFigure", DoubleValue          (enbNoise->GetValue()));
  Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(80));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();


  // Create a single remote host to act as hosts on the interent
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);


  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);


  // Create Nodes: eNodeB and UE
  NodeContainer eNBNodesArg;
  NodeContainer ueNodesArg;
  //NodeContainer movingNodes;
  //movingueNodes.Create(1);


  int numUEsNEW = UECords.size()/2;  //number of UEs defined by user
  int numeNBsNEW = eNBCords.size()/2;  //number of UEs defined by user //TODO


  ueNodesArg.Create (numUEsNEW);
  eNBNodesArg.Create (numeNBsNEW);


  if(arrayPosBuildingsString == "[]")
    {
      enablebuilding = false;
    }
  else
    {
      enablebuilding = true;
    }


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


  lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (eNBDlEarfcn));
  lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (eNBDlEarfcn + 18000));
  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (eNBBandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (eNBBandwidth));


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
  MobilityHelper ue1mobility;
  ue1mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ue1mobility.SetPositionAllocator (positionAllocue); //why? // this doesnt seem nessecary //this has been fixed !
  ue1mobility.Install (ueNodesArg);


  //Objects to get <<cout
  Ptr<ConstantPositionMobilityModel> mmEnb = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mmUe1 = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mmUe2 = CreateObject<ConstantPositionMobilityModel> ();


  if(enablebuilding)
    {
      BuildingsHelper::Install (eNBNodesArg);
      BuildingsHelper::Install (ueNodesArg);
      //BuildingsHelper::Install (movingueNodes);   DISABLE

      //std::cout << "The amount of buildings in the list of BuildingsList object: " << BuildingList::GetNBuildings() << std::endl;

      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (2));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (2));
      lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (2));

     }
  else
    {
      // lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
      // lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue (3.0));
      // lteHelper->SetPathlossModelAttribute("ReferenceDistance", DoubleValue (10));
      // lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue (2120000000));

      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));
      lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue (1.0));
      lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue (3.0));
      lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue (10.0));
      lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue (100));
      lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue (200));
      lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue (400));
//      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::NakagamiPropagationLossModel"));
//      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::NakagamiPropagationLossModel"));
//      channelHelper.AddPropagationLoss(fastFadingLModel);
//      channelHelper.AddPropagationLoss(slowFadingLModel1);
    }





  Ptr<ThreeLogDistancePropagationLossModel> slowFadingLModel = CreateObject<ThreeLogDistancePropagationLossModel> ();

//Other propagation models

//  Ptr<FriisPropagationLossModel> slowFadingLModel1 = CreateObject<Cost231PropagationLossModel> ();
//
//  SpectrumChannelHelper channelHelper;
//
//  Config::SetDefault ("ns3::NakagamiPropagationLossModel::m0", DoubleValue (1.0));
//  Config::SetDefault ("ns3::NakagamiPropagationLossModel::m1", DoubleValue (1.0));
//  Config::SetDefault ("ns3::NakagamiPropagationLossModel::m2", DoubleValue (1.0));










//  if(enablebuilding)
//     {

//       Ptr<MobilityBuildingInfo> buildingInfoEnb = CreateObject<MobilityBuildingInfo> ();
//       Ptr<MobilityBuildingInfo> buildingInfoUe1 = CreateObject<MobilityBuildingInfo> ();
//       Ptr<MobilityBuildingInfo> buildingInfoUe2 = CreateObject<MobilityBuildingInfo> ();
//       Ptr<MobilityBuildingInfo> buildingInfoMovingUe = CreateObject<MobilityBuildingInfo> ();

//       mmEnb->AggregateObject (buildingInfoEnb); // operation usually done by BuildingsHelper::Install
//       buildingInfoEnb->MakeConsistent (mmEnb);
//       mmUe1->AggregateObject (buildingInfoUe1); // operation usually done by BuildingsHelper::Install
//       buildingInfoUe1->MakeConsistent (mmUe1);
//       mmUe2->AggregateObject (buildingInfoUe2); // operation usually done by BuildingsHelper::Install
//       buildingInfoUe2->MakeConsistent (mmUe2);
//       wayMobility->AggregateObject(buildingInfoMovingUe);
//       buildingInfoMovingUe->MakeConsistent (wayMobility);

//       lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (2));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (2));
//       lteHelper->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (2));

//     }
//   else
//     {
//       // lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::LogDistancePropagationLossModel"));
//       // lteHelper->SetPathlossModelAttribute("Exponent", DoubleValue (3.0));
//       // lteHelper->SetPathlossModelAttribute("ReferenceDistance", DoubleValue (10));
//       // lteHelper->SetPathlossModelAttribute("Frequency", DoubleValue (2120000000));

//       lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::ThreeLogDistancePropagationLossModel"));
//       lteHelper->SetPathlossModelAttribute("Exponent0", DoubleValue (1.0));
//       lteHelper->SetPathlossModelAttribute("Exponent1", DoubleValue (3.0));
//       lteHelper->SetPathlossModelAttribute("Exponent2", DoubleValue (10.0));
//       lteHelper->SetPathlossModelAttribute("Distance0", DoubleValue (100));
//       lteHelper->SetPathlossModelAttribute("Distance1", DoubleValue (200));
//       lteHelper->SetPathlossModelAttribute("Distance2", DoubleValue (400));
// //      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::NakagamiPropagationLossModel"));
// //      lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::NakagamiPropagationLossModel"));
// //      channelHelper.AddPropagationLoss(fastFadingLModel);
// //      channelHelper.AddPropagationLoss(slowFadingLModel1);
//     }












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


  //  movingueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  //  movingueMobility.Install (movingueNodes);
  //  movingueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector ((-6*radius)/7, (6*radius)/7, 0));
  //  movingueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, speed, 0));



  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (eNBNodesArg);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodesArg);



  // Install the IP stack on the UEs
  internet.Install (ueNodesArg);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  for (uint32_t u = 0; u < ueNodesArg.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodesArg.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  lteHelper->Attach(ueLteDevs);


  // Install and start applications on UEs and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // randomize a bit start times to avoid simulation artifacts
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));
  Time startTime = Seconds (startTimeSeconds->GetValue ());
  //
  //  for (uint32_t i = 0; i < numUes; ++i)
  //    {
  //
  //    }

  //  std::cout << ueNodes.GetN() <<" "<< ueIpIfaces.GetN() <<std::endl;

  for (uint32_t u = 0; u < ueNodesArg.GetN(); ++u)
    {
      Ptr<Node> ue = ueNodesArg.Get (u);
      //std::cout <<"UE IP: "<< ueIpIfaces.GetAddress (u) <<std::endl;

      // Set the default gateway for the UE
      //      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      //      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < numBearersPerUe; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          //      UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          //      clientApps.Add (dlClientHelper.Install (remoteHost));
          //      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
          //                                           InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          //      serverApps.Add (dlPacketSinkHelper.Install (ue));
          //
          //      UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          //      clientApps.Add (ulClientHelper.Install (ue));
          //      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
          //                                           InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          //      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          //DL
          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          dlClientHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0 / 1000)));
          dlClientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          //UL
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          ulClientHelper.SetAttribute ("Interval", TimeValue (Seconds (1.0 / 1000)));
          ulClientHelper.SetAttribute ("PacketSize", UintegerValue (1024));
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));


          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), bearer, tft);

          serverApps.Start (startTime);
          clientApps.Start (startTime);
          serverApps.Stop (Seconds(simTime));
          clientApps.Stop (Seconds(simTime));

        } //end bearers


    } //end apps


  lteHelper->AddX2Interface (eNBNodesArg);


  if(enableTraces)
    {
      //#####################################File Inits########################################################
      //#######################################UEs#############################################################
      if(enablersrp)
        {
          //          out_uephyrsrprsrqfile.open (uephyrsrprsrqfile.c_str(), std::ios_base::out | std::ios_base::trunc);
          //          if (!out_uephyrsrprsrqfile.is_open ())
          //            {
          //              std::cout << uephyrsrprsrqfile << " failed to open..." << std::endl;
          //            }
          //          out_uephyrsrprsrqfile << "#";
          //          out_uephyrsrprsrqfile << std::setw(9) << "time";
          //          out_uephyrsrprsrqfile << std::setw(8) << "cellID";
          //          out_uephyrsrprsrqfile << std::setw(8) << "rnti" ;
          //          out_uephyrsrprsrqfile << std::setw(12) << "rsrp" ;
          //          out_uephyrsrprsrqfile << std::setw(12) << "rsrq" << std::endl;

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
          //          out_enbphysinrfile.open (enbphysinrfile.c_str(), std::ios_base::out | std::ios_base::trunc);
          //          if (!out_enbphysinrfile.is_open ())
          //            {
          //              std::cout << enbphysinrfile << " failed to open..." << std::endl;
          //            }
          //          out_enbphysinrfile << "#";
          //          out_enbphysinrfile << std::setw(9) << "time";
          //          out_enbphysinrfile << std::setw(8) << "cellID";
          //          out_enbphysinrfile << std::setw(8) << "rnti" ;
          //          out_enbphysinrfile << std::setw(12) << "sinr" << std::endl;



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
          //          out_uephyrsrpsinrfile.open (uephyrsrpsinrfile.c_str(), std::ios_base::out | std::ios_base::trunc);
          //          if (!out_uephyrsrpsinrfile.is_open ())
          //            {
          //              std::cout << uephyrsrpsinrfile << " failed to open..." << std::endl;
          //            }
          //          out_uephyrsrpsinrfile << "#";
          //          out_uephyrsrpsinrfile << std::setw(9) << "time";
          //          out_uephyrsrpsinrfile << std::setw(8) << "cellID";
          //          out_uephyrsrpsinrfile << std::setw(8) << "rnti" ;
          //          out_uephyrsrpsinrfile << std::setw(12) << "rsrp" ;
          //          out_uephyrsrpsinrfile << std::setw(12) << "sinr" << std::endl;

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
      remHelper->SetAttribute ("OutputFile", StringValue ("test-rem.out"));
      remHelper->SetAttribute ("XMin", DoubleValue (-enbDist));
      remHelper->SetAttribute ("XMax", DoubleValue (enbDist));
      remHelper->SetAttribute ("YMin", DoubleValue (-enbDist));
      remHelper->SetAttribute ("YMax", DoubleValue (enbDist));
      remHelper->SetAttribute ("Z", DoubleValue (1.5));
      remHelper->Install ();
    }

  FlowMonitorHelper flowmonHelper;
  Ptr<ns3::FlowMonitor> monitor;
  if(enableflowstats)           //todo fix
    {

      NodeContainer endpointNodes;
      endpointNodes.Add (ueNodesArg);
      //endpointNodes.Add (movingueNodes); DISABLE
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


//  PrintGnuplottableBuildingListToFile ("buildings.txt");



//  std::string logFile = "ue-mob.log";
//  // open log file for output
//  std::ofstream os;
//  os.open (logFile.c_str ());
//  // Prints position and velocities
//  os << "<TIME>," << "<ID>," << "<IMSI>," <<"<POSITION>," << "<VELOCITY>" << std::endl;
//  Simulator::Schedule(Seconds(0.00), &SaveUePosition, movingueNodes, &os, 0.05);

//  for (int i = 0; i < simTime / timeRes.GetDouble(); i++)
//    {
//      Simulator::Schedule (timeRes * i, &RecordMovement, rxMob);
//    }





  if(enableInstTput)            // todo make flexible
    {

      //std::vector<std::ostringstream> ossGroup(numUEsNEW);                //clean up move to top
      //std::vector<uint32_t> oldByteCounterVector(numUEsNEW, 0);           //todo create byte vector of size n
      //std::vector<double> throughputVector(numUEsNEW, 0);

      for (int i = 0; i < numUEsNEW; i++)
      {


//         std::cout << "Number of numUEsNEW: " << numUEsNEW << std::endl;

//         std::cout << "Counter is the value: " << counterRP << std::endl;

        auto function = [i](Ptr<const Packet> packet, const Address&) {
            byteCounterVector.at(i) += packet->GetSize();
          };


        ossGroup.at(i) << "/NodeList/" << ueNodesArg.Get (counterRP)->GetId () << "/ApplicationList/0/$ns3::PacketSink/Rx";

        Config::ConnectWithoutContext(
               ossGroup.at(i).str(),
            make_nice_callback<void, Ptr<const Packet>, const Address&>(function)
          );

        // std::cout << "Connected to the UE device " << counterRP << std::endl;

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

  Simulator::Stop (Seconds (simTime));
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
          //      std::cout << "   Packet Lost: " << i->second.lostPackets << "\n";

          if (i->second.rxPackets > 0)
            {
              // Measure the duration of the flow from receiver's perspective
              //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
              double rxDuration = simTime - startTime.GetSeconds();

              averageFlowThroughput += (((i->second.rxBytes * 8.0) / rxDuration) / 1000) / 1000;
              averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

              std::cout  << "  Throughput: " << (((i->second.rxBytes * 8.0) / rxDuration) / 1000) / 1000  << " Mbps\n";
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

  Simulator::Destroy ();
  return 0;
}
