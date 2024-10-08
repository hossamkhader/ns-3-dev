/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

#include "snr-to-block-error-rate-record.h"

#include "ns3/assert.h"
#include "ns3/simulator.h"

namespace ns3
{

SNRToBlockErrorRateRecord::SNRToBlockErrorRateRecord(double snrValue,
                                                     double bitErrorRate,
                                                     double blockErrorRate,
                                                     double sigma2,
                                                     double I1,
                                                     double I2)
{
    m_snrValue = snrValue;
    m_bitErrorRate = bitErrorRate;
    m_blockErrorRate = blockErrorRate;
    m_sigma2 = sigma2;
    m_i1 = I1;
    m_i2 = I2;
}

SNRToBlockErrorRateRecord*
SNRToBlockErrorRateRecord::Copy() const
{
    return (new SNRToBlockErrorRateRecord(m_snrValue,
                                          m_bitErrorRate,
                                          m_blockErrorRate,
                                          m_sigma2,
                                          m_i1,
                                          m_i2));
}

double
SNRToBlockErrorRateRecord::GetSNRValue() const
{
    return m_snrValue;
}

SNRToBlockErrorRateRecord::~SNRToBlockErrorRateRecord()
{
    m_snrValue = 0;
    m_bitErrorRate = 0;
    m_blockErrorRate = 0;
    m_sigma2 = 0;
    m_i1 = 0;
    m_i2 = 0;
}

double
SNRToBlockErrorRateRecord::GetBitErrorRate() const
{
    return m_bitErrorRate;
}

double
SNRToBlockErrorRateRecord::GetBlockErrorRate() const
{
    return m_blockErrorRate;
}

double
SNRToBlockErrorRateRecord::GetSigma2() const
{
    return m_sigma2;
}

double
SNRToBlockErrorRateRecord::GetI1() const
{
    return m_i1;
}

double
SNRToBlockErrorRateRecord::GetI2() const
{
    return m_i2;
}

void
SNRToBlockErrorRateRecord::SetSNRValue(double snrValue)
{
    m_snrValue = snrValue;
}

void
SNRToBlockErrorRateRecord::SetBitErrorRate(double bitErrorRate)
{
    m_bitErrorRate = bitErrorRate;
}

void
SNRToBlockErrorRateRecord::SetBlockErrorRate(double blockErrorRate)
{
    m_blockErrorRate = blockErrorRate;
}

void
SNRToBlockErrorRateRecord::SetI1(double i1)
{
    m_i1 = i1;
}

void
SNRToBlockErrorRateRecord::SetI2(double i2)
{
    m_i2 = i2;
}

} // namespace ns3
