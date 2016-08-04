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
using namespace ns3;
using namespace std;
static uint32_t g_received = 0;
static double avgloss=0;
//double loss[20] ;
//int inc=0;
static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){
 g_received++ ;
 
 //NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
  

}

static void pathlossdb(std::string context,Ptr< SpectrumPhy > txPhy, Ptr< SpectrumPhy > rxPhy, double lossDb){
      Address id = rxPhy->GetDevice ()->GetObject<LrWpanNetDevice>()->GetAddress();
  if(id==Mac16Address("00:01")){
    // if(txPhy->GetDevice ()->GetObject<LrWpanNetDevice>()->GetAddress()==Mac16Address("00:02")){
avgloss+=lossDb;
//loss[inc]=lossDb;
//inc++;
//NS_LOG_UNCOND(context<<"  path loss fired  "<<lossDb);
//NS_LOG_UNCOND(context<<"avg "<<avgloss);
}
//}
}


NS_LOG_COMPONENT_DEFINE ("parameters_check");

int main (int argc, char *argv[])
{
  int t=1;
CommandLine cmd;
  cmd.AddValue("t","node currently viewed",t);
cmd.Parse (argc, argv);
  std::ostringstream os;
  std::ofstream berfile ("802.15.4-logshadow.plt");
  Gnuplot psrplot = Gnuplot ("802.15.4-logshadow.eps");
  Gnuplot2dDataset psrdataset ("802.15.4-logshadow");
//  std::ofstream outfile;
  //   outfile.open ("logshadow.dat", std::ofstream::out | std::ofstream::app); 
  int r=1;  
  int maxPackets =50 ;
  int packetSize = 50;
  int minDistance = 1;
  int maxDistance =100 ;  // meters


   NodeContainer nodes;
   nodes.Create (10);

  LrWpanHelper helper;
  SpectrumChannelHelper h;


  Ptr<LogDistancePropagationLossModel> model = CreateObject<LogDistancePropagationLossModel> ();
  model->SetPathLossExponent(4);
  model->SetReference(1,55);	

    Ptr<RandomPropagationLossModel> random = CreateObject<RandomPropagationLossModel> ();
    Ptr<NormalRandomVariable> Var = CreateObjectWithAttributes<NormalRandomVariable> ("Mean", DoubleValue (0),"Variance",DoubleValue(10.00));
    random->SetAttribute ("Variable", PointerValue (Var));

  Ptr<NakagamiPropagationLossModel> nak=CreateObject<NakagamiPropagationLossModel>();
  nak->SetAttribute("m0",DoubleValue(1));
  nak->SetAttribute("m1",DoubleValue(1));
  nak->SetAttribute("m2",DoubleValue(1));
 
  h.SetChannel("ns3::SingleModelSpectrumChannel");
  h.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");

  h.AddPropagationLoss(model); 
  h.AddPropagationLoss(random);
  //h.AddPropagationLoss(nak); 
  Ptr<SpectrumChannel> channel = h.Create ();
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
  

 /* for(int i=1;i<(int)nodes.GetN();i++){
       
    Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> (); 
  sender0Mobility->SetPosition (Vector (i,0,0));
  lrdev[i]->GetPhy ()->SetMobility (sender0Mobility);
  }
  Ptr<ConstantPositionMobilityModel> sender1Mobility = CreateObject<ConstantPositionMobilityModel> ();
  sender1Mobility->SetPosition (Vector (0,0,0));
  lrdev[0]->GetPhy ()->SetMobility (sender1Mobility);
*/

 for(int i=1;i<(int)nodes.GetN();i++){
   
   float rad=0.0174532925*360/(nodes.GetN());
   Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> (); 
   
  double x=r*sin(rad*i);
  double y=r*cos(rad*i);
  sender0Mobility->SetPosition (Vector (x,y,0));
    
  lrdev[i]->GetPhy ()->SetMobility (sender0Mobility);
  }

  Ptr<ConstantPositionMobilityModel> sender1Mobility = CreateObject<ConstantPositionMobilityModel> ();
  sender1Mobility->SetPosition (Vector (0,0,0));
  lrdev[0]->GetPhy ()->SetMobility (sender1Mobility);

    std::ostringstream oss;
   // oss<<"/NodeList/0/DeviceList/*/$ns3::LrWpanNetDevice/Channel/$ns3::SingleModelSpectrumChannel/PathLoss" ;
   oss<< "/ChannelList/*/$ns3::SpectrumChannel/$ns3::SingleModelSpectrumChannel/PathLoss"   ;   
 Config::Connect (oss.str (), MakeCallback (&pathlossdb ));


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


 for(int k=0;k<5;k++){
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
    //  NS_LOG_DEBUG ("Received " << g_received << " packets for distance "<<l  );
    //  psrdataset.Add (l, g_received/(50.0*(float)(nodes.GetN()-1))); 
// outfile<<l<<"\t"<<0-avgloss/(50.0*(float)(nodes.GetN()-1))<<std::endl;
               psrdataset.Add (l, 0+30-avgloss/(50.0*(float)(nodes.GetN()-1))); 
       
  //   for (int i=0;i<20;i++){
  //      psrdataset.Add(l,0-loss[i]);
  //   }
  //   inc=0;
     g_received = 0;
      avgloss=0;
    // l+=increment;
  //sender1Mobility->SetPosition (Vector (0,l,0));
  //lrdev[0]->GetPhy ()->SetMobility (sender1Mobility);
  r=l;  
   
 for(int i=1;i<(int)nodes.GetN();i++){
    
   float rad=0.0174532925*360/(nodes.GetN());
  
   
  double x=r*sin(rad*i);
  double y=r*cos(rad*i);
 
 
  lrdev[i]->GetPhy ()->GetMobility()-> SetPosition(Vector(x,y,0));
}
 } 
 }

 psrplot.AddDataset (psrdataset);

  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("Distance", "Rx power (dbm)");
  psrplot.SetExtra  ("set xrange [1:100]\n\
set yrange [-130:0]\n\
set grid\n\
set style line 1 lt 3 lc 3 \n\
set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();

 // outfile.close();
  Simulator::Destroy ();
  return 0;




}


  



