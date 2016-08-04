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
#include <ns3/waveform-generator-helper.h>
#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/non-communicating-net-device.h>
#include <ns3/microwave-oven-spectrum-value-helper.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/waveform-generator.h>
#include <ns3/spectrum-analyzer.h>
#include <ns3/mobility-module.h>
using namespace ns3;



static bool g_verbose = false;
static uint32_t g_received = 0;

static void DataIndication (McpsDataIndicationParams params, Ptr<Packet> p){
 g_received++ ;
 
 NS_LOG_UNCOND ("Received packet at node ,total packets"<<g_received);
  

}



void
PhyTxStartTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << context << " PHY TX START p: " << p << std::endl;
    }
}


void
PhyTxEndTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << context << " PHY TX END p: " << p << std::endl;
    }
}

void
PhyRxStartTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << context << " PHY RX START p:" << p << std::endl;
    }
}

void
PhyRxEndOkTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << context << " PHY RX END OK p:" << p << std::endl;
    }
}

void
PhyRxEndErrorTrace (std::string context, Ptr<const Packet> p)
{
  if (g_verbose)
    {
      std::cout << context << " PHY RX END ERROR p:" << p << std::endl;
    }
}







int main (int argc, char** argv)
{
  CommandLine cmd;
  cmd.AddValue ("verbose", "Print trace information if true", g_verbose);
  cmd.Parse (argc, argv);

std::ostringstream os;
  std::ofstream berfile ("cross_tech.plt");
  Gnuplot psrplot = Gnuplot ("cross_tech.eps");
  Gnuplot2dDataset psrdataset ("cross_tech");
//  std::ofstream outfile;
  //   outfile.open ("logshadow.dat", std::ofstream::out | std::ofstream::app); 
    
  int maxPackets =500 ;
  int packetSize = 50;
  



  NodeContainer lrwpanNodes;
  NodeContainer waveformGeneratorNodes;
  NodeContainer spectrumAnalyzerNodes;
  NodeContainer allNodes;

  lrwpanNodes.Create (2);
  waveformGeneratorNodes.Create (1);
  spectrumAnalyzerNodes.Create (1);
  allNodes.Add (lrwpanNodes);
  allNodes.Add (waveformGeneratorNodes);
  allNodes.Add (spectrumAnalyzerNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
  nodePositionList->Add (Vector (5.0, 0.0, 0.0));  // TX node
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // RX node
  nodePositionList->Add (Vector (30.0, 0.0, 0.0)); // Microwave Oven
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // Spectrum Analyzer
  mobility.SetPositionAllocator (nodePositionList);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (allNodes);


  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
  channelHelper.SetChannel ("ns3::MultiModelSpectrumChannel");
  Ptr<SpectrumChannel> channel = channelHelper.Create ();


///////Lrwpan nodes ///////

 double txPower = 0;
  uint32_t channelNumber = 23;
  LrWpanHelper helper;
  helper.SetChannel(channel);

  NetDeviceContainer devices;
  devices=helper.Install(lrwpanNodes);
 LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower, channelNumber);


  char vec[]="00:";
  
  for (int i=0;i<(int)lrwpanNodes.GetN();i++){
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
     lrdev[0]->GetPhy ()->SetTxPowerSpectralDensity (psd);
   lrdev[1]->GetPhy ()->SetTxPowerSpectralDensity (psd);
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
 for (int j =1; j < (int)lrwpanNodes.GetN();j++  )
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

      psrdataset.Add ( lrwpanNodes.GetN(),g_received/(50.0*(float)(lrwpanNodes.GetN()-1))); 


 /////////////////////////////////
  // Configure waveform generator
  /////////////////////////////////

  Ptr<SpectrumValue> mwoPsd =  MicrowaveOvenSpectrumValueHelper::CreatePowerSpectralDensityMwo2 ();
 // NS_LOG_INFO ("mwoPsd : " << *mwoPsd);

  WaveformGeneratorHelper waveformGeneratorHelper;
  waveformGeneratorHelper.SetChannel (channel);
  waveformGeneratorHelper.SetTxPowerSpectralDensity (mwoPsd);

  waveformGeneratorHelper.SetPhyAttribute ("Period", TimeValue (Seconds (1.0 / 60)));   // corresponds to 60 Hz
  waveformGeneratorHelper.SetPhyAttribute ("DutyCycle", DoubleValue (0.5));
  NetDeviceContainer waveformGeneratorDevices = waveformGeneratorHelper.Install (waveformGeneratorNodes);

  Simulator::Schedule (Seconds (0.1), &WaveformGenerator::Start,
                       waveformGeneratorDevices.Get (0)->GetObject<NonCommunicatingNetDevice> ()->GetPhy ()->GetObject<WaveformGenerator> ());
  


  /////////////////////////////////
  // Configure spectrum analyzer
  /////////////////////////////////


  SpectrumAnalyzerHelper spectrumAnalyzerHelper;
  spectrumAnalyzerHelper.SetChannel (channel);
  spectrumAnalyzerHelper.SetRxSpectrumModel (SpectrumModelIsm2400MhzRes1Mhz);
  spectrumAnalyzerHelper.SetPhyAttribute ("Resolution", TimeValue (MilliSeconds (2)));
  spectrumAnalyzerHelper.SetPhyAttribute ("NoisePowerSpectralDensity", DoubleValue (1e-15));  // -120 dBm/Hz
  spectrumAnalyzerHelper.EnableAsciiAll ("spectrum-analyzer-output");
  NetDeviceContainer spectrumAnalyzerDevices = spectrumAnalyzerHelper.Install (spectrumAnalyzerNodes);

  /*
    you can get a nice plot of the output of SpectrumAnalyzer with this gnuplot script:

    unset surface
    set pm3d at s 
    set palette
    set key off
    set view 50,50
    set xlabel "time (ms)"
    set ylabel "freq (MHz)"
    set zlabel "PSD (dBW/Hz)" offset 15,0,0
    splot "./spectrum-analyzer-output-3-0.tr" using ($1*1000.0):($2/1e6):(10*log10($3))
  */





  Config::Connect ("/NodeList/*/DeviceList/*/Phy/TxStart", MakeCallback (&PhyTxStartTrace));
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/TxEnd", MakeCallback (&PhyTxEndTrace));
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/RxStart", MakeCallback (&PhyRxStartTrace));
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/RxEndOk", MakeCallback (&PhyRxEndOkTrace));
  Config::Connect ("/NodeList/*/DeviceList/*/Phy/RxEndError", MakeCallback (&PhyRxEndErrorTrace));




  Simulator::Stop (Seconds (50.0));

  Simulator::Run ();

  Simulator::Destroy ();





}



















