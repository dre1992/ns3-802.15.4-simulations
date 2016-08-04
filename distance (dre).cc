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
#include <math.h>
#include <ns3/netanim-module.h>
using namespace ns3;
using namespace std;
static uint32_t g_received = 0;

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){
 g_received++ ;
 
 //NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
  

}

static void pded(LrWpanPhyEnumeration en,uint8_t level ){

//NS_LOG_UNCOND("ed level = "<<(int)level);

}



/*
static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState)
{
  NS_LOG_UNCOND (context << " state change at " << now.GetSeconds ()
                         << " from " << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldState)
                         << " to " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newState));
}
static void PhyTxStart (std::string context,Ptr<const Packet> packet){
 
 NS_LOG_UNCOND("TX start time ="<<Simulator::Now ().GetSeconds ());
 
}

static void PhyTxEnd (std::string context,Ptr<const Packet> packet){
 
 NS_LOG_UNCOND("TX end time ="<<Simulator::Now ().GetSeconds ());
 
}

*/


NS_LOG_COMPONENT_DEFINE ("parameters_check");

int main (int argc, char *argv[])
{
CommandLine cmd;
cmd.Parse (argc, argv);
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-distance.plt");
  Gnuplot psrplot = Gnuplot ("802.15.4-distance.eps");
  Gnuplot2dDataset psrdataset ("802.15.4-distance");
  
  int maxPackets = 100;
  int packetSize = 50;
  int minDistance = 2;
  int maxDistance = 80;  // meters
  int r=1;
 // int increment = 1;
  //LogComponentEnable ("LrWpanNetDevice", LOG_LEVEL_INFO);
   NodeContainer nodes;
   nodes.Create (6);
   os<<"lognormal 10 nodes 100packet,50bytes n=4 r=1 pr=55";
  LrWpanHelper helper;
  
  Ptr<MultiModelSpectrumChannel> channel = CreateObject<MultiModelSpectrumChannel> ();
  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();
  model->SetPathLossExponent(4);
  model->SetReference(1,55);	
  channel->AddPropagationLossModel (model);
  
  helper.SetChannel(channel);
  //helper.GetChannel()->AddPropagationLossModel (model);
  NetDeviceContainer devices;
  devices=helper.Install(nodes);
 // LrWpanSpectrumValueHelper svh;
  //Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (0, 11);
  

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
  
 /*  for(int i=1;i<(int)nodes.GetN();i++){
  lrdev[i]->GetPhy ()->SetTxPowerSpectralDensity (psd);
  }
  */
  // Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> (); 
  for(int i=1;i<(int)nodes.GetN();i++){
   
   float rad=0.0174532925*360/(nodes.GetN());
   Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> (); 
   
  double x=r*sin(rad*i);
  double y=r*cos(rad*i);
  sender0Mobility->SetPosition (Vector (x,y,0));
   //AnimationInterface::SetConstantPosition ( nodes.Get (i), x, y); 
  lrdev[i]->GetPhy ()->SetMobility (sender0Mobility);
  }

  Ptr<ConstantPositionMobilityModel> sender1Mobility = CreateObject<ConstantPositionMobilityModel> ();
  sender1Mobility->SetPosition (Vector (0,0,0));
  lrdev[0]->GetPhy ()->SetMobility (sender1Mobility);
  AnimationInterface::SetConstantPosition ( nodes.Get (0), 0, 0); 
   McpsDataIndicationCallback cb0;
  cb0 = MakeCallback (&DataIndication);
  lrdev[0]->GetMac ()->SetMcpsDataIndicationCallback (cb0);
  
  PlmeEdConfirmCallback cb1;
  cb1=MakeCallback(&pded);
    lrdev[1]->GetPhy ()->SetPlmeEdConfirmCallback (cb1);  
    lrdev[2]->GetPhy ()->SetPlmeEdConfirmCallback (cb1);  
    lrdev[3]->GetPhy ()->SetPlmeEdConfirmCallback (cb1);  
    lrdev[4]->GetPhy ()->SetPlmeEdConfirmCallback (cb1);  
    lrdev[5]->GetPhy ()->SetPlmeEdConfirmCallback (cb1);  





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

           Simulator::Schedule (Seconds (value), &LrWpanPhy::PlmeEdRequest, lrdev[j]->GetPhy ());      
       }
    }
       // AnimationInterface anim ("test-animation.xml"); // Mandatory
      Simulator::Run ();
      NS_LOG_DEBUG ("Received " << g_received << " packets for distance "<<l  );
      psrdataset.Add (l, g_received/(100.0*(float)(nodes.GetN()-1))); 
      g_received = 0;
   r=l;  
    // l+=increment;
  //sender1Mobility->SetPosition (Vector (0,l,0));
  //lrdev[0]->GetPhy ()->SetMobility (sender1Mobility);
 for(int i=1;i<(int)nodes.GetN();i++){
    
   float rad=0.0174532925*360/(nodes.GetN());
 //  Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> (); 
   
  double x=r*sin(rad*i);
  double y=r*cos(rad*i);
 // sender0Mobility->SetPosition (Vector (x,y,0));
  //  AnimationInterface::SetConstantPosition ( nodes.Get (i), x, y); 
//  lrdev[i]->GetPhy ()->SetMobility (sender0Mobility);
  lrdev[i]->GetPhy ()->GetMobility()-> SetPosition(Vector(x,y,0));

//Vector position=lrdev[i]->GetPhy ()->GetMobility()-> GetPosition();
//NS_LOG_UNCOND ("node "<<i<<",R= "<<r<<", x = " << position.x << ", y = " << position.y);
  } 
} 

 psrplot.AddDataset (psrdataset);

  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("Distance", "Packet Success Rate (PSR)");
  psrplot.SetExtra  ("set xrange [1:80]\n\
set yrange [0:1]\n\
set grid\n\
set style line 1 linewidth 5\n\
set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();

  Simulator::Destroy ();
  return 0;




}


  



