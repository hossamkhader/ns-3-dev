/*
 * Copyright (c) 2025 Tokushima University, Japan
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors:
 *
 *  Alberto Gallegos Ramonet <alramonet@is.tokushima-u.ac.jp>
 */

#include "zigbee-aps.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace zigbee
{

NS_LOG_COMPONENT_DEFINE("ZigbeeAps");
NS_OBJECT_ENSURE_REGISTERED(ZigbeeAps);

TypeId
ZigbeeAps::GetTypeId()
{
    static TypeId tid = TypeId("ns3::zigbee::ZigbeeAps")
                            .SetParent<Object>()
                            .SetGroupName("Zigbee")
                            .AddConstructor<ZigbeeAps>();
    return tid;
}

ZigbeeAps::ZigbeeAps()
{
    NS_LOG_FUNCTION(this);
}

void
ZigbeeAps::NotifyConstructionCompleted()
{
    NS_LOG_FUNCTION(this);
}

ZigbeeAps::~ZigbeeAps()
{
    NS_LOG_FUNCTION(this);
}

void
ZigbeeAps::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    Object::DoInitialize();
}

void
ZigbeeAps::DoDispose()
{
    m_nwk = nullptr;
    // m_apsBindingTable.Dispose();
    m_apsdeDataConfirmCallback = MakeNullCallback<void, ApsdeDataConfirmParams>();

    Object::DoDispose();
}

void
ZigbeeAps::SetNwk(Ptr<ZigbeeNwk> nwk)
{
    m_nwk = nwk;
}

Ptr<ZigbeeNwk>
ZigbeeAps::GetNwk() const
{
    return m_nwk;
}

void
ZigbeeAps::ApsdeDataRequest(ApsdeDataRequestParams params, Ptr<Packet> asdu)
{
    NS_LOG_FUNCTION(this);

    // Fill APSDE-data.confirm parameters in case we need to return an error
    ApsdeDataConfirmParams confirmParams;
    confirmParams.m_dstAddrMode = params.m_dstAddrMode;
    confirmParams.m_dstAddr16 = params.m_dstAddr16;
    confirmParams.m_dstAddr64 = params.m_dstAddr64;
    confirmParams.m_dstEndPoint = params.m_dstEndPoint;
    confirmParams.m_srcEndPoint = params.m_srcEndPoint;

    ZigbeeApsTxOptions txOptions(params.m_txOptions);

    if (txOptions.IsSecurityEnabled())
    {
        // TODO: Add support for security options
        if (!m_apsdeDataConfirmCallback.IsNull())
        {
            confirmParams.m_status = ApsStatus::SECURITY_FAIL;
            confirmParams.m_txTime = Simulator::Now();
            m_apsdeDataConfirmCallback(confirmParams);
        }
        NS_LOG_WARN("Security is not currently supported");
        return;
    }

    // TODO: Fragmentation

    // TODO: Add ACK support
    if (txOptions.IsAckRequired())
    {
        NS_ABORT_MSG("Transmission with ACK not supported");
        return;
    }

    // See See 2.2.4.1.1
    switch (params.m_dstAddrMode)
    {
    case ApsDstAddressMode::DST_ADDR_AND_DST_ENDPOINT_NOT_PRESENT: {
        // Use BINDING TABLE to send data to one or more destinations
        // (Groupcast or IEEE address destination transmission)
        NS_ABORT_MSG("Binded destination groupcast not supported");
        // SendDataWithBindingTable(params, asdu);
        break;
    }
    case ApsDstAddressMode::GROUP_ADDR_DST_ENDPOINT_NOT_PRESENT: {
        // TODO: Add Groupcast (multicast) support
        NS_ABORT_MSG("GROUP ADDRESS (MCST) not supported");
        break;
    }
    case ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT: {
        // Regular UCST or BCST transmission with the 16 bit destination address
        SendDataUcstBcst(params, asdu);
        break;
    }
    case ApsDstAddressMode::DST_ADDR64_DST_ENDPOINT_PRESENT: {
        // TODO: Add Extended address transmission support.
        // The NWK do not accept direct extended address transmissions,
        // therefore, the APS must translate the extended address
        // to a short address using the nwkAddressMap NIB.
        if (!m_apsdeDataConfirmCallback.IsNull())
        {
            confirmParams.m_status = ApsStatus::NO_SHORT_ADDRESS;
            confirmParams.m_txTime = Simulator::Now();
            m_apsdeDataConfirmCallback(confirmParams);
        }
        NS_LOG_WARN("Extended address mode not supported");
        break;
    }
    default:
        NS_ABORT_MSG("Invalid Option");
        break;
    }
}

