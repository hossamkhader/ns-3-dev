/*
 * Copyright (c) 2008,2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Kirill Andreev <andreev@iitp.ru>
 */
#include "dot11s-installer.h"

#include "ns3/hwmp-protocol.h"
#include "ns3/mesh-wifi-interface-mac.h"
#include "ns3/peer-management-protocol.h"
#include "ns3/wifi-net-device.h"

namespace ns3
{
using namespace dot11s;
NS_OBJECT_ENSURE_REGISTERED(Dot11sStack);

TypeId
Dot11sStack::GetTypeId()
{
    static TypeId tid = TypeId("ns3::Dot11sStack")
                            .SetParent<MeshStack>()
                            .SetGroupName("Mesh")
                            .AddConstructor<Dot11sStack>()
                            .AddAttribute("Root",
                                          "The MAC address of root mesh point.",
                                          Mac48AddressValue(Mac48Address("ff:ff:ff:ff:ff:ff")),
                                          MakeMac48AddressAccessor(&Dot11sStack::m_root),
                                          MakeMac48AddressChecker());
    return tid;
}

Dot11sStack::Dot11sStack()
    : m_root(Mac48Address("ff:ff:ff:ff:ff:ff"))
{
}

Dot11sStack::~Dot11sStack()
{
}

void
Dot11sStack::DoDispose()
{
}

bool
Dot11sStack::InstallStack(Ptr<MeshPointDevice> mp)
{
    // Install Peer management protocol:
    Ptr<PeerManagementProtocol> pmp = CreateObject<PeerManagementProtocol>();
    pmp->SetMeshId("mesh");
    bool install_ok = pmp->Install(mp);
    if (!install_ok)
    {
        return false;
    }
    // Install HWMP:
    Ptr<HwmpProtocol> hwmp = CreateObject<HwmpProtocol>();
    install_ok = hwmp->Install(mp);
    if (!install_ok)
    {
        return false;
    }
    if (mp->GetAddress() == m_root)
    {
        hwmp->SetRoot();
    }
    // Install interaction between HWMP and Peer management protocol:
    // PeekPointer()'s to avoid circular Ptr references
    pmp->SetPeerLinkStatusCallback(MakeCallback(&HwmpProtocol::PeerLinkStatus, PeekPointer(hwmp)));
    hwmp->SetNeighboursCallback(MakeCallback(&PeerManagementProtocol::GetPeers, PeekPointer(pmp)));
    return true;
}

void
Dot11sStack::Report(const Ptr<MeshPointDevice> mp, std::ostream& os)
{
    mp->Report(os);

    std::vector<Ptr<NetDevice>> ifaces = mp->GetInterfaces();
    for (auto i = ifaces.begin(); i != ifaces.end(); ++i)
    {
        Ptr<WifiNetDevice> device = (*i)->GetObject<WifiNetDevice>();
        NS_ASSERT(device);
        Ptr<MeshWifiInterfaceMac> mac = device->GetMac()->GetObject<MeshWifiInterfaceMac>();
        NS_ASSERT(mac);
        mac->Report(os);
    }
    Ptr<HwmpProtocol> hwmp = mp->GetObject<HwmpProtocol>();
    NS_ASSERT(hwmp);
    hwmp->Report(os);

    Ptr<PeerManagementProtocol> pmp = mp->GetObject<PeerManagementProtocol>();
    NS_ASSERT(pmp);
    pmp->Report(os);
}

void
Dot11sStack::ResetStats(const Ptr<MeshPointDevice> mp)
{
    mp->ResetStats();

    std::vector<Ptr<NetDevice>> ifaces = mp->GetInterfaces();
    for (auto i = ifaces.begin(); i != ifaces.end(); ++i)
    {
        Ptr<WifiNetDevice> device = (*i)->GetObject<WifiNetDevice>();
        NS_ASSERT(device);
        Ptr<MeshWifiInterfaceMac> mac = device->GetMac()->GetObject<MeshWifiInterfaceMac>();
        NS_ASSERT(mac);
        mac->ResetStats();
    }
    Ptr<HwmpProtocol> hwmp = mp->GetObject<HwmpProtocol>();
    NS_ASSERT(hwmp);
    hwmp->ResetStats();

    Ptr<PeerManagementProtocol> pmp = mp->GetObject<PeerManagementProtocol>();
    NS_ASSERT(pmp);
    pmp->ResetStats();
}
} // namespace ns3
