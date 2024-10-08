/*
 * Copyright (c) 2009 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Leonard Tracy <lentracy@gmail.com>
 */

#include "uan-mac-cw.h"

#include "uan-header-common.h"

#include "ns3/attribute.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("UanMacCw");

NS_OBJECT_ENSURE_REGISTERED(UanMacCw);

UanMacCw::UanMacCw()
    : UanMac(),
      m_phy(nullptr),
      m_pktTx(nullptr),
      m_txOngoing(false),
      m_state(IDLE),
      m_cleared(false)

{
    m_rv = CreateObject<UniformRandomVariable>();
}

UanMacCw::~UanMacCw()
{
}

void
UanMacCw::Clear()
{
    if (m_cleared)
    {
        return;
    }
    m_cleared = true;
    m_pktTx = nullptr;
    if (m_phy)
    {
        m_phy->Clear();
        m_phy = nullptr;
    }
    m_sendEvent.Cancel();
    m_txOngoing = false;
}

void
UanMacCw::DoDispose()
{
    Clear();
    UanMac::DoDispose();
}

TypeId
UanMacCw::GetTypeId()
{
    static TypeId tid = TypeId("ns3::UanMacCw")
                            .SetParent<UanMac>()
                            .SetGroupName("Uan")
                            .AddConstructor<UanMacCw>()
                            .AddAttribute("CW",
                                          "The MAC parameter CW.",
                                          UintegerValue(10),
                                          MakeUintegerAccessor(&UanMacCw::m_cw),
                                          MakeUintegerChecker<uint32_t>())
                            .AddAttribute("SlotTime",
                                          "Time slot duration for MAC backoff.",
                                          TimeValue(MilliSeconds(20)),
                                          MakeTimeAccessor(&UanMacCw::m_slotTime),
                                          MakeTimeChecker())
                            .AddTraceSource("Enqueue",
                                            "A packet arrived at the MAC for transmission.",
                                            MakeTraceSourceAccessor(&UanMacCw::m_enqueueLogger),
                                            "ns3::UanMacCw::QueueTracedCallback")
                            .AddTraceSource("Dequeue",
                                            "A was passed down to the PHY from the MAC.",
                                            MakeTraceSourceAccessor(&UanMacCw::m_dequeueLogger),
                                            "ns3::UanMacCw::QueueTracedCallback")
                            .AddTraceSource("RX",
                                            "A packet was destined for this MAC and was received.",
                                            MakeTraceSourceAccessor(&UanMacCw::m_rxLogger),
                                            "ns3::UanMac::PacketModeTracedCallback")

        ;
    return tid;
}

bool
UanMacCw::Enqueue(Ptr<Packet> packet, uint16_t protocolNumber, const Address& dest)
{
    switch (m_state)
    {
    case CCABUSY:
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " MAC " << GetAddress()
                             << " Starting enqueue CCABUSY");
        if (m_txOngoing)
        {
            NS_LOG_DEBUG("State is TX");
        }
        else
        {
            NS_LOG_DEBUG("State is not TX");
        }

        NS_ASSERT(!m_phy->GetTransducer()->GetArrivalList().empty() || m_phy->IsStateTx());
        return false;
    case RUNNING:
        NS_LOG_DEBUG("MAC " << GetAddress() << " Starting enqueue RUNNING");
        NS_ASSERT(m_phy->GetTransducer()->GetArrivalList().empty() && !m_phy->IsStateTx());
        return false;
    case TX:
    case IDLE: {
        NS_ASSERT(!m_pktTx);

        UanHeaderCommon header;
        header.SetDest(Mac8Address::ConvertFrom(dest));
        header.SetSrc(Mac8Address::ConvertFrom(GetAddress()));
        header.SetType(0);
        header.SetProtocolNumber(0);
        packet->AddHeader(header);

        m_enqueueLogger(packet, GetTxModeIndex());

        if (m_phy->IsStateBusy())
        {
            m_pktTx = packet;
            m_pktTxProt = GetTxModeIndex();
            m_state = CCABUSY;
            auto cw = (uint32_t)m_rv->GetValue(0, m_cw);
            m_savedDelayS = cw * m_slotTime;
            m_sendTime = Simulator::Now() + m_savedDelayS;
            NS_LOG_DEBUG("Time " << Now().As(Time::S) << ": Addr " << GetAddress()
                                 << ": Enqueuing new packet while busy:  (Chose CW " << cw
                                 << ", Sending at " << m_sendTime.As(Time::S)
                                 << " Packet size: " << packet->GetSize());
            NS_ASSERT(!m_phy->GetTransducer()->GetArrivalList().empty() || m_phy->IsStateTx());
        }
        else
        {
            NS_ASSERT(m_state != TX);
            NS_LOG_DEBUG("Time " << Now().As(Time::S) << ": Addr " << GetAddress()
                                 << ": Enqueuing new packet while idle (sending)");
            NS_ASSERT(m_phy->GetTransducer()->GetArrivalList().empty() && !m_phy->IsStateTx());
            m_state = TX;
            m_phy->SendPacket(packet, GetTxModeIndex());
        }
        break;
    }
    default:
        NS_LOG_DEBUG("MAC " << GetAddress() << " Starting enqueue SOMETHING ELSE");
        return false;
    }

    return true;
}

