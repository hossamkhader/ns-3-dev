/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Author: Faker Moatamri <faker.moatamri@sophia.inria.fr>
 */

#include "internet-stack-helper.h"

#include "ipv4-global-routing-helper.h"
#include "ipv4-list-routing-helper.h"
#include "ipv4-static-routing-helper.h"
#include "ipv6-static-routing-helper.h"

#include "ns3/arp-l3-protocol.h"
#include "ns3/assert.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/global-router-interface.h"
#include "ns3/icmpv6-l4-protocol.h"
#include "ns3/ipv4-global-routing.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6-extension-demux.h"
#include "ns3/ipv6-extension-header.h"
#include "ns3/ipv6-extension.h"
#include "ns3/ipv6.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node-list.h"
#include "ns3/node.h"
#include "ns3/object.h"
#include "ns3/packet-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/traffic-control-layer.h"

#include <limits>
#include <map>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("InternetStackHelper");

//
// Historically, the only context written to ascii traces was the protocol.
// Traces from the protocols include the interface, though.  It is not
// possible to really determine where an event originated without including
// this.  If you want the additional context information, define
// INTERFACE_CONTEXT.  If you want compatibility with the old-style traces
// comment it out.
//
#define INTERFACE_CONTEXT

//
// Things are going to work differently here with respect to trace file handling
// than in most places because the Tx and Rx trace sources we are interested in
// are going to multiplex receive and transmit callbacks for all Ipv4 and
// interface pairs through one callback.  We want packets to or from each
// distinct pair to go to an individual file, so we have got to demultiplex the
// Ipv4 and interface pair into a corresponding Ptr<PcapFileWrapper> at the
// callback.
//
// A complication in this situation is that the trace sources are hooked on
// a protocol basis.  There is no trace source hooked by an Ipv4 and interface
// pair.  This means that if we naively proceed to hook, say, a drop trace
// for a given Ipv4 with interface 0, and then hook for Ipv4 with interface 1
// we will hook the drop trace twice and get two callbacks per event.  What
// we need to do is to hook the event once, and that will result in a single
// callback per drop event, and the trace source will provide the interface
// which we filter on in the trace sink.
//
// The use of global maps allows this to continue to work properly even if
// the helper is destroyed before the simulation completes.  If the maps
// are populated, the reference counting smart pointers to
// OutputStreamWrapper and PcapFileWrapper will cause those objects to be
// destroyed at static object destruction time; i.e., the simulator does
// not explicitly clear these maps before the program ends.
//
typedef std::pair<uint32_t, uint32_t> InterfacePairIpv4; //!< Ipv4/interface pair
typedef std::map<InterfacePairIpv4, Ptr<PcapFileWrapper>>
    InterfaceFileMapIpv4; //!< Ipv4/interface and Pcap file wrapper container
typedef std::map<InterfacePairIpv4, Ptr<OutputStreamWrapper>>
    InterfaceStreamMapIpv4; //!< Ipv4/interface and output stream container

static InterfaceFileMapIpv4
    g_interfaceFileMapIpv4; //!< A mapping of Ipv4/interface pairs to pcap files
static InterfaceStreamMapIpv4
    g_interfaceStreamMapIpv4; //!< A mapping of Ipv4/interface pairs to ascii streams

typedef std::pair<uint32_t, uint32_t> InterfacePairIpv6; //!< Ipv6/interface pair
typedef std::map<InterfacePairIpv6, Ptr<PcapFileWrapper>>
    InterfaceFileMapIpv6; //!< Ipv6/interface and Pcap file wrapper container
typedef std::map<InterfacePairIpv6, Ptr<OutputStreamWrapper>>
    InterfaceStreamMapIpv6; //!< Ipv6/interface and output stream container

static InterfaceFileMapIpv6
    g_interfaceFileMapIpv6; //!< A mapping of Ipv6/interface pairs to pcap files
static InterfaceStreamMapIpv6
    g_interfaceStreamMapIpv6; //!< A mapping of Ipv6/interface pairs to pcap files

InternetStackHelper::InternetStackHelper()
    : m_routing(nullptr),
      m_routingv6(nullptr),
      m_ipv4Enabled(true),
      m_ipv6Enabled(true),
      m_ipv4ArpJitterEnabled(true),
      m_ipv6NsRsJitterEnabled(true)

