/*
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
 * University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#include "ns3/abort.h"
#include "ns3/buildings-channel-condition-model.h"
#include "ns3/buildings-module.h"
#include "ns3/config.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("BuildingsChannelConditionModelsTest");

/**
 * @ingroup building-test
 *
 * Test case for the class BuildingsChannelConditionModel. It checks if the
 * channel condition is correctly determined when a building is deployed in the
 * scenario
 */
class BuildingsChannelConditionModelTestCase : public TestCase
{
  public:
    /**
     * Constructor
     */
    BuildingsChannelConditionModelTestCase();

    /**
     * Destructor
     */
    ~BuildingsChannelConditionModelTestCase() override;

  private:
    /**
     * Builds the simulation scenario and perform the tests
     */
    void DoRun() override;

    /**
     * Struct containing the parameters for each test
     */
    struct TestVector
    {
        Vector m_positionA;                            //!< the position of the first node
        Vector m_positionB;                            //!< the position of the second node
        ChannelCondition::LosConditionValue m_losCond; //!< the correct channel condition
    };

    TestVectors<TestVector> m_testVectors; //!< array containing all the test vectors
};

BuildingsChannelConditionModelTestCase::BuildingsChannelConditionModelTestCase()
    : TestCase("Test case for the BuildingsChannelConditionModel"),
      m_testVectors()
{
}

BuildingsChannelConditionModelTestCase::~BuildingsChannelConditionModelTestCase()
{
}

void
BuildingsChannelConditionModelTestCase::DoRun()
{
    TestVector testVector;

    testVector.m_positionA = Vector(0.0, 5.0, 1.5);
    testVector.m_positionB = Vector(20.0, 5.0, 1.5);
    testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
    m_testVectors.Add(testVector);

    testVector.m_positionA = Vector(0.0, 11.0, 1.5);
    testVector.m_positionB = Vector(20.0, 11.0, 1.5);
    testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
    m_testVectors.Add(testVector);

    testVector.m_positionA = Vector(5.0, 5.0, 1.5);
    testVector.m_positionB = Vector(20.0, 5.0, 1.5);
    testVector.m_losCond = ChannelCondition::LosConditionValue::NLOS;
    m_testVectors.Add(testVector);

    testVector.m_positionA = Vector(4.0, 5.0, 1.5);
    testVector.m_positionB = Vector(5.0, 5.0, 1.5);
    testVector.m_losCond = ChannelCondition::LosConditionValue::LOS;
    m_testVectors.Add(testVector);

    // Deploy nodes and building and get the channel condition
    NodeContainer nodes;
    nodes.Create(2);

    Ptr<MobilityModel> a = CreateObject<ConstantPositionMobilityModel>();
    nodes.Get(0)->AggregateObject(a);

    Ptr<MobilityModel> b = CreateObject<ConstantPositionMobilityModel>();
    nodes.Get(1)->AggregateObject(b);

    Ptr<BuildingsChannelConditionModel> condModel = CreateObject<BuildingsChannelConditionModel>();

    auto building = CreateObject<Building>();
    building->SetNRoomsX(1);
    building->SetNRoomsY(1);
    building->SetNFloors(1);
    building->SetBoundaries(Box(0.0, 10.0, 0.0, 10.0, 0.0, 5.0));
    building->SetExtWallsType(Building::ExtWallsType_t::Wood);

    BuildingsHelper::Install(nodes);

    for (uint32_t i = 0; i < m_testVectors.GetN(); ++i)
    {
        testVector = m_testVectors.Get(i);
        a->SetPosition(testVector.m_positionA);
        b->SetPosition(testVector.m_positionB);
        Ptr<MobilityBuildingInfo> buildingInfoA = a->GetObject<MobilityBuildingInfo>();
        buildingInfoA->MakeConsistent(a);
        Ptr<MobilityBuildingInfo> buildingInfoB = b->GetObject<MobilityBuildingInfo>();
        buildingInfoA->MakeConsistent(b);
        Ptr<ChannelCondition> cond = condModel->GetChannelCondition(a, b);

        NS_LOG_DEBUG("Got " << cond->GetLosCondition() << " expected condition "
                            << testVector.m_losCond);
        NS_TEST_ASSERT_MSG_EQ(cond->GetLosCondition(),
                              testVector.m_losCond,
                              " Got unexpected channel condition");
    }

    Simulator::Destroy();
}

/**
 * @ingroup building-test
 * Test suite for the buildings channel condition model
 */
class BuildingsChannelConditionModelsTestSuite : public TestSuite
{
  public:
    BuildingsChannelConditionModelsTestSuite();
};

BuildingsChannelConditionModelsTestSuite::BuildingsChannelConditionModelsTestSuite()
    : TestSuite("buildings-channel-condition-model", Type::UNIT)
{
    AddTestCase(new BuildingsChannelConditionModelTestCase, TestCase::Duration::QUICK);
}

/// Static variable for test initialization
static BuildingsChannelConditionModelsTestSuite BuildingsChannelConditionModelsTestSuite;
