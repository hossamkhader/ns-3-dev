/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "lte-test-ue-phy.h"

#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("LteTestUePhy");

NS_OBJECT_ENSURE_REGISTERED(LteTestUePhy);

LteTestUePhy::LteTestUePhy()
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("This constructor should not be called");
}

LteTestUePhy::LteTestUePhy(Ptr<LteSpectrumPhy> dlPhy, Ptr<LteSpectrumPhy> ulPhy)
    : LtePhy(dlPhy, ulPhy)
{
    NS_LOG_FUNCTION(this);
}

LteTestUePhy::~LteTestUePhy()
{
}

void
LteTestUePhy::DoDispose()
{
    NS_LOG_FUNCTION(this);

    LtePhy::DoDispose();
}

TypeId
LteTestUePhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LteTestUePhy").SetParent<LtePhy>().AddConstructor<LteTestUePhy>();
    return tid;
}

void
LteTestUePhy::DoSendMacPdu(Ptr<Packet> p)
{
    NS_LOG_FUNCTION(this);
}

Ptr<SpectrumValue>
LteTestUePhy::CreateTxPowerSpectralDensity()
{
    NS_LOG_FUNCTION(this);
    Ptr<SpectrumValue> psd;

    return psd;
}

void
LteTestUePhy::GenerateCtrlCqiReport(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);

    // Store calculated SINR, it will be retrieved at the end of the test
    m_sinr = sinr;
}

void
LteTestUePhy::GenerateDataCqiReport(const SpectrumValue& sinr)
{
    NS_LOG_FUNCTION(this);

    // Store calculated SINR, it will be retrieved at the end of the test
    m_sinr = sinr;
}

void
LteTestUePhy::ReportRsReceivedPower(const SpectrumValue& power)
{
    NS_LOG_FUNCTION(this);
    // Not used by the LteTestUePhy
}

void
LteTestUePhy::ReportInterference(const SpectrumValue& interf)
{
    NS_LOG_FUNCTION(this);
    // Not used by the LteTestUePhy
}

void
LteTestUePhy::ReceiveLteControlMessage(Ptr<LteControlMessage> msg)
{
    NS_LOG_FUNCTION(this << msg);
}

SpectrumValue
LteTestUePhy::GetSinr()
{
    NS_LOG_FUNCTION(this);

    return m_sinr;
}

} // namespace ns3