void
UanMacCw::SetForwardUpCb(Callback<void, Ptr<Packet>, uint16_t, const Mac8Address&> cb)
{
    m_forwardUpCb = cb;
}

void
UanMacCw::AttachPhy(Ptr<UanPhy> phy)
{
    m_phy = phy;
    m_phy->SetReceiveOkCallback(MakeCallback(&UanMacCw::PhyRxPacketGood, this));
    m_phy->SetReceiveErrorCallback(MakeCallback(&UanMacCw::PhyRxPacketError, this));
    m_phy->RegisterListener(this);
}

void
UanMacCw::NotifyRxStart()
{
    if (m_state == RUNNING)
    {
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << ": Switching to channel busy");
        SaveTimer();
        m_state = CCABUSY;
    }
}

void
UanMacCw::NotifyRxEndOk()
{
    if (m_state == CCABUSY && !m_phy->IsStateCcaBusy())
    {
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << ": Switching to channel idle");
        m_state = RUNNING;
        StartTimer();
    }
}

void
UanMacCw::NotifyRxEndError()
{
    if (m_state == CCABUSY && !m_phy->IsStateCcaBusy())
    {
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << ": Switching to channel idle");
        m_state = RUNNING;
        StartTimer();
    }
}

void
UanMacCw::NotifyCcaStart()
{
    if (m_state == RUNNING)
    {
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << ": Switching to channel busy");
        m_state = CCABUSY;
        SaveTimer();
    }
}

void
UanMacCw::NotifyCcaEnd()
{
    if (m_state == CCABUSY)
    {
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << ": Switching to channel idle");
        m_state = RUNNING;
        StartTimer();
    }
}

void
UanMacCw::NotifyTxStart(Time duration)
{
    m_txOngoing = true;

    NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Tx Start Notified");

    if (m_state == RUNNING)
    {
        NS_ASSERT(0);
        m_state = CCABUSY;
        SaveTimer();
    }
}

void
UanMacCw::NotifyTxEnd()
{
    m_txOngoing = false;

    EndTx();
}

int64_t
UanMacCw::AssignStreams(int64_t stream)
{
    NS_LOG_FUNCTION(this << stream);
    m_rv->SetStream(stream);
    return 1;
}

void
UanMacCw::EndTx()
{
    NS_ASSERT(m_state == TX || m_state == CCABUSY);
    if (m_state == TX)
    {
        m_state = IDLE;
    }
    else if (m_state == CCABUSY)
    {
        if (m_phy->IsStateIdle())
        {
            NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                                 << ": Switching to channel idle (After TX!)");
            m_state = RUNNING;
            StartTimer();
        }
    }
    else
    {
        NS_FATAL_ERROR("In strange state at UanMacCw EndTx");
    }
}

void
UanMacCw::SetCw(uint32_t cw)
{
    m_cw = cw;
}

void
UanMacCw::SetSlotTime(Time duration)
{
    m_slotTime = duration;
}

uint32_t
UanMacCw::GetCw()
{
    return m_cw;
}

Time
UanMacCw::GetSlotTime()
{
    return m_slotTime;
}

void
UanMacCw::PhyRxPacketGood(Ptr<Packet> packet, double /* sinr */, UanTxMode /* mode */)
{
    UanHeaderCommon header;
    packet->RemoveHeader(header);

    if (header.GetDest() == Mac8Address::ConvertFrom(GetAddress()) ||
        header.GetDest() == Mac8Address::GetBroadcast())
    {
        m_forwardUpCb(packet, header.GetProtocolNumber(), header.GetSrc());
    }
}

void
UanMacCw::PhyRxPacketError(Ptr<Packet> /* packet */, double /* sinr */)
{
}

void
UanMacCw::SaveTimer()
{
    NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                         << " Saving timer (Delay = "
                         << (m_savedDelayS = m_sendTime - Now()).As(Time::S) << ")");
    NS_ASSERT(m_pktTx);
    NS_ASSERT(m_sendTime >= Simulator::Now());
    m_savedDelayS = m_sendTime - Simulator::Now();
    Simulator::Cancel(m_sendEvent);
}

void
UanMacCw::StartTimer()
{
    m_sendTime = Simulator::Now() + m_savedDelayS;
    if (m_sendTime == Simulator::Now())
    {
        SendPacket();
    }
    else
    {
        m_sendEvent = Simulator::Schedule(m_savedDelayS, &UanMacCw::SendPacket, this);
        NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress()
                             << " Starting timer (New send time = " << this->m_sendTime.As(Time::S)
                             << ")");
    }
}

void
UanMacCw::SendPacket()
{
    NS_LOG_DEBUG("Time " << Now().As(Time::S) << " Addr " << GetAddress() << " Transmitting ");
    NS_ASSERT(m_state == RUNNING);
    m_state = TX;
    m_phy->SendPacket(m_pktTx, m_pktTxProt);
    m_pktTx = nullptr;
    m_sendTime = Seconds(0);
    m_savedDelayS = Seconds(0);
}

} // namespace ns3
