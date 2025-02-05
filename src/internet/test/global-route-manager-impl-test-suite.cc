/*
 * Copyright 2007 University of Washington
 * Copyright (C) 1999, 2000 Kunihiro Ishiguro, Toshiaki Takada
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors:  Tom Henderson (tomhend@u.washington.edu)
 *
 * Kunihiro Ishigura, Toshiaki Takada (GNU Zebra) are attributed authors
 * of the quagga 0.99.7/src/ospfd/ospf_spf.c code which was ported here
 */

#include "ns3/candidate-queue.h"
#include "ns3/global-route-manager-impl.h"
#include "ns3/simulator.h"
#include "ns3/test.h"

#include <cstdlib> // for rand()

using namespace ns3;

/**
 * @ingroup internet
 * @ingroup tests
 * @defgroup internet-test internet module tests
 */

/**
 * @ingroup internet-test
 *
 * @brief Global Route Manager Test
 */
class GlobalRouteManagerImplTestCase : public TestCase
{
  public:
    GlobalRouteManagerImplTestCase();
    void DoRun() override;
};

GlobalRouteManagerImplTestCase::GlobalRouteManagerImplTestCase()
    : TestCase("GlobalRouteManagerImplTestCase")
{
}

void
GlobalRouteManagerImplTestCase::DoRun()
{
    CandidateQueue candidate;

    for (int i = 0; i < 100; ++i)
    {
        auto v = new SPFVertex;
        v->SetDistanceFromRoot(std::rand() % 100);
        candidate.Push(v);
    }

    for (int i = 0; i < 100; ++i)
    {
        SPFVertex* v = candidate.Pop();
        delete v;
        v = nullptr;
    }

    // Build fake link state database; four routers (0-3), 3 point-to-point
    // links
    //
    //   n0
    //      \ link 0
    //       \          link 2
    //        n2 -------------------------n3
    //       /
    //      / link 1
    //    n1
    //
    //  link0:  10.1.1.1/30, 10.1.1.2/30
    //  link1:  10.1.2.1/30, 10.1.2.2/30
    //  link2:  10.1.3.1/30, 10.1.3.2/30
    //
    // Router 0
    auto lr0 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                           "0.0.0.2",  // router ID 0.0.0.2
                                           "10.1.1.1", // local ID
                                           1);         // metric

    auto lr1 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                           "10.1.1.1",
                                           "255.255.255.252",
                                           1);

    auto lsa0 = new GlobalRoutingLSA();
    lsa0->SetLSType(GlobalRoutingLSA::RouterLSA);
    lsa0->SetLinkStateId("0.0.0.0");
    lsa0->SetAdvertisingRouter("0.0.0.0");
    lsa0->AddLinkRecord(lr0);
    lsa0->AddLinkRecord(lr1);

    // Router 1
    auto lr2 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                           "0.0.0.2",
                                           "10.1.2.1",
                                           1);

    auto lr3 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                           "10.1.2.1",
                                           "255.255.255.252",
                                           1);

    auto lsa1 = new GlobalRoutingLSA();
    lsa1->SetLSType(GlobalRoutingLSA::RouterLSA);
    lsa1->SetLinkStateId("0.0.0.1");
    lsa1->SetAdvertisingRouter("0.0.0.1");
    lsa1->AddLinkRecord(lr2);
    lsa1->AddLinkRecord(lr3);

    // Router 2
    auto lr4 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                           "0.0.0.0",
                                           "10.1.1.2",
                                           1);

    auto lr5 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                           "10.1.1.2",
                                           "255.255.255.252",
                                           1);

    auto lr6 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                           "0.0.0.1",
                                           "10.1.2.2",
                                           1);

    auto lr7 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                           "10.1.2.2",
                                           "255.255.255.252",
                                           1);

    auto lr8 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                           "0.0.0.3",
                                           "10.1.3.2",
                                           1);

    auto lr9 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                           "10.1.3.2",
                                           "255.255.255.252",
                                           1);

    auto lsa2 = new GlobalRoutingLSA();
    lsa2->SetLSType(GlobalRoutingLSA::RouterLSA);
    lsa2->SetLinkStateId("0.0.0.2");
    lsa2->SetAdvertisingRouter("0.0.0.2");
    lsa2->AddLinkRecord(lr4);
    lsa2->AddLinkRecord(lr5);
    lsa2->AddLinkRecord(lr6);
    lsa2->AddLinkRecord(lr7);
    lsa2->AddLinkRecord(lr8);
    lsa2->AddLinkRecord(lr9);

    // Router 3
    auto lr10 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::PointToPoint,
                                            "0.0.0.2",
                                            "10.1.2.1",
                                            1);

    auto lr11 = new GlobalRoutingLinkRecord(GlobalRoutingLinkRecord::StubNetwork,
                                            "10.1.2.1",
                                            "255.255.255.252",
                                            1);

    auto lsa3 = new GlobalRoutingLSA();
    lsa3->SetLSType(GlobalRoutingLSA::RouterLSA);
    lsa3->SetLinkStateId("0.0.0.3");
    lsa3->SetAdvertisingRouter("0.0.0.3");
    lsa3->AddLinkRecord(lr10);
    lsa3->AddLinkRecord(lr11);

    // Test the database
    auto srmlsdb = new GlobalRouteManagerLSDB();
    srmlsdb->Insert(lsa0->GetLinkStateId(), lsa0);
    srmlsdb->Insert(lsa1->GetLinkStateId(), lsa1);
    srmlsdb->Insert(lsa2->GetLinkStateId(), lsa2);
    srmlsdb->Insert(lsa3->GetLinkStateId(), lsa3);
    NS_TEST_ASSERT_MSG_EQ(lsa2,
                          srmlsdb->GetLSA(lsa2->GetLinkStateId()),
                          "The Ipv4Address is not stored as the link state ID");

    // next, calculate routes based on the manually created LSDB
    auto srm = new GlobalRouteManagerImpl();
    srm->DebugUseLsdb(srmlsdb); // manually add in an LSDB
    // Note-- this will succeed without any nodes in the topology
    // because the NodeList is empty
    srm->DebugSPFCalculate(lsa0->GetLinkStateId()); // node n0

    Simulator::Run();

    /// @todo here we should do some verification of the routes built

    Simulator::Destroy();

    // This delete clears the srm, which deletes the LSDB, which clears
    // all of the LSAs, which each destroys the attached LinkRecords.
    delete srm;

    /// @todo Testing
    // No testing has actually been done other than making sure that this code
    // does not crash
}

/**
 * @ingroup internet-test
 *
 * @brief Global Route Manager TestSuite
 */
class GlobalRouteManagerImplTestSuite : public TestSuite
{
  public:
    GlobalRouteManagerImplTestSuite();

  private:
};

GlobalRouteManagerImplTestSuite::GlobalRouteManagerImplTestSuite()
    : TestSuite("global-route-manager-impl", Type::UNIT)
{
    AddTestCase(new GlobalRouteManagerImplTestCase(), TestCase::Duration::QUICK);
}

static GlobalRouteManagerImplTestSuite
    g_globalRoutingManagerImplTestSuite; //!< Static variable for test initialization