{
    Initialize();
}

// private method called by both constructor and Reset ()
void
InternetStackHelper::Initialize()
{
    Ipv4StaticRoutingHelper staticRouting;
    Ipv4GlobalRoutingHelper globalRouting;
    Ipv4ListRoutingHelper listRouting;
    Ipv6StaticRoutingHelper staticRoutingv6;
    listRouting.Add(staticRouting, 0);
    listRouting.Add(globalRouting, -10);
    SetRoutingHelper(listRouting);
    SetRoutingHelper(staticRoutingv6);
}

InternetStackHelper::~InternetStackHelper()
{
    delete m_routing;
    delete m_routingv6;
}

InternetStackHelper::InternetStackHelper(const InternetStackHelper& o)
{
    m_routing = o.m_routing->Copy();
    m_routingv6 = o.m_routingv6->Copy();
    m_ipv4Enabled = o.m_ipv4Enabled;
    m_ipv6Enabled = o.m_ipv6Enabled;
    m_ipv4ArpJitterEnabled = o.m_ipv4ArpJitterEnabled;
    m_ipv6NsRsJitterEnabled = o.m_ipv6NsRsJitterEnabled;
}

InternetStackHelper&
InternetStackHelper::operator=(const InternetStackHelper& o)
{
    if (this == &o)
    {
        return *this;
    }
    m_routing = o.m_routing->Copy();
    m_routingv6 = o.m_routingv6->Copy();
    return *this;
}

void
InternetStackHelper::Reset()
{
    delete m_routing;
    m_routing = nullptr;
    delete m_routingv6;
    m_routingv6 = nullptr;
    m_ipv4Enabled = true;
    m_ipv6Enabled = true;
    m_ipv4ArpJitterEnabled = true;
    m_ipv6NsRsJitterEnabled = true;
    Initialize();
}

void
InternetStackHelper::SetRoutingHelper(const Ipv4RoutingHelper& routing)
{
    delete m_routing;
    m_routing = routing.Copy();
}

void
InternetStackHelper::SetRoutingHelper(const Ipv6RoutingHelper& routing)
{
    delete m_routingv6;
    m_routingv6 = routing.Copy();
}

void
InternetStackHelper::SetIpv4StackInstall(bool enable)
{
    m_ipv4Enabled = enable;
}

void
InternetStackHelper::SetIpv6StackInstall(bool enable)
{
    m_ipv6Enabled = enable;
}

void
InternetStackHelper::SetIpv4ArpJitter(bool enable)
{
    m_ipv4ArpJitterEnabled = enable;
}

void
InternetStackHelper::SetIpv6NsRsJitter(bool enable)
{
    m_ipv6NsRsJitterEnabled = enable;
}

int64_t
InternetStackHelper::AssignStreams(NodeContainer c, int64_t stream)
{
    int64_t currentStream = stream;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<GlobalRouter> router = node->GetObject<GlobalRouter>();
        if (router)
        {
            Ptr<Ipv4GlobalRouting> gr = router->GetRoutingProtocol();
            if (gr)
            {
                currentStream += gr->AssignStreams(currentStream);
            }
        }
        Ptr<Ipv6ExtensionDemux> demux = node->GetObject<Ipv6ExtensionDemux>();
        if (demux)
        {
            Ptr<Ipv6Extension> fe = demux->GetExtension(Ipv6ExtensionFragment::EXT_NUMBER);
            NS_ASSERT(fe); // should always exist in the demux
            currentStream += fe->AssignStreams(currentStream);
        }
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (ipv4)
        {
            Ptr<ArpL3Protocol> arpL3Protocol = ipv4->GetObject<ArpL3Protocol>();
            if (arpL3Protocol)
            {
                currentStream += arpL3Protocol->AssignStreams(currentStream);
            }
        }
        Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
        if (ipv6)
        {
            Ptr<Icmpv6L4Protocol> icmpv6L4Protocol = ipv6->GetObject<Icmpv6L4Protocol>();
            if (icmpv6L4Protocol)
            {
                currentStream += icmpv6L4Protocol->AssignStreams(currentStream);
            }
        }
    }
    return (currentStream - stream);
}

void
InternetStackHelper::Install(NodeContainer c) const
{
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        Install(*i);
    }
}

