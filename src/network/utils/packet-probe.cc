/*
 * Copyright (c) 2011 Bucknell University
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: L. Felipe Perrone (perrone@bucknell.edu)
 *          Tiago G. Rodrigues (tgr002@bucknell.edu)
 *
 * Modified by: Mitch Watrous (watrous@u.washington.edu)
 */

#include "packet-probe.h"

#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/object.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("PacketProbe");

NS_OBJECT_ENSURE_REGISTERED(PacketProbe);

TypeId
PacketProbe::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PacketProbe")
                            .SetParent<Probe>()
                            .SetGroupName("Network")
                            .AddConstructor<PacketProbe>()
                            .AddTraceSource("Output",
                                            "The packet that serve as the output for this probe",
                                            MakeTraceSourceAccessor(&PacketProbe::m_output),
                                            "ns3::Packet::TracedCallback")
                            .AddTraceSource("OutputBytes",
                                            "The number of bytes in the packet",
                                            MakeTraceSourceAccessor(&PacketProbe::m_outputBytes),
                                            "ns3::Packet::SizeTracedCallback");
    return tid;
}

PacketProbe::PacketProbe()
{
    NS_LOG_FUNCTION(this);
    m_packet = nullptr;
}

PacketProbe::~PacketProbe()
{
    NS_LOG_FUNCTION(this);
}

void
PacketProbe::SetValue(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    m_packet = packet;
    m_output(packet);

    uint32_t packetSizeNew = packet->GetSize();
    m_outputBytes(m_packetSizeOld, packetSizeNew);
    m_packetSizeOld = packetSizeNew;
}

void
PacketProbe::SetValueByPath(std::string path, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(path << packet);
    Ptr<PacketProbe> probe = Names::Find<PacketProbe>(path);
    NS_ASSERT_MSG(probe, "Error:  Can't find probe for path " << path);
    probe->SetValue(packet);
}

bool
PacketProbe::ConnectByObject(std::string traceSource, Ptr<Object> obj)
{
    NS_LOG_FUNCTION(this << traceSource << obj);
    NS_LOG_DEBUG("Name of probe (if any) in names database: " << Names::FindPath(obj));
    bool connected =
        obj->TraceConnectWithoutContext(traceSource,
                                        MakeCallback(&ns3::PacketProbe::TraceSink, this));
    return connected;
}

void
PacketProbe::ConnectByPath(std::string path)
{
    NS_LOG_FUNCTION(this << path);
    NS_LOG_DEBUG("Name of probe to search for in config database: " << path);
    Config::ConnectWithoutContext(path, MakeCallback(&ns3::PacketProbe::TraceSink, this));
}

void
PacketProbe::TraceSink(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    if (IsEnabled())
    {
        m_packet = packet;
        m_output(packet);

        uint32_t packetSizeNew = packet->GetSize();
        m_outputBytes(m_packetSizeOld, packetSizeNew);
        m_packetSizeOld = packetSizeNew;
    }
}

} // namespace ns3
