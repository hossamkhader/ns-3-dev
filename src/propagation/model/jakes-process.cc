/*
 * Copyright (c) 2012 Telum (www.telum.ru)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Kirill Andreev <andreev@telum.ru>, Alexander Sofronov <sofronov@telum.ru>
 */

#include "jakes-process.h"

#include "jakes-propagation-loss-model.h"
#include "propagation-loss-model.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("JakesProcess");

/// Represents a single oscillator
JakesProcess::Oscillator::Oscillator(std::complex<double> amplitude,
                                     double initialPhase,
                                     double omega)
    : m_amplitude(amplitude),
      m_phase(initialPhase),
      m_omega(omega)
{
}

std::complex<double>
JakesProcess::Oscillator::GetValueAt(Time at) const
{
    return (m_amplitude * std::cos(at.GetSeconds() * m_omega + m_phase));
}

NS_OBJECT_ENSURE_REGISTERED(JakesProcess);

TypeId
JakesProcess::GetTypeId()
{
    static TypeId tid = TypeId("ns3::JakesProcess")
                            .SetParent<Object>()
                            .SetGroupName("Propagation")
                            .AddConstructor<JakesProcess>()
                            .AddAttribute("DopplerFrequencyHz",
                                          "Corresponding doppler frequency[Hz]",
                                          DoubleValue(80),
                                          MakeDoubleAccessor(&JakesProcess::SetDopplerFrequencyHz),
                                          MakeDoubleChecker<double>(0.0, 1e4))
                            .AddAttribute("NumberOfOscillators",
                                          "The number of oscillators",
                                          UintegerValue(20),
                                          MakeUintegerAccessor(&JakesProcess::SetNOscillators),
                                          MakeUintegerChecker<unsigned int>(4, 1000));
    return tid;
}

void
JakesProcess::SetPropagationLossModel(Ptr<const PropagationLossModel> propagationModel)
{
    Ptr<const JakesPropagationLossModel> jakes =
        propagationModel->GetObject<JakesPropagationLossModel>();
    NS_ASSERT_MSG(jakes, "Jakes Process can work only with JakesPropagationLossModel!");
    m_jakes = jakes;

    NS_ASSERT(m_nOscillators != 0);
    NS_ASSERT(m_omegaDopplerMax != 0);

    ConstructOscillators();
}

void
JakesProcess::SetNOscillators(unsigned int nOscillators)
{
    m_nOscillators = nOscillators;
}

void
JakesProcess::SetDopplerFrequencyHz(double dopplerFrequencyHz)
{
    m_omegaDopplerMax = 2 * dopplerFrequencyHz * M_PI;
}

void
JakesProcess::ConstructOscillators()
{
    NS_ASSERT(m_jakes);
    // Initial phase is common for all oscillators:
    double phi = m_jakes->GetUniformRandomVariable()->GetValue();
    // Theta is common for all oscillators:
    double theta = m_jakes->GetUniformRandomVariable()->GetValue();
    for (unsigned int i = 0; i < m_nOscillators; i++)
    {
        unsigned int n = i + 1;
        /// 1. Rotation speed
        /// 1a. Initiate \f[ \alpha_n = \frac{2\pi n - \pi + \theta}{4M},  n=1,2, \ldots,M\f], n is
        /// oscillatorNumber, M is m_nOscillators
        double alpha = (2.0 * M_PI * n - M_PI + theta) / (4.0 * m_nOscillators);
        /// 1b. Initiate rotation speed:
        double omega = m_omegaDopplerMax * std::cos(alpha);
        /// 2. Initiate complex amplitude:
        double psi = m_jakes->GetUniformRandomVariable()->GetValue();
        std::complex<double> amplitude =
            std::complex<double>(std::cos(psi), std::sin(psi)) * 2.0 / std::sqrt(m_nOscillators);
        /// 3. Construct oscillator:
        m_oscillators.emplace_back(amplitude, phi, omega);
    }
}

JakesProcess::JakesProcess()
    : m_omegaDopplerMax(0),
      m_nOscillators(0)
{
}

JakesProcess::~JakesProcess()
{
    m_oscillators.clear();
}

void
JakesProcess::DoDispose()
{
    m_uniformVariable = nullptr;
    m_jakes = nullptr;
}

std::complex<double>
JakesProcess::GetComplexGain() const
{
    std::complex<double> sumAmplitude = std::complex<double>(0, 0);
    for (unsigned int i = 0; i < m_oscillators.size(); i++)
    {
        sumAmplitude += m_oscillators[i].GetValueAt(Now());
    }
    return sumAmplitude;
}

double
JakesProcess::GetChannelGainDb() const
{
    std::complex<double> complexGain = GetComplexGain();
    return (10 *
            std::log10((std::pow(complexGain.real(), 2) + std::pow(complexGain.imag(), 2)) / 2));
}

} // namespace ns3
