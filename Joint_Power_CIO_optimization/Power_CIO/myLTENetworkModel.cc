/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 *
 * Author: Kareem M. Attiah <kareemattiah@gmail.com>
 * Acknowledgement: Code is based on implmentation of dual-stripe model of  Nicola Baldo <nbaldo@cttc.es>
 */

#include <ns3/core-module.h>
#include "ns3/opengym-module.h"
#include <ns3/network-module.h>
#include <ns3/mobility-module.h>
#include <ns3/internet-module.h>
#include <ns3/lte-module.h>
#include <ns3/config-store-module.h>
//#include <ns3/buildings-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/applications-module.h>
#include <ns3/log.h>
#include <iomanip>
#include <ios>
#include <string>
#include <vector>
#include <ns3/cell-individual-offset.h>
#include "mygym.h"
#include "ns3/netanim-module.h"



// The topology of this simulation program is inspired from
// 3GPP R4-092042, Section 4.2.1 Dual Stripe Model
// note that the term "apartments" used in that document matches with
// the term "room" used in the BuildingsMobilityModel

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MyLTENetwork");
uint32_t RunNum;
uint32_t m_mobility=1;
std::vector<uint32_t> m_uesnumber;
std::string m_traceFile; ///< trace file

void reportmobility (Ptr<MobilityModel> mobility)
{

	NS_LOG_UNCOND("******************* reportmobility *******************  time= " << Simulator::Now ().GetSeconds () << " sec");
	  Simulator::Schedule (Seconds (1), &reportmobility, mobility);
	  Vector pos = mobility->GetPosition ();
	  Vector vel = mobility->GetVelocity ();
	 NS_LOG_UNCOND( Simulator::Now ().GetMilliSeconds()/1000 << " Sec, POS: x=" << pos.x << ", y=" << pos.y
	            << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
	            << ", z=" << vel.z );

}


/*void SetInitialUeNum (Ptr<LteEnbRrc> enbRrc, uint16_t cellID,Ptr<MyGymEnv> gymEnv)
{
	NS_LOG_UNCOND("******************* SetInitialUeNum *******************  time= " << Simulator::Now ().GetSeconds () << " sec");
	m_uesnumber.at(cellID-1)=enbRrc->GetUeCount(cellID);
	NS_LOG_UNCOND(" cell: "<<cellID<<" Initial users count: " << m_uesnumber.at(cellID-1));
	gymEnv->SetInitNumofUEs(gymEnv,m_uesnumber);

}*/

void
NotifyHandoverStartEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti,
                        uint16_t targetCellId)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": start handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << " to CellId " << targetCellId
            << std::endl;
}


void
NotifyHandoverEndOkEnb (std::string context,
                        uint64_t imsi,
                        uint16_t cellid,
                        uint16_t rnti)
{
  std::cout << context
            << " eNB CellId " << cellid
            << ": completed handover of UE with IMSI " << imsi
            << " RNTI " << rnti
            << std::endl;
}

//static void
//CourseChange (std::string foo, Ptr<const MobilityModel> mobility)
//{
//  NS_LOG_UNCOND(" &&&&&&&&&&&&&& CourseChange &&&&&&&&&&&&&&&&&&&&&&&&&&&& ");
//  Vector pos = mobility->GetPosition ();
//  Vector vel = mobility->GetVelocity ();
//  std::cout << Simulator::Now () << ", model=" << mobility << ", POS: x=" << pos.x << ", y=" << pos.y
//            << ", z=" << pos.z << "; VEL:" << vel.x << ", y=" << vel.y
//            << ", z=" << vel.z << std::endl;
//}
/**
 * Class that takes care of installing blocks of the
 * buildings in a given area. Buildings are installed in pairs
 * as in dual stripe scenario.
 */
std::vector<double> convertStringtoDouble(std::string cioList, uint16_t rep)
{
  std::vector<double> v;
  double tempVal;
  std::string delimiter = "_";
  size_t pos = cioList.find(delimiter);
  std::string token;
  while (pos != std::string::npos) {
    token = cioList.substr(0, pos);
    tempVal = std::stod(token);

    // V 1.0: Every eNB has 3 sectors
    // V 2.3: Sectors removed, same code works when rep = 1
    for(uint16_t i = 0; i < rep; i++)
        v.push_back(tempVal);

    cioList.erase(0, pos + delimiter.length());
    pos = cioList.find(delimiter);
  }

  tempVal = std::stod(cioList);
  // Every eNB has 3 sectors
  for(uint16_t i = 0; i < rep; i++)
      v.push_back(tempVal);
  //v.push_back(tempVal);
  //v.push_back(tempVal);
  return v;
}


static ns3::GlobalValue g_nMacroEnbSites ("nMacroEnbSites",
                                          "How many macro sites there are",
                                          ns3::UintegerValue (3),
                                          ns3::MakeUintegerChecker<uint32_t> ());
static ns3::GlobalValue g_interSiteDistance ("interSiteDistance",
                                             "min distance between two nearby macro cell sites",
                                             ns3::DoubleValue (500),
                                             ns3::MakeDoubleChecker<double> ());
