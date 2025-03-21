/*
 * Copyright (c) 2017 Jadavpur University, India
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Manoj Kumar Rana <manoj24.rana@gmail.com>
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/lte-helper.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-helper.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 * It configures IPv6 addresses for UEs by setting the 48 bit prefix attribute in epc helper
 */

NS_LOG_COMPONENT_DEFINE("EpcFirstExampleForIpv6");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    Ptr<LteHelper> lteHelper = CreateObject<LteHelper>();
    Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper>();
    lteHelper->SetEpcHelper(epcHelper);

    Ptr<Node> pgw = epcHelper->GetPgwNode();

    // Create a single RemoteHost
    NodeContainer remoteHostContainer;
    remoteHostContainer.Create(1);
    Ptr<Node> remoteHost = remoteHostContainer.Get(0);
    InternetStackHelper internet;
    internet.Install(remoteHostContainer);

    // Create the Internet
    PointToPointHelper p2ph;
    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
    p2ph.SetDeviceAttribute("Mtu", UintegerValue(1500));
    p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0.010)));
    NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);

    NodeContainer ueNodes;
    NodeContainer enbNodes;
    enbNodes.Create(2);
    ueNodes.Create(2);

    // Install Mobility Model
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
    for (uint16_t i = 0; i < 2; i++)
    {
        positionAlloc->Add(Vector(60.0 * i, 0, 0));
    }
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.SetPositionAllocator(positionAlloc);
    mobility.Install(enbNodes);
    mobility.Install(ueNodes);

    // Install the IP stack on the UEs
    internet.Install(ueNodes);

    // Install LTE Devices to the nodes
    NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice(enbNodes);
    NetDeviceContainer ueLteDevs1 = lteHelper->InstallUeDevice(NodeContainer(ueNodes.Get(0)));
    NetDeviceContainer ueLteDevs2 = lteHelper->InstallUeDevice(NodeContainer(ueNodes.Get(1)));

    Ipv6AddressHelper ipv6h;
    ipv6h.SetBase(Ipv6Address("6001:db80::"), Ipv6Prefix(64));
    Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign(internetDevices);

    internetIpIfaces.SetForwarding(0, true);
    internetIpIfaces.SetDefaultRouteInAllNodes(0);

    Ipv6InterfaceContainer ueIpIface;

    // Assign IP address to the first UE
    ueIpIface = epcHelper->AssignUeIpv6Address(NetDeviceContainer(ueLteDevs1));

    Ipv6StaticRoutingHelper ipv6RoutingHelper;
    Ptr<Ipv6StaticRouting> remoteHostStaticRouting =
        ipv6RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv6>());
    remoteHostStaticRouting
        ->AddNetworkRouteTo("7777:f00d::", Ipv6Prefix(64), internetIpIfaces.GetAddress(0, 1), 1, 0);

    // Assign IP address to the second UE
    ueIpIface.Add(epcHelper->AssignUeIpv6Address(NetDeviceContainer(ueLteDevs2)));

    for (uint32_t u = 0; u < ueNodes.GetN(); ++u)
    {
        Ptr<Node> ueNode = ueNodes.Get(u);
        // Set the default gateway for the UEs
        Ptr<Ipv6StaticRouting> ueStaticRouting =
            ipv6RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv6>());
        ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress6(), 1);
    }

    // Attach one UE per eNodeB
    lteHelper->Attach(ueLteDevs1.Get(0), enbLteDevs.Get(0));
    lteHelper->Attach(ueLteDevs2.Get(0), enbLteDevs.Get(1));

    // interface 0 is localhost, 1 is the p2p device
    Ipv6Address remoteHostAddr = internetIpIfaces.GetAddress(1, 1);

    // Install and start applications on UEs and remote host

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(remoteHost);

    serverApps.Start(Seconds(4));
    serverApps.Stop(Seconds(20));

    UdpEchoClientHelper echoClient1(remoteHostAddr, 9);
    UdpEchoClientHelper echoClient2(remoteHostAddr, 9);

    echoClient1.SetAttribute("MaxPackets", UintegerValue(1000));
    echoClient1.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient1.SetAttribute("PacketSize", UintegerValue(1024));

    echoClient2.SetAttribute("MaxPackets", UintegerValue(1000));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps1 = echoClient1.Install(ueNodes.Get(0));
    ApplicationContainer clientApps2 = echoClient2.Install(ueNodes.Get(1));

    clientApps1.Start(Seconds(4));
    clientApps1.Stop(Seconds(14));

    clientApps2.Start(Seconds(4.5));
    clientApps2.Stop(Seconds(14.5));

    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_ALL);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_ALL);

    internet.EnablePcapIpv6("LenaIpv6-AddrConf-Ue0.pcap", ueNodes.Get(0)->GetId(), 1, true);
    internet.EnablePcapIpv6("LenaIpv6-AddrConf-Ue1.pcap", ueNodes.Get(1)->GetId(), 1, true);
    internet.EnablePcapIpv6("LenaIpv6-AddrConf-RH.pcap",
                            remoteHostContainer.Get(0)->GetId(),
                            1,
                            true);
    internet.EnablePcapIpv6("LenaIpv6-AddrConf-Pgw-Iface1.pcap", pgw->GetId(), 1, true);
    internet.EnablePcapIpv6("LenaIpv6-AddrConf-Pgw-Iface2.pcap", pgw->GetId(), 2, true);

    Simulator::Stop(Seconds(20));
    Simulator::Run();

    Simulator::Destroy();
    return 0;
}
