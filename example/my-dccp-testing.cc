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

NS_LOG_COMPONENT_DEFINE ("UserCooperation");

void setPos (Ptr<Node> n, int x, int y, int z)
{
  Ptr<ConstantPositionMobilityModel> loc = CreateObject<ConstantPositionMobilityModel> ();
  n->AggregateObject (loc);
  Vector locVec2 (x, y, z);
  loc->SetPosition (locVec2);
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


  NodeContainer nodes;
  nodes.Create (2);




  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("1Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("20ms"));
  NetDeviceContainer devices = csma.Install (nodes);

  // PointToPointHelper p2p;
  // p2p.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  // p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
  // //p2p.SetQueue("ns3:DropTailQueue", "MaxPackets", UintegerValue(140));
  // NetDeviceContainer devices = p2p.Install (nodes);



  DceManagerHelper dceManager;
  dceManager.SetTaskManagerAttribute ("FiberManagerType",
                                      StringValue ("UcontextFiberManager"));
  dceManager.SetNetworkStack ("ns3::LinuxSocketFdFactory",
                              "Library", StringValue ("liblinux.so"));



  LinuxStackHelper stack;
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);
  dceManager.Install (nodes);


  Ptr<RateErrorModel> em1 =
    CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
                                                "ErrorRate", DoubleValue (0.12),
                                                "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
                                                );




/*
  //lte 
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
  YansWifiPhyHelper phy;



  Ptr<RateErrorModel> em1 =
    CreateObjectWithAttributes<RateErrorModel> ("RanVar", StringValue ("ns3::UniformRandomVariable[Min=0.0,Max=1.0]"),
                                                "ErrorRate", DoubleValue (0.00),
                                                "ErrorUnit", EnumValue (RateErrorModel::ERROR_UNIT_PACKET)
                                                );



  setPos (nodes.Get (0), -20, 30 / 2, 0);
  setPos (nodes.Get (1), 100, 30 / 2, 0);    
  // Left link: H1 <-> LTE-R
  NodeContainer enbNodes;
  enbNodes.Create(1);

  lteHelper->SetEpcHelper (epcHelper);
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  setPos (enbNodes.Get (0), 60, -4000, 0);


  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (nodes.Get (0));

  Ipv4InterfaceContainer if1, if2;
  //Assign ip addresses
  if1 = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
  

  std::ostringstream cmd_oss;

      // setup ip routes
  cmd_oss.str ("");
  cmd_oss << "rule add from " << if1.GetAddress (0, 0) << " table " << 1;
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());
  cmd_oss.str ("");
  cmd_oss << "route add default via " << "7.0.0.1 "  << " dev sim" << 0 << " table " << 1;
  LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());

*/

  // std::ostringstream cmd_oss;
  // cmd_oss.str("");
  // cmd_oss << "ulimit -u";
  // LinuxStackHelper::RunIp (nodes.Get (0), Seconds (0.1), cmd_oss.str ().c_str ());

  devices.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em1));


  DceApplicationHelper dce;
  ApplicationContainer apps;

  dce.SetBinary ("dccp-server");
  //dce.SetBinary ("dccp-server");

  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  apps = dce.Install (nodes.Get (0));
  apps.Start (Seconds (0.0));

  dce.SetBinary ("dccp-client");
  //dce.SetBinary ("dccp-client");
  dce.SetStackSize (1 << 16);
  dce.ResetArguments ();
  dce.AddArgument ("10.0.0.1");
  apps = dce.Install (nodes.Get (1));
  apps.Start (Seconds (0.5));



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



  csma.EnablePcapAll ("my-dccp");

  //p2p.EnablePcapAll ("my-dccp");



  Simulator::Stop (Seconds (1000000.0));
  Simulator::Run ();


  // std::vector <ProcStatus> v = dceManager.GetProcStatus ();

  //  long pn = 0;

  //unsigned long Bps = pn * 1500 / st.GetRealDuration ();

  //std::cout << "Duration: " << v[0].GetRealDuration ()<< std::endl;


 // Ptr<PacketSink> pktsink = apps.Get (0)->GetObject<PacketSink> ();
  //std::cout << "total duration: " << pktsink->GetTotalRx ()<< std::endl;


  Simulator::Destroy ();

  return 0;
}
