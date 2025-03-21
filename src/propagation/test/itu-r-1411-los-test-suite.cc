/*
 * Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#include "ns3/constant-position-mobility-model.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/itu-r-1411-los-propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ItuR1411LosPropagationLossModelTest");

/**
 * @ingroup propagation-tests
 *
 * @brief ItuR1411LosPropagationLossModel Test Case
 *
 */
class ItuR1411LosPropagationLossModelTestCase : public TestCase
{
  public:
    /**
     * Constructor
     *
     * @param freq carrier frequency in Hz
     * @param dist 2D distance between UT and BS in meters
     * @param hb height of BS in meters
     * @param hm height of UT in meters
     * @param refValue reference loss value
     * @param name TestCase name
     */
    ItuR1411LosPropagationLossModelTestCase(double freq,
                                            double dist,
                                            double hb,
                                            double hm,
                                            double refValue,
                                            std::string name);
    ~ItuR1411LosPropagationLossModelTestCase() override;

  private:
    void DoRun() override;

    /**
     * Create a MobilityModel
     * @param index mobility model index
     * @return a new MobilityModel
     */
    Ptr<MobilityModel> CreateMobilityModel(uint16_t index);

    double m_freq;    //!< carrier frequency in Hz
    double m_dist;    //!< 2D distance between UT and BS in meters
    double m_hb;      //!< height of BS in meters
    double m_hm;      //!< height of UT in meters
    double m_lossRef; //!< reference loss
};

ItuR1411LosPropagationLossModelTestCase::ItuR1411LosPropagationLossModelTestCase(double freq,
                                                                                 double dist,
                                                                                 double hb,
                                                                                 double hm,
                                                                                 double refValue,
                                                                                 std::string name)
    : TestCase(name),
      m_freq(freq),
      m_dist(dist),
      m_hb(hb),
      m_hm(hm),
      m_lossRef(refValue)
{
}

ItuR1411LosPropagationLossModelTestCase::~ItuR1411LosPropagationLossModelTestCase()
{
}

void
ItuR1411LosPropagationLossModelTestCase::DoRun()
{
    NS_LOG_FUNCTION(this);

    Ptr<MobilityModel> mma = CreateObject<ConstantPositionMobilityModel>();
    mma->SetPosition(Vector(0.0, 0.0, m_hb));

    Ptr<MobilityModel> mmb = CreateObject<ConstantPositionMobilityModel>();
    mmb->SetPosition(Vector(m_dist, 0.0, m_hm));

    Ptr<ItuR1411LosPropagationLossModel> propagationLossModel =
        CreateObject<ItuR1411LosPropagationLossModel>();
    propagationLossModel->SetAttribute("Frequency", DoubleValue(m_freq));

    double loss = propagationLossModel->GetLoss(mma, mmb);

    NS_LOG_INFO("Calculated loss: " << loss);
    NS_LOG_INFO("Theoretical loss: " << m_lossRef);

    NS_TEST_ASSERT_MSG_EQ_TOL(loss, m_lossRef, 0.1, "Wrong loss!");
}

/**
 * @ingroup propagation-tests
 *
 * @brief ItuR1411LosPropagationLossModel TestSuite
 *
 */
class ItuR1411LosPropagationLossModelTestSuite : public TestSuite
{
  public:
    ItuR1411LosPropagationLossModelTestSuite();
};

ItuR1411LosPropagationLossModelTestSuite::ItuR1411LosPropagationLossModelTestSuite()
    : TestSuite("itu-r-1411-los", Type::SYSTEM)
{
    LogComponentEnable("ItuR1411LosPropagationLossModelTest", LOG_LEVEL_ALL);

    // reference values obtained with the octave scripts in src/propagation/test/reference/
    AddTestCase(new ItuR1411LosPropagationLossModelTestCase(2.1140e9,
                                                            100,
                                                            30,
                                                            1,
                                                            81.005,
                                                            "freq=2114MHz, dist=100m"),
                TestCase::Duration::QUICK);
    AddTestCase(new ItuR1411LosPropagationLossModelTestCase(1999e6,
                                                            200,
                                                            30,
                                                            1,
                                                            87.060,
                                                            "freq=1999MHz, dist=200m"),
                TestCase::Duration::QUICK);
}

/// Static variable for test initialization
static ItuR1411LosPropagationLossModelTestSuite g_ituR1411LosTestSuite;
