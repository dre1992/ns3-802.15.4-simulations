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

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){
 g_received++ ;
 
 //NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
  

}


static void StateChangeNotification (std::string context, Time now, LrWpanPhyEnumeration oldState, LrWpanPhyEnumeration newState)
{
 /* NS_LOG_UNCOND (context << " state change at " << now.GetSeconds ()
                         << " from " << LrWpanHelper::LrWpanPhyEnumerationPrinter (oldState)
                         << " to " << LrWpanHelper::LrWpanPhyEnumerationPrinter (newState));*/
}
static void PhyTxStart (std::string context,Ptr<const Packet> packet){
 
// NS_LOG_UNCOND("TX start time ="<<Simulator::Now ().GetSeconds ());
 //outFile<<context<<"TX start time = "<<Simulator::Now ().GetSeconds ()<<"\t";
}

static void PhyTxEnd (std::string context,Ptr<const Packet> packet){
 
 //NS_LOG_UNCOND("TX end time ="<<Simulator::Now ().GetSeconds ());
 //outFile<<context<<"TX end time = "<<Simulator::Now ().GetSeconds ()<<std::endl;
}




NS_LOG_COMPONENT_DEFINE ("parameters_check");

int main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  std::ostringstream os;
  std::ofstream berfile ("parameters-20nodes-50ps-maxframeret.plt");
  Gnuplot psrplot = Gnuplot ("parameters-20nodes-50ps-maxframeret.eps");
  Gnuplot2dDataset psrdataset ("parameters-20nodes-50ps-maxframeret");
  
  std::ofstream outfile("parameters-20nodes-50ps-maxframeret.dat"); 
 
  os<<"Packet Reception Rate for maxframeret 0-7 ,10nodes,packetsize 100,singlemodel,no prop,50p/s";
  int maxPackets = 500;
  int packetSize = 100;
  //  LogComponentEnable ("LrWpanCsmaCa", LOG_LEVEL_INFO);
  NodeContainer nodes;
  nodes.Create (20);

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
  stringstream ss;
  std::ostringstream oss;
  std::ostringstream ozz;
  for(int i=0;i<(int)nodes.GetN();i++){
  	ss<<i;
  	string str=ss.str();
  	string str1="phy";
   	string res=str1+str;
  	lrdev[i]->GetPhy ()->TraceConnect ("TrxState", std::string (res), MakeCallback (&StateChangeNotification));
  	res="";
  	ss.str(""); 
  }
 oss <<"/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyTxBegin" ;

 ozz <<"/NodeList/*/DeviceList/*/$ns3::LrWpanNetDevice/Phy/PhyTxEnd" ;

 Config::Connect (oss.str (), MakeCallback (&PhyTxStart ));
 Config::Connect (ozz.str (), MakeCallback (&PhyTxEnd ));

 
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
 
 double mean = 0.02;
 double bound = 0.0;
 Ptr<ExponentialRandomVariable> x = CreateObject<ExponentialRandomVariable> ();
 x->SetAttribute ("Mean", DoubleValue (mean));
 x->SetAttribute ("Bound", DoubleValue (bound));
 double value=0;

 Ptr<Packet> p;

 for(int z=0;z<=7;z++){
     for(int l=0;l<(int)nodes.GetN();l++){
          // lrdev[l]->GetCsmaCa()->SetMacMaxCSMABackoffs(z); 
           lrdev[l]->GetCsmaCa()->SetMacMinBE(z); 
           //lrdev[l]->GetCsmaCa()->SetMacMaxBE(8);
        // std::cout<<" minbe  "<< (int) lrdev[l]->GetCsmaCa()->GetMacMinBE()<<"\t";
         //std::cout<< " maxbe "<<(int) lrdev[l]->GetCsmaCa()->GetMacMaxBE()<<std::endl;   
          lrdev[l]->GetMac()->SetMacMaxFrameRetries(z);
      }

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
     // NS_LOG_UNCOND ("Received " << g_received << " packets for parameter "  );
      float temp=g_received/(500.0*(float)(nodes.GetN()-1));
      outfile<<z<<" "<<temp<<std::endl;
      psrdataset.Add (z, g_received/(500.0*(float)(nodes.GetN()-1))); 
      g_received = 0;

  }
  
 psrplot.AddDataset (psrdataset);

 psrplot.SetTitle (os.str ());
 psrplot.SetTerminal ("postscript eps color enh \"Times-BoldItalic\"");
 psrplot.SetLegend ("maxFrameRetries", "Packet Success Rate (PSR)");
 psrplot.SetExtra  ("set xrange [0:5]\n\
 set yrange [0:1]\n\
 set grid\n\
 set style line 1 linewidth 5\n\
 set style increment user");
 psrplot.GenerateOutput (berfile);
 berfile.close ();
 outfile.close();
 Simulator::Destroy ();
 return 0;
}


  