void
InternetStackHelper::InstallAll() const
{
    Install(NodeContainer::GetGlobal());
}

void
InternetStackHelper::CreateAndAggregateObjectFromTypeId(Ptr<Node> node, const std::string typeId)
{
    TypeId tid = TypeId::LookupByName(typeId);
    if (node->GetObject<Object>(tid))
    {
        return;
    }

    ObjectFactory factory;
    factory.SetTypeId(typeId);
    Ptr<Object> protocol = factory.Create<Object>();
    node->AggregateObject(protocol);
}

void
InternetStackHelper::Install(Ptr<Node> node) const
{
    if (m_ipv4Enabled)
    {
        /* IPv4 stack */
        CreateAndAggregateObjectFromTypeId(node, "ns3::ArpL3Protocol");
        CreateAndAggregateObjectFromTypeId(node, "ns3::Ipv4L3Protocol");
        CreateAndAggregateObjectFromTypeId(node, "ns3::Icmpv4L4Protocol");
        if (!m_ipv4ArpJitterEnabled)
        {
            Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol>();
            NS_ASSERT(arp);
            arp->SetAttribute("RequestJitter",
                              StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        }

        // Set routing
        Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
        if (!ipv4->GetRoutingProtocol())
        {
            Ptr<Ipv4RoutingProtocol> ipv4Routing = m_routing->Create(node);
            ipv4->SetRoutingProtocol(ipv4Routing);
        }
    }

    if (m_ipv6Enabled)
    {
        /* IPv6 stack */
        CreateAndAggregateObjectFromTypeId(node, "ns3::Ipv6L3Protocol");
        CreateAndAggregateObjectFromTypeId(node, "ns3::Icmpv6L4Protocol");
        if (!m_ipv6NsRsJitterEnabled)
        {
            Ptr<Icmpv6L4Protocol> icmpv6l4 = node->GetObject<Icmpv6L4Protocol>();
            NS_ASSERT(icmpv6l4);
            icmpv6l4->SetAttribute("SolicitationJitter",
                                   StringValue("ns3::ConstantRandomVariable[Constant=0.0]"));
        }
        // Set routing
        Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
        if (!ipv6->GetRoutingProtocol())
        {
            Ptr<Ipv6RoutingProtocol> ipv6Routing = m_routingv6->Create(node);
            ipv6->SetRoutingProtocol(ipv6Routing);
        }
        /* register IPv6 extensions and options */
        ipv6->RegisterExtensions();
        ipv6->RegisterOptions();
    }

    if (m_ipv4Enabled || m_ipv6Enabled)
    {
        CreateAndAggregateObjectFromTypeId(node, "ns3::TrafficControlLayer");
        CreateAndAggregateObjectFromTypeId(node, "ns3::UdpL4Protocol");
        CreateAndAggregateObjectFromTypeId(node, "ns3::TcpL4Protocol");
        if (!node->GetObject<PacketSocketFactory>())
        {
            Ptr<PacketSocketFactory> factory = CreateObject<PacketSocketFactory>();
            node->AggregateObject(factory);
        }
    }

    if (m_ipv4Enabled)
    {
        Ptr<ArpL3Protocol> arp = node->GetObject<ArpL3Protocol>();
        Ptr<TrafficControlLayer> tc = node->GetObject<TrafficControlLayer>();
        NS_ASSERT(arp);
        NS_ASSERT(tc);
        arp->SetTrafficControl(tc);
    }
}

void
InternetStackHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    Install(node);
}