void
ZigbeeAps::SendDataWithBindingTable(ApsdeDataRequestParams params, Ptr<Packet> asdu)
{
    NS_LOG_FUNCTION(this);

    // Fill APSDE-data.confirm parameters in case we need to return an error
    ApsdeDataConfirmParams confirmParams;
    confirmParams.m_dstAddrMode = params.m_dstAddrMode;
    confirmParams.m_dstAddr16 = params.m_dstAddr16;
    confirmParams.m_dstAddr64 = params.m_dstAddr64;
    confirmParams.m_dstEndPoint = params.m_dstEndPoint;
    confirmParams.m_srcEndPoint = params.m_srcEndPoint;

    // APS Header
    ZigbeeApsTxOptions txOptions(params.m_txOptions);
    ZigbeeApsHeader apsHeader;
    apsHeader.SetFrameType(ApsFrameType::APS_DATA);
    apsHeader.SetSrcEndpoint(params.m_srcEndPoint);
    apsHeader.SetProfileId(params.m_profileId);
    apsHeader.SetClusterId(params.m_clusterId);
    apsHeader.SetExtHeaderPresent(false);
    apsHeader.SetDeliveryMode(ApsDeliveryMode::APS_UCST);

    // NLDE-data.request params
    NldeDataRequestParams nwkParams;
    nwkParams.m_radius = params.m_radius;
    nwkParams.m_discoverRoute = DiscoverRouteType::ENABLE_ROUTE_DISCOVERY;
    nwkParams.m_securityEnable = txOptions.IsSecurityEnabled();
    nwkParams.m_dstAddrMode = AddressMode::UCST_BCST;

    SrcBindingEntry srcEntry;
    std::vector<DstBindingEntry> dstEntries;

    if (m_apsBindingTable.LookUpEntries(srcEntry, dstEntries))
    {
        for (const auto& dst : dstEntries)
        {
            if (dst.GetDstAddrMode() == ApsDstAddressModeBind::DST_ADDR64_DST_ENDPOINT_PRESENT)
            {
                // We must look into the nwkAddressMap to transform the
                // 64 bit address destination to a 16 bit destination
                NS_LOG_WARN("Bound destination found but 64bit destination not supported");
                // TODO: drop trace here
            }
            else
            {
                // Send a UCST message to each destination
                nwkParams.m_dstAddr = dst.GetDstAddr16();
                apsHeader.SetApsCounter(m_apsCounter.GetValue());
                m_apsCounter++;
                Ptr<Packet> p = asdu->Copy();
                p->AddHeader(apsHeader);
                Simulator::ScheduleNow(&ZigbeeNwk::NldeDataRequest, m_nwk, nwkParams, p);
            }
        }

        if (!m_apsdeDataConfirmCallback.IsNull())
        {
            confirmParams.m_status = ApsStatus::SUCCESS;
            confirmParams.m_txTime = Simulator::Now();
            m_apsdeDataConfirmCallback(confirmParams);
        }
    }
    else
    {
        if (!m_apsdeDataConfirmCallback.IsNull())
        {
            confirmParams.m_status = ApsStatus::NO_BOUND_DEVICE;
            confirmParams.m_txTime = Simulator::Now();
            m_apsdeDataConfirmCallback(confirmParams);
        }
    }
}

