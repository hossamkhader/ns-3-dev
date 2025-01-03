/*
 * Copyright (c) 2020 Universita' di Firenze, Italy
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Tommaso Pecorella <tommaso.pecorella@unifi.it>
 */
#include "ns3/bit-deserializer.h"
#include "ns3/bit-serializer.h"
#include "ns3/test.h"

#include <cstdarg>
#include <iostream>
#include <sstream>

using namespace ns3;

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Bit serialization test
 */
class BitSerializerTest : public TestCase
{
  public:
    void DoRun() override;
    BitSerializerTest();
};

BitSerializerTest::BitSerializerTest()
    : TestCase("BitSerializer")
{
}

void
BitSerializerTest::DoRun()
{
    BitSerializer testBitSerializer1;

    testBitSerializer1.PushBits(0x55, 7);
    testBitSerializer1.PushBits(0x7, 3);
    testBitSerializer1.PushBits(0x0, 2);

    std::vector<uint8_t> result = testBitSerializer1.GetBytes();
    NS_TEST_EXPECT_MSG_EQ((result[0] == 0xab) && (result[1] == 0xc0),
                          true,
                          "Incorrect serialization " << std::hex << +result[0] << +result[1]
                                                     << " instead of " << 0xab << " " << 0xc0
                                                     << std::dec);

    BitSerializer testBitSerializer2;

    testBitSerializer2.PushBits(0x55, 7);
    testBitSerializer2.PushBits(0x7, 3);
    testBitSerializer2.PushBits(0x0, 2);

    testBitSerializer2.InsertPaddingAtEnd(false);

    result = testBitSerializer2.GetBytes();
    NS_TEST_EXPECT_MSG_EQ((result[0] == 0x0a) && (result[1] == 0xbc),
                          true,
                          "Incorrect serialization " << std::hex << +result[0] << +result[1]
                                                     << " instead of " << 0x0a << " " << 0xbc
                                                     << std::dec);
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Bit deserialization test
 */
class BitDeserializerTest : public TestCase
{
  public:
    void DoRun() override;
    BitDeserializerTest();
};

BitDeserializerTest::BitDeserializerTest()
    : TestCase("BitDeserializer")
{
}

void
BitDeserializerTest::DoRun()
{
    uint16_t nibble1;
    uint16_t nibble2;
    uint16_t nibble3;
    bool result;

    BitDeserializer testBitDeserializer1;
    uint8_t test1[2];
    test1[0] = 0xab;
    test1[1] = 0xc0;

    testBitDeserializer1.PushBytes(test1, 2);
    nibble1 = testBitDeserializer1.GetBits(7);
    nibble2 = testBitDeserializer1.GetBits(3);
    nibble3 = testBitDeserializer1.GetBits(2);
    result = (nibble1 == 0x55) && (nibble2 == 0x7) && (nibble3 == 0x0);

    NS_TEST_EXPECT_MSG_EQ(result,
                          true,
                          "Incorrect deserialization " << std::hex << nibble1 << " " << nibble2
                                                       << " " << nibble3 << " << instead of "
                                                       << " " << 0x55 << " " << 0x7 << " " << 0x0
                                                       << std::dec);

    BitDeserializer testBitDeserializer2;
    std::vector<uint8_t> test2;
    test2.push_back(0xab);
    test2.push_back(0xc0);

    testBitDeserializer2.PushBytes(test2);
    nibble1 = testBitDeserializer2.GetBits(7);
    nibble2 = testBitDeserializer2.GetBits(3);
    nibble3 = testBitDeserializer2.GetBits(2);

    result = (nibble1 == 0x55) && (nibble2 == 0x7) && (nibble3 == 0x0);

    NS_TEST_EXPECT_MSG_EQ(result,
                          true,
                          "Incorrect deserialization " << std::hex << nibble1 << " " << nibble2
                                                       << " " << nibble3 << " << instead of "
                                                       << " " << 0x55 << " " << 0x7 << " " << 0x0
                                                       << std::dec);
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Packet Metadata TestSuite
 */
class BitSerializerTestSuite : public TestSuite
{
  public:
    BitSerializerTestSuite();
};

BitSerializerTestSuite::BitSerializerTestSuite()
    : TestSuite("bit-serializer", Type::UNIT)
{
    AddTestCase(new BitSerializerTest, TestCase::Duration::QUICK);
    AddTestCase(new BitDeserializerTest, TestCase::Duration::QUICK);
}

static BitSerializerTestSuite g_bitSerializerTest; //!< Static variable for test initialization