/**
 * @brief Sync function for IPv4 packet - Pcap output
 * @param p smart pointer to the packet
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolRxTxSink(Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t interface)
{
    NS_LOG_FUNCTION(p << ipv4 << interface);

    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceFileMapIpv4.find(pair) == g_interfaceFileMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv4[pair];
    file->Write(Simulator::Now(), p);
}

bool
InternetStackHelper::PcapHooked(Ptr<Ipv4> ipv4)
{
    auto id = ipv4->GetObject<Node>()->GetId();

    for (auto i = g_interfaceFileMapIpv4.begin(); i != g_interfaceFileMapIpv4.end(); ++i)
    {
        if ((*i).first.first == id)
        {
            return true;
        }
    }
    return false;
}

void
InternetStackHelper::EnablePcapIpv4Internal(std::string prefix,
                                            Ptr<Ipv4> ipv4,
                                            uint32_t interface,
                                            bool explicitFilename)
{
    NS_LOG_FUNCTION(prefix << ipv4 << interface);

    if (!m_ipv4Enabled)
    {
        NS_LOG_INFO("Call to enable Ipv4 pcap tracing but Ipv4 not enabled");
        return;
    }

    //
    // We have to create a file and a mapping from protocol/interface to file
    // irrespective of how many times we want to trace a particular protocol.
    //
    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename)
    {
        filename = prefix;
    }
    else
    {
        filename = pcapHelper.GetFilenameFromInterfacePair(prefix, ipv4, interface);
    }

    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_RAW);

    //
    // However, we only hook the trace source once to avoid multiple trace sink
    // calls per event (connect is independent of interface).
    //
    if (!PcapHooked(ipv4))
    {
        //
        // Ptr<Ipv4> is aggregated to node and Ipv4L3Protocol is aggregated to
        // node so we can get to Ipv4L3Protocol through Ipv4.
        //
        Ptr<Ipv4L3Protocol> ipv4L3Protocol = ipv4->GetObject<Ipv4L3Protocol>();
        NS_ASSERT_MSG(ipv4L3Protocol,
                      "InternetStackHelper::EnablePcapIpv4Internal(): "
                      "m_ipv4Enabled and ipv4L3Protocol inconsistent");

        bool result =
            ipv4L3Protocol->TraceConnectWithoutContext("Tx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
        NS_ASSERT_MSG(result == true,
                      "InternetStackHelper::EnablePcapIpv4Internal():  "
                      "Unable to connect ipv4L3Protocol \"Tx\"");

        result =
            ipv4L3Protocol->TraceConnectWithoutContext("Rx", MakeCallback(&Ipv4L3ProtocolRxTxSink));
        NS_ASSERT_MSG(result == true,
                      "InternetStackHelper::EnablePcapIpv4Internal():  "
                      "Unable to connect ipv4L3Protocol \"Rx\"");
    }

    g_interfaceFileMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(), interface)] = file;
}

/**
 * @brief Sync function for IPv6 packet - Pcap output
 * @param p smart pointer to the packet
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolRxTxSink(Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface)
{
    NS_LOG_FUNCTION(p << ipv6 << interface);

    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceFileMapIpv6.find(pair) == g_interfaceFileMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<PcapFileWrapper> file = g_interfaceFileMapIpv6[pair];
    file->Write(Simulator::Now(), p);
}

bool
InternetStackHelper::PcapHooked(Ptr<Ipv6> ipv6)
{
    auto id = ipv6->GetObject<Node>()->GetId();

    for (auto i = g_interfaceFileMapIpv6.begin(); i != g_interfaceFileMapIpv6.end(); ++i)
    {
        if ((*i).first.first == id)
        {
            return true;
        }
    }
    return false;
}

void
InternetStackHelper::EnablePcapIpv6Internal(std::string prefix,
                                            Ptr<Ipv6> ipv6,
                                            uint32_t interface,
                                            bool explicitFilename)
{
    NS_LOG_FUNCTION(prefix << ipv6 << interface);

    if (!m_ipv6Enabled)
    {
        NS_LOG_INFO("Call to enable Ipv6 pcap tracing but Ipv6 not enabled");
        return;
    }

    //
    // We have to create a file and a mapping from protocol/interface to file
    // irrespective of how many times we want to trace a particular protocol.
    //
    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename)
    {
        filename = prefix;
    }
    else
    {
        filename = pcapHelper.GetFilenameFromInterfacePair(prefix, ipv6, interface);
    }

    Ptr<PcapFileWrapper> file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_RAW);

    //
    // However, we only hook the trace source once to avoid multiple trace sink
    // calls per event (connect is independent of interface).
    //
    if (!PcapHooked(ipv6))
    {
        //
        // Ptr<Ipv6> is aggregated to node and Ipv6L3Protocol is aggregated to
        // node so we can get to Ipv6L3Protocol through Ipv6.
        //
        Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol>();
        NS_ASSERT_MSG(ipv6L3Protocol,
                      "InternetStackHelper::EnablePcapIpv6Internal(): "
                      "m_ipv6Enabled and ipv6L3Protocol inconsistent");

        bool result =
            ipv6L3Protocol->TraceConnectWithoutContext("Tx", MakeCallback(&Ipv6L3ProtocolRxTxSink));
        NS_ASSERT_MSG(result == true,
                      "InternetStackHelper::EnablePcapIpv6Internal():  "
                      "Unable to connect ipv6L3Protocol \"Tx\"");

        result =
            ipv6L3Protocol->TraceConnectWithoutContext("Rx", MakeCallback(&Ipv6L3ProtocolRxTxSink));
        NS_ASSERT_MSG(result == true,
                      "InternetStackHelper::EnablePcapIpv6Internal():  "
                      "Unable to connect ipv6L3Protocol \"Rx\"");
    }

    g_interfaceFileMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(), interface)] = file;
}

/**
 * @brief Sync function for IPv4 dropped packet - Ascii output
 * @param stream the output stream
 * @param header IPv4 header
 * @param packet smart pointer to the packet
 * @param reason the reason for the dropping
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolDropSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                     const Ipv4Header& header,
                                     Ptr<const Packet> packet,
                                     Ipv4L3Protocol::DropReason reason,
                                     Ptr<Ipv4> ipv4,
                                     uint32_t interface)
{
    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<Packet> p = packet->Copy();
    p->AddHeader(header);
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p << std::endl;
}

/**
 * @brief Sync function for IPv4 transmitted packet - Ascii output
 * @param stream the output stream
 * @param packet smart pointer to the packet
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolTxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                   Ptr<const Packet> packet,
                                   Ptr<Ipv4> ipv4,
                                   uint32_t interface)
{
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);

    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }
    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << *packet << std::endl;
}

/**
 * @brief Sync function for IPv4 received packet - Ascii output
 * @param stream the output stream
 * @param packet smart pointer to the packet
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolRxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                   Ptr<const Packet> packet,
                                   Ptr<Ipv4> ipv4,
                                   uint32_t interface)
{
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << *packet << std::endl;
}

/**
 * @brief Sync function for IPv4 dropped packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param header IPv4 header
 * @param packet smart pointer to the packet
 * @param reason the reason for the dropping
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolDropSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                  std::string context,
                                  const Ipv4Header& header,
                                  Ptr<const Packet> packet,
                                  Ipv4L3Protocol::DropReason reason,
                                  Ptr<Ipv4> ipv4,
                                  uint32_t interface)
{
    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<Packet> p = packet->Copy();
    p->AddHeader(header);
#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *p << std::endl;
#else
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << context << " " << *p
                         << std::endl;
#endif
}

/**
 * @brief Sync function for IPv4 transmitted packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param packet smart pointer to the packet
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolTxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                std::string context,
                                Ptr<const Packet> packet,
                                Ptr<Ipv4> ipv4,
                                uint32_t interface)
{
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *packet << std::endl;
#else
    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << context << " "
                         << *packet << std::endl;
#endif
}

/**
 * @brief Sync function for IPv4 received packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param packet smart pointer to the packet
 * @param ipv4 smart pointer to the node's IPv4 stack
 * @param interface incoming interface
 */