void
ZigbeeAps::SendDataUcstBcst(ApsdeDataRequestParams params, Ptr<Packet> asdu)
{
    NS_LOG_FUNCTION(this);

    // Fill APSDE-data.confirm parameters in case we need to return an error
    ApsdeDataConfirmParams confirmParams;
    confirmParams.m_dstAddrMode = params.m_dstAddrMode;
    confirmParams.m_dstAddr16 = params.m_dstAddr16;
    confirmParams.m_dstAddr64 = params.m_dstAddr64;
    confirmParams.m_dstEndPoint = params.m_dstEndPoint;
    confirmParams.m_srcEndPoint = params.m_srcEndPoint;

    // APS Header
    ZigbeeApsTxOptions txOptions(params.m_txOptions);
    ZigbeeApsHeader apsHeader;
    apsHeader.SetFrameType(ApsFrameType::APS_DATA);
    apsHeader.SetSrcEndpoint(params.m_srcEndPoint);
    apsHeader.SetProfileId(params.m_profileId);
    apsHeader.SetClusterId(params.m_clusterId);
    apsHeader.SetExtHeaderPresent(false);
    apsHeader.SetDeliveryMode(ApsDeliveryMode::APS_UCST);

    // NLDE-data.request params
    NldeDataRequestParams nwkParams;
    nwkParams.m_radius = params.m_radius;
    nwkParams.m_discoverRoute = DiscoverRouteType::ENABLE_ROUTE_DISCOVERY;
    nwkParams.m_securityEnable = txOptions.IsSecurityEnabled();
    nwkParams.m_dstAddrMode = AddressMode::UCST_BCST;

    if (params.m_dstAddr16 == "FF:FF" || params.m_dstAddr16 == "FF:FD" ||
        params.m_dstAddr16 == "FF:FC" || params.m_dstAddr16 == "FF:FB")
    {
        // Destination is a broadcast address
        apsHeader.SetDeliveryMode(ApsDeliveryMode::APS_BCST);
    }
    else
    {
        // Destination is a unicast address
        apsHeader.SetDeliveryMode(ApsDeliveryMode::APS_UCST);
    }

    if (params.m_useAlias)
    {
        if (txOptions.IsAckRequired()) // 0b1 = 00000001 = 1 dec
        {
            if (!m_apsdeDataConfirmCallback.IsNull())
            {
                confirmParams.m_status = ApsStatus::NOT_SUPPORTED;
                confirmParams.m_txTime = Simulator::Now();
                m_apsdeDataConfirmCallback(confirmParams);
            }
            return;
        }

        nwkParams.m_useAlias = params.m_useAlias;
        nwkParams.m_aliasSeqNumber = params.m_aliasSeqNumb;
        nwkParams.m_aliasSrcAddr = params.m_aliasSrcAddr;

        apsHeader.SetApsCounter(params.m_aliasSeqNumb);
    }
    else
    {
        apsHeader.SetApsCounter(m_apsCounter.GetValue());
        m_apsCounter++;
    }

    nwkParams.m_dstAddrMode = AddressMode::UCST_BCST;
    nwkParams.m_dstAddr = params.m_dstAddr16;
    apsHeader.SetDstEndpoint(params.m_dstEndPoint);

    asdu->AddHeader(apsHeader);

    Simulator::ScheduleNow(&ZigbeeNwk::NldeDataRequest, m_nwk, nwkParams, asdu);
}

