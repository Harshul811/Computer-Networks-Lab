#include <fstream>
#include <cstdlib>
#include <string.h>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/gnuplot.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"

typedef uint32_t uint;

using namespace ns3;

#define ERROR 0.000001

NS_LOG_COMPONENT_DEFINE ("main1");


int main (int argc, char *argv[]){

    uint32_t maxBytes = 0;
	uint32_t port;
	uint32_t packetsize = 1024;
	uint32_t run_time = 1;
	uint32_t for_loop = 1;
	uint32_t offset=0;
	uint32_t pack_inc = 100;

	bool simultaneously = false;
    std::string protocol = "TcpHighSpeed";
	CommandLine cmd;
    cmd.AddValue ("maxBytes", "number of bytes", maxBytes);
    cmd.AddValue ("packetsize", "packetsize", packetsize);
    cmd.AddValue ("protocol", "prtocol TcpHighSpeed, TcpScalable, TcpVegas", protocol);
    cmd.AddValue ("for_loop", "no of for loop runs", for_loop);
    cmd.AddValue ("run_time", "run_time in", run_time);
    cmd.AddValue ("simultaneously", "whether both H1,H2 will run simultaneously", simultaneously);
    cmd.AddValue ("offset", "gap", offset);
	cmd.AddValue ("pack_inc", "increment in packet size", pack_inc);
    cmd.Parse (argc, argv);

    std::cout<<"Command line Arguments:\n";		
	std::cout<<"packetsize: "<<packetsize<<"\n";
	std::cout<<"protocol: "<<protocol<<"\n";
	std::cout<<"run time: "<<run_time<<"\n";
	std::cout<<"for_loop: "<<for_loop<<"\n";
	std::cout<<"Offset: "<<offset<<"\n";
	std::cout<<"Packet Size Increment: "<<pack_inc<<"\n";
	std::cout<<"simultaneously: "<<simultaneously<<"\n"<<std::endl;
	std::cout<<std::endl;

    if (protocol == "TcpScalable"){
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpScalable::GetTypeId ()));
    }else if (protocol == "TcpVegas"){
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpVegas::GetTypeId ()));
    }else{
        Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpHighSpeed::GetTypeId ()));
    }

	//Dataset for gnuplot 

    Gnuplot2dDataset dataset_udp;
	Gnuplot2dDataset dataset_tcp;
    Gnuplot2dDataset dataset_udp_delay;
	Gnuplot2dDataset dataset_tcp_delay;

	for(uint i=0;i<for_loop; i++){		
		uint32_t udpPacketSize= packetsize+pack_inc*i;  
		uint32_t tcpPacketSize= packetsize+pack_inc*i;  

		NodeContainer nodes;
		nodes.Create (6);
		NodeContainer n0n2 = NodeContainer (nodes.Get (0), nodes.Get (2));
		NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
		NodeContainer n2n3 = NodeContainer (nodes.Get (2), nodes.Get (3));
		NodeContainer n3n4 = NodeContainer (nodes.Get (3), nodes.Get (4));
		NodeContainer n3n5 = NodeContainer (nodes.Get (3), nodes.Get (5));


		// installing internet stack in all nodes
		InternetStackHelper internet;
		internet.Install (nodes);

		//point to point helper is used to create p2p links between nodes
		PointToPointHelper p2p;
		
		//router to host links
		p2p.SetDeviceAttribute ("DataRate", StringValue ("80Mbps"));
		p2p.SetChannelAttribute ("Delay", StringValue ("20ms"));
	    // p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", QueueSizeValue (QueueSize (queueSizeHR2)));
		p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("375000B"));


		NetDeviceContainer d0d2 = p2p.Install (n0n2);
		NetDeviceContainer d1d2 = p2p.Install (n1n2);
		NetDeviceContainer d3d4 = p2p.Install (n3n4);
		NetDeviceContainer d3d5 = p2p.Install (n3n5);

		//router to router link
		p2p.SetDeviceAttribute ("DataRate", StringValue ("30Mbps"));
		p2p.SetChannelAttribute ("Delay", StringValue ("100ms"));
	    //p2p.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue(queueSizeRR));
	    // p2p.SetQueue ("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize (queueSizeRR2)));
		p2p.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("200000B")); 
		NetDeviceContainer d2d3 = p2p.Install (n2n3);

		// //error model
		Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
	    em->SetAttribute ("ErrorRate", DoubleValue (ERROR));
	    d3d4.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));
	    d3d5.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

		//assigning IP to the netdevice containers having 2 nodes each
		Ipv4AddressHelper ipv4;
		ipv4.SetBase ("10.1.0.0", "255.255.255.0");
		Ipv4InterfaceContainer i0i2 = ipv4.Assign (d0d2);
		
		ipv4.SetBase ("10.1.1.0", "255.255.255.0");
		Ipv4InterfaceContainer i1i2 = ipv4.Assign (d1d2);

		ipv4.SetBase ("10.1.3.0", "255.255.255.0");
		Ipv4InterfaceContainer i2i3 = ipv4.Assign (d2d3);

		ipv4.SetBase ("10.1.4.0", "255.255.255.0");
		Ipv4InterfaceContainer i3i4 = ipv4.Assign (d3d4);

		ipv4.SetBase ("10.1.5.0", "255.255.255.0");
		Ipv4InterfaceContainer i3i5 = ipv4.Assign (d3d5);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		if(i==0){
			std::cout << "Assigned IPs to Receivers:" << std::endl;
			std::cout<<"H3: "<<i3i4.GetAddress(1)<<std::endl;
			std::cout<<"H4: "<<i3i5.GetAddress(1)<<std::endl;
			std::cout << "Assigned IPs to Senders:" << std::endl;
			std::cout<<"H1: "<< i0i2.GetAddress(0)<<std::endl;
			std::cout<<"H2: "<< i1i2.GetAddress(0)<<std::endl;
			std::cout << "Assigned IPs to Router:" << std::endl;
			std::cout<<"R2<--H3: "<<i3i4.GetAddress(0)<<std::endl;
			std::cout<<"R2<--H4: "<<i3i5.GetAddress(0)<<std::endl;
			std::cout<<"R1<--H1: "<< i0i2.GetAddress(1)<<std::endl;
			std::cout<<"R1<--H2: "<< i1i2.GetAddress(1)<<std::endl;
			std::cout<<"R1<--R2: "<< i2i3.GetAddress(0)<<std::endl;
			std::cout<<"R1-->R2: "<< i2i3.GetAddress(1)<<std::endl;
			std::cout<<std::endl;
		}
		Ipv4GlobalRoutingHelper g;
		Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("routing.routes", std::ios::out);
		g.PrintRoutingTableAllAt (Seconds (2), routingStream);

		port = 9;  

		// on off helper is for CBR traffic, we tell INET socket address here that receiver is HOST-3
		OnOffHelper onoff ("ns3::UdpSocketFactory", Address (InetSocketAddress (i3i4.GetAddress (1), port)));
		// onoff.SetConstantRate (DataRate ("10000kb/s"),udpPacketSize);
	    onoff.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));

		
		//install the on off app on HOST-1 and run for 1-5 seconds
		ApplicationContainer udp_apps_s = onoff.Install (n0n2.Get (0));
		
		//runtime =  (total time of simulation in multiple of 10 for given packet size) 
		//i = for loop counter(packet size is increasing after every loop iteration)
		if(simultaneously==false){
			udp_apps_s.Start (Seconds ( (0.0+(10*i))*run_time  ) );
			udp_apps_s.Stop (Seconds ((5.0+(10* i))*run_time) );			
		}else{
			udp_apps_s.Start (Seconds ( (0.0+(10*i))*run_time  ) );
			udp_apps_s.Stop (Seconds ((10.0+(10*i))*run_time) );
		}

		PacketSinkHelper sink_udp ("ns3::UdpSocketFactory",Address (InetSocketAddress (Ipv4Address::GetAny (), port)));

		ApplicationContainer udp_apps_d = sink_udp.Install (n3n4.Get (1));

		if(simultaneously==false){
			udp_apps_d.Start (Seconds ((0.0+(10*i))*run_time) );
			udp_apps_d.Stop (Seconds ((5.0+(10*i))*run_time) );			
		}else{
			udp_apps_d.Start (Seconds ((0.0+(10*i))*run_time) );
			udp_apps_d.Stop (Seconds ((10.0+(10*i))*run_time) );			
		}

	    port = 12344;
	    BulkSendHelper source ("ns3::TcpSocketFactory",InetSocketAddress (i3i5.GetAddress (1), port));
	    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
		source.SetAttribute ("SendSize", UintegerValue (tcpPacketSize));
	    ApplicationContainer tcp_apps_s = source.Install (n1n2.Get (0));
	    
	    if(simultaneously==false){
			tcp_apps_s.Start (Seconds ((5.0+(10*i))*run_time) );
	    	tcp_apps_s.Stop (Seconds ((10.0+(10*i))*run_time) );
		}else{
			tcp_apps_s.Start (Seconds ((0.0+(10*i))*run_time) );
	    	tcp_apps_s.Stop (Seconds ((10.0+(10*i))*run_time) );	
		}

	    PacketSinkHelper sink_tcp ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (), port));
	    ApplicationContainer tcp_apps_d = sink_tcp.Install (n3n5.Get (1));

	    if(simultaneously==false){
			tcp_apps_d.Start (Seconds ((5.0+(10*i))*run_time) );
	    	tcp_apps_d.Stop (Seconds ((10.0+(10*i))*run_time) );
		}else{
			tcp_apps_d.Start (Seconds ((0.0+(10*i))*run_time+offset ));
	    	tcp_apps_d.Stop (Seconds ((10.0+(10*i))*run_time+offset) );	
		}
	    
		Ptr<FlowMonitor> flowmon;
		FlowMonitorHelper flowmonHelper;
		flowmon = flowmonHelper.InstallAll();
		if(!simultaneously)	Simulator::Stop(Seconds((10+(10*i))*run_time) );
		else	Simulator::Stop(Seconds((10+(10*i))*run_time+offset) );

		Simulator::Run();
		flowmon->CheckForLostPackets();

		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmonHelper.GetClassifier());
		std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats();
		
		double throughput_udp;
		double throughput_tcp;
		double delay_udp;
		double delay_tcp;
		
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); i++) {
		
			Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
			std::cout<<"Flow: "<<i->first<<"\n";
			std::cout<<"Source: "<<t.sourceAddress<<"\n";
			std::cout<<"Destination: "<<t.destinationAddress<<"\n";

			if(t.sourceAddress == "10.1.0.1") {
				throughput_udp = (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / (double)1000;
				delay_udp = (double)i->second.delaySum.GetSeconds()/(i->second.rxPackets) ;

				dataset_udp.Add (udpPacketSize,throughput_udp);
				dataset_udp_delay.Add (udpPacketSize,delay_udp);

				std::cout << "UDP Flow over CBR " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
				std::cout << "Tx Packets: " << i->second.txPackets << "\n";
				std::cout << "Tx Bytes:" << i->second.txBytes << "\n";
				std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
				std::cout << "Rx Bytes:" << i->second.rxBytes << "\n";
				std::cout << "Net Packet Lost: " << i->second.lostPackets << "\n";
				std::cout << "Lost due to droppackets: " << i->second.packetsDropped.size() << "\n";
				std::cout << "Total Delay(in seconds): " << i->second.delaySum.GetSeconds() << std::endl;
				std::cout << "Mean Delay(in seconds): " << (double)i->second.delaySum.GetSeconds()/(i->second.rxPackets) << std::endl;
				std::cout << "Offered Load: " << (double)i->second.txBytes * 8.0 / (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / (double)1000 << " Kbps" << std::endl;
				std::cout << "Throughput: " << (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / (double)1000 << " Kbps" << std::endl;
				std::cout << "Mean jitter:" << (double)i->second.jitterSum.GetSeconds () / (i->second.rxPackets - 1) << std::endl;
				std::cout<<std::endl;

			}else if(t.sourceAddress == "10.1.1.1") {
				throughput_tcp = (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / 1000;
				delay_tcp = (double)i->second.delaySum.GetSeconds()/(i->second.rxPackets);
				
				dataset_tcp.Add (tcpPacketSize,throughput_tcp);
				dataset_tcp_delay.Add(tcpPacketSize,delay_tcp);

				std::cout << protocol <<" Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
				std::cout << "Tx Packets: " << i->second.txPackets << "\n";
				std::cout << "Tx Bytes:" << i->second.txBytes << "\n";
				std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
				std::cout << "Rx Bytes:" << i->second.rxBytes << "\n";
				std::cout << "Net Packet Lost: " << i->second.lostPackets << "\n";
				std::cout << "Lost due to droppackets: " << i->second.packetsDropped.size() << "\n";
				std::cout << "Total Delay(in seconds): " << i->second.delaySum.GetSeconds() << std::endl;
				std::cout << "Mean Delay(in secinds): " << (double)i->second.delaySum.GetSeconds()/(i->second.rxPackets) << std::endl;
				std::cout << "Offered Load: " << (double)i->second.txBytes * 8.0 / (i->second.timeLastTxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ()) / (double)1000 << " Kbps" << std::endl;
				std::cout << "Throughput: " << (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstRxPacket.GetSeconds ()) / (double)1000 << " Kbps" << std::endl;
				std::cout << "Mean jitter:" << (double)i->second.jitterSum.GetSeconds () / (i->second.rxPackets - 1) << std::endl;
				std::cout<<std::endl;
			}
		}

		std::cout<<"Run: "<<i<<" finished\n"<<std::endl;
		std::cout<<"**************************************************"<<std::endl;
		std::cout<<std::endl;
		
		Simulator::Destroy ();
	}

	std::string simultaneously_str="Seperate";
	if(simultaneously&&offset==0)
		simultaneously_str="Simultaneous_Same_Start";
	else if(simultaneously&&offset!=0)
		simultaneously_str="Simultaneous_Different_Start";
	std::string fileNameWithNoExtension = protocol+"_throughput_"+simultaneously_str;
	std::string graphicsFileName        = fileNameWithNoExtension + ".png";
	std::string plotFileName            = fileNameWithNoExtension + ".plt";
	std::string plotTitle               = protocol + " vs UDP throughput";
	
	std::string fileNameWithNoExtension_delay = protocol+"_delay_"+simultaneously_str;
	std::string graphicsFileName_delay        = fileNameWithNoExtension_delay + ".png";
	std::string plotFileName_delay            = fileNameWithNoExtension_delay + ".plt";
	std::string plotTitle_delay               = protocol + " vs UDP delay";

	// Instantiate the plot and set its title.
	Gnuplot plot (graphicsFileName);
	Gnuplot plot_delay (graphicsFileName_delay);
	
	plot.SetTitle (plotTitle);
	plot_delay.SetTitle (plotTitle_delay);

	// Make the graphics file, which the plot file will create when it
	// is used with Gnuplot, be a png file
	plot.SetTerminal ("png");
	plot_delay.SetTerminal ("png");

	// Set the labels for each axis.
	plot.SetLegend ("Packet Size(in Bytes)", "Throughput Values(in Kbps)");
	plot_delay.SetLegend ("Packet Size(in Bytes)", "Delay(in s)");

	dataset_tcp.SetTitle ("Throughput FTP over TCP");
	dataset_tcp.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	// dataset_tcp.SetExtra ("lw 2");
	dataset_udp.SetTitle ("Throughput CBR over UDP");
	dataset_udp.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	// dataset_udp.SetExtra ("lw 2");
	
	dataset_tcp_delay.SetTitle ("Delay FTP over TCP");
	dataset_tcp_delay.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	// dataset_tcp_delay.SetExtra ("lw 2");
	dataset_udp_delay.SetTitle ("Delay CBR over UDP");
	dataset_udp_delay.SetStyle (Gnuplot2dDataset::LINES_POINTS);
	// dataset_udp_delay.SetExtra ("lw 2");

	plot.AddDataset (dataset_tcp);
	plot.AddDataset (dataset_udp);
	
	plot_delay.AddDataset (dataset_udp_delay);
	plot_delay.AddDataset (dataset_tcp_delay);

	std::ofstream plotFile (plotFileName.c_str());

	plot.GenerateOutput (plotFile);
	plotFile.close ();

	std::ofstream plotFile_delay (plotFileName_delay.c_str());

	plot_delay.GenerateOutput (plotFile_delay);
	plotFile_delay.close ();

	return 0;
}	