//static ns3::GlobalValue g_macroUeDensity ("macroUeDensity",
//                                          "For each cell, how many macrocell UEs there are per square meter",
//                                          ns3::StringValue ("0.00002_0.00002_0.00002"),
//                                          ns3::MakeStringChecker());
static ns3::GlobalValue g_macroEnbTxPowerDbm ("macroEnbTxPowerDbm",
                                              "TX power [dBm] used by macro eNBs",
                                              ns3::DoubleValue (32.0),
                                              ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_macroEnbDlEarfcn ("macroEnbDlEarfcn",
                                            "DL EARFCN used by macro eNBs",
                                            ns3::UintegerValue (100),
                                            ns3::MakeUintegerChecker<uint16_t> ());
static ns3::GlobalValue g_macroEnbBandwidth ("macroEnbBandwidth",
                                             "bandwidth [num RBs] used by macro eNBs",
                                             ns3::UintegerValue (50),
                                             ns3::MakeUintegerChecker<uint16_t> ());
static ns3::GlobalValue g_fadingTrace ("fadingTrace",
                                       "The path of the fading trace (by default no fading trace "
                                       "is loaded, i.e., fading is not considered)",
                                       ns3::StringValue (""),
                                       ns3::MakeStringChecker ());
static ns3::GlobalValue g_hystersis ("hysteresisCoefficient",
                                     "The value of hysteresis coefficient",
                                     ns3::DoubleValue(3.0),
                                     ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_timeToTrigger ("TTT",
                                         "The value of time to tigger coefficient in milliseconds",
                                         ns3::DoubleValue(256),
                                         ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_cioList ("cioList",
                                   "The CIO values arranged in a string.",
                                   ns3::StringValue("0 0 0"),
                                   ns3::MakeStringChecker());
static ns3::GlobalValue g_simTime ("simTime",
                                   "Total duration of the simulation [s]",
                                   ns3::DoubleValue (1000),
                                   ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_envStepTime ("envStepTime",
                                      "Environment Step time [s]",
                                      ns3::DoubleValue (0.2),
                                      ns3::MakeDoubleChecker<double> ());
static ns3::GlobalValue g_openGymPort ("openGymPort",
                                      "Open Gym Port Number",
                                      ns3::UintegerValue(2222),
                                      ns3::MakeUintegerChecker<uint16_t> ());
static ns3::GlobalValue g_outputTraceFiles ("outputTraceFiles",
                                                 "If true, trace files will be output in ns3 working directory. "
                                                 "If false, no files will be generated.",
                                                 ns3::BooleanValue(true),
                                                 ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_epcDl ("epcDl",
                                 "if true, will activate data flows in the downlink when EPC is being used. "
                                 "If false, downlink flows won't be activated. "
                                 "If EPC is not used, this parameter will be ignored.",
                                 ns3::BooleanValue (true),
                                 ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_epcUl ("epcUl",
                                 "if true, will activate data flows in the uplink when EPC is being used. "
                                 "If false, uplink flows won't be activated. "
                                 "If EPC is not used, this parameter will be ignored.",
                                 ns3::BooleanValue (true),
                                 ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_useUdp ("useUdp",
                                  "if true, the UdpClient application will be used. "
                                  "Otherwise, the BulkSend application will be used over a TCP connection. "
                                  "If EPC is not used, this parameter will be ignored.",
                                  ns3::BooleanValue (true),
                                  ns3::MakeBooleanChecker ());
static ns3::GlobalValue g_numBearersPerUe ("numBearersPerUe",
                                           "How many bearers per UE there are in the simulation",
                                           ns3::UintegerValue (1),
                                           ns3::MakeUintegerChecker<uint16_t> ());
static ns3::GlobalValue g_srsPeriodicity ("srsPeriodicity",
                                          "SRS Periodicity (has to be at least "
                                          "greater than the number of UEs per eNB)",
                                          ns3::UintegerValue (80),
                                          ns3::MakeUintegerChecker<uint16_t> ());
//static ns3::GlobalValue g_outdoorUeMinSpeed ("outdoorUeMinSpeed",
//                                             "Minimum speed value of macro UE with random waypoint model [m/s].",
//                                             ns3::DoubleValue (0.0),
//                                             ns3::MakeDoubleChecker<double> ());
//static ns3::GlobalValue g_outdoorUeMaxSpeed ("outdoorUeMaxSpeed",
//                                             "Maximum speed value of macro UE with random waypoint model [m/s].",
//                                             ns3::DoubleValue (0.0),
//                                             ns3::MakeDoubleChecker<double> ());
////static ns3::GlobalValue g_edgeCellRatio ("edgeCellRatio",
//                                        "Ratio of number of users on cell edge to total number of users in the cell.",
//                                        ns3::DoubleValue (0.5),
//                                        ns3::MakeDoubleChecker<double> ());
//static ns3::GlobalValue g_deltaEdge ("deltaEdge",
//                                    "The width of the cell edge measured as a ratio of interSiteDistance",
//                                    ns3::DoubleValue (0.02),
//                                    ns3::MakeDoubleChecker<double> ());

int main (int argc, char *argv[])
 {
      // change some default attributes so that they are reasonable for
      // this scenario, but do this before processing command line
      // arguments, so that the user is allowed to override these settings
      Config::SetDefault ("ns3::LteHelper::UseIdealRrc", BooleanValue (false));
      Config::SetDefault ("ns3::LteHelper::Scheduler", StringValue ("ns3::RrFfMacScheduler"));
      Config::SetDefault ("ns3::UdpClient::Interval", TimeValue (MilliSeconds (1)));
      Config::SetDefault ("ns3::UdpClient::PacketSize", UintegerValue(12)); // bytes
      Config::SetDefault ("ns3::UdpClient::MaxPackets", UintegerValue (1000000));
      Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (10 * 1024));


      Config::SetDefault ("ns3::ConfigStore::Filename", StringValue("Real_model-attributes.txt"));
      Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue("RawText"));
      Config::SetDefault ("ns3::ConfigStore::Mode", StringValue("Load"));

      CommandLine cmd;
      cmd.AddValue ("RunNum", "1...10", RunNum);

      cmd.Parse (argc, argv);

      ConfigStore inputConfig;
      inputConfig.ConfigureDefaults ();

      cmd.Parse (argc, argv);

      NS_LOG_UNCOND ("RunNum = " << RunNum);




      if (RunNum < 1) RunNum = 1;
        SeedManager::SetSeed (1);
        SeedManager::SetRun (RunNum);
      // the scenario parameters get their values from the global attributes defined above
      UintegerValue uintegerValue;
      IntegerValue integerValue;
      DoubleValue doubleValue;
      BooleanValue booleanValue;
      StringValue stringValue;

      //GlobalValue::GetValueByName ("debugScenario", booleanValue);
      //bool debugScenario = booleanValue.Get ();
//      GlobalValue::GetValueByName ("nBlocks", uintegerValue);
//      uint32_t nBlocks = uintegerValue.Get ();
//       GlobalValue::GetValueByName ("nApartmentsX", uintegerValue);
//      uint32_t nApartmentsX = uintegerValue.Get ();
//      GlobalValue::GetValueByName ("nFloors", uintegerValue);
//      uint32_t nFloors = uintegerValue.Get ();
      GlobalValue::GetValueByName ("nMacroEnbSites", uintegerValue);
      uint32_t nMacroEnbSites = uintegerValue.Get ();
//      GlobalValue::GetValueByName ("nMacroEnbSitesX", uintegerValue);
//      uint32_t nMacroEnbSitesX = uintegerValue.Get ();
      GlobalValue::GetValueByName ("interSiteDistance", doubleValue);
      double interSiteDistance = doubleValue.Get ();
//      GlobalValue::GetValueByName ("areaMarginFactor", doubleValue);
//      double areaMarginFactor = doubleValue.Get ();
//      GlobalValue::GetValueByName ("macroUeDensity", stringValue);
//      std::string macroUeDensity = stringValue.Get ();
//      GlobalValue::GetValueByName ("homeUesDeploymentRatio", doubleValue);
//      double homeUesDeploymentRatio = doubleValue.Get ();
      GlobalValue::GetValueByName ("macroEnbTxPowerDbm", doubleValue);
      double macroEnbTxPowerDbm = doubleValue.Get ();
      GlobalValue::GetValueByName ("macroEnbDlEarfcn", uintegerValue);
      uint32_t macroEnbDlEarfcn = uintegerValue.Get ();
      GlobalValue::GetValueByName ("macroEnbBandwidth", uintegerValue);
      uint16_t macroEnbBandwidth = uintegerValue.Get ();
      GlobalValue::GetValueByName ("fadingTrace", stringValue);
      std::string fadingTrace = stringValue.Get ();
      GlobalValue::GetValueByName("hysteresisCoefficient", doubleValue);
      double hysteresisCoefficient = doubleValue.Get ();
      GlobalValue::GetValueByName ("TTT", doubleValue);
      double timeToTrigger = doubleValue.Get ();
      GlobalValue::GetValueByName ("simTime", doubleValue);
      double simTime = doubleValue.Get ();
      GlobalValue::GetValueByName ("envStepTime", doubleValue);
      double envStepTime = doubleValue.Get ();
      GlobalValue::GetValueByName ("openGymPort", uintegerValue);
      uint16_t openGymPort = uintegerValue.Get ();
      GlobalValue::GetValueByName ("outputTraceFiles", booleanValue);
      bool outputTraceFiles = booleanValue.Get ();
      GlobalValue::GetValueByName ("cioList", stringValue);
      std::string cioList = stringValue.Get ();
      GlobalValue::GetValueByName ("epcDl", booleanValue);
      bool epcDl = booleanValue.Get ();
      GlobalValue::GetValueByName ("epcUl", booleanValue);
      bool epcUl = booleanValue.Get ();
      GlobalValue::GetValueByName ("useUdp", booleanValue);
      bool useUdp = booleanValue.Get ();
      GlobalValue::GetValueByName ("numBearersPerUe", uintegerValue);
      uint16_t numBearersPerUe = uintegerValue.Get ();
      GlobalValue::GetValueByName ("srsPeriodicity", uintegerValue);
      uint16_t srsPeriodicity = uintegerValue.Get ();
//      GlobalValue::GetValueByName ("outdoorUeMinSpeed", doubleValue);
//      uint16_t outdoorUeMinSpeed = doubleValue.Get ();
//      GlobalValue::GetValueByName ("outdoorUeMaxSpeed", doubleValue);
//      uint16_t outdoorUeMaxSpeed = doubleValue.Get ();
//      GlobalValue::GetValueByName ("edgeCellRatio", doubleValue);
//      double edgeCellRatio = doubleValue.Get ();
//      GlobalValue::GetValueByName ("deltaEdge", doubleValue);
//      double deltaEdge = doubleValue.Get ();

      Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (srsPeriodicity));
      //std::string animFile = "LTENetwork-animation.xml" ;  // Name of file for animation output
//      NS_LOG_UNCOND (" ====== Creating Grid: ======");
//      uint32_t nMacroEnbSitesY;
//      Box macroUeBox;
//      double ueZ = 1.5;
//      if (nMacroEnbSites > 0)
//        {
//          uint32_t currentSite = nMacroEnbSites -1;
//          uint32_t biRowIndex = (currentSite / (nMacroEnbSitesX + nMacroEnbSitesX + 1));
//          uint32_t biRowRemainder = currentSite % (nMacroEnbSitesX + nMacroEnbSitesX + 1);
//          uint32_t rowIndex = biRowIndex*2 + 1;
//          if (biRowRemainder >= nMacroEnbSitesX)
//            {
//              ++rowIndex;
//            }
//          nMacroEnbSitesY = rowIndex;
//          NS_LOG_UNCOND ("nEnbSites = " << nMacroEnbSites);
//          NS_LOG_UNCOND ("nMacroEnbSitesX = " << nMacroEnbSitesX);
//          NS_LOG_UNCOND ("nMacroEnbSitesY = " << nMacroEnbSitesY);
//          NS_LOG_UNCOND ("InterSiteDistance = " << interSiteDistance);
//          if(nMacroEnbSitesY == 1)
//          {
//              macroUeBox = Box (-areaMarginFactor*interSiteDistance,
//                                (nMacroEnbSitesX - 1 + areaMarginFactor)*interSiteDistance,
//                                -areaMarginFactor*interSiteDistance, areaMarginFactor*interSiteDistance,
//                                ueZ, ueZ);
//          }
//          else
//          {
//              macroUeBox = Box (-areaMarginFactor*interSiteDistance,
//                                (nMacroEnbSitesX + areaMarginFactor)*interSiteDistance,
//                                -areaMarginFactor*interSiteDistance,
//                                (nMacroEnbSitesY -1)*interSiteDistance*sqrt (0.75) + areaMarginFactor*interSiteDistance,
//                                ueZ, ueZ);
//          }
//
//          NS_LOG_UNCOND ("Grid bounds = " << macroUeBox);
//
//        }
//        else
//        {
//          NS_LOG_UNCOND ("nEnbSites = 0, creating default grid");
//          // still need the box to place femtocell blocks
//          macroUeBox = Box (0, 150, 0, 150, ueZ, ueZ);
//          NS_LOG_UNCOND ("Grid bounds = " << macroUeBox);
//        }
//
//
//
//        /*if(debugScenario)
//        {
//            std::cout << "Debug enabled" << std::endl;
//            std::cout << "Scenario bounds " << macroUeBox << std::endl;
//
//            for(uint32_t it = 0; it != BuildingList::GetNBuildings(); ++it)
//            {
//                std::cout << "Building #" << it << ", Dim =  " << BuildingList::GetBuilding(it) -> GetBoundaries() << std::endl;
//            }
//        }*/
      uint32_t nHomeUes = 0;
      uint16_t nMacroUes=40;//25
      openGymPort=2221;

      /*Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
      Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv> (envStepTime, nMacroEnbSites
                                                      , nHomeUes+nMacroUes, macroEnbBandwidth);

      myGymEnv->SetOpenGymInterface(openGymInterface);*/
        NS_LOG_UNCOND(" ====== Creating UEs ====== ");
        NS_LOG_UNCOND ("nHomeUes = " << nHomeUes);

        // macroUEs deployment within each cell
        //double macroUeAreaSize = (macroUeBox.xMax - macroUeBox.xMin) * (macroUeBox.yMax - macroUeBox.yMin);
        //uint32_t nMacroUes = round (macroUeAreaSize * macroUeDensity);


        NodeContainer homeUes;
        homeUes.Create (nHomeUes);

        // Will change later
        Ptr <LteHelper> lteHelper = CreateObject<LteHelper> ();
        lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::Cost231PropagationLossModel"));

        //lteHelper->SetPathlossModelAttribute ("InternalWallLoss", DoubleVal)
        // use always LOS model
        //lteHelper->SetPathlossModelAttribute ("Los2NlosThr", DoubleValue (10));
        lteHelper->SetSpectrumChannelType ("ns3::MultiModelSpectrumChannel");


        // Set handover paratmeters
        NS_LOG_UNCOND( " ====== Setting handover parameters ====== ");
        NS_LOG_UNCOND( "Hysteresis = " << hysteresisCoefficient << "dB");
        NS_LOG_UNCOND( "TimeToTrigger = " << timeToTrigger << "ms");
        lteHelper->SetHandoverAlgorithmType ("ns3::A3RsrpHandoverAlgorithm");
        lteHelper->SetHandoverAlgorithmAttribute ("Hysteresis", DoubleValue(hysteresisCoefficient));
        lteHelper->SetHandoverAlgorithmAttribute ("TimeToTrigger", TimeValue (MilliSeconds (timeToTrigger)));

        std::vector<double> cioListDouble = convertStringtoDouble(cioList, 1);
        /*NS_LOG_UNCOND(" === Printing cioList === ");
        int cellId = 0;
        for(std::vector<double>::iterator it = cioListDouble.begin(); it != cioListDouble.end(); ++it)
        {
          NS_LOG_UNCOND( "cell ID = " << macroEnbs.Get(cellId++)->GetId() << ", CIO = " << *it);
        }*/
        CellIndividualOffset::setOffsetList(cioListDouble);

        NS_LOG_UNCOND(" ====== Setting up fading ====== ");
        if (!fadingTrace.empty ())
          {
            lteHelper->SetAttribute ("FadingModel", StringValue ("ns3::TraceFadingLossModel"));
            lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue (fadingTrace));
            NS_LOG_UNCOND("Filename " << fadingTrace);
          } else
          {
            NS_LOG_UNCOND("No fading specified");
          }


        Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
        lteHelper->SetEpcHelper (epcHelper);

        // Set other scenrio paratmeters
        Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (macroEnbTxPowerDbm));
        lteHelper->SetEnbAntennaModelType ("ns3::IsotropicAntennaModel");
        lteHelper->SetEnbDeviceAttribute ("DlEarfcn", UintegerValue (macroEnbDlEarfcn));
        lteHelper->SetEnbDeviceAttribute ("UlEarfcn", UintegerValue (macroEnbDlEarfcn + 18000));
        lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (macroEnbBandwidth));
        lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (macroEnbBandwidth));

        NS_LOG_UNCOND(" ====== Deploying eNBs ====== ");
               NodeContainer macroEnbs;
               macroEnbs.Create (nMacroEnbSites);
               Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
               if (m_mobility == 1)
               {
            	   enbPositionAlloc->Add (Vector (12291.12,4332.80, 30));//L6183
            	   //enbPositionAlloc->Add (Vector (12319.28,4663.71, 30));//L6198
            	   //enbPositionAlloc->Add (Vector (13614.55,4649.63, 30));//L6530
            	   enbPositionAlloc->Add (Vector (13437.43,4162.39, 30));//L6541
            	   enbPositionAlloc->Add (Vector (13431.70,4656.67, 30));//LK2313
            	   //enbPositionAlloc->Add (Vector (11249.10,4044.13, 30));//LK2309
            	   enbPositionAlloc->Add (Vector (13093.75,3720.26, 30));//LK2331
            	   enbPositionAlloc->Add (Vector (14002.00,4248.31, 30));//LK2526
            	   //enbPositionAlloc->Add (Vector (12192.55,3579.44, 30));//LK2547
            	   enbPositionAlloc->Add (Vector (12938.86,4515.85, 30));//LK6606


               }
               else
               {
                 for (uint16_t i = 0; i < nMacroEnbSites; i++)
				 {
				   Vector enbPosition (interSiteDistance * i, 0, 30);
				   enbPositionAlloc->Add (enbPosition);
				 }
               }
               MobilityHelper mobility;
               mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
               mobility.SetPositionAllocator (enbPositionAlloc);
               mobility.Install (macroEnbs);
               NetDeviceContainer macroEnbDevs = lteHelper->InstallEnbDevice (macroEnbs);

               std::vector<Vector> eNBsLocation;
               Vector tempLocation;
               for(uint32_t it = 0; it != macroEnbs.GetN(); ++it)
               {
                   Ptr<Node> node = macroEnbs.Get(it);
                   Ptr<NetDevice> netDevice = macroEnbDevs.Get(it);
                   Ptr<LteEnbNetDevice> enbNetDevice = netDevice->GetObject<LteEnbNetDevice> ();
                   Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel> ();
                   tempLocation = mobilityModel->GetPosition();
                   eNBsLocation.push_back(tempLocation);
                   NS_LOG_UNCOND("Cell #" << enbNetDevice->GetCellId() << " Pos =  " << tempLocation << ", CIO = " << cioListDouble[it]);
               }

               // this enables handover for macro eNBs
               lteHelper->AddX2Interface (macroEnbs);
               // HomeUes randomly indoor
               Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface> (openGymPort);
               Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv> (envStepTime, nMacroEnbSites
                                                               , nHomeUes+nMacroUes, macroEnbBandwidth, macroEnbDevs);

               myGymEnv->SetOpenGymInterface(openGymInterface);

       //
       //        Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
       //          Vector uePosition (distance, yForUe, 0);
       //          uePositionAlloc->Add (uePosition);
       //          MobilityHelper ueMobility;
       //          //ueMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
       //          ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
       //          ueMobility.SetPositionAllocator (uePositionAlloc);
       //          ueMobility.Install (ueNodes);
       //          //ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, yForUe, 0));
       //          //ueNodes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));


                 Ptr<ListPositionAllocator> homeUesPositionAlloc = CreateObject<ListPositionAllocator> ();

                 for (uint16_t i = 0; i < nHomeUes/2; i++)
                 {
                      Vector enbPosition (30 * i, 50, 2);
                      homeUesPositionAlloc->Add (enbPosition);
                 }

                 for (uint16_t i = 0; i < nHomeUes/2; i++)
                 {
                      Vector enbPosition (30 * i, 70, 2);
                      homeUesPositionAlloc->Add (enbPosition);
                 }
       //                      Vector huePosition (0, 100, 1.8);
       //                      homeUesPositionAlloc->Add (huePosition);
       //
       //                      Vector huePosition1 (50, 100, 1.8);
       //                      homeUesPositionAlloc->Add (huePosition1);
                         MobilityHelper homeUesMobility;
                         homeUesMobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
                         homeUesMobility.SetPositionAllocator (homeUesPositionAlloc);
                         //homeUes.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (100, 0, 0));







       //        Ptr<ListPositionAllocator> homeUesPositionAlloc = CreateObject<ListPositionAllocator> ();
       //        for (uint16_t i = 0; i < nHomeUes; i++)
       //          {
       //            Vector enbPosition (interSiteDistance * i/2, 100, 30);
       //            homeUesPositionAlloc->Add (enbPosition);
       //          }
       //        MobilityHelper homeUesMobility;
       //        homeUesMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
       //        homeUesMobility.SetPositionAllocator (homeUesPositionAlloc);
       //
       //


               homeUesMobility.Install (homeUes);

               for (uint16_t i = 0; i < nHomeUes; i++)
               {

                    Ptr<ConstantVelocityMobilityModel> mob = homeUes.Get(i)->GetObject<ConstantVelocityMobilityModel>();
                    mob->SetVelocity(Vector(20, 0, 0));   	//go to the right

               }

       //        Ptr<ConstantVelocityMobilityModel> mob = homeUes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
       //        	mob->SetVelocity(Vector(30, 0, 0));   	//go to the right


               NetDeviceContainer homeUeDevs = lteHelper->InstallUeDevice (homeUes);

               // macro Ues
               NS_LOG_UNCOND(" ====== Deploying Macro UEs ====== ");
               // Get eNBLocations
               NodeContainer macroUes;
               macroUes.Create (nMacroUes);

               // Install Mobility Model in UE
       	if (m_mobility == 1)
       		{
       	      m_traceFile = "scratch/satt_6c_r_mob_5000_ped_veh_r2.tcl";//"src/wave/examples/low99-ct-unterstrass-1day.filt.7.adj.mob";

       		  // Create Ns2MobilityHelper with the specified trace log file as parameter
       		  Ns2MobilityHelper ns2 = Ns2MobilityHelper (m_traceFile);
       		  ns2.Install (macroUes.Begin(), macroUes.End()); // configure movements for each node, while reading trace file

       		}
       	  else if (m_mobility == 2)
       		{
                Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
                Vector uePosition (500, 100, 0);
                uePositionAlloc->Add (uePosition);


                	   MobilityHelper ueMobility;

                	   int64_t m_streamIndex=0;
                      ObjectFactory pos;
                      pos.SetTypeId ("ns3::RandomBoxPositionAllocator");
                      pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=-500.0|Max=1500.0]"));
                      pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=-300.0|Max=300.0]"));
                      // we need antenna height uniform [1.0 .. 2.0] for loss model
                      pos.Set ("Z", StringValue ("ns3::UniformRandomVariable[Min=1.5|Max=2.0]"));

                      Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
                      m_streamIndex += taPositionAlloc->AssignStreams (m_streamIndex);

                      ueMobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                                                   "Mode", StringValue ("Time"),
                                                   "Time", StringValue ("0.1s"),
                                                   "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"),
                                                   "Bounds", StringValue ("-500|1500|-500|500"));
                      ueMobility.SetPositionAllocator (taPositionAlloc);
                      ueMobility.Install (macroUes);

                      m_streamIndex += ueMobility.AssignStreams (macroUes, m_streamIndex);
       		}
       			NetDeviceContainer macroUeDevs = lteHelper->InstallUeDevice (macroUes);

                      for(uint32_t it = 0; it != macroUes.GetN(); ++it)
       			  {
       				  Ptr<Node> node = macroUes.Get(it);
       				  Ptr<NetDevice> netDevice = macroUeDevs.Get(it);
       				  Ptr<LteUeNetDevice> uebNetDevice = netDevice->GetObject<LteUeNetDevice> ();
       				  Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel> ();
       				  tempLocation = mobilityModel->GetPosition();
       				  eNBsLocation.push_back(tempLocation);
       				  NS_LOG_UNCOND(" "<<it<<" Ue #" << uebNetDevice->GetImsi() << " Pos =  " << tempLocation << ", CIO = " << cioListDouble[it]);
       			  }


               NS_LOG_UNCOND(" ====== Deployment Summary ======");
               for(uint16_t i = 0; i < macroUes.GetN(); ++i)
               {
                 Ptr<Node> node = macroUes.Get(i);
                 Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel> ();

                 NS_LOG_UNCOND("Node# " << node->GetId() << " is initially at " << mobilityModel->GetPosition());
               }
       ////////////////////////////////////////////////////////////////////////////////////////

       /////////////////////////////////////////////////////////////////////////////////////////

        NS_LOG_UNCOND ("setting up internet and remote host");
        // Create a single RemoteHost
        Ptr<Node> remoteHost;
        NodeContainer remoteHostContainer;
        remoteHostContainer.Create (1);
        remoteHost = remoteHostContainer.Get (0);
        InternetStackHelper internet;
        internet.Install (remoteHostContainer);
        Ipv4Address remoteHostAddr;


        NodeContainer ues;
        Ipv4InterfaceContainer ueIpIfaces;
        NetDeviceContainer ueDevs;

        // Create the Internet
        PointToPointHelper p2ph;
        p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Mb/s")));
        p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
        p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
        Ptr<Node> pgw = epcHelper->GetPgwNode ();
        NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
        Ipv4AddressHelper ipv4h;
        ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
        Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
        // in this container, interface 0 is the pgw, 1 is the remoteHost
        remoteHostAddr = internetIpIfaces.GetAddress (1);

        Ipv4StaticRoutingHelper ipv4RoutingHelper;
        Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
        remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

        // for internetworking purposes, consider together home UEs and macro UEs
        ues.Add (homeUes);
        ues.Add (macroUes);
        ueDevs.Add (homeUeDevs);
        ueDevs.Add (macroUeDevs);

        // Install the IP stack on the UEs
        internet.Install (ues);
        ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

        // attachment (needs to be done after IP stack configuration)
        // using initial cell selection
        lteHelper->Attach (macroUeDevs);
        lteHelper->Attach (homeUeDevs);

        NS_LOG_UNCOND ("setting up applications");

        // Install and start applications on UEs and remote host
        uint16_t dlPort = 10000;
        uint16_t ulPort = 20000;

        // randomize a bit start times to avoid simulation artifacts
        // (e.g., buffer overflows due to packet transmissions happening
        // exactly at the same time)
        Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
        if (useUdp)
          {
            startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
            startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));
          }
        else
          {
            // TCP needs to be started late enough so that all UEs are connected
            // otherwise TCP SYN packets will get lost
            startTimeSeconds->SetAttribute ("Min", DoubleValue (0.100));
            startTimeSeconds->SetAttribute ("Max", DoubleValue (0.110));
          }

        for (uint32_t u = 0; u < ues.GetN (); ++u)
          {
            Ptr<Node> ue = ues.Get (u);
            // Set the default gateway for the UE
            Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
            ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

            for (uint32_t b = 0; b < numBearersPerUe; ++b)
              {
                ++dlPort;
                ++ulPort;

                ApplicationContainer clientApps;
                ApplicationContainer serverApps;

                if (useUdp)
                  {
                    if (epcDl)
                      {
                        NS_LOG_UNCOND ("installing UDP DL app for UE " << u);
                        UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
                        clientApps.Add (dlClientHelper.Install (remoteHost));
                        PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                                                 InetSocketAddress (Ipv4Address::GetAny (), dlPort));
                        serverApps.Add (dlPacketSinkHelper.Install (ue));
                      }
                    if (epcUl)
                      {
                        NS_LOG_UNCOND ("installing UDP UL app for UE " << u);
                        UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
                        clientApps.Add (ulClientHelper.Install (ue));
                        PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                                             InetSocketAddress (Ipv4Address::GetAny (), ulPort));
                          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
                      }
                  }
                else // use TCP
                  {
                    if (epcDl)
                      {
                        NS_LOG_UNCOND ("installing TCP DL app for UE " << u);
                        BulkSendHelper dlClientHelper ("ns3::TcpSocketFactory",
                                                       InetSocketAddress (ueIpIfaces.GetAddress (u), dlPort));
                        dlClientHelper.SetAttribute ("MaxBytes", UintegerValue (0));
                        clientApps.Add (dlClientHelper.Install (remoteHost));
                        PacketSinkHelper dlPacketSinkHelper ("ns3::TcpSocketFactory",
                                                             InetSocketAddress (Ipv4Address::GetAny (), dlPort));
                        serverApps.Add (dlPacketSinkHelper.Install (ue));
                      }
                    if (epcUl)
                      {
                        NS_LOG_UNCOND ("installing TCP UL app for UE " << u);
                        BulkSendHelper ulClientHelper ("ns3::TcpSocketFactory",
                                                       InetSocketAddress (remoteHostAddr, ulPort));
                        ulClientHelper.SetAttribute ("MaxBytes", UintegerValue (0));
                        clientApps.Add (ulClientHelper.Install (ue));
                        PacketSinkHelper ulPacketSinkHelper ("ns3::TcpSocketFactory",
                                                             InetSocketAddress (Ipv4Address::GetAny (), ulPort));
                        serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
                      }
                  } // end if (useUdp)

                Ptr<EpcTft> tft = Create<EpcTft> ();
                if (epcDl)
                  {
                    EpcTft::PacketFilter dlpf;
                    dlpf.localPortStart = dlPort;
                    dlpf.localPortEnd = dlPort;
                    tft->Add (dlpf);
                  }
                if (epcUl)
                  {
                    EpcTft::PacketFilter ulpf;
                    ulpf.remotePortStart = ulPort;
                    ulpf.remotePortEnd = ulPort;
                    tft->Add (ulpf);
                  }

                if (epcDl || epcUl)
                  {
                    EpsBearer bearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
                    lteHelper->ActivateDedicatedEpsBearer (ueDevs.Get (u), bearer, tft);
                  }
                Time startTime = Seconds (startTimeSeconds->GetValue ());
                serverApps.Start (startTime);
                clientApps.Start (startTime);

              } // end for b
          }


        //BuildingsHelper::MakeMobilityModelConsistent ();

        // Configure OpenGym
        // The states will be sent to the python agent every envStepTime
        //bool useOpenGym = false;
        m_uesnumber.assign(macroEnbs.GetN(), 0);
        for(uint32_t it = 0; it != macroEnbs.GetN(); ++it)
        {
              //Ptr<Node> node = macroEnbs.GetNode(it);
              Ptr<NetDevice> netDevice = macroEnbDevs.Get(it);
              Ptr<LteEnbNetDevice> enbNetDevice = netDevice->GetObject<LteEnbNetDevice> ();
              Ptr<LteEnbPhy> enbPhy = enbNetDevice->GetPhy();
              enbPhy->TraceConnectWithoutContext ("DlPhyTransmission", MakeBoundCallback (&MyGymEnv::GetPhyStats, myGymEnv));
			  enbPhy -> TraceConnectWithoutContext("ReportCqiValues", MakeBoundCallback( & MyGymEnv::GetCQI, myGymEnv));

              Ptr<LteEnbRrc> enbRrc = enbNetDevice->GetRrc();
              //enbRrc->TraceConnectWithoutContext ("NumberofUesChanged",MakeBoundCallback (&MyGymEnv::GetNumofUEs, myGymEnv));
              //Simulator::Schedule (Seconds (0.4), &SetInitialUeNum, enbRrc, enbNetDevice->GetCellId(),myGymEnv);
              //Simulator::Schedule (Seconds (1.00001), &ReportNumUE, enbRrc, enbNetDevice->GetCellId());

        }


        // Enable output traces
        if(outputTraceFiles)
        {
            lteHelper->EnablePhyTraces ();
            //lteHelper->EnableMacTraces ();
            //lteHelper->EnableRlcTraces ();
            //lteHelper->EnablePdcpTraces ();
        }

        // Uncomment the following six lines to create config-file
        //Config::SetDefault ("ns3::ConfigStore::Filename", StringValue("model-attributes-output.txt"));
        //Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue("RawText"));
        //Config::SetDefault ("ns3::ConfigStore::Mode", StringValue("Save"));

        //ConfigStore outputConfig;
        //outputConfig.ConfigureDefaults ();
        //outputConfig.ConfigureAttributes ();

		  Ptr<Node> node = macroUes.Get(11);
		  Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel> ();
	  Simulator::Schedule (Seconds (1), &reportmobility, mobilityModel);

	  node = macroUes.Get(10);
	  mobilityModel = node->GetObject<MobilityModel> ();
  Simulator::Schedule (Seconds (1.00001), &reportmobility, mobilityModel);



	  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",
	                     MakeCallback (&NotifyHandoverStartEnb));
	  Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",
	                   MakeCallback (&NotifyHandoverEndOkEnb));
//        Config::Connect ("/NodeList/*/$ns3::MobilityModel/CourseChange",
//                           MakeCallback (&CourseChange));
        //AnimationInterface anim (animFile);
	simTime=51;
        Simulator::Stop (Seconds(simTime));
     AnimationInterface anim ("mobanim1.xml");
     anim.EnablePacketMetadata (true);
     anim.SetMaxPktsPerTraceFile(5000000);
     anim.SetConstantPosition(remoteHost, 500,500,3);

//
//        Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
//        remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
//        remHelper->SetAttribute ("OutputFile", StringValue ("rrem.out"));
//        remHelper->SetAttribute ("XMin", DoubleValue (-2000.0));
//        remHelper->SetAttribute ("XMax", DoubleValue (+2000.0));
//        remHelper->SetAttribute ("YMin", DoubleValue (-500.0));
//        remHelper->SetAttribute ("YMax", DoubleValue (+4500.0));
//        remHelper->SetAttribute ("Z", DoubleValue (1.5));
//        remHelper->Install ();

		Simulator::Run ();

        //GtkConfigStore config;
        //config.ConfigureAttributes ();

        lteHelper = 0;

        myGymEnv->NotifySimulationEnd();
        Simulator::Destroy ();

        return 0;
 }
