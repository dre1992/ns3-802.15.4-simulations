//experiment for mean service time

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

using namespace ns3;
using namespace std;
static uint32_t g_received = 0;
static double mst=0;
static double begin=0;
static double end=0;
static double mstot=0;
static Mac16Address addr="00:00";

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){

 if(params.m_srcAddr==addr){
   g_received++;
  //NS_LOG_UNCOND("address  "<<params. m_srcAddr);
//  NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
 }
}





static void MacTx (std::string context,Ptr<const Packet> packet){
 
// NS_LOG_UNCOND(context<<" TX start time = "<<Simulator::Now ().GetSeconds ());
   begin=Simulator::Now().GetSeconds();
 
}


static void MacTxOk (std::string context,Ptr<const Packet> packet){
 
  
 end=Simulator::Now().GetSeconds();
 mst=end-begin;
 mstot+=mst;
// NS_LOG_UNCOND(context<<"  rx end time = "<<Simulator::Now ().GetSeconds ()<<"   servicetime= "<<mst); 
 
}

/*

static void MacRxEnd (std::string context,Ptr<const Packet> packet){
    

  
 end=Simulator::Now().GetSeconds();
 mst=end-begin;
 mstot+=mst;
 NS_LOG_UNCOND(context<<"  rx end time = "<<Simulator::Now ().GetSeconds ()<<"   servicetime= "<<mst);
  
}
*/
NS_LOG_COMPONENT_DEFINE ("traffic_check");

int main (int argc, char *argv[])
{
int k=10;
int t=1;
CommandLine cmd;
cmd.AddValue("k","number of nodes",k);
cmd.AddValue("t","node currently viewed",t);
cmd.Parse (argc, argv);
  std::ostringstream os;
//  std::ofstream berfile ("testservice-10nodes-traffic.plt");
//  Gnuplot psrplot = Gnuplot ("testservice-10nodes-traffic.eps");
//  Gnuplot2dDataset psrdataset ("testservice-10nodes-traffic");
  std::ofstream outfile;
     outfile.open ("testservice-40nodes-40ps-traffic.dat", std::ofstream::out | std::ofstream::app); 
 int maxPackets = 500;
 int packetSize = 100;

   NodeContainer nodes;
   nodes.Create (k);
   os<<"mean service time for traffic, 10-40nodes,packetsize 100,singelmodel,noprop,20ps";
   LrWpanHelper helper;
   Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
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
  

  for(int i=0;i<(int)nodes.GetN();i++){
      
  Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> ();
  sender0Mobility->SetPosition (Vector (i,0,0));
  lrdev[i]->GetPhy ()->SetMobility (sender0Mobility);
}
  
  std::ostringstream oss;
   std::ostringstream ozz;
  std::ostringstream oll;
//for(int t=1;t<(int)nodes.GetN();t++){



// oss <<"/NodeList/"<<nodes.Get(i)->GetId()<<"/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyTxBegin" ;
//ozz <<"/NodeList/"<<nodes.Get(i)->GetId()<<"/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyTxEnd" ;
 //oll<<"/NodeList/0/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacRx"  ;
 oss <<"/NodeList/"<<nodes.Get(t)->GetId()<<"/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacTx" ;
  ozz <<"/NodeList/"<<nodes.Get(t)->GetId()<<"/DeviceList/*/$ns3::LrWpanNetDevice/Mac/MacTxOk" ;

//Config::Disconnect (oss.str (), MakeCallback (&MacTx ));
 //Config::Disconnect (ozz.str (), MakeCallback (&MacTxOk ));
 Config::Connect (oss.str (), MakeCallback (&MacTx ));
 Config::Connect (ozz.str (), MakeCallback (&MacTxOk ));
 //Config::Connect (oll.str (), MakeCallback (&MacRxEnd ));
oss.str("");
ozz.str("");
 addr=lrdev[t]->GetMac()->GetShortAddress();
 std::cout<<"CURRENT NODE  =  "<<addr;
   McpsDataIndicationCallback cb0;
  cb0 = MakeCallback (&DataIndication);
  lrdev[0]->GetMac ()->SetMcpsDataIndicationCallback (cb0);



  McpsDataRequestParams params;
  params.m_srcAddrMode = SHORT_ADDR;
  params.m_dstAddrMode = SHORT_ADDR;
  params.m_dstPanId = 0;
  params.m_dstAddr = Mac16Address ("00:01");
  params.m_msduHandle = 0;
  params.m_txOptions =0 ;
  double mean = 0.025;
  double bound = 0.0;
  Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
  x->SetAttribute ("Mean", DoubleValue (mean));
  x->SetAttribute ("Bound", DoubleValue (bound));
  double value=0;

  Ptr<Packet> p;
 
 // for(int z=1;z<=4;z++){
   //    mean=(float)(0.02/(4*z));
     //   x->SetAttribute ("Mean", DoubleValue (mean));
      //  x->SetAttribute ("Bound", DoubleValue (bound));
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
    //  NS_LOG_DEBUG ("Received " << g_received << " packets for parameter "  );
   //  psrdataset.Add (z, g_received/(500.0*(float)(nodes.GetN()-1))); 
    
      NS_LOG_DEBUG ("Received " << g_received << " packets from node  for parameter "  );
      double meanservice;
       meanservice=(double)( mstot/(double)g_received);
            NS_LOG_DEBUG ("mean service time "<<meanservice<<" for node traffic "<<t  );
  outfile<<k<<"\t"<<g_received<<"\t"<<meanservice<<std::endl;  
  // psrdataset.Add (t, meanservice); 
   /* if(z==1){
      outfile<<t<<"\t"<<meanservice<<"\t";
      g_received=0;
      mstot=0;
     }
     else if(z<4){
          outfile<<meanservice<<"\t";
           g_received=0;
            mstot=0;
     }
     else if (z==4){
           outfile<<meanservice<<std::endl;
           g_received=0;
           mstot=0;
     }  
*/

  //}


//}
/*
 psrplot.AddDataset (psrdataset);

  psrplot.SetTitle (os.str ());
  psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
  psrplot.SetLegend ("nodes", "Packet Success Rate (PSR)");
  psrplot.SetExtra  ("set xrange [0:50]\n\
set yrange [0:1]\n\
set grid\n\
set style line 1 linewidth 5\n\
set style increment user");
  psrplot.GenerateOutput (berfile);
  berfile.close ();
*/
  outfile.close();
  Simulator::Destroy ();
  return 0;




}


  