void
ZigbeeAps::ApsmeBindRequest(ApsmeBindRequestParams params)
{
    ApsmeBindConfirmParams confirmParams;
    confirmParams.m_srcAddr = params.m_srcAddr;
    confirmParams.m_srcEndPoint = params.m_srcEndPoint;
    confirmParams.m_clusterId = params.m_clusterId;
    confirmParams.m_dstAddr16 = params.m_dstAddr16;
    confirmParams.m_dstAddr64 = params.m_dstAddr64;
    confirmParams.m_dstAddrMode = params.m_dstAddrMode;
    confirmParams.m_dstEndPoint = params.m_dstEndPoint;

    // TODO: confirm the device has joined the network
    // How? APS have no access to join information.

    // Verify params are in valid range (2.2.4.3.1)
    if ((params.m_srcEndPoint < 0x01 || params.m_srcEndPoint > 0xfe) ||
        (params.m_dstEndPoint < 0x01))
    {
        if (!m_apsmeBindConfirmCallback.IsNull())
        {
            confirmParams.m_status = ApsStatus::ILLEGAL_REQUEST;
            m_apsmeBindConfirmCallback(confirmParams);
        }
        return;
    }

    SrcBindingEntry srcEntry(params.m_srcAddr, params.m_srcEndPoint, params.m_clusterId);

    DstBindingEntry dstEntry;

    if (params.m_dstAddrMode == ApsDstAddressModeBind::GROUP_ADDR_DST_ENDPOINT_NOT_PRESENT)
    {
        // Group Addressing binding
        dstEntry.SetDstAddrMode(ApsDstAddressModeBind::GROUP_ADDR_DST_ENDPOINT_NOT_PRESENT);
        dstEntry.SetDstAddr16(params.m_dstAddr16);
    }
    else
    {
        // Unicast binding
        dstEntry.SetDstAddrMode(ApsDstAddressModeBind::DST_ADDR64_DST_ENDPOINT_PRESENT);
        dstEntry.SetDstEndPoint(params.m_dstEndPoint);
    }

    switch (m_apsBindingTable.Bind(srcEntry, dstEntry))
    {
    case BindingTableStatus::BOUND:
        confirmParams.m_status = ApsStatus::SUCCESS;
        break;
    case BindingTableStatus::ENTRY_EXISTS:
        confirmParams.m_status = ApsStatus::INVALID_BINDING;
        break;
    case BindingTableStatus::TABLE_FULL:
        confirmParams.m_status = ApsStatus::TABLE_FULL;
        break;
    default:
        NS_LOG_ERROR("Invalid binding option");
    }

    if (!m_apsmeBindConfirmCallback.IsNull())
    {
        m_apsmeBindConfirmCallback(confirmParams);
    }
}

void
ZigbeeAps::ApsmeUnbindRequest(ApsmeBindRequestParams params)
{
}

void
ZigbeeAps::NldeDataConfirm(NldeDataConfirmParams params)
{
}

void
ZigbeeAps::NldeDataIndication(NldeDataIndicationParams params, Ptr<Packet> nsdu)
{
    NS_LOG_FUNCTION(this);

    ZigbeeApsHeader apsHeader;
    nsdu->RemoveHeader(apsHeader);

    ApsdeDataIndicationParams indicationParams;
    indicationParams.m_status = ApsStatus::SUCCESS;

    // TODO:
    // See section 2.2.4.1.3.
    // - Handle Security
    // - Handle grouping(MCST) (Note group table shared by NWK and APS)
    // - Handle binding
    // - Handle fragmentation
    // - Detect Duplicates
    // - Handle ACK

    // Check if packet is fragmented
    if (apsHeader.IsExtHeaderPresent())
    {
        indicationParams.m_status = ApsStatus::DEFRAG_UNSUPPORTED;
        if (!m_apsdeDataIndicationCallback.IsNull())
        {
            m_apsdeDataIndicationCallback(indicationParams, nsdu);
        }
        NS_LOG_WARN("Extended Header (Fragmentation) not supported");
        return;
    }

    if (apsHeader.GetFrameType() == ApsFrameType::APS_DATA)
    {
        if (apsHeader.GetDeliveryMode() == ApsDeliveryMode::APS_UCST ||
            apsHeader.GetDeliveryMode() == ApsDeliveryMode::APS_BCST)
        {
            indicationParams.m_dstAddrMode = ApsDstAddressMode::DST_ADDR16_DST_ENDPOINT_PRESENT;
            // Note: Extracting the Address directly from the NWK, creates a dependency on this NWK
            // implementation. This is not a very good design, but in practice, it is unavoidable
            // due to the descriptions in the specification.
            indicationParams.m_dstAddr16 = m_nwk->GetNetworkAddress();
            indicationParams.m_dstEndPoint = apsHeader.GetDstEndpoint();
            indicationParams.m_srcAddrMode = ApsSrcAddressMode::SRC_ADDR16_SRC_ENDPOINT_PRESENT;
            indicationParams.m_srcAddress16 = params.m_srcAddr;
            indicationParams.m_srcEndpoint = apsHeader.GetSrcEndpoint();
            indicationParams.m_profileId = apsHeader.GetProfileId();
            indicationParams.m_clusterId = apsHeader.GetClusterId();
            indicationParams.asduLength = nsdu->GetSize();
            indicationParams.m_securityStatus = ApsSecurityStatus::UNSECURED;
            indicationParams.m_linkQuality = params.m_linkQuality;
            indicationParams.m_rxTime = Simulator::Now();

            if (!m_apsdeDataIndicationCallback.IsNull())
            {
                m_apsdeDataIndicationCallback(indicationParams, nsdu);
            }
        }
        else
        {
            // TODO: Group deliveryMode == (MCST)
            NS_LOG_WARN("Group delivery not supported");
        }
    }
}

