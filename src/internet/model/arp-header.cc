/*
 * Copyright (c) 2005 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "arp-header.h"

#include "ns3/address-utils.h"
#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ArpHeader");

NS_OBJECT_ENSURE_REGISTERED(ArpHeader);

void
ArpHeader::SetRequest(Address sourceHardwareAddress,
                      Ipv4Address sourceProtocolAddress,
                      Address destinationHardwareAddress,
                      Ipv4Address destinationProtocolAddress)
{
    NS_LOG_FUNCTION(this << sourceHardwareAddress << sourceProtocolAddress
                         << destinationHardwareAddress << destinationProtocolAddress);
    m_type = ARP_TYPE_REQUEST;
    m_macSource = sourceHardwareAddress;
    m_macDest = destinationHardwareAddress;
    m_ipv4Source = sourceProtocolAddress;
    m_ipv4Dest = destinationProtocolAddress;
}

void
ArpHeader::SetReply(Address sourceHardwareAddress,
                    Ipv4Address sourceProtocolAddress,
                    Address destinationHardwareAddress,
                    Ipv4Address destinationProtocolAddress)
{
    NS_LOG_FUNCTION(this << sourceHardwareAddress << sourceProtocolAddress
                         << destinationHardwareAddress << destinationProtocolAddress);
    m_type = ARP_TYPE_REPLY;
    m_macSource = sourceHardwareAddress;
    m_macDest = destinationHardwareAddress;
    m_ipv4Source = sourceProtocolAddress;
    m_ipv4Dest = destinationProtocolAddress;
}

bool
ArpHeader::IsRequest() const
{
    NS_LOG_FUNCTION(this);
    return m_type == ARP_TYPE_REQUEST;
}

bool
ArpHeader::IsReply() const
{
    NS_LOG_FUNCTION(this);
    return m_type == ARP_TYPE_REPLY;
}

Address
ArpHeader::GetSourceHardwareAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_macSource;
}

Address
ArpHeader::GetDestinationHardwareAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_macDest;
}

Ipv4Address
ArpHeader::GetSourceIpv4Address() const
{
    NS_LOG_FUNCTION(this);
    return m_ipv4Source;
}

Ipv4Address
ArpHeader::GetDestinationIpv4Address() const
{
    NS_LOG_FUNCTION(this);
    return m_ipv4Dest;
}

TypeId
ArpHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ArpHeader")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<ArpHeader>();
    return tid;
}

TypeId
ArpHeader::GetInstanceTypeId() const
{
    NS_LOG_FUNCTION(this);
    return GetTypeId();
}

void
ArpHeader::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this << &os);
    if (IsRequest())
    {
        os << "request "
           << "source mac: " << m_macSource << " "
           << "source ipv4: " << m_ipv4Source << " "
           << "dest ipv4: " << m_ipv4Dest;
    }
    else
    {
        NS_ASSERT(IsReply());
        os << "reply "
           << "source mac: " << m_macSource << " "
           << "source ipv4: " << m_ipv4Source << " "
           << "dest mac: " << m_macDest << " "
           << "dest ipv4: " << m_ipv4Dest;
    }
}

uint32_t
ArpHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT((m_macSource.GetLength() == 6) || (m_macSource.GetLength() == 8) ||
              (m_macSource.GetLength() == 1));
    NS_ASSERT(m_macSource.GetLength() == m_macDest.GetLength());

    uint32_t length = 16; // Length minus two hardware addresses
    length += m_macSource.GetLength() * 2;

    return length;
}

void
ArpHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    NS_ASSERT(m_macSource.GetLength() == m_macDest.GetLength());

    /* ethernet */
    i.WriteHtonU16(0x0001);
    /* ipv4 */
    i.WriteHtonU16(0x0800);
    i.WriteU8(m_macSource.GetLength());
    i.WriteU8(4);
    i.WriteHtonU16(m_type);
    WriteTo(i, m_macSource);
    WriteTo(i, m_ipv4Source);
    WriteTo(i, m_macDest);
    WriteTo(i, m_ipv4Dest);
}

uint32_t
ArpHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this << &start);
    Buffer::Iterator i = start;
    i.Next(2);                                // Skip HRD
    uint32_t protocolType = i.ReadNtohU16();  // Read PRO
    uint32_t hardwareAddressLen = i.ReadU8(); // Read HLN
    uint32_t protocolAddressLen = i.ReadU8(); // Read PLN

    //
    // It is implicit here that we have a protocol type of 0x800 (IP).
    // It is also implicit here that we are using Ipv4 (PLN == 4).
    // If this isn't the case, we need to return an error since we don't want to
    // be too fragile if we get connected to real networks.
    //
    if (protocolType != 0x800 || protocolAddressLen != 4)
    {
        return 0;
    }

    m_type = i.ReadNtohU16();                     // Read OP
    ReadFrom(i, m_macSource, hardwareAddressLen); // Read SHA (size HLN)
    ReadFrom(i, m_ipv4Source);                    // Read SPA (size PLN == 4)
    ReadFrom(i, m_macDest, hardwareAddressLen);   // Read THA (size HLN)
    ReadFrom(i, m_ipv4Dest);                      // Read TPA (size PLN == 4)
    return GetSerializedSize();
}

} // namespace ns3
