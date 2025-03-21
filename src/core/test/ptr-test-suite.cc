/*
 * Copyright (c) 2005,2006 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/ptr.h"
#include "ns3/test.h"

/**
 * @file
 * @ingroup core-tests
 * @ingroup ptr
 * @ingroup ptr-tests
 * Smart pointer test suite.
 */

/**
 * @ingroup core-tests
 * @defgroup ptr-tests Smart pointer test suite
 */

namespace ns3
{

namespace tests
{

class PtrTestCase;

/**
 * @ingroup ptr-tests
 * Pointer base test class
 */
class PtrTestBase
{
  public:
    /** Constructor. */
    PtrTestBase();
    /** Destructor. */
    virtual ~PtrTestBase();
    /** Increment the reference count. */
    void Ref() const;
    /** Decrement the reference count, and delete if necessary. */
    void Unref() const;

  private:
    mutable uint32_t m_count; //!< The reference count.
};

/**
 * @ingroup ptr-tests
 * No Count class
 */
class NoCount : public PtrTestBase
{
  public:
    /**
     * Constructor
     *
     * @param [in] test The object to track.
     */
    NoCount(PtrTestCase* test);
    /**
     * Destructor.
     * The object being tracked will also be destroyed,
     * by calling DestroyNotify()
     */
    ~NoCount() override;
    /** Noop function. */
    void Nothing() const;

  private:
    PtrTestCase* m_test; //!< The object being tracked.
};

/**
 * @ingroup ptr-tests
 * Test case for pointer
 */
class PtrTestCase : public TestCase
{
  public:
    /** Constructor. */
    PtrTestCase();
    /** Count the destruction of an object. */
    void DestroyNotify();

  private:
    void DoRun() override;
    /**
     * Test that \pname{p} is a valid object, by calling a member function.
     * @param [in] p The object pointer to test.
     * @returns The object pointer.
     */
    Ptr<NoCount> CallTest(Ptr<NoCount> p);
    /** @copydoc CallTest(Ptr<NoCount>) */
    const Ptr<NoCount> CallTestConst(const Ptr<NoCount> p);
    uint32_t m_nDestroyed; //!< Counter of number of objects destroyed.
};

PtrTestBase::PtrTestBase()
    : m_count(1)
{
}

PtrTestBase::~PtrTestBase()
{
}

void
PtrTestBase::Ref() const
{
    m_count++;
}

void
PtrTestBase::Unref() const
{
    m_count--;
    if (m_count == 0)
    {
        delete this;
    }
}

NoCount::NoCount(PtrTestCase* test)
    : m_test(test)
{
}

NoCount::~NoCount()
{
    m_test->DestroyNotify();
}

void
NoCount::Nothing() const
{
}

PtrTestCase::PtrTestCase()
    : TestCase("Sanity checking of Ptr<>")
{
}

void
PtrTestCase::DestroyNotify()
{
    m_nDestroyed++;
}

Ptr<NoCount>
PtrTestCase::CallTest(Ptr<NoCount> p)
{
    return p;
}

const Ptr<NoCount>
PtrTestCase::CallTestConst(const Ptr<NoCount> p)
{
    return p;
}

void
PtrTestCase::DoRun()
{
    m_nDestroyed = 0;
    {
        Ptr<NoCount> p = Create<NoCount>(this);
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "001");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p;
        p = Create<NoCount>(this);
#if defined(__clang__)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
#endif
        p = p;
#if defined(__clang__)
#if __has_warning("-Wself-assign-overloaded")
#pragma clang diagnostic pop
#endif
#endif
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "002");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        p1 = Create<NoCount>(this);
        Ptr<NoCount> p2 = p1;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "003");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        p1 = Create<NoCount>(this);
        Ptr<NoCount> p2;
        p2 = p1;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "004");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        p1 = Create<NoCount>(this);
        Ptr<NoCount> p2 = Create<NoCount>(this);
        p2 = p1;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "005");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        p1 = Create<NoCount>(this);
        Ptr<NoCount> p2;
        p2 = Create<NoCount>(this);
        p2 = p1;
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "006");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        p1 = Create<NoCount>(this);
        p1 = Create<NoCount>(this);
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "007");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        {
            Ptr<NoCount> p2;
            p1 = Create<NoCount>(this);
            p2 = Create<NoCount>(this);
            p2 = p1;
        }
        NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "008");
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "009");

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p1;
        {
            Ptr<NoCount> p2;
            p1 = Create<NoCount>(this);
            p2 = Create<NoCount>(this);
            p2 = CallTest(p1);
        }
        NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "010");
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 2, "011");

    {
        Ptr<NoCount> p1;
        const Ptr<NoCount> p2 = CallTest(p1);
        const Ptr<NoCount> p3 = CallTestConst(p1);
        Ptr<NoCount> p4 = CallTestConst(p1);
        Ptr<const NoCount> p5 = p4;
        // p4 = p5; You cannot make a const pointer be a non-const pointer.
        //  but if you use ConstCast, you can.
        p4 = ConstCast<NoCount>(p5);
        p5 = p1;
        Ptr<NoCount> p;
        if (!p)
        {
        }
        if (p)
        {
        }
        if (!p)
        {
        }
        if (p)
        {
        }
        if (p)
        {
        }
        if (!p)
        {
        }
    }

    m_nDestroyed = 0;
    {
        NoCount* raw;
        {
            Ptr<NoCount> p = Create<NoCount>(this);
            {
                Ptr<const NoCount> p1 = p;
            }
            raw = GetPointer(p);
            p = nullptr;
        }
        NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 0, "012");
        delete raw;
    }

    m_nDestroyed = 0;
    {
        Ptr<NoCount> p = Create<NoCount>(this);
        const NoCount* v1 = PeekPointer(p);
        NoCount* v2 = PeekPointer(p);
        v1->Nothing();
        v2->Nothing();
    }
    NS_TEST_EXPECT_MSG_EQ(m_nDestroyed, 1, "013");

    {
        Ptr<PtrTestBase> p0 = Create<NoCount>(this);
        Ptr<NoCount> p1 = Create<NoCount>(this);
        NS_TEST_EXPECT_MSG_EQ((p0 == p1), false, "operator == failed");
        NS_TEST_EXPECT_MSG_EQ((p0 != p1), true, "operator != failed");
    }
}

/**
 * @ingroup ptr-tests
 * Test suite for pointer
 */
class PtrTestSuite : public TestSuite
{
  public:
    /** Constructor. */
    PtrTestSuite()
        : TestSuite("ptr")
    {
        AddTestCase(new PtrTestCase());
    }
};

/**
 * @ingroup ptr-tests
 * PtrTestSuite instance variable.
 */
static PtrTestSuite g_ptrTestSuite;

} // namespace tests

} // namespace ns3
