/*
 * Copyright (c) 2011 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */

#include "sixlowpan-helper.h"

#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/sixlowpan-net-device.h"

namespace ns3
{

class Address;

NS_LOG_COMPONENT_DEFINE("SixLowPanHelper");

SixLowPanHelper::SixLowPanHelper()
{
    NS_LOG_FUNCTION(this);
    m_deviceFactory.SetTypeId("ns3::SixLowPanNetDevice");
}

void
SixLowPanHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this);
    m_deviceFactory.Set(n1, v1);
}

NetDeviceContainer
SixLowPanHelper::Install(const NetDeviceContainer c)
{
    NS_LOG_FUNCTION(this);

    NetDeviceContainer devs;

    for (uint32_t i = 0; i < c.GetN(); ++i)
    {
        Ptr<NetDevice> device = c.Get(i);
        NS_ASSERT_MSG(device, "No NetDevice found in the node " << int(i));

        Ptr<Node> node = device->GetNode();
        NS_LOG_LOGIC("**** Install 6LoWPAN on node " << node->GetId());

        Ptr<SixLowPanNetDevice> dev = m_deviceFactory.Create<SixLowPanNetDevice>();
        devs.Add(dev);
        node->AddDevice(dev);
        dev->SetNetDevice(device);
    }
    return devs;
}

void
SixLowPanHelper::AddContext(NetDeviceContainer c,
                            uint8_t contextId,
                            Ipv6Prefix context,
                            Time validity)
{
    NS_LOG_FUNCTION(this << +contextId << context << validity);

    for (uint32_t i = 0; i < c.GetN(); ++i)
    {
        Ptr<NetDevice> device = c.Get(i);
        Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(device);
        if (sixDevice)
        {
            sixDevice->AddContext(contextId, context, true, validity);
        }
    }
}

void
SixLowPanHelper::RenewContext(NetDeviceContainer c, uint8_t contextId, Time validity)
{
    NS_LOG_FUNCTION(this << +contextId << validity);

    for (uint32_t i = 0; i < c.GetN(); ++i)
    {
        Ptr<NetDevice> device = c.Get(i);
        Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(device);
        if (sixDevice)
        {
            sixDevice->RenewContext(contextId, validity);
        }
    }
}

void
SixLowPanHelper::InvalidateContext(NetDeviceContainer c, uint8_t contextId)
{
    NS_LOG_FUNCTION(this << +contextId);

    for (uint32_t i = 0; i < c.GetN(); ++i)
    {
        Ptr<NetDevice> device = c.Get(i);
        Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(device);
        if (sixDevice)
        {
            sixDevice->InvalidateContext(contextId);
        }
    }
}

void
SixLowPanHelper::RemoveContext(NetDeviceContainer c, uint8_t contextId)
{
    NS_LOG_FUNCTION(this << +contextId);

    for (uint32_t i = 0; i < c.GetN(); ++i)
    {
        Ptr<NetDevice> device = c.Get(i);
        Ptr<SixLowPanNetDevice> sixDevice = DynamicCast<SixLowPanNetDevice>(device);
        if (sixDevice)
        {
            sixDevice->RemoveContext(contextId);
        }
    }
}

int64_t
SixLowPanHelper::AssignStreams(NetDeviceContainer c, int64_t stream)
{
    int64_t currentStream = stream;
    Ptr<NetDevice> netDevice;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        netDevice = (*i);
        Ptr<SixLowPanNetDevice> dev = DynamicCast<SixLowPanNetDevice>(netDevice);
        if (dev)
        {
            currentStream += dev->AssignStreams(currentStream);
        }
    }
    return (currentStream - stream);
}

} // namespace ns3
