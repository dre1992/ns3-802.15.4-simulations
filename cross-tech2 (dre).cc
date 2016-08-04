/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 CTTC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */



#include <iostream>
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
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/spectrum-model-ism2400MHz-res1MHz.h>
#include <ns3/spectrum-model-300kHz-300GHz-log.h>
#include <ns3/wifi-spectrum-value-helper.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/waveform-generator.h>
#include <ns3/spectrum-analyzer.h>
#include <ns3/log.h>
#include <string>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/mobility-module.h>
#include <ns3/spectrum-helper.h>
#include <ns3/applications-module.h>
#include <ns3/adhoc-aloha-noack-ideal-phy-helper.h>
#include <ns3/waveform-generator-helper.h>
#include <ns3/spectrum-analyzer-helper.h>
#include <ns3/non-communicating-net-device.h>
#include <ns3/microwave-oven-spectrum-value-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("OfdmWithWaveformGenerator");

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


void
ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  uint64_t bytes = 0;
  while ((packet = socket->Recv ()))
    {
      bytes += packet->GetSize ();
    }
  if (g_verbose)
    {
      std::cout << "SOCKET received " << bytes << " bytes" << std::endl;
    }
}

Ptr<Socket>
SetupPacketReceive (Ptr<Node> node)
{
  TypeId tid = TypeId::LookupByName ("ns3::PacketSocketFactory");
  Ptr<Socket> sink = Socket::CreateSocket (node, tid);
  sink->Bind ();
  sink->SetRecvCallback (MakeCallback (&ReceivePacket));
  return sink;
}

int main (int argc, char** argv)
{
  CommandLine cmd;
  cmd.AddValue ("verbose", "Print trace information if true", g_verbose);
  cmd.Parse (argc, argv);


std::ostringstream os;
  std::ofstream berfile ("cross_tech2.plt");
  Gnuplot psrplot = Gnuplot ("cross_tech2.eps");
  Gnuplot2dDataset psrdataset ("cross_tech2");
//  std::ofstream outfile;
  //   outfile.open ("logshadow.dat", std::ofstream::out | std::ofstream::app); 
    
  int maxPackets =2000 ;
  int packetSize = 100;

  
  NodeContainer ofdmNodes;
  NodeContainer lrwpanNodes;
  NodeContainer spectrumAnalyzerNodes;
  NodeContainer allNodes;

  ofdmNodes.Create (2);
  lrwpanNodes.Create (2);
  spectrumAnalyzerNodes.Create (1);
  allNodes.Add (ofdmNodes);
  allNodes.Add (lrwpanNodes);
  allNodes.Add (spectrumAnalyzerNodes);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> nodePositionList = CreateObject<ListPositionAllocator> ();
  nodePositionList->Add (Vector (10.0, 0.0, 0.0));  // WLPAN node
  nodePositionList->Add (Vector (6.0, 0.0, 0.0));  // WLPAN node
  nodePositionList->Add (Vector (0.0, 0.0, 0.0));  // TX node
  nodePositionList->Add (Vector (5.0, 0.0, 0.0)); // RX node
  nodePositionList->Add (Vector (1.0, 0.0, 0.0));  // Spectrum Analyzer
  mobility.SetPositionAllocator (nodePositionList);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (allNodes);


  SpectrumChannelHelper channelHelper = SpectrumChannelHelper::Default ();
  channelHelper.SetChannel ("ns3::MultiModelSpectrumChannel");
  Ptr<SpectrumChannel> channel = channelHelper.Create ();


  /////////////////////////
  // Configure ofdm nodes
  ////////////////////////

  WifiSpectrumValue5MhzFactory sf;

  double txPower1 = 0.1; // Watts
  uint32_t channelNumber1 = 1;
  Ptr<SpectrumValue> txPsd =  sf.CreateTxPowerSpectralDensity (txPower1, channelNumber1);

  // for the noise, we use the Power Spectral Density of thermal noise
  // at room temperature. The value of the PSD will be constant over the band of interest.
  const double k = 1.381e-23; //Boltzmann's constant
  const double T = 290; // temperature in Kelvin
  double noisePsdValue = k * T; // watts per hertz
  Ptr<SpectrumValue> noisePsd = sf.CreateConstant (noisePsdValue);


  AdhocAlohaNoackIdealPhyHelper adhocAlohaOfdmHelper;
  adhocAlohaOfdmHelper.SetChannel (channel);
  adhocAlohaOfdmHelper.SetTxPowerSpectralDensity (txPsd);
  adhocAlohaOfdmHelper.SetNoisePowerSpectralDensity (noisePsd);
  adhocAlohaOfdmHelper.SetPhyAttribute ("Rate", DataRateValue (DataRate ("1Mbps")));
  NetDeviceContainer ofdmDevices = adhocAlohaOfdmHelper.Install (ofdmNodes);

  PacketSocketHelper packetSocket;
  packetSocket.Install (ofdmNodes);

  PacketSocketAddress socket;
  socket.SetSingleDevice (ofdmDevices.Get (0)->GetIfIndex ());
  socket.SetPhysicalAddress (ofdmDevices.Get (1)->GetAddress ());
  socket.SetProtocol (1);

  OnOffHelper onoff ("ns3::PacketSocketFactory", Address (socket));
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.04]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ExponentialRandomVariable[Mean=0.01]"));
  onoff.SetAttribute ("DataRate", DataRateValue (DataRate ("0.4Mbps")));
  onoff.SetAttribute ("PacketSize", UintegerValue (1500));

  ApplicationContainer apps = onoff.Install (ofdmNodes.Get (0));
  apps.Start (Seconds (0.0));
  apps.Stop (Seconds (50));

  Ptr<Socket> recvSink = SetupPacketReceive (ofdmNodes.Get (1));





  /////////////////////////////////
  // Configure Lrwpan
  /////////////////////////////////

double txPower2 = 0;
  uint32_t channelNumber2 = 11;
  LrWpanHelper helper;
  helper.SetChannel(channel);

  NetDeviceContainer devices;
  devices=helper.Install(lrwpanNodes);
 LrWpanSpectrumValueHelper svh;
  Ptr<SpectrumValue> psd = svh.CreateTxPowerSpectralDensity (txPower2, channelNumber2);


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

      psrdataset.Add ( lrwpanNodes.GetN(),g_received/(2000.0*(float)(lrwpanNodes.GetN()-1)));
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


