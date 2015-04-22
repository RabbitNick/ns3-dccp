#include "ns3/network-module.h"
#include "ns3/core-module.h"
#include "ns3/dce-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"


#include "ns3/log.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/packet-sink-helper.h"


#include "ns3/wifi-module.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/config-store-module.h"

#include "ns3/drop-tail-queue.h" 


using namespace ns3;

/**

This experiment is about DCCP over LTE+EPC. The topology is simple, like:

UE----eNB----LTE+EPC----SGW/PGW-------remote host(Internet)

**/




NS_LOG_COMPONENT_DEFINE ("UserCooperation");

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
}


void
PrintTcpFlags (std::string key, std::string value)
{
  NS_LOG_INFO (key << "=" << value);
}

int main (int argc, char *argv[])
{

  std::string bufSize = "";
  bool disWifi = false;
  bool disLte = false;
  double stopTime = 450.0;
  std::string p2pdelay = "10ms";

  CommandLine cmd;
  cmd.AddValue ("bufsize", "Snd/Rcv buffer size.", bufSize);
  cmd.AddValue ("disWifi", "Disable WiFi.", disWifi);
  cmd.AddValue ("disLte", "Disable LTE.", disLte);
  cmd.AddValue ("stopTime", "StopTime of simulatino.", stopTime);
  cmd.AddValue ("p2pDelay", "Delay of p2p links. default is 10ms.", p2pdelay);
  cmd.Parse (argc, argv);

  if (disWifi && disLte)
  {
     NS_LOG_INFO ("no active interface");
     return 0;
  }

  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  std::ostringstream cmd_oss;


  // NodeContainer nodes;
  // nodes.Create (2);




  // CsmaHelper csma;
  // csma.SetChannelAttribute ("DataRate", StringValue ("1Mbps"));
  // csma.SetChannelAttribute ("Delay", StringValue ("20ms"));
  // NetDeviceContainer devices = csma.Install (nodes);

  // PointToPointHelper p2p;
  // p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  // p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  // NetDeviceContainer devices = p2p.Install (nodes);



  // DceManagerHelper dceManager;
  //dceManager.SetTaskManagerAttribute ("FiberManagerType",                                      StringValue ("UcontextFiberManager"));
  // dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
  //                             "Library", StringValue ("liblinux.so"));



  // LinuxStackHelper stack;
  // stack.Install (nodes);
  // Ipv4AddressHelper address;
  // address.SetBase ("10.0.0.0", "255.255.255.0");
  // Ipv4InterfaceContainer interfaces = address.Assign (devices);
  // dceManager.Install (nodes);


  Ptr<RateErrorModel> em1 =
    CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
                                                "ErrorRate", DoubleValue (0.5),
                                                "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
                                                );


  //lte 

  // nodes for UE and eNB
  uint16_t numberOfNodes = 1;


  // distance for UE and eNB
  double distance = 10.0;


  // left link : UE <---> LTE(eNB)

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);


     // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  //InternetStackHelper internet;
  //internet.Install (remoteHostContainer);



  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));

  dceManager.Install (remoteHostContainer);
  dceManager.Install (ueNodes);

  LinuxStackHelper stack;
  stack.Install (remoteHostContainer);
  stack.Install (ueNodes);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", StringValue ("1000Mbps"));
  //p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", StringValue ("0.1ms"));


  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  



  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);



  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  //internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  // Attach one UE per eNodeB
  for (uint16_t i = 0; i < numberOfNodes; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
        // side effect: the default EPS bearer will be activated
      }



      // setup ip routes
      cmd_oss.str ("");
      cmd_oss << "rule add from " << ueIpIface.GetAddress (0, 0) << " table " << 1;
      LinuxStackHelper::RunIp (ueNodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      cmd_oss.str ("");
      cmd_oss << "route add default via " << "7.0.0.1 "  << " dev sim" << 0 << " table " << 1;
      LinuxStackHelper::RunIp (ueNodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());

  // Assign IP address to UEs, and install applications
  // for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  //   {
  //     Ptr<Node> ueNode = ueNodes.Get (u);
  //     // Set the default gateway for the UE
  //     Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  //     ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  //   }

  // ConfigStore inputConfig;
  // inputConfig.ConfigureDefaults();





  // Right link : LTE(PGW) <---> LTE(remote host)





  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHostContainer.Get(0));

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  ipv4h.NewNetwork();
  // interface 0 is localhost, 1 is the p2p device
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);


      // setup ip routes
  cmd_oss.str ("");
  cmd_oss << "rule add from " << internetIpIfaces.GetAddress (0, 0) << " table " << (1);
  LinuxStackHelper::RunIp (remoteHostContainer.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
  cmd_oss.str ("");
  cmd_oss << "route add 1.0." << 0 << ".0/24 dev sim" << 0 << " scope link table " << (1);
  LinuxStackHelper::RunIp (remoteHostContainer.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
      //setPos (pgw, 70, 0, 0);


  //routing 
  // Ipv4StaticRoutingHelper ipv4RoutingHelper;
  // Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



  // default route
  LinuxStackHelper::RunIp (ueNodes.Get (0), Seconds (0.1), "route add default via 7.0.0.1 dev sim0");
  LinuxStackHelper::RunIp (remoteHostContainer.Get (0), Seconds (0.1), "route add default via 1.0.0.1 dev sim0");
  LinuxStackHelper::RunIp (ueNodes.Get (0), Seconds (0.1), "rule show");
  LinuxStackHelper::RunIp (ueNodes.Get (0), Seconds (5.1), "route show table all");
  LinuxStackHelper::RunIp (remoteHostContainer.Get (0), Seconds (0.1), "rule show");
  LinuxStackHelper::RunIp (remoteHostContainer.Get (0), Seconds (5.1), "route show table all");




  // Install and start applications on UEs and remote host





  DceApplicationHelper dce;
  ApplicationContainer apps;

  //dce.SetStackSize (1 << 20);

  // Launch iperf client on node 0
  // dce.SetBinary ("iperf");
  // dce.ResetArguments ();
  // dce.ResetEnvironment ();
  // dce.AddArgument ("-c");
  // dce.AddArgument ("7.0.0.2");
  // dce.ParseArguments ("-y C");
  // dce.AddArgument ("-i");
  // dce.AddArgument ("1");
  // dce.AddArgument ("--time");
  // dce.AddArgument ("40");

  // apps = dce.Install (remoteHostContainer.Get (0));

  // apps.Start (Seconds (5.0));
  // //  apps.Stop (Seconds (15));

  // // Launch iperf server on node 1
  // dce.SetBinary ("iperf");
  // dce.ResetArguments ();
  // dce.ResetEnvironment ();
  // dce.AddArgument ("-s");
  // dce.AddArgument ("-P");
  // dce.AddArgument ("1");

  // apps = dce.Install (ueNodes.Get (0));

  // apps.Start (Seconds (4));


  dce.SetBinary ("dccp-server");
  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  //apps = dce.Install (ueNodes.Get (0));
  apps = dce.Install (remoteHostContainer.Get (0));
  apps.Start (Seconds (1.0));


  dce.SetBinary ("dccp-client");
  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  dce.AddArgument ("1.0.0.2");
  apps = dce.Install (ueNodes.Get (0));

  //dce.AddArgument ("7.0.0.2");
  //apps = dce.Install (remoteHostContainer.Get (0));

  apps.Start (Seconds (5.0));










  // std::ostringstream cmd_oss;
  // cmd_oss.str("");
  // cmd_oss << "ulimit -u";
  // LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());

  //devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em1));

/*
  DceApplicationHelper dce;
  ApplicationContainer apps;

  dce.SetBinary ("dccp-server");
  //dce.SetBinary ("tcp-server");

  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  apps = dce.Install (nodes.Get (0));
  apps.Start (Seconds (0.0));

  dce.SetBinary ("dccp-client");
  //dce.SetBinary ("tcp-client");
  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  dce.AddArgument ("10.0.0.1");
  apps = dce.Install (nodes.Get (1));
  apps.Start (Seconds (0.5));

*/

  // dce.SetBinary ("dccp-client");
  // dce.SetStackSize (1 << 16);
  // dce.ResetArguments ();
  // dce.AddArgument ("10.0.0.1");
  // apps = dce.Install (nodes.Get (2));
  // apps.Start (Seconds (4.5));

  // std::string sock_factory = "ns3::LinuxDccpSocketFactory";
  // PacketSinkHelper sink = PacketSinkHelper (sock_factory,
  //                                           InetSocketAddress (Ipv4Address::GetAny (), 2000));

  // apps = sink.Install (nodes);



  //csma.EnablePcapAll ("my-dccp");

  //p2p.EnablePcapAll ("my-dccp");
  p2ph.EnablePcapAll("my-dccp");

  lteHelper->EnableTraces ();

  Simulator::Stop (Seconds (180.0));
  Simulator::Run ();


  // std::vector <ProcStatus> v = dceManager.GetProcStatus ();

  //  long pn = 0;

  //unsigned long Bps = pn * 1500 / st.GetRealDuration ();

  //std::cout << "Duration: " << v[0].GetRealDuration ()<< std::endl;


 // Ptr<PacketSink> pktsink = apps.Get (0)->GetObject<PacketSink> ();
  //std::cout << "total duration: " << pktsink->GetTotalRx ()<< std::endl;
  // Output config store to txt format
  // Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  // Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  // Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  // ConfigStore outputConfig2;
  // outputConfig2.ConfigureDefaults ();
  // outputConfig2.ConfigureAttributes ();



  Simulator::Destroy ();

  return 0;
}
