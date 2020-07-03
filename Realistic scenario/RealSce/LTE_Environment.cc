/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*

 *

 *

 * Acknowledgement: Code is based on implmentation of dual-stripe model of  Nicola Baldo <nbaldo@cttc.es>

 */



#include <ns3/core-module.h>

#include "ns3/opengym-module.h"

#include <ns3/network-module.h>

#include <ns3/mobility-module.h>

#include <ns3/internet-module.h>

#include <ns3/lte-module.h>

#include <ns3/config-store-module.h>

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





using namespace ns3;



NS_LOG_COMPONENT_DEFINE("MyLTENetwork");

uint32_t RunNum;

uint32_t m_mobility = 1;//1 Realistic mobility model (real map), 2 random walk (3 cells on a line)

std::string m_traceFile; ///< trace file





void NotifyHandoverStartEnb(std::string context,

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



void NotifyHandoverEndOkEnb(std::string context,

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



        for (uint16_t i = 0; i < rep; i++)

            v.push_back(tempVal);



        cioList.erase(0, pos + delimiter.length());

        pos = cioList.find(delimiter);

    }



    tempVal = std::stod(cioList);

    for (uint16_t i = 0; i < rep; i++)

        v.push_back(tempVal);



    return v;

}



static ns3::GlobalValue g_nMacroEnbSites("nMacroEnbSites",

    "How many macro sites there are",

    ns3::UintegerValue(6),

    ns3::MakeUintegerChecker<uint32_t>());