static void
Ipv4L3ProtocolRxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                std::string context,
                                Ptr<const Packet> packet,
                                Ptr<Ipv4> ipv4,
                                uint32_t interface)
{
    InterfacePairIpv4 pair = std::make_pair(ipv4->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv4.find(pair) == g_interfaceStreamMapIpv4.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *packet << std::endl;
#else
    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << context << " "
                         << *packet << std::endl;
#endif
}

bool
InternetStackHelper::AsciiHooked(Ptr<Ipv4> ipv4)
{
    auto id = ipv4->GetObject<Node>()->GetId();

    for (auto i = g_interfaceStreamMapIpv4.begin(); i != g_interfaceStreamMapIpv4.end(); ++i)
    {
        if ((*i).first.first == id)
        {
            return true;
        }
    }
    return false;
}

void
InternetStackHelper::EnableAsciiIpv4Internal(Ptr<OutputStreamWrapper> stream,
                                             std::string prefix,
                                             Ptr<Ipv4> ipv4,
                                             uint32_t interface,
                                             bool explicitFilename)
{
    if (!m_ipv4Enabled)
    {
        NS_LOG_INFO("Call to enable Ipv4 ascii tracing but Ipv4 not enabled");
        return;
    }

    //
    // Our trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and hook WithoutContext
    // since there will be one file per context and therefore the context would
    // be redundant.
    //
    if (!stream)
    {
        //
        // Set up an output stream object to deal with private ofstream copy
        // constructor and lifetime issues.  Let the helper decide the actual
        // name of the file given the prefix.
        //
        // We have to create a stream and a mapping from protocol/interface to
        // stream irrespective of how many times we want to trace a particular
        // protocol.
        //
        AsciiTraceHelper asciiTraceHelper;

        std::string filename;
        if (explicitFilename)
        {
            filename = prefix;
        }
        else
        {
            filename = asciiTraceHelper.GetFilenameFromInterfacePair(prefix, ipv4, interface);
        }

        Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

        //
        // However, we only hook the trace sources once to avoid multiple trace sink
        // calls per event (connect is independent of interface).
        //
        if (!AsciiHooked(ipv4))
        {
            //
            // We can use the default drop sink for the ArpL3Protocol since it has
            // the usual signature.  We can get to the Ptr<ArpL3Protocol> through
            // our Ptr<Ipv4> since they must both be aggregated to the same node.
            //
            Ptr<ArpL3Protocol> arpL3Protocol = ipv4->GetObject<ArpL3Protocol>();
            asciiTraceHelper.HookDefaultDropSinkWithoutContext<ArpL3Protocol>(arpL3Protocol,
                                                                              "Drop",
                                                                              theStream);

            //
            // The drop sink for the Ipv4L3Protocol uses a different signature than
            // the default sink, so we have to cook one up for ourselves.  We can get
            // to the Ptr<Ipv4L3Protocol> through our Ptr<Ipv4> since they must both
            // be aggregated to the same node.
            //
            Ptr<Ipv4L3Protocol> ipv4L3Protocol = ipv4->GetObject<Ipv4L3Protocol>();
            bool result = ipv4L3Protocol->TraceConnectWithoutContext(
                "Drop",
                MakeBoundCallback(&Ipv4L3ProtocolDropSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv4Internal():  "
                          "Unable to connect ipv4L3Protocol \"Drop\"");
            result = ipv4L3Protocol->TraceConnectWithoutContext(
                "Tx",
                MakeBoundCallback(&Ipv4L3ProtocolTxSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv4Internal():  "
                          "Unable to connect ipv4L3Protocol \"Tx\"");
            result = ipv4L3Protocol->TraceConnectWithoutContext(
                "Rx",
                MakeBoundCallback(&Ipv4L3ProtocolRxSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv4Internal():  "
                          "Unable to connect ipv4L3Protocol \"Rx\"");
        }

        g_interfaceStreamMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(), interface)] =
            theStream;
        return;
    }

    //
    // If we are provided an OutputStreamWrapper, we are expected to use it, and
    // to provide a context.  We are free to come up with our own context if we
    // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
    // compatibility and simplicity, we just use Config::Connect and let it deal
    // with the context.
    //
    // We need to associate the ipv4/interface with a stream to express interest
    // in tracing events on that pair, however, we only hook the trace sources
    // once to avoid multiple trace sink calls per event (connect is independent
    // of interface).
    //
    if (!AsciiHooked(ipv4))
    {
        Ptr<Node> node = ipv4->GetObject<Node>();
        std::ostringstream oss;

        //
        // For the ARP Drop, we are going to use the default trace sink provided by
        // the ascii trace helper.  There is actually no AsciiTraceHelper in sight
        // here, but the default trace sinks are actually publicly available static
        // functions that are always there waiting for just such a case.
        //
        oss << "/NodeList/" << node->GetId() << "/$ns3::ArpL3Protocol/Drop";
        Config::Connect(oss.str(),
                        MakeBoundCallback(&AsciiTraceHelper::DefaultDropSinkWithContext, stream));

        //
        // This has all kinds of parameters coming with, so we have to cook up our
        // own sink.
        //
        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Drop";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv4L3ProtocolDropSinkWithContext, stream));
        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Tx";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv4L3ProtocolTxSinkWithContext, stream));
        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv4L3Protocol/Rx";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv4L3ProtocolRxSinkWithContext, stream));
    }

    g_interfaceStreamMapIpv4[std::make_pair(ipv4->GetObject<Node>()->GetId(), interface)] = stream;
}

/**
 * @brief Sync function for IPv6 dropped packet - Ascii output
 * @param stream the output stream
 * @param header IPv6 header
 * @param packet smart pointer to the packet
 * @param reason the reason for the dropping
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolDropSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                     const Ipv6Header& header,
                                     Ptr<const Packet> packet,
                                     Ipv6L3Protocol::DropReason reason,
                                     Ptr<Ipv6> ipv6,
                                     uint32_t interface)
{
    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<Packet> p = packet->Copy();
    p->AddHeader(header);
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << *p << std::endl;
}

/**
 * @brief Sync function for IPv6 transmitted packet - Ascii output
 * @param stream the output stream
 * @param packet smart pointer to the packet
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolTxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                   Ptr<const Packet> packet,
                                   Ptr<Ipv6> ipv6,
                                   uint32_t interface)
{
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << *packet << std::endl;
}

/**
 * @brief Sync function for IPv6 received packet - Ascii output
 * @param stream the output stream
 * @param packet smart pointer to the packet
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolRxSinkWithoutContext(Ptr<OutputStreamWrapper> stream,
                                   Ptr<const Packet> packet,
                                   Ptr<Ipv6> ipv6,
                                   uint32_t interface)
{
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << *packet << std::endl;
}

/**
 * @brief Sync function for IPv6 dropped packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param header IPv6 header
 * @param packet smart pointer to the packet
 * @param reason the reason for the dropping
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolDropSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                  std::string context,
                                  const Ipv6Header& header,
                                  Ptr<const Packet> packet,
                                  Ipv6L3Protocol::DropReason reason,
                                  Ptr<Ipv6> ipv6,
                                  uint32_t interface)
{
    //
    // Since trace sources are independent of interface, if we hook a source
    // on a particular protocol we will get traces for all of its interfaces.
    // We need to filter this to only report interfaces for which the user
    // has expressed interest.
    //
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

    Ptr<Packet> p = packet->Copy();
    p->AddHeader(header);
#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *p << std::endl;
#else
    *stream->GetStream() << "d " << Simulator::Now().GetSeconds() << " " << context << " " << *p
                         << std::endl;
#endif
}

/**
 * @brief Sync function for IPv6 transmitted packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param packet smart pointer to the packet
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolTxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                std::string context,
                                Ptr<const Packet> packet,
                                Ptr<Ipv6> ipv6,
                                uint32_t interface)
{
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *packet << std::endl;
#else
    *stream->GetStream() << "t " << Simulator::Now().GetSeconds() << " " << context << " "
                         << *packet << std::endl;
#endif
}

/**
 * @brief Sync function for IPv6 received packet - Ascii output
 * @param stream the output stream
 * @param context the context
 * @param packet smart pointer to the packet
 * @param ipv6 smart pointer to the node's IPv6 stack
 * @param interface incoming interface
 */
static void
Ipv6L3ProtocolRxSinkWithContext(Ptr<OutputStreamWrapper> stream,
                                std::string context,
                                Ptr<const Packet> packet,
                                Ptr<Ipv6> ipv6,
                                uint32_t interface)
{
    InterfacePairIpv6 pair = std::make_pair(ipv6->GetObject<Node>()->GetId(), interface);
    if (g_interfaceStreamMapIpv6.find(pair) == g_interfaceStreamMapIpv6.end())
    {
        NS_LOG_INFO("Ignoring packet to/from interface " << interface);
        return;
    }

#ifdef INTERFACE_CONTEXT
    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << context << "("
                         << interface << ") " << *packet << std::endl;
#else
    *stream->GetStream() << "r " << Simulator::Now().GetSeconds() << " " << context << " "
                         << *packet << std::endl;
#endif
}

bool
InternetStackHelper::AsciiHooked(Ptr<Ipv6> ipv6)
{
    auto id = ipv6->GetObject<Node>()->GetId();

    for (auto i = g_interfaceStreamMapIpv6.begin(); i != g_interfaceStreamMapIpv6.end(); ++i)
    {
        if ((*i).first.first == id)
        {
            return true;
        }
    }
    return false;
}

void
InternetStackHelper::EnableAsciiIpv6Internal(Ptr<OutputStreamWrapper> stream,
                                             std::string prefix,
                                             Ptr<Ipv6> ipv6,
                                             uint32_t interface,
                                             bool explicitFilename)
{
    if (!m_ipv6Enabled)
    {
        NS_LOG_INFO("Call to enable Ipv6 ascii tracing but Ipv6 not enabled");
        return;
    }

    //
    // Our trace sinks are going to use packet printing, so we have to
    // make sure that is turned on.
    //
    Packet::EnablePrinting();

    //
    // If we are not provided an OutputStreamWrapper, we are expected to create
    // one using the usual trace filename conventions and do a hook WithoutContext
    // since there will be one file per context and therefore the context would
    // be redundant.
    //
    if (!stream)
    {
        //
        // Set up an output stream object to deal with private ofstream copy
        // constructor and lifetime issues.  Let the helper decide the actual
        // name of the file given the prefix.
        //
        // We have to create a stream and a mapping from protocol/interface to
        // stream irrespective of how many times we want to trace a particular
        // protocol.
        //
        AsciiTraceHelper asciiTraceHelper;

        std::string filename;
        if (explicitFilename)
        {
            filename = prefix;
        }
        else
        {
            filename = asciiTraceHelper.GetFilenameFromInterfacePair(prefix, ipv6, interface);
        }

        Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream(filename);

        //
        // However, we only hook the trace sources once to avoid multiple trace sink
        // calls per event (connect is independent of interface).
        //
        if (!AsciiHooked(ipv6))
        {
            //
            // The drop sink for the Ipv6L3Protocol uses a different signature than
            // the default sink, so we have to cook one up for ourselves.  We can get
            // to the Ptr<Ipv6L3Protocol> through our Ptr<Ipv6> since they must both
            // be aggregated to the same node.
            //
            Ptr<Ipv6L3Protocol> ipv6L3Protocol = ipv6->GetObject<Ipv6L3Protocol>();
            bool result = ipv6L3Protocol->TraceConnectWithoutContext(
                "Drop",
                MakeBoundCallback(&Ipv6L3ProtocolDropSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv6Internal():  "
                          "Unable to connect ipv6L3Protocol \"Drop\"");
            result = ipv6L3Protocol->TraceConnectWithoutContext(
                "Tx",
                MakeBoundCallback(&Ipv6L3ProtocolTxSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv6Internal():  "
                          "Unable to connect ipv6L3Protocol \"Tx\"");
            result = ipv6L3Protocol->TraceConnectWithoutContext(
                "Rx",
                MakeBoundCallback(&Ipv6L3ProtocolRxSinkWithoutContext, theStream));
            NS_ASSERT_MSG(result == true,
                          "InternetStackHelper::EnableAsciiIpv6Internal():  "
                          "Unable to connect ipv6L3Protocol \"Rx\"");
        }

        g_interfaceStreamMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(), interface)] =
            theStream;
        return;
    }

    //
    // If we are provided an OutputStreamWrapper, we are expected to use it, and
    // to provide a context.  We are free to come up with our own context if we
    // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
    // compatibility and simplicity, we just use Config::Connect and let it deal
    // with the context.
    //
    // We need to associate the ipv4/interface with a stream to express interest
    // in tracing events on that pair, however, we only hook the trace sources
    // once to avoid multiple trace sink calls per event (connect is independent
    // of interface).
    //
    if (!AsciiHooked(ipv6))
    {
        Ptr<Node> node = ipv6->GetObject<Node>();
        std::ostringstream oss;

        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Drop";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv6L3ProtocolDropSinkWithContext, stream));
        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Tx";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv6L3ProtocolTxSinkWithContext, stream));
        oss.str("");
        oss << "/NodeList/" << node->GetId() << "/$ns3::Ipv6L3Protocol/Rx";
        Config::Connect(oss.str(), MakeBoundCallback(&Ipv6L3ProtocolRxSinkWithContext, stream));
    }

    g_interfaceStreamMapIpv6[std::make_pair(ipv6->GetObject<Node>()->GetId(), interface)] = stream;
}

} // namespace ns3
