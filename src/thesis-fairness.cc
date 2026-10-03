#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ThesisBaseline");

int
main(int argc, char* argv[])
{
    // ---------------------------------------------------------
    // 1. Create three wireless nodes
    // ---------------------------------------------------------
    NodeContainer nodes;
    nodes.Create(3);

    // ---------------------------------------------------------
    // 2. Configure IEEE 802.11 ad hoc Wi-Fi
    // ---------------------------------------------------------
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);

    YansWifiChannelHelper channel;

channel.SetPropagationDelay(
    "ns3::ConstantSpeedPropagationDelayModel");

channel.AddPropagationLoss(
    "ns3::RangePropagationLossModel",
    "MaxRange",
    DoubleValue(60.0));

    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());

    WifiMacHelper mac;
    mac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(phy, mac, nodes);

    // ---------------------------------------------------------
    // 3. Position the nodes in a straight line
    // ---------------------------------------------------------
    MobilityHelper mobility;

    Ptr<ListPositionAllocator> positionAlloc =
        CreateObject<ListPositionAllocator>();

    positionAlloc->Add(Vector(0.0, 0.0, 0.0));      // Node 0
    positionAlloc->Add(Vector(50.0, 0.0, 0.0));     // Node 1
    positionAlloc->Add(Vector(100.0, 0.0, 0.0));    // Node 2

    mobility.SetPositionAllocator(positionAlloc);

    mobility.SetMobilityModel(
        "ns3::ConstantPositionMobilityModel");

    mobility.Install(nodes);

    // ---------------------------------------------------------
    // 4. Install AODV routing and Internet stack
    // ---------------------------------------------------------
    AodvHelper aodv;

    InternetStackHelper internet;
    internet.SetRoutingHelper(aodv);
    internet.Install(nodes);

    // ---------------------------------------------------------
    // 5. Assign IPv4 addresses
    // ---------------------------------------------------------
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");

    Ipv4InterfaceContainer interfaces =
        address.Assign(devices);

    // ---------------------------------------------------------
    // 6. Create UDP receiver on Node 2
    // ---------------------------------------------------------
    uint16_t port = 9;

    UdpServerHelper server(port);

    ApplicationContainer serverApp =
        server.Install(nodes.Get(2));

    serverApp.Start(Seconds(1.0));
    serverApp.Stop(Seconds(20.0));

    // ---------------------------------------------------------
    // 7. Generate UDP traffic from Node 0 to Node 2
    // ---------------------------------------------------------
    UdpClientHelper client(interfaces.GetAddress(2), port);

    client.SetAttribute("MaxPackets", UintegerValue(100));
    client.SetAttribute("Interval", TimeValue(MilliSeconds(100)));
    client.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApp =
        client.Install(nodes.Get(0));

    clientApp.Start(Seconds(5.0));
    clientApp.Stop(Seconds(15.0));

    // ---------------------------------------------------------
    // 8. Run simulation
    // ---------------------------------------------------------
    AnimationInterface anim("thesis-animation.xml");

anim.UpdateNodeDescription(0, "Source");
anim.UpdateNodeDescription(1, "Relay");
anim.UpdateNodeDescription(2, "Destination");

    Simulator::Stop(Seconds(20.0));
    FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();
Simulator::Run();
monitor->CheckForLostPackets();

Ptr<Ipv4FlowClassifier> classifier =
    DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());

FlowMonitor::FlowStatsContainer stats =
    monitor->GetFlowStats();
std::ofstream csvFile("baseline-results.csv");

csvFile << "Flow,Source,Destination,TxPackets,RxPackets,LostPackets,PDR,AvgDelay_ms,Throughput_kbps\n";

for (auto const& flow : stats)
{
    Ipv4FlowClassifier::FiveTuple t =
        classifier->FindFlow(flow.first);
	// Process only the main UDP application flow
	if (t.sourceAddress != Ipv4Address("10.1.1.1") ||
    		t.destinationAddress != Ipv4Address("10.1.1.3"))
	{
    		continue;
	}

    std::cout << "\nFlow " << flow.first << std::endl;
    std::cout << "Source: " << t.sourceAddress << std::endl;
    std::cout << "Destination: " << t.destinationAddress << std::endl;
    std::cout << "Tx Packets: " << flow.second.txPackets << std::endl;
    std::cout << "Rx Packets: " << flow.second.rxPackets << std::endl;
    std::cout << "Lost Packets: " << flow.second.lostPackets << std::endl;

    if (flow.second.txPackets > 0)
    {
        double pdr =
            100.0 * flow.second.rxPackets / flow.second.txPackets;

        std::cout << "PDR: " << pdr << " %" << std::endl;
    }

    if (flow.second.rxPackets > 0)
    {
        double avgDelay =
            flow.second.delaySum.GetSeconds() /
            flow.second.rxPackets;

        double duration =
            flow.second.timeLastRxPacket.GetSeconds() -
            flow.second.timeFirstTxPacket.GetSeconds();

        double throughput = 0.0;

        if (duration > 0)
        {
            throughput =
                (flow.second.rxBytes * 8.0) /
                duration / 1000.0;
        }

        std::cout << "Average Delay: "
                  << avgDelay * 1000                  << " ms" << std::endl;

        std::cout << "Throughput: "
                  << throughput
                  << " kbps" << std::endl;
	double pdr =
    		100.0 * flow.second.rxPackets / flow.second.txPackets;

	csvFile << flow.first << ","
        	<< t.sourceAddress << ","
        	<< t.destinationAddress << ","
        	<< flow.second.txPackets << ","
        	<< flow.second.rxPackets << ","
        	<< flow.second.lostPackets << ","
        	<< pdr << ","
        	<< avgDelay * 1000 << ","
        	<< throughput << "\n";
    }
}


    // ---------------------------------------------------------
    // 9. Print received packets
    // ---------------------------------------------------------
    Ptr<UdpServer> serverPtr =
        DynamicCast<UdpServer>(serverApp.Get(0));

    std::cout << "\n===== Thesis Baseline Results =====" << std::endl;
    std::cout << "Source: Node 0" << std::endl;
    std::cout << "Relay: Node 1" << std::endl;
    std::cout << "Destination: Node 2" << std::endl;
    std::cout << "Packets received: "
              << serverPtr->GetReceived() << std::endl;
    std::cout << "===================================" << std::endl;

	csvFile.close();

    Simulator::Destroy();

    return 0;
}