static ns3::GlobalValue g_interSiteDistance("interSiteDistance",

    "min distance between two nearby macro cell sites",

    ns3::DoubleValue(500),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_macroEnbTxPowerDbm("macroEnbTxPowerDbm",

    "TX power [dBm] used by macro eNBs",

    ns3::DoubleValue(32.0),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_macroEnbDlEarfcn("macroEnbDlEarfcn",

    "DL EARFCN used by macro eNBs",

    ns3::UintegerValue(100),

    ns3::MakeUintegerChecker<uint16_t>());

static ns3::GlobalValue g_macroEnbBandwidth("macroEnbBandwidth",

    "bandwidth [num RBs] used by macro eNBs",

    ns3::UintegerValue(25),

    ns3::MakeUintegerChecker<uint16_t>());

static ns3::GlobalValue g_fadingTrace("fadingTrace",

    "The path of the fading trace (by default no fading trace "

    "is loaded, i.e., fading is not considered)",

    ns3::StringValue(""),

    ns3::MakeStringChecker());

static ns3::GlobalValue g_hystersis("hysteresisCoefficient",

    "The value of hysteresis coefficient",

    ns3::DoubleValue(3.0),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_timeToTrigger("TTT",

    "The value of time to tigger coefficient in milliseconds",

    ns3::DoubleValue(40),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_cioList("cioList",

    "The CIO values arranged in a string.",

    ns3::StringValue("0 0 0 0 0"),

    ns3::MakeStringChecker());

static ns3::GlobalValue g_simTime("simTime",

    "Total duration of the simulation [s]",

    ns3::DoubleValue(51),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_envStepTime("envStepTime",

    "Environment Step time [s]",

    ns3::DoubleValue(0.2),

    ns3::MakeDoubleChecker<double>());

static ns3::GlobalValue g_openGymPort("openGymPort",

    "Open Gym Port Number",

    ns3::UintegerValue(9999),

    ns3::MakeUintegerChecker<uint16_t>());

static ns3::GlobalValue g_outputTraceFiles("outputTraceFiles",

    "If true, trace files will be output in ns3 working directory. "

    "If false, no files will be generated.",

    ns3::BooleanValue(true),

    ns3::MakeBooleanChecker());

static ns3::GlobalValue g_epcDl("epcDl",

    "if true, will activate data flows in the downlink when EPC is being used. "

    "If false, downlink flows won't be activated. "

    "If EPC is not used, this parameter will be ignored.",

    ns3::BooleanValue(true),

    ns3::MakeBooleanChecker());

static ns3::GlobalValue g_epcUl("epcUl",

    "if true, will activate data flows in the uplink when EPC is being used. "

    "If false, uplink flows won't be activated. "

    "If EPC is not used, this parameter will be ignored.",

    ns3::BooleanValue(true),

    ns3::MakeBooleanChecker());

static ns3::GlobalValue g_useUdp("useUdp",

    "if true, the UdpClient application will be used. "

    "Otherwise, the BulkSend application will be used over a TCP connection. "

    "If EPC is not used, this parameter will be ignored.",

    ns3::BooleanValue(true),

    ns3::MakeBooleanChecker());

static ns3::GlobalValue g_numBearersPerUe("numBearersPerUe",

    "How many bearers per UE there are in the simulation",

    ns3::UintegerValue(1),

    ns3::MakeUintegerChecker<uint16_t>());

static ns3::GlobalValue g_srsPeriodicity("srsPeriodicity",

    "SRS Periodicity (has to be at least "

    "greater than the number of UEs per eNB)",

    ns3::UintegerValue(80),

    ns3::MakeUintegerChecker<uint16_t>());



int main(int argc, char* argv[])

{

    // change some default attributes so that they are reasonable for

    // this scenario, but do this before processing command line

    // arguments, so that the user is allowed to override these settings

    Config::SetDefault("ns3::LteHelper::UseIdealRrc", BooleanValue(false));

    Config::SetDefault("ns3::LteHelper::Scheduler", StringValue("ns3::RrFfMacScheduler"));

    Config::SetDefault("ns3::UdpClient::Interval", TimeValue(MilliSeconds(1)));

    Config::SetDefault("ns3::UdpClient::PacketSize", UintegerValue(12)); // bytes

    Config::SetDefault("ns3::UdpClient::MaxPackets", UintegerValue(1000000));

    Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(10 * 1024));



    Config::SetDefault("ns3::ConfigStore::Filename", StringValue("Real_model-attributes.txt"));

    Config::SetDefault("ns3::ConfigStore::FileFormat", StringValue("RawText"));

    Config::SetDefault("ns3::ConfigStore::Mode", StringValue("Load"));



    CommandLine cmd;

    cmd.AddValue("RunNum", "1...10", RunNum);



    cmd.Parse(argc, argv);



    ConfigStore inputConfig;

    inputConfig.ConfigureDefaults();



    cmd.Parse(argc, argv);



    NS_LOG_UNCOND("RunNum = " << RunNum);



    if (RunNum < 1)

        RunNum = 1;

    SeedManager::SetSeed(1);

    SeedManager::SetRun(RunNum);

    // the scenario parameters get their values from the global attributes defined above

    UintegerValue uintegerValue;

    IntegerValue integerValue;

    DoubleValue doubleValue;

    BooleanValue booleanValue;

    StringValue stringValue;



    GlobalValue::GetValueByName("nMacroEnbSites", uintegerValue);

    uint32_t nMacroEnbSites = uintegerValue.Get();

    GlobalValue::GetValueByName("interSiteDistance", doubleValue);

    double interSiteDistance = doubleValue.Get();

    GlobalValue::GetValueByName("macroEnbTxPowerDbm", doubleValue);

    double macroEnbTxPowerDbm = doubleValue.Get();

    GlobalValue::GetValueByName("macroEnbDlEarfcn", uintegerValue);

    uint32_t macroEnbDlEarfcn = uintegerValue.Get();

    GlobalValue::GetValueByName("macroEnbBandwidth", uintegerValue);

    uint16_t macroEnbBandwidth = uintegerValue.Get();

    GlobalValue::GetValueByName("fadingTrace", stringValue);

    std::string fadingTrace = stringValue.Get();

    GlobalValue::GetValueByName("hysteresisCoefficient", doubleValue);

    double hysteresisCoefficient = doubleValue.Get();

    GlobalValue::GetValueByName("TTT", doubleValue);

    double timeToTrigger = doubleValue.Get();

    GlobalValue::GetValueByName("simTime", doubleValue);

    double simTime = doubleValue.Get();

    GlobalValue::GetValueByName("envStepTime", doubleValue);

    double envStepTime = doubleValue.Get();

    GlobalValue::GetValueByName("openGymPort", uintegerValue);

    uint16_t openGymPort = uintegerValue.Get();

    GlobalValue::GetValueByName("outputTraceFiles", booleanValue);

    bool outputTraceFiles = booleanValue.Get();

    GlobalValue::GetValueByName("cioList", stringValue);

    std::string cioList = stringValue.Get();

    GlobalValue::GetValueByName("epcDl", booleanValue);

    bool epcDl = booleanValue.Get();

    GlobalValue::GetValueByName("epcUl", booleanValue);

    bool epcUl = booleanValue.Get();

    GlobalValue::GetValueByName("useUdp", booleanValue);

    bool useUdp = booleanValue.Get();

    GlobalValue::GetValueByName("numBearersPerUe", uintegerValue);

    uint16_t numBearersPerUe = uintegerValue.Get();

    GlobalValue::GetValueByName("srsPeriodicity", uintegerValue);

    uint16_t srsPeriodicity = uintegerValue.Get();





    Config::SetDefault("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue(srsPeriodicity));

    

    uint32_t nCSpeedUes = 0;// Number of users with constant speed mobility model

    uint16_t nMacroUes = 40; // Number of users with realistic mobility model


    Ptr<OpenGymInterface> openGymInterface = CreateObject<OpenGymInterface>(openGymPort);

    Ptr<MyGymEnv> myGymEnv = CreateObject<MyGymEnv>(envStepTime, nMacroEnbSites, nCSpeedUes + nMacroUes, macroEnbBandwidth);



    myGymEnv->SetOpenGymInterface(openGymInterface);

    NS_LOG_UNCOND(" ====== Creating UEs ====== ");


    NodeContainer CSpeedUes;

    CSpeedUes.Create(nCSpeedUes);



    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();

    lteHelper->SetAttribute("PathlossModel", StringValue("ns3::Cost231PropagationLossModel"));



    lteHelper->SetSpectrumChannelType("ns3::MultiModelSpectrumChannel");



    // Set handover paratmeters

    NS_LOG_UNCOND(" ====== Setting handover parameters ====== ");

    NS_LOG_UNCOND("Hysteresis = " << hysteresisCoefficient << "dB");

    NS_LOG_UNCOND("TimeToTrigger = " << timeToTrigger << "ms");

    lteHelper->SetHandoverAlgorithmType("ns3::A3RsrpHandoverAlgorithm");

    lteHelper->SetHandoverAlgorithmAttribute("Hysteresis", DoubleValue(hysteresisCoefficient));

    lteHelper->SetHandoverAlgorithmAttribute("TimeToTrigger", TimeValue(MilliSeconds(timeToTrigger)));



    std::vector<double> cioListDouble = convertStringtoDouble(cioList, 1);

    CellIndividualOffset::setOffsetList(cioListDouble);



    NS_LOG_UNCOND(" ====== Setting up fading ====== ");

    if (!fadingTrace.empty()) {

        lteHelper->SetAttribute("FadingModel", StringValue("ns3::TraceFadingLossModel"));

        lteHelper->SetFadingModelAttribute("TraceFilename", StringValue(fadingTrace));

        NS_LOG_UNCOND("Filename " << fadingTrace);

    }

    else {

        NS_LOG_UNCOND("No fading specified");

    }



    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();

    lteHelper->SetEpcHelper(epcHelper);



    // Set other scenrio paratmeters

    Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(macroEnbTxPowerDbm));

    lteHelper->SetEnbAntennaModelType("ns3::IsotropicAntennaModel");

    lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(macroEnbDlEarfcn));

    lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(macroEnbDlEarfcn + 18000));

    lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(macroEnbBandwidth));

    lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(macroEnbBandwidth));



    NS_LOG_UNCOND(" ====== Deploying eNBs ====== ");

    NodeContainer macroEnbs;

    macroEnbs.Create(nMacroEnbSites);

    Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();

    if (m_mobility == 1) {

        enbPositionAlloc->Add(Vector(12291.12, 4332.80, 30)); //L6183

        enbPositionAlloc->Add(Vector(13437.43, 4162.39, 30)); //L6541

        enbPositionAlloc->Add(Vector(13431.70, 4656.67, 30)); //LK2313

        enbPositionAlloc->Add(Vector(13093.75, 3720.26, 30)); //LK2331

        enbPositionAlloc->Add(Vector(14002.00, 4248.31, 30)); //LK2526

        enbPositionAlloc->Add(Vector(12938.86, 4515.85, 30)); //LK6606

    }

    else {

        for (uint16_t i = 0; i < nMacroEnbSites; i++) {

            Vector enbPosition(interSiteDistance * i, 0, 30);

            enbPositionAlloc->Add(enbPosition);

        }

    }

    MobilityHelper mobility;

    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    mobility.SetPositionAllocator(enbPositionAlloc);

    mobility.Install(macroEnbs);

    NetDeviceContainer macroEnbDevs = lteHelper->InstallEnbDevice(macroEnbs);



    std::vector<Vector> eNBsLocation;

    Vector tempLocation;

    for (uint32_t it = 0; it != macroEnbs.GetN(); ++it) {

        Ptr<Node> node = macroEnbs.Get(it);

        Ptr<NetDevice> netDevice = macroEnbDevs.Get(it);

        Ptr<LteEnbNetDevice> enbNetDevice = netDevice->GetObject<LteEnbNetDevice>();

        Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel>();

        tempLocation = mobilityModel->GetPosition();

        eNBsLocation.push_back(tempLocation);

        NS_LOG_UNCOND("Cell #" << enbNetDevice->GetCellId() << " Pos =  " << tempLocation << ", CIO = " << cioListDouble[it]);

    }



    // this enables handover for macro eNBs

    lteHelper->AddX2Interface(macroEnbs);



    Ptr<ListPositionAllocator> CSpeedUesPositionAlloc = CreateObject<ListPositionAllocator>();



    for (uint16_t i = 0; i < nCSpeedUes / 2; i++) {

        Vector enbPosition(30 * i, 50, 2);

        CSpeedUesPositionAlloc->Add(enbPosition);

    }



    for (uint16_t i = 0; i < nCSpeedUes / 2; i++) {

        Vector enbPosition(30 * i, 70, 2);

        CSpeedUesPositionAlloc->Add(enbPosition);

    }



    MobilityHelper CSpeedUesMobility;

    CSpeedUesMobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");

    CSpeedUesMobility.SetPositionAllocator(CSpeedUesPositionAlloc);



    CSpeedUesMobility.Install(CSpeedUes);



    for (uint16_t i = 0; i < nCSpeedUes; i++) {



        Ptr<ConstantVelocityMobilityModel> mob = CSpeedUes.Get(i)->GetObject<ConstantVelocityMobilityModel>();

        mob->SetVelocity(Vector(20, 0, 0)); //go to the right

    }



    NetDeviceContainer CSpeedUeDevs = lteHelper->InstallUeDevice(CSpeedUes);



    // macro Ues

    NodeContainer macroUes;

    macroUes.Create(nMacroUes);



    // Install Mobility Model in UE

    if (m_mobility == 1) {

        m_traceFile = "scratch/satt_6c_r_mob_5000_ped_veh_r2.tcl"; 



        // Create Ns2MobilityHelper with the specified trace log file as parameter

        Ns2MobilityHelper ns2 = Ns2MobilityHelper(m_traceFile);

        ns2.Install(macroUes.Begin(), macroUes.End()); // configure movements for each node, while reading trace file

    }

    else if (m_mobility == 2) {

        Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();

        Vector uePosition(500, 100, 0);

        uePositionAlloc->Add(uePosition);



        MobilityHelper ueMobility;



        int64_t m_streamIndex = 0;

        ObjectFactory pos;

        pos.SetTypeId("ns3::RandomBoxPositionAllocator");

        pos.Set("X", StringValue("ns3::UniformRandomVariable[Min=-500.0|Max=1500.0]"));

        pos.Set("Y", StringValue("ns3::UniformRandomVariable[Min=-300.0|Max=300.0]"));

        pos.Set("Z", StringValue("ns3::UniformRandomVariable[Min=1.5|Max=2.0]"));



        Ptr<PositionAllocator> taPositionAlloc = pos.Create()->GetObject<PositionAllocator>();

        m_streamIndex += taPositionAlloc->AssignStreams(m_streamIndex);



        ueMobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",

            "Mode", StringValue("Time"),

            "Time", StringValue("0.1s"),

            "Speed", StringValue("ns3::ConstantRandomVariable[Constant=0.0]"),

            "Bounds", StringValue("-500|1500|-500|500"));

        ueMobility.SetPositionAllocator(taPositionAlloc);

        ueMobility.Install(macroUes);



        m_streamIndex += ueMobility.AssignStreams(macroUes, m_streamIndex);

    }

    NetDeviceContainer macroUeDevs = lteHelper->InstallUeDevice(macroUes);



    for (uint32_t it = 0; it != macroUes.GetN(); ++it) {

        Ptr<Node> node = macroUes.Get(it);

        Ptr<NetDevice> netDevice = macroUeDevs.Get(it);

        Ptr<LteUeNetDevice> uebNetDevice = netDevice->GetObject<LteUeNetDevice>();

        Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel>();

        tempLocation = mobilityModel->GetPosition();

        eNBsLocation.push_back(tempLocation);

        NS_LOG_UNCOND(" " << it << " Ue #" << uebNetDevice->GetImsi() << " Pos =  " << tempLocation << ", CIO = " << cioListDouble[it]);

    }



    NS_LOG_UNCOND(" ====== Deployment Summary ======");

    for (uint16_t i = 0; i < macroUes.GetN(); ++i) {

        Ptr<Node> node = macroUes.Get(i);

        Ptr<MobilityModel> mobilityModel = node->GetObject<MobilityModel>();



        NS_LOG_UNCOND("Node# " << node->GetId() << " is initially at " << mobilityModel->GetPosition());

    }



    /////////////////////////////////////////////////////////////////////////////////////////



    NS_LOG_UNCOND("setting up internet and remote host");

    // Create a single RemoteHost

    Ptr<Node> remoteHost;

    NodeContainer remoteHostContainer;

    remoteHostContainer.Create(1);

    remoteHost = remoteHostContainer.Get(0);

    InternetStackHelper internet;

    internet.Install(remoteHostContainer);

    Ipv4Address remoteHostAddr;



    NodeContainer ues;

    Ipv4InterfaceContainer ueIpIfaces;

    NetDeviceContainer ueDevs;



    // Create the Internet

    PointToPointHelper p2ph;

    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Mb/s")));

    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));

    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

    Ipv4AddressHelper ipv4h;

    ipv4h.SetBase("1.0.0.0", "255.0.0.0");

    Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign(internetDevices);

    // in this container, interface 0 is the pgw, 1 is the remoteHost

    remoteHostAddr = internetIpIfaces.GetAddress(1);



    Ipv4StaticRoutingHelper ipv4RoutingHelper;

    Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());

    remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);



    // for internetworking purposes, consider together home UEs and macro UEs

    ues.Add(CSpeedUes);

    ues.Add(macroUes);

    ueDevs.Add(CSpeedUeDevs);

    ueDevs.Add(macroUeDevs);



    // Install the IP stack on the UEs

    internet.Install(ues);

    ueIpIfaces = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueDevs));



    // attachment (needs to be done after IP stack configuration)

    // using initial cell selection

    lteHelper->Attach(macroUeDevs);

    lteHelper->Attach(CSpeedUeDevs);



    NS_LOG_UNCOND("setting up applications");



    // Install and start applications on UEs and remote host

    uint16_t dlPort = 10000;

    uint16_t ulPort = 20000;



    // randomize a bit start times to avoid simulation artifacts

    // (e.g., buffer overflows due to packet transmissions happening

    // exactly at the same time)

    Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable>();

    if (useUdp) {

        startTimeSeconds->SetAttribute("Min", DoubleValue(0));

        startTimeSeconds->SetAttribute("Max", DoubleValue(0.010));

    }

    else {

        // TCP needs to be started late enough so that all UEs are connected

        // otherwise TCP SYN packets will get lost

        startTimeSeconds->SetAttribute("Min", DoubleValue(0.100));

        startTimeSeconds->SetAttribute("Max", DoubleValue(0.110));

    }



    for (uint32_t u = 0; u < ues.GetN(); ++u) {

        Ptr<Node> ue = ues.Get(u);

        // Set the default gateway for the UE

        Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ue->GetObject<Ipv4>());

        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);



        for (uint32_t b = 0; b < numBearersPerUe; ++b) {

            ++dlPort;

            ++ulPort;



            ApplicationContainer clientApps;

            ApplicationContainer serverApps;



            if (useUdp) {

                if (epcDl) {

                    NS_LOG_UNCOND("installing UDP DL app for UE " << u);

                    UdpClientHelper dlClientHelper(ueIpIfaces.GetAddress(u), dlPort);

                    clientApps.Add(dlClientHelper.Install(remoteHost));

                    PacketSinkHelper dlPacketSinkHelper("ns3::UdpSocketFactory",

                        InetSocketAddress(Ipv4Address::GetAny(), dlPort));

                    serverApps.Add(dlPacketSinkHelper.Install(ue));

                }

                if (epcUl) {

                    NS_LOG_UNCOND("installing UDP UL app for UE " << u);

                    UdpClientHelper ulClientHelper(remoteHostAddr, ulPort);

                    clientApps.Add(ulClientHelper.Install(ue));

                    PacketSinkHelper ulPacketSinkHelper("ns3::UdpSocketFactory",

                        InetSocketAddress(Ipv4Address::GetAny(), ulPort));

                    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

                }

            }

            else // use TCP

            {

                if (epcDl) {

                    NS_LOG_UNCOND("installing TCP DL app for UE " << u);

                    BulkSendHelper dlClientHelper("ns3::TcpSocketFactory",

                        InetSocketAddress(ueIpIfaces.GetAddress(u), dlPort));

                    dlClientHelper.SetAttribute("MaxBytes", UintegerValue(0));

                    clientApps.Add(dlClientHelper.Install(remoteHost));

                    PacketSinkHelper dlPacketSinkHelper("ns3::TcpSocketFactory",

                        InetSocketAddress(Ipv4Address::GetAny(), dlPort));

                    serverApps.Add(dlPacketSinkHelper.Install(ue));

                }

                if (epcUl) {

                    NS_LOG_UNCOND("installing TCP UL app for UE " << u);

                    BulkSendHelper ulClientHelper("ns3::TcpSocketFactory",

                        InetSocketAddress(remoteHostAddr, ulPort));

                    ulClientHelper.SetAttribute("MaxBytes", UintegerValue(0));

                    clientApps.Add(ulClientHelper.Install(ue));

                    PacketSinkHelper ulPacketSinkHelper("ns3::TcpSocketFactory",

                        InetSocketAddress(Ipv4Address::GetAny(), ulPort));

                    serverApps.Add(ulPacketSinkHelper.Install(remoteHost));

                }

            } // end if (useUdp)



            Ptr<EpcTft> tft = Create<EpcTft>();

            if (epcDl) {

                EpcTft::PacketFilter dlpf;

                dlpf.localPortStart = dlPort;

                dlpf.localPortEnd = dlPort;

                tft->Add(dlpf);

            }

            if (epcUl) {

                EpcTft::PacketFilter ulpf;

                ulpf.remotePortStart = ulPort;

                ulpf.remotePortEnd = ulPort;

                tft->Add(ulpf);

            }



            if (epcDl || epcUl) {

                EpsBearer bearer(EpsBearer::NGBR_VIDEO_TCP_DEFAULT);

                lteHelper->ActivateDedicatedEpsBearer(ueDevs.Get(u), bearer, tft);

            }

            Time startTime = Seconds(startTimeSeconds->GetValue());

            serverApps.Start(startTime);

            clientApps.Start(startTime);



        } 

    }






    for (uint32_t it = 0; it != macroEnbs.GetN(); ++it) {

        //Ptr<Node> node = macroEnbs.GetNode(it);

        Ptr<NetDevice> netDevice = macroEnbDevs.Get(it);

        Ptr<LteEnbNetDevice> enbNetDevice = netDevice->GetObject<LteEnbNetDevice>();

        Ptr<LteEnbPhy> enbPhy = enbNetDevice->GetPhy();

        enbPhy->TraceConnectWithoutContext("DlPhyTransmission", MakeBoundCallback(&MyGymEnv::GetPhyStats, myGymEnv));

    }



    // Enable output traces

    if (outputTraceFiles) {

        lteHelper->EnablePhyTraces();

        //lteHelper->EnableMacTraces ();

        //lteHelper->EnableRlcTraces ();

        //lteHelper->EnablePdcpTraces ();

    }



    Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverStart",

        MakeCallback(&NotifyHandoverStartEnb));

    Config::Connect("/NodeList/*/DeviceList/*/LteEnbRrc/HandoverEndOk",

        MakeCallback(&NotifyHandoverEndOkEnb));



    Simulator::Stop(Seconds(simTime));

    Simulator::Run();



    lteHelper = 0;



    myGymEnv->NotifySimulationEnd();

    Simulator::Destroy();



    return 0;

}

