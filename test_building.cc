#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>
#include <ns3/network-module.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ns3/gnuplot.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/spectrum-helper.h>
#include <ns3/buildings-helper.h>
#include <ns3/building.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>
#include <ns3/mobility-building-info.h>
using namespace ns3;
using namespace std;
static uint32_t g_received = 0;
//double loss[20] ;
//int inc=0;
static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){
 g_received++ ;
 
 //NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
  

}
/*
static void pathlossdb(std::string context,Ptr< SpectrumPhy > txPhy, Ptr< SpectrumPhy > rxPhy, double lossDb){
      Address id = rxPhy->GetDevice ()->GetObject<LrWpanNetDevice>()->GetAddress();
  if(id==Mac16Address("00:01")){
     if(txPhy->GetDevice ()->GetObject<LrWpanNetDevice>()->GetAddress()==Mac16Address("00:04")){
//avgloss+=lossDb;
loss[inc]=lossDb;
inc++;
NS_LOG_UNCOND(context<<"  path loss fired  "<<lossDb);
//NS_LOG_UNCOND(context<<"avg "<<avgloss);
}
}
}
*/

NS_LOG_COMPONENT_DEFINE ("parameters_check");

int main (int argc, char *argv[])
{
CommandLine cmd;
cmd.Parse (argc, argv);
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-building.plt");
  Gnuplot psrplot = Gnuplot ("802.15.4-building.eps");
  Gnuplot2dDataset psrdataset ("802.15.4-building");
  int minDistance=20;
  int maxDistance=800;
  int maxPackets = 20;
  int packetSize = 50;
 // LogComponentEnable ("ItuR1411LosPropagationLossModel", LOG_LEVEL_DEBUG);
 // LogComponentEnable ("ItuR1411NlosOverRooftopPropagationLossModel", LOG_LEVEL_DEBUG);
 // LogComponentEnable ("LrWpanMac", LOG_LEVEL_DEBUG);
   NodeContainer nodes;
   nodes.Create (5);
  
     Ptr<Building> building1 = CreateObject<Building> ();
     building1->SetBoundaries (Box (-15, 15, -15, 15, 0, 20));
      building1->SetBuildingType (Building::Residential);
      building1->SetExtWallsType (Building::ConcreteWithWindows);

  Ptr<HybridBuildingsPropagationLossModel> propagationLossModel = CreateObject<HybridBuildingsPropagationLossModel> ();
  // cancel shadowing effect
 // propagationLossModel->SetAttribute ("ShadowSigmaOutdoor", DoubleValue (10.0));
 // propagationLossModel->SetAttribute ("ShadowSigmaIndoor", DoubleValue (10.0));
 // propagationLossModel->SetAttribute ("ShadowSigmaExtWalls", DoubleValue (10.0));
  propagationLossModel->SetAttribute("Los2NlosThr",DoubleValue(200.0));
  propagationLossModel->SetAttribute("Frequency",DoubleValue(2450e6));
  LrWpanHelper helper;
 // SpectrumChannelHelper h;

  Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
 
  channel->AddPropagationLossModel (propagationLossModel);
   LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (0, 11);
  
   
  
  helper.SetChannel(channel);
  
  NetDeviceContainer devices;
  devices=helper.Install(nodes);

  char vec[]="00:";
  
  for (int i=0;i<(int)nodes.GetN();i++){
   if(i<9){
  char buffer[2];
  sprintf(buffer,"%d",i+1);
  vec[3]=(char)48;
  vec[4]=buffer[0];
  vec[5]=buffer[1];
  devices.Get(i)->SetAddress (Mac16Address (vec));
 
  }
   else if (i>=9){
     char buffer[3];
  sprintf(buffer,"%d",i+1);
  vec[3]=buffer[0];
  vec[4]=buffer[1];
  vec[5]=buffer[2];
  devices.Get(i)->SetAddress (Mac16Address (vec));
  }

 }


  //cast NetDevice to LrWpanNetDevice
 Ptr<NetDevice> netDevice;
 NetDeviceContainer::Iterator i; 
  std::vector<Ptr<LrWpanNetDevice> > lrdev; 
 for (i = devices.Begin (); i != devices.End (); ++i){

    netDevice = (*i);

    Ptr<LrWpanNetDevice> dev= DynamicCast<LrWpanNetDevice> (netDevice);
    lrdev.push_back(dev);
 
  }
  

  for(int i=1;i<(int)nodes.GetN();i++){
   Ptr<ConstantPositionMobilityModel> mob = CreateObject<ConstantPositionMobilityModel> ();
      mob->SetPosition (Vector (i, i, 15.0));
  lrdev[i]->GetPhy ()->SetMobility (mob);
      Ptr<MobilityBuildingInfo> buildingInfoUe = CreateObject<MobilityBuildingInfo> ();
     
      mob->AggregateObject (buildingInfoUe); // operation usually done by BuildingsHelper::Install
      BuildingsHelper::MakeConsistent (mob);     
    lrdev[i]->GetPhy ()->SetTxPowerSpectralDensity (psd);


  }
//Ptr<ConstantPositionMobilityModel> mob1 = CreateObject<ConstantPositionMobilityModel> ();
 //     mob1->SetPosition (Vector (2.0, 2.0, 15.0));
 //  lrdev[2]->GetPhy ()->SetMobility (mob1);
 //  Ptr<MobilityBuildingInfo> buildingInfoUe1 = CreateObject<MobilityBuildingInfo> ();
  
   //   mob1->AggregateObject (buildingInfoUe1); 
   //BuildingsHelper::MakeConsistent (mob1);      

   
//setting sink node as an outdoor node
     Ptr<ConstantPositionMobilityModel> mob0 = CreateObject<ConstantPositionMobilityModel> ();
      mob0->SetPosition (Vector (20.0, 20.0, 2.0));
   lrdev[0]->GetPhy ()->SetMobility (mob0);
   Ptr<MobilityBuildingInfo> buildingInfoUe0 = CreateObject<MobilityBuildingInfo> ();
   
      mob0->AggregateObject (buildingInfoUe0); 
   BuildingsHelper::MakeConsistent (mob0);      
  
 //std::ostringstream oss;
   // oss<<"/NodeList/0/DeviceList/*/$ns3::LrWpanNetDevice/Channel/$ns3::SingleModelSpectrumChannel/PathLoss" ;
  // oss<< "/ChannelList/*/$ns3::SpectrumChannel/$ns3::SingleModelSpectrumChannel/PathLoss"   ;   
 //Config::Connect (oss.str (), MakeCallback (&pathlossdb ));

   McpsDataIndicationCallback cb0;
  cb0 = MakeCallback (&DataIndication);
  lrdev[0]->GetMac ()->SetMcpsDataIndicationCallback (cb0);
  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address ("00:01");
  params.m_msduHandle = 0;
  params.m_txOptions = 0;
  double mean = 0.1;
  double bound = 0.0;
  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
  x->SetAttribute ("Mean", DoubleValue (mean));
  x->SetAttribute ("Bound", DoubleValue (bound));
  double value=0;

  Ptr<Packet> p;
  
for(int l=minDistance;l<=maxDistance;l++){


// for(int k=0;k<2;k++){
  for (int j =1; j < (int)nodes.GetN();j++  )
    {
      value=0;
      for (int i = 0; i <maxPackets ; i++)
        {
          p = Create<Packet> (packetSize);
           value+=x->GetValue();
             Simulator::Schedule (Seconds (value),
                               &LrWpanMac::McpsDataRequest,
                               lrdev[j]->GetMac (), params, p);

                 
       }
    }
      Simulator::Run ();
   //   NS_LOG_DEBUG ("Received " << g_received << " packets for trial "<<k  );
      psrdataset.Add (l, g_received/(20.0*(float)(nodes.GetN()-1))); 
  // for (int i=0;i<20;i++){
  //      psrdataset.Add(l,0-loss[i]);
  // }   
   //  inc=0;     
   g_received = 0;
     
    
  mob0->SetPosition (Vector (l,l,30.0));

        
  lrdev[0]->GetPhy ()->SetMobility (mob0);
   

BuildingsHelper::MakeConsistent(mob0);
 //} 
 }
 psrplot.AddDataset (psrdataset);

  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("Distance", "Packet Success Rate (PSR)");
  psrplot.SetExtra  ("set xrange [20:800]\n\
set yrange [0:1]\n\
set grid\n\
set style line 1 lt 3 linewidth 2\n\
set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();

  Simulator::Destroy ();
  return 0;




}


  


//BuildingsHelper::Install(nodes);
//BuildingsHelper::MakeMobilityModelConsistent();