void
ZigbeeAps::SetApsdeDataConfirmCallback(ApsdeDataConfirmCallback c)
{
    m_apsdeDataConfirmCallback = c;
}

void
ZigbeeAps::SetApsdeDataIndicationCallback(ApsdeDataIndicationCallback c)
{
    m_apsdeDataIndicationCallback = c;
}

void
ZigbeeAps::SetApsmeBindConfirmCallback(ApsmeBindConfirmCallback c)
{
    m_apsmeBindConfirmCallback = c;
}

void
ZigbeeAps::SetApsmeUnbindConfirmCallback(ApsmeUnbindConfirmCallback c)
{
    m_apsmeUnbindConfirmCallback = c;
}

//////////////////////////
//  ZigbeeApsTxOptions  //
//////////////////////////

ZigbeeApsTxOptions::ZigbeeApsTxOptions(uint8_t value)
    : m_txOptions(value)
{
}

void
ZigbeeApsTxOptions::SetSecurityEnabled(bool enable)
{
    SetBit(0, enable);
}

void
ZigbeeApsTxOptions::SetUseNwkKey(bool enable)
{
    SetBit(1, enable);
}

void
ZigbeeApsTxOptions::SetAckRequired(bool enable)
{
    SetBit(2, enable);
}

void
ZigbeeApsTxOptions::SetFragmentationPermitted(bool enable)
{
    SetBit(3, enable);
}

void
ZigbeeApsTxOptions::SetIncludeExtendedNonce(bool enable)
{
    SetBit(4, enable);
}

bool
ZigbeeApsTxOptions::IsSecurityEnabled() const
{
    return GetBit(0);
}

bool
ZigbeeApsTxOptions::IsUseNwkKey() const
{
    return GetBit(1);
}

bool
ZigbeeApsTxOptions::IsAckRequired() const
{
    return GetBit(2);
}

bool
ZigbeeApsTxOptions::IsFragmentationPermitted() const
{
    return GetBit(3);
}

bool
ZigbeeApsTxOptions::IsIncludeExtendedNonce() const
{
    return GetBit(4);
}

uint8_t
ZigbeeApsTxOptions::GetTxOptions() const
{
    return m_txOptions;
}

void
ZigbeeApsTxOptions::SetBit(int pos, bool value)
{
    if (value)
    {
        m_txOptions |= (1 << pos);
    }
    else
    {
        m_txOptions &= ~(1 << pos);
    }
}

bool
ZigbeeApsTxOptions::GetBit(int pos) const
{
    return (m_txOptions >> pos) & 1;
}

} // namespace zigbee
} // namespace ns3
