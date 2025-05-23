/*
 * Copyright (c) 2009 CTTC
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Sébastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/dsss-phy.h"
#include "ns3/eht-phy.h"  //includes OFDM, HT, VHT and HE
#include "ns3/eht-ppdu.h" //includes OFDM, HT, VHT and HE
#include "ns3/erp-ofdm-phy.h"
#include "ns3/he-ru.h"
#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/test.h"
#include "ns3/wifi-psdu.h"
#include "ns3/yans-wifi-phy.h"

#include <list>
#include <numeric>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("TxDurationTest");

/**
 * @ingroup wifi-test
 * @ingroup tests
 *
 * @brief Tx Duration Test
 */
class TxDurationTest : public TestCase
{
  public:
    TxDurationTest();
    ~TxDurationTest() override;
    void DoRun() override;

  private:
    /**
     * Check if the payload tx duration returned by InterferenceHelper
     * corresponds to a known value
     *
     * @param size size of payload in octets (includes everything after the PHY header)
     * @param payloadMode the WifiMode used for the transmission
     * @param channelWidth the channel width used for the transmission
     * @param guardInterval the guard interval duration used for the transmission
     * @param preamble the WifiPreamble used for the transmission
     * @param knownDuration the known duration value of the transmission
     *
     * @return true if values correspond, false otherwise
     */
    bool CheckPayloadDuration(uint32_t size,
                              WifiMode payloadMode,
                              MHz_u channelWidth,
                              Time guardInterval,
                              WifiPreamble preamble,
                              Time knownDuration);

    /**
     * Check if the overall tx duration returned by InterferenceHelper
     * corresponds to a known value
     *
     * @param size size of payload in octets (includes everything after the PHY header)
     * @param payloadMode the WifiMode used for the transmission
     * @param channelWidth the channel width used for the transmission
     * @param guardInterval the guard interval duration used for the transmission
     * @param preamble the WifiPreamble used for the transmission
     * @param knownDuration the known duration value of the transmission
     *
     * @return true if values correspond, false otherwise
     */
    bool CheckTxDuration(uint32_t size,
                         WifiMode payloadMode,
                         MHz_u channelWidth,
                         Time guardInterval,
                         WifiPreamble preamble,
                         Time knownDuration);

    /**
     * Check if the overall Tx duration returned by WifiPhy for a MU PPDU
     * corresponds to a known value
     *
     * @param sizes the list of PSDU sizes for each station in octets
     * @param userInfos the list of HE MU specific user transmission parameters
     * @param channelWidth the channel width used for the transmission
     * @param guardInterval the guard interval duration used for the transmission
     * @param preamble the WifiPreamble used for the transmission
     * @param knownDuration the known duration value of the transmission
     *
     * @return true if values correspond, false otherwise
     */
    static bool CheckMuTxDuration(std::list<uint32_t> sizes,
                                  std::list<HeMuUserInfo> userInfos,
                                  MHz_u channelWidth,
                                  Time guardInterval,
                                  WifiPreamble preamble,
                                  Time knownDuration);

    /**
     * Calculate the overall Tx duration returned by WifiPhy for list of sizes.
     * A map of WifiPsdu indexed by STA-ID is built using the provided lists
     * and handed over to the corresponding SU/MU WifiPhy Tx duration computing
     * method.
     * Note that provided lists should be of same size.
     *
     * @param sizes the list of PSDU sizes for each station in octets
     * @param staIds the list of STA-IDs of each station
     * @param txVector the TXVECTOR used for the transmission of the PPDU
     * @param band the selected wifi PHY band
     *
     * @return the overall Tx duration for the list of sizes (SU or MU PPDU)
     */
    static Time CalculateTxDurationUsingList(std::list<uint32_t> sizes,
                                             std::list<uint16_t> staIds,
                                             WifiTxVector txVector,
                                             WifiPhyBand band);
};

TxDurationTest::TxDurationTest()
    : TestCase("Wifi TX Duration")
{
}

TxDurationTest::~TxDurationTest()
{
}

bool
TxDurationTest::CheckPayloadDuration(uint32_t size,
                                     WifiMode payloadMode,
                                     MHz_u channelWidth,
                                     Time guardInterval,
                                     WifiPreamble preamble,
                                     Time knownDuration)
{
    WifiTxVector txVector;
    txVector.SetMode(payloadMode);
    txVector.SetPreambleType(preamble);
    txVector.SetChannelWidth(channelWidth);
    txVector.SetGuardInterval(guardInterval);
    txVector.SetNss(1);
    txVector.SetStbc(false);
    txVector.SetNess(0);
    std::list<WifiPhyBand> testedBands;
    Ptr<YansWifiPhy> phy = CreateObject<YansWifiPhy>();
    if (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_OFDM)
    {
        testedBands.push_back(WIFI_PHY_BAND_5GHZ);
    }
    if (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_HE)
    {
        testedBands.push_back(WIFI_PHY_BAND_6GHZ);
    }
    if (payloadMode.GetModulationClass() != WIFI_MOD_CLASS_VHT)
    {
        testedBands.push_back(WIFI_PHY_BAND_2_4GHZ);
    }
    for (auto& testedBand : testedBands)
    {
        if ((testedBand == WIFI_PHY_BAND_2_4GHZ) &&
            (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_OFDM))
        {
            knownDuration +=
                MicroSeconds(6); // 2.4 GHz band should be at the end of the bands to test
        }
        Time calculatedDuration = YansWifiPhy::GetPayloadDuration(size, txVector, testedBand);
        if (calculatedDuration != knownDuration)
        {
            std::cerr << "size=" << size << " band=" << testedBand << " mode=" << payloadMode
                      << " channelWidth=" << channelWidth << " guardInterval=" << guardInterval
                      << " datarate=" << payloadMode.GetDataRate(channelWidth, guardInterval, 1)
                      << " known=" << knownDuration << " calculated=" << calculatedDuration
                      << std::endl;
            return false;
        }
    }
    return true;
}

bool
TxDurationTest::CheckTxDuration(uint32_t size,
                                WifiMode payloadMode,
                                MHz_u channelWidth,
                                Time guardInterval,
                                WifiPreamble preamble,
                                Time knownDuration)
{
    WifiTxVector txVector;
    txVector.SetMode(payloadMode);
    txVector.SetPreambleType(preamble);
    txVector.SetChannelWidth(channelWidth);
    txVector.SetGuardInterval(guardInterval);
    txVector.SetNss(1);
    txVector.SetStbc(false);
    txVector.SetNess(0);
    std::list<WifiPhyBand> testedBands;
    Ptr<YansWifiPhy> phy = CreateObject<YansWifiPhy>();
    if (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_OFDM)
    {
        testedBands.push_back(WIFI_PHY_BAND_5GHZ);
    }
    if (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_HE)
    {
        testedBands.push_back(WIFI_PHY_BAND_6GHZ);
    }
    if (payloadMode.GetModulationClass() != WIFI_MOD_CLASS_VHT)
    {
        testedBands.push_back(WIFI_PHY_BAND_2_4GHZ);
    }
    for (auto& testedBand : testedBands)
    {
        if ((testedBand == WIFI_PHY_BAND_2_4GHZ) &&
            (payloadMode.GetModulationClass() >= WIFI_MOD_CLASS_OFDM))
        {
            knownDuration +=
                MicroSeconds(6); // 2.4 GHz band should be at the end of the bands to test
        }
        Time calculatedDuration = YansWifiPhy::CalculateTxDuration(size, txVector, testedBand);
        Time calculatedDurationUsingList =
            CalculateTxDurationUsingList(std::list<uint32_t>{size},
                                         std::list<uint16_t>{SU_STA_ID},
                                         txVector,
                                         testedBand);
        if (calculatedDuration != knownDuration ||
            calculatedDuration != calculatedDurationUsingList)
        {
            std::cerr << "size=" << size << " band=" << testedBand << " mode=" << payloadMode
                      << " channelWidth=" << +channelWidth << " guardInterval=" << guardInterval
                      << " datarate=" << payloadMode.GetDataRate(channelWidth, guardInterval, 1)
                      << " preamble=" << preamble << " known=" << knownDuration
                      << " calculated=" << calculatedDuration
                      << " calculatedUsingList=" << calculatedDurationUsingList << std::endl;
            return false;
        }
    }
    return true;
}

bool
TxDurationTest::CheckMuTxDuration(std::list<uint32_t> sizes,
                                  std::list<HeMuUserInfo> userInfos,
                                  MHz_u channelWidth,
                                  Time guardInterval,
                                  WifiPreamble preamble,
                                  Time knownDuration)
{
    NS_ASSERT(sizes.size() == userInfos.size() && sizes.size() > 1);
    NS_ABORT_MSG_IF(
        channelWidth < std::accumulate(
                           std::begin(userInfos),
                           std::end(userInfos),
                           MHz_u{0},
                           [](const MHz_u prevBw, const HeMuUserInfo& info) {
                               return prevBw + WifiRu::GetBandwidth(WifiRu::GetRuType(info.ru));
                           }),
        "Cannot accommodate all the RUs in the provided band"); // MU-MIMO (for which allocations
                                                                // use the same RU) is not supported
    WifiTxVector txVector;
    txVector.SetPreambleType(preamble);
    txVector.SetChannelWidth(channelWidth);
    txVector.SetGuardInterval(guardInterval);
    txVector.SetStbc(false);
    txVector.SetNess(0);
    if (IsEht(preamble))
    {
        txVector.SetEhtPpduType(0);
    }
    std::list<uint16_t> staIds;

    uint16_t staId = 1;
    for (const auto& userInfo : userInfos)
    {
        txVector.SetHeMuUserInfo(staId, userInfo);
        staIds.push_back(staId++);
    }
    txVector.SetSigBMode(VhtPhy::GetVhtMcs0());
    txVector.SetRuAllocation({192, 192}, 0);

    Ptr<YansWifiPhy> phy = CreateObject<YansWifiPhy>();
    std::list<WifiPhyBand> testedBands{
        WIFI_PHY_BAND_5GHZ,
        WIFI_PHY_BAND_6GHZ,
        WIFI_PHY_BAND_2_4GHZ}; // Durations vary depending on frequency; test also 2.4 GHz (bug
                               // 1971)
    for (auto& testedBand : testedBands)
    {
        if (testedBand == WIFI_PHY_BAND_2_4GHZ)
        {
            knownDuration +=
                MicroSeconds(6); // 2.4 GHz band should be at the end of the bands to test
        }
        Time calculatedDuration;
        uint32_t longestSize = 0;
        auto iterStaId = staIds.begin();
        for (auto& size : sizes)
        {
            Time ppduDurationForSta =
                YansWifiPhy::CalculateTxDuration(size, txVector, testedBand, *iterStaId);
            if (ppduDurationForSta > calculatedDuration)
            {
                calculatedDuration = ppduDurationForSta;
                staId = *iterStaId;
                longestSize = size;
            }
            ++iterStaId;
        }
        Time calculatedDurationUsingList =
            CalculateTxDurationUsingList(sizes, staIds, txVector, testedBand);
        if (calculatedDuration != knownDuration ||
            calculatedDuration != calculatedDurationUsingList)
        {
            std::cerr << "size=" << longestSize << " band=" << testedBand << " staId=" << staId
                      << " nss=" << +txVector.GetNss(staId) << " mode=" << txVector.GetMode(staId)
                      << " channelWidth=" << channelWidth << " guardInterval=" << guardInterval
                      << " datarate="
                      << txVector.GetMode(staId).GetDataRate(channelWidth,
                                                             guardInterval,
                                                             txVector.GetNss(staId))
                      << " known=" << knownDuration << " calculated=" << calculatedDuration
                      << " calculatedUsingList=" << calculatedDurationUsingList << std::endl;
            return false;
        }
    }
    return true;
}

Time
TxDurationTest::CalculateTxDurationUsingList(std::list<uint32_t> sizes,
                                             std::list<uint16_t> staIds,
                                             WifiTxVector txVector,
                                             WifiPhyBand band)
{
    NS_ASSERT(sizes.size() == staIds.size());
    WifiConstPsduMap psduMap;
    auto itStaId = staIds.begin();
    WifiMacHeader hdr;
    hdr.SetType(WIFI_MAC_CTL_ACK); // so that size may not be empty while being as short as possible
    for (auto& size : sizes)
    {
        // MAC header and FCS are to deduce from size
        psduMap[*itStaId++] =
            Create<WifiPsdu>(Create<Packet>(size - hdr.GetSerializedSize() - 4), hdr);
    }
    return WifiPhy::CalculateTxDuration(psduMap, txVector, band);
}

void
TxDurationTest::DoRun()
{
    bool retval = true;

    // IEEE Std 802.11-2007 Table 18-2 "Example of LENGTH calculations for CCK"
    retval = retval &&
             CheckPayloadDuration(1023,
                                  DsssPhy::GetDsssRate11Mbps(),
                                  MHz_u{22},
                                  NanoSeconds(800),
                                  WIFI_PREAMBLE_LONG,
                                  MicroSeconds(744)) &&
             CheckPayloadDuration(1024,
                                  DsssPhy::GetDsssRate11Mbps(),
                                  MHz_u{22},
                                  NanoSeconds(800),
                                  WIFI_PREAMBLE_LONG,
                                  MicroSeconds(745)) &&
             CheckPayloadDuration(1025,
                                  DsssPhy::GetDsssRate11Mbps(),
                                  MHz_u{22},
                                  NanoSeconds(800),
                                  WIFI_PREAMBLE_LONG,
                                  MicroSeconds(746)) &&
             CheckPayloadDuration(1026,
                                  DsssPhy::GetDsssRate11Mbps(),
                                  MHz_u{22},
                                  NanoSeconds(800),
                                  WIFI_PREAMBLE_LONG,
                                  MicroSeconds(747));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11b CCK duration failed");

    // Similar, but we add PHY preamble and header durations
    // and we test different rates.
    // The payload durations for modes other than 11mbb have been
    // calculated by hand according to  IEEE Std 802.11-2007 18.2.3.5
    retval = retval &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(744 + 96)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(745 + 96)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(746 + 96)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(747 + 96)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(744 + 192)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(745 + 192)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(746 + 192)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(747 + 192)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(1488 + 96)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(1490 + 96)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(1491 + 96)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(1493 + 96)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(1488 + 192)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(1490 + 192)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(1491 + 192)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate5_5Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(1493 + 192)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(4092 + 96)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(4096 + 96)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(4100 + 96)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(4104 + 96)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(4092 + 192)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(4096 + 192)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(4100 + 192)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate2Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(4104 + 192)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(8184 + 192)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(8192 + 192)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(8200 + 192)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_SHORT,
                             MicroSeconds(8208 + 192)) &&
             CheckTxDuration(1023,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(8184 + 192)) &&
             CheckTxDuration(1024,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(8192 + 192)) &&
             CheckTxDuration(1025,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(8200 + 192)) &&
             CheckTxDuration(1026,
                             DsssPhy::GetDsssRate1Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(8208 + 192));

    // values from
    // https://web.archive.org/web/20100711002639/http://mailman.isi.edu/pipermail/ns-developers/2009-July/006226.html
    retval = retval && CheckTxDuration(14,
                                       DsssPhy::GetDsssRate1Mbps(),
                                       MHz_u{22},
                                       NanoSeconds(800),
                                       WIFI_PREAMBLE_LONG,
                                       MicroSeconds(304));

    // values from http://www.oreillynet.com/pub/a/wireless/2003/08/08/wireless_throughput.html
    retval = retval &&
             CheckTxDuration(1536,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(1310)) &&
             CheckTxDuration(76,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(248)) &&
             CheckTxDuration(14,
                             DsssPhy::GetDsssRate11Mbps(),
                             MHz_u{22},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(203));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11b duration failed");

    // 802.11a durations
    // values from http://www.oreillynet.com/pub/a/wireless/2003/08/08/wireless_throughput.html
    retval = retval &&
             CheckTxDuration(1536,
                             OfdmPhy::GetOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(248)) &&
             CheckTxDuration(76,
                             OfdmPhy::GetOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(32)) &&
             CheckTxDuration(14,
                             OfdmPhy::GetOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(24));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11a duration failed");

    // 802.11g durations are same as 802.11a durations but with 6 us signal extension
    retval = retval &&
             CheckTxDuration(1536,
                             ErpOfdmPhy::GetErpOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(254)) &&
             CheckTxDuration(76,
                             ErpOfdmPhy::GetErpOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(38)) &&
             CheckTxDuration(14,
                             ErpOfdmPhy::GetErpOfdmRate54Mbps(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_LONG,
                             MicroSeconds(30));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11g duration failed");

    // 802.11n durations
    retval = retval &&
             CheckTxDuration(1536,
                             HtPhy::GetHtMcs7(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(228)) &&
             CheckTxDuration(76,
                             HtPhy::GetHtMcs7(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(48)) &&
             CheckTxDuration(14,
                             HtPhy::GetHtMcs7(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             HtPhy::GetHtMcs0(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(1742400)) &&
             CheckTxDuration(76,
                             HtPhy::GetHtMcs0(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(126)) &&
             CheckTxDuration(14,
                             HtPhy::GetHtMcs0(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HtPhy::GetHtMcs6(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(226800)) &&
             CheckTxDuration(76,
                             HtPhy::GetHtMcs6(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(46800)) &&
             CheckTxDuration(14,
                             HtPhy::GetHtMcs6(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(39600)) &&
             CheckTxDuration(1536,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(128)) &&
             CheckTxDuration(76,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(44)) &&
             CheckTxDuration(14,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HT_MF,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(118800)) &&
             CheckTxDuration(76,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(43200)) &&
             CheckTxDuration(14,
                             HtPhy::GetHtMcs7(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_HT_MF,
                             NanoSeconds(39600));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11n duration failed");

    // 802.11ac durations
    retval = retval &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(196)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(48)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(180)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(46800)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{20},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(108)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(100800)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{40},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(460)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(44)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(417600)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs0(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(43200)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(68)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(64800)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs9(),
                             MHz_u{80},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(56)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(40)) &&
             CheckTxDuration(1536,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             MicroSeconds(54)) &&
             CheckTxDuration(76,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600)) &&
             CheckTxDuration(14,
                             VhtPhy::GetVhtMcs8(),
                             MHz_u{160},
                             NanoSeconds(400),
                             WIFI_PREAMBLE_VHT_SU,
                             NanoSeconds(39600));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11ac duration failed");

    // 802.11ax SU durations
    retval = retval &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(1485600)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(125600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(71200)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(764800)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(84800)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(397600)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(71200)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(220800)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(1570400)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(130400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(72800)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(807200)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(87200)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(418400)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(72800)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(231200)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(1740)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(140)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(76)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(892)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(92)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(460)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(76)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(252)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs0(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(139200)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(98400)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(71200)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(800),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(57600)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(144800)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(101600)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(72800)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(1600),
                             WIFI_PREAMBLE_HE_SU,
                             NanoSeconds(58400)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(156)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{20},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(108)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{40},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(76)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{80},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(1536,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(76,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60)) &&
             CheckTxDuration(14,
                             HePhy::GetHeMcs11(),
                             MHz_u{160},
                             NanoSeconds(3200),
                             WIFI_PREAMBLE_HE_SU,
                             MicroSeconds(60));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11ax SU duration failed");

    // 802.11ax MU durations
    retval = retval &&
             CheckMuTxDuration(
                 std::list<uint32_t>{1536, 1536},
                 std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 0, 1},
                                         {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                 MHz_u{40},
                 NanoSeconds(800),
                 WIFI_PREAMBLE_HE_MU,
                 NanoSeconds(
                     1493600)) // equivalent to HE_SU for 20 MHz with 2 extra HE-SIG-B (i.e. 8 us)
             && CheckMuTxDuration(
                    std::list<uint32_t>{1536, 1536},
                    std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 1, 1},
                                            {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                    MHz_u{40},
                    NanoSeconds(800),
                    WIFI_PREAMBLE_HE_MU,
                    NanoSeconds(1493600)) // shouldn't change if first PSDU is shorter
             && CheckMuTxDuration(
                    std::list<uint32_t>{1536, 76},
                    std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 0, 1},
                                            {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                    MHz_u{40},
                    NanoSeconds(800),
                    WIFI_PREAMBLE_HE_MU,
                    NanoSeconds(1493600));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11ax MU duration failed");

    // 802.11be MU durations
    retval = retval &&
             CheckMuTxDuration(
                 std::list<uint32_t>{1536, 1536},
                 std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 0, 1},
                                         {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                 MHz_u{40},
                 NanoSeconds(800),
                 WIFI_PREAMBLE_EHT_MU,
                 NanoSeconds(1493600)) // equivalent to 802.11ax MU
             && CheckMuTxDuration(
                    std::list<uint32_t>{1536, 1536},
                    std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 1, 1},
                                            {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                    MHz_u{40},
                    NanoSeconds(800),
                    WIFI_PREAMBLE_EHT_MU,
                    NanoSeconds(1493600)) // shouldn't change if first PSDU is shorter
             && CheckMuTxDuration(
                    std::list<uint32_t>{1536, 76},
                    std::list<HeMuUserInfo>{{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 0, 1},
                                            {HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 0, 1}},
                    MHz_u{40},
                    NanoSeconds(800),
                    WIFI_PREAMBLE_EHT_MU,
                    NanoSeconds(1493600));

    NS_TEST_EXPECT_MSG_EQ(retval, true, "an 802.11be MU duration failed");

    Simulator::Destroy();
}

/**
 * @ingroup wifi-test
 * @ingroup tests
 *
 * @brief HE-SIG-B duration test
 */
class HeSigBDurationTest : public TestCase
{
  public:
    /**
     * OFDMA or MU-MIMO
     */
    enum MuType
    {
        OFDMA = 0,
        MU_MIMO
    };

    /**
     * Constructor
     *
     * @param userInfos the HE MU specific per-user information to use for the test
     * @param sigBMode the mode to transmit HE-SIG-B for the test
     * @param channelWidth the channel width to select for the test
     * @param p20Index the index of the primary20 channel
     * @param expectedMuType the expected MU type (OFDMA or MU-MIMO)
     * @param expectedRuAllocation the expected RuType::RU_ALLOCATION
     * @param expectedNumUsersPerCc the expected number of users per content channel
     * @param expectedSigBDuration the expected duration of the HE-SIG-B header
     */
    HeSigBDurationTest(const std::list<HeMuUserInfo>& userInfos,
                       const WifiMode& sigBMode,
                       MHz_u channelWidth,
                       uint8_t p20Index,
                       MuType expectedMuType,
                       const RuAllocation& expectedRuAllocation,
                       const std::pair<std::size_t, std::size_t>& expectedNumUsersPerCc,
                       Time expectedSigBDuration);

  private:
    void DoRun() override;
    void DoTeardown() override;

    /**
     * Build a TXVECTOR for HE MU.
     *
     * @return the configured HE MU TXVECTOR
     */
    WifiTxVector BuildTxVector() const;

    Ptr<YansWifiPhy> m_phy; ///< the PHY under test

    std::list<HeMuUserInfo> m_userInfos; ///< HE MU specific per-user information
    WifiMode m_sigBMode;                 ///< Mode used to transmit HE-SIG-B
    MHz_u m_channelWidth;                ///< Channel width
    uint8_t m_p20Index;                  ///< index of the primary20 channel
    MuType m_expectedMuType;             ///< Expected MU type (OFDMA or MU-MIMO)
    RuAllocation m_expectedRuAllocation; ///< Expected RuType::RU_ALLOCATION
    std::pair<std::size_t, std::size_t>
        m_expectedNumUsersPerCc; ///< Expected number of users per content channel
    Time m_expectedSigBDuration; ///< Expected duration of the HE-SIG-B header
};

HeSigBDurationTest::HeSigBDurationTest(
    const std::list<HeMuUserInfo>& userInfos,
    const WifiMode& sigBMode,
    MHz_u channelWidth,
    uint8_t p20Index,
    MuType expectedMuType,
    const RuAllocation& expectedRuAllocation,
    const std::pair<std::size_t, std::size_t>& expectedNumUsersPerCc,
    Time expectedSigBDuration)
    : TestCase{"Check HE-SIG-B duration computation"},
      m_userInfos{userInfos},
      m_sigBMode{sigBMode},
      m_channelWidth{channelWidth},
      m_p20Index{p20Index},
      m_expectedMuType{expectedMuType},
      m_expectedRuAllocation{expectedRuAllocation},
      m_expectedNumUsersPerCc{expectedNumUsersPerCc},
      m_expectedSigBDuration{expectedSigBDuration}
{
}

WifiTxVector
HeSigBDurationTest::BuildTxVector() const
{
    WifiTxVector txVector;
    txVector.SetPreambleType(WIFI_PREAMBLE_HE_MU);
    txVector.SetChannelWidth(m_channelWidth);
    txVector.SetGuardInterval(NanoSeconds(3200));
    txVector.SetStbc(false);
    txVector.SetNess(0);
    std::list<uint16_t> staIds;
    uint16_t staId = 1;
    for (const auto& userInfo : m_userInfos)
    {
        txVector.SetHeMuUserInfo(staId, userInfo);
        staIds.push_back(staId++);
    }
    txVector.SetSigBMode(m_sigBMode);
    NS_ASSERT(m_expectedMuType == OFDMA ? txVector.IsDlOfdma() : txVector.IsDlMuMimo());
    return txVector;
}

void
HeSigBDurationTest::DoRun()
{
    m_phy = CreateObject<YansWifiPhy>();
    auto channelNum = WifiPhyOperatingChannel::FindFirst(0,
                                                         MHz_u{0},
                                                         MHz_u{160},
                                                         WIFI_STANDARD_80211ax,
                                                         WIFI_PHY_BAND_6GHZ)
                          ->number;
    m_phy->SetOperatingChannel(
        WifiPhy::ChannelTuple{channelNum, 160, WIFI_PHY_BAND_6GHZ, m_p20Index});
    m_phy->ConfigureStandard(WIFI_STANDARD_80211ax);

    const auto& txVector = BuildTxVector();
    const auto& hePhy = m_phy->GetPhyEntity(WIFI_MOD_CLASS_HE);

    // Verify mode for HE-SIG-B field
    NS_TEST_EXPECT_MSG_EQ(hePhy->GetSigMode(WIFI_PPDU_FIELD_SIG_B, txVector),
                          m_sigBMode,
                          "Incorrect mode used to send HE-SIG-B");

    // Verify RuType::RU_ALLOCATION in TXVECTOR
    NS_TEST_EXPECT_MSG_EQ((txVector.GetRuAllocation(0) == m_expectedRuAllocation),
                          true,
                          "Incorrect RuType::RU_ALLOCATION");

    // Verify number of users for content channels 1 and 2
    const auto& numUsersPerCc = HePpdu::GetNumRusPerHeSigBContentChannel(
        txVector.GetChannelWidth(),
        txVector.GetRuAllocation(m_p20Index),
        txVector.GetCenter26ToneRuIndication(),
        txVector.IsSigBCompression(),
        txVector.IsSigBCompression() ? txVector.GetHeMuUserInfoMap().size() : 0);
    const auto contentChannels = HePpdu::GetHeSigBContentChannels(txVector, 0);
    NS_TEST_EXPECT_MSG_EQ(numUsersPerCc.first,
                          m_expectedNumUsersPerCc.first,
                          "Incorrect number of users in HE-SIG-B content channel 1");
    NS_TEST_EXPECT_MSG_EQ(numUsersPerCc.second,
                          m_expectedNumUsersPerCc.second,
                          "Incorrect number of users in HE-SIG-B content channel 2");
    NS_TEST_EXPECT_MSG_EQ(contentChannels.at(0).size(),
                          m_expectedNumUsersPerCc.first,
                          "Incorrect number of users in HE-SIG-B content channel 1");
    NS_TEST_EXPECT_MSG_EQ((contentChannels.size() > 1 ? contentChannels.at(1).size() : 0),
                          m_expectedNumUsersPerCc.second,
                          "Incorrect number of users in HE-SIG-B content channel 2");

    // Verify total HE-SIG-B duration
    NS_TEST_EXPECT_MSG_EQ(hePhy->GetDuration(WIFI_PPDU_FIELD_SIG_B, txVector),
                          m_expectedSigBDuration,
                          "Incorrect duration for HE-SIG-B");

    // Verify user infos in reconstructed TX vector
    WifiConstPsduMap psdus;
    Time ppduDuration;
    for (std::size_t i = 0; i < m_userInfos.size(); ++i)
    {
        WifiMacHeader hdr;
        auto psdu = Create<WifiPsdu>(Create<Packet>(1000), hdr);
        ppduDuration = std::max(
            ppduDuration,
            WifiPhy::CalculateTxDuration(psdu->GetSize(), txVector, m_phy->GetPhyBand(), i + 1));
        psdus.insert(std::make_pair(i, psdu));
    }
    auto ppdu = hePhy->BuildPpdu(psdus, txVector, ppduDuration);
    ppdu->ResetTxVector();
    const auto& rxVector = ppdu->GetTxVector();
    NS_TEST_EXPECT_MSG_EQ((txVector.GetHeMuUserInfoMap() == rxVector.GetHeMuUserInfoMap()),
                          true,
                          "Incorrect user infos in reconstructed TXVECTOR");

    Simulator::Destroy();
}

void
HeSigBDurationTest::DoTeardown()
{
    m_phy->Dispose();
    m_phy = nullptr;
}

/**
 * @ingroup wifi-test
 * @ingroup tests
 *
 * @brief PHY header sections consistency test
 */
class PhyHeaderSectionsTest : public TestCase
{
  public:
    PhyHeaderSectionsTest();
    ~PhyHeaderSectionsTest() override;
    void DoRun() override;

  private:
    /**
     * Check if map of PHY header sections returned by a given PHY entity
     * corresponds to a known value
     *
     * @param obtained the map of PHY header sections to check
     * @param expected the expected map of PHY header sections
     */
    void CheckPhyHeaderSections(PhyEntity::PhyHeaderSections obtained,
                                PhyEntity::PhyHeaderSections expected);
};

PhyHeaderSectionsTest::PhyHeaderSectionsTest()
    : TestCase("PHY header sections consistency")
{
}

PhyHeaderSectionsTest::~PhyHeaderSectionsTest()
{
}

void
PhyHeaderSectionsTest::CheckPhyHeaderSections(PhyEntity::PhyHeaderSections obtained,
                                              PhyEntity::PhyHeaderSections expected)
{
    NS_ASSERT_MSG(obtained.size() == expected.size(),
                  "The expected map size (" << expected.size() << ") was not obtained ("
                                            << obtained.size() << ")");

    auto itObtained = obtained.begin();
    auto itExpected = expected.begin();
    for (; itObtained != obtained.end() || itExpected != expected.end();)
    {
        WifiPpduField field = itObtained->first;
        auto window = itObtained->second.first;
        auto mode = itObtained->second.second;

        WifiPpduField fieldRef = itExpected->first;
        auto windowRef = itExpected->second.first;
        auto modeRef = itExpected->second.second;

        NS_TEST_EXPECT_MSG_EQ(field,
                              fieldRef,
                              "The expected PPDU field (" << fieldRef << ") was not obtained ("
                                                          << field << ")");
        NS_TEST_EXPECT_MSG_EQ(window.first,
                              windowRef.first,
                              "The expected start time (" << windowRef.first
                                                          << ") was not obtained (" << window.first
                                                          << ")");
        NS_TEST_EXPECT_MSG_EQ(window.second,
                              windowRef.second,
                              "The expected stop time (" << windowRef.second
                                                         << ") was not obtained (" << window.second
                                                         << ")");
        NS_TEST_EXPECT_MSG_EQ(mode,
                              modeRef,
                              "The expected mode (" << modeRef << ") was not obtained (" << mode
                                                    << ")");
        ++itObtained;
        ++itExpected;
    }
}

void
PhyHeaderSectionsTest::DoRun()
{
    Time ppduStart = Seconds(1);
    Ptr<PhyEntity> phyEntity;
    PhyEntity::PhyHeaderSections sections;
    WifiTxVector txVector;
    WifiMode nonHtMode;

    // ==================================================================================
    // 11b (HR/DSSS)
    phyEntity = Create<DsssPhy>();
    txVector.SetMode(DsssPhy::GetDsssRate1Mbps());
    txVector.SetChannelWidth(MHz_u{22});

    // -> long PPDU format
    txVector.SetPreambleType(WIFI_PREAMBLE_LONG);
    nonHtMode = DsssPhy::GetDsssRate1Mbps();
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(144)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(144), ppduStart + MicroSeconds(192)}, nonHtMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> long PPDU format if data rate is 1 Mbps (even if preamble is tagged short)
    txVector.SetPreambleType(WIFI_PREAMBLE_SHORT);
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> short PPDU format
    txVector.SetMode(DsssPhy::GetDsssRate11Mbps());
    nonHtMode = DsssPhy::GetDsssRate2Mbps();
    txVector.SetPreambleType(WIFI_PREAMBLE_SHORT);
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(72)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(72), ppduStart + MicroSeconds(96)}, nonHtMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // ==================================================================================
    // 11a (OFDM)
    txVector.SetPreambleType(WIFI_PREAMBLE_LONG);

    // -> one iteration per variant: default, 10 MHz, and 5 MHz
    std::map<OfdmPhyVariant, std::size_t> variants{
        // number to use to deduce rate and BW info for each variant
        {OFDM_PHY_DEFAULT, 1},
        {OFDM_PHY_10_MHZ, 2},
        {OFDM_PHY_5_MHZ, 4},
    };
    for (auto variant : variants)
    {
        phyEntity = Create<OfdmPhy>(variant.first);
        std::size_t ratio = variant.second;
        const auto bw = MHz_u{20} / ratio;
        txVector.SetChannelWidth(bw);
        txVector.SetMode(OfdmPhy::GetOfdmRate(12000000 / ratio, bw));
        nonHtMode = OfdmPhy::GetOfdmRate(6000000 / ratio, bw);
        sections = {
            {WIFI_PPDU_FIELD_PREAMBLE,
             {{ppduStart, ppduStart + MicroSeconds(16 * ratio)}, nonHtMode}},
            {WIFI_PPDU_FIELD_NON_HT_HEADER,
             {{ppduStart + MicroSeconds(16 * ratio), ppduStart + MicroSeconds(20 * ratio)},
              nonHtMode}},
        };
        CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
    }

    // ==================================================================================
    // 11g (ERP-OFDM)
    phyEntity = Create<ErpOfdmPhy>();
    txVector.SetChannelWidth(MHz_u{20});
    txVector.SetMode(ErpOfdmPhy::GetErpOfdmRate(54000000));
    nonHtMode = ErpOfdmPhy::GetErpOfdmRate6Mbps();
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(16)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(16), ppduStart + MicroSeconds(20)}, nonHtMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // ==================================================================================
    // 11n (HT)
    phyEntity = Create<HtPhy>(4);
    txVector.SetChannelWidth(MHz_u{20});
    txVector.SetMode(HtPhy::GetHtMcs6());
    nonHtMode = OfdmPhy::GetOfdmRate6Mbps();
    WifiMode htSigMode = nonHtMode;

    // -> HT-mixed format for 2 SS and no ESS
    txVector.SetPreambleType(WIFI_PREAMBLE_HT_MF);
    txVector.SetNss(2);
    txVector.SetNess(0);
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(16)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(16), ppduStart + MicroSeconds(20)}, nonHtMode}},
        {WIFI_PPDU_FIELD_HT_SIG,
         {{ppduStart + MicroSeconds(20), ppduStart + MicroSeconds(28)}, htSigMode}},
        {WIFI_PPDU_FIELD_TRAINING,
         {{ppduStart + MicroSeconds(28), ppduStart + MicroSeconds(40)}, // 1 HT-STF + 2 HT-LTFs
          htSigMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
    txVector.SetChannelWidth(MHz_u{20}); // shouldn't have any impact
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> HT-mixed format for 3 SS and 1 ESS
    txVector.SetNss(3);
    txVector.SetNess(1);
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(28),
         ppduStart + MicroSeconds(52)}, // 1 HT-STF + 5 HT-LTFs (4 data + 1 extension)
        htSigMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // ==================================================================================
    // 11ac (VHT)
    phyEntity = Create<VhtPhy>();
    txVector.SetChannelWidth(MHz_u{20});
    txVector.SetNess(0);
    txVector.SetMode(VhtPhy::GetVhtMcs7());
    WifiMode sigAMode = nonHtMode;
    WifiMode sigBMode = VhtPhy::GetVhtMcs0();

    // -> VHT SU format for 5 SS
    txVector.SetPreambleType(WIFI_PREAMBLE_VHT_SU);
    txVector.SetNss(5);
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(16)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(16), ppduStart + MicroSeconds(20)}, nonHtMode}},
        {WIFI_PPDU_FIELD_SIG_A,
         {{ppduStart + MicroSeconds(20), ppduStart + MicroSeconds(28)}, sigAMode}},
        {WIFI_PPDU_FIELD_TRAINING,
         {{ppduStart + MicroSeconds(28), ppduStart + MicroSeconds(56)}, // 1 VHT-STF + 6 VHT-LTFs
          sigAMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> VHT SU format for 7 SS
    txVector.SetNss(7);
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(28), ppduStart + MicroSeconds(64)}, // 1 VHT-STF + 8 VHT-LTFs
        sigAMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> VHT MU format for 3 SS
    txVector.SetPreambleType(WIFI_PREAMBLE_VHT_MU);
    txVector.SetNss(3);
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(28), ppduStart + MicroSeconds(48)}, // 1 VHT-STF + 4 VHT-LTFs
        sigAMode};
    sections[WIFI_PPDU_FIELD_SIG_B] = {{ppduStart + MicroSeconds(48), ppduStart + MicroSeconds(52)},
                                       sigBMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
    txVector.SetChannelWidth(MHz_u{80}); // shouldn't have any impact
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // ==================================================================================
    // 11ax (HE)
    phyEntity = Create<HePhy>();
    txVector.SetChannelWidth(MHz_u{20});
    txVector.SetNss(2); // HE-LTF duration assumed to be always 8 us for the time being (see note in
                        // HePhy::GetTrainingDuration)
    txVector.SetMode(HePhy::GetHeMcs9());
    std::map<uint16_t, HeMuUserInfo> userInfoMap = {
        {1, {HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 4, 2}},
        {2, {HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 9, 1}}};
    sigAMode = HePhy::GetVhtMcs0();
    sigBMode = HePhy::GetVhtMcs4(); // because of first user info map

    // -> HE SU format
    txVector.SetPreambleType(WIFI_PREAMBLE_HE_SU);
    sections = {
        {WIFI_PPDU_FIELD_PREAMBLE, {{ppduStart, ppduStart + MicroSeconds(16)}, nonHtMode}},
        {WIFI_PPDU_FIELD_NON_HT_HEADER,
         {{ppduStart + MicroSeconds(16), ppduStart + MicroSeconds(24)}, // L-SIG + RL-SIG
          nonHtMode}},
        {WIFI_PPDU_FIELD_SIG_A,
         {{ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(32)}, sigAMode}},
        {WIFI_PPDU_FIELD_TRAINING,
         {{ppduStart + MicroSeconds(32),
           ppduStart + MicroSeconds(52)}, // 1 HE-STF (@ 4 us) + 2 HE-LTFs (@ 8 us)
          sigAMode}},
    };
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> HE ER SU format
    txVector.SetPreambleType(WIFI_PREAMBLE_HE_ER_SU);
    sections[WIFI_PPDU_FIELD_SIG_A] = {
        {ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(40)}, // 16 us HE-SIG-A
        sigAMode};
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(40),
         ppduStart + MicroSeconds(60)}, // 1 HE-STF (@ 4 us) + 2 HE-LTFs (@ 8 us)
        sigAMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> HE TB format
    txVector.SetPreambleType(WIFI_PREAMBLE_HE_TB);
    txVector.SetHeMuUserInfo(1, userInfoMap.at(1));
    txVector.SetHeMuUserInfo(2, userInfoMap.at(2));
    sections[WIFI_PPDU_FIELD_SIG_A] = {{ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(32)},
                                       sigAMode};
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(32),
         ppduStart + MicroSeconds(56)}, // 1 HE-STF (@ 8 us) + 2 HE-LTFs (@ 8 us)
        sigAMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> HE MU format
    txVector.SetPreambleType(WIFI_PREAMBLE_HE_MU);
    txVector.SetSigBMode(sigBMode);
    txVector.SetRuAllocation({96}, 0);
    sections[WIFI_PPDU_FIELD_SIG_A] = {{ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(32)},
                                       sigAMode};
    sections[WIFI_PPDU_FIELD_SIG_B] = {
        {ppduStart + MicroSeconds(32), ppduStart + MicroSeconds(36)}, // only one symbol
        sigBMode};
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(36),
         ppduStart + MicroSeconds(56)}, // 1 HE-STF (@ 4 us) + 2 HE-LTFs (@ 8 us)
        sigBMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
    txVector.SetChannelWidth(MHz_u{160}); // shouldn't have any impact
    txVector.SetRuAllocation({96, 113, 113, 113, 113, 113, 113, 113}, 0);

    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // ==================================================================================
    // 11be (EHT)
    sections.erase(WIFI_PPDU_FIELD_SIG_A); // FIXME: do we keep using separate type for 11be?
    sections.erase(WIFI_PPDU_FIELD_SIG_B); // FIXME: do we keep using separate type for 11be?
    phyEntity = Create<EhtPhy>();
    txVector.SetChannelWidth(MHz_u{20});
    txVector.SetNss(2); // EHT-LTF duration assumed to be always 8 us for the time being (see note
                        // in HePhy::GetTrainingDuration)
    txVector.SetMode(EhtPhy::GetEhtMcs9());
    userInfoMap = {{1, {HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 4, 2}},
                   {2, {HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 9, 1}}};
    WifiMode uSigMode = EhtPhy::GetVhtMcs0();
    WifiMode ehtSigMode = EhtPhy::GetVhtMcs4(); // because of first user info map

    // -> EHT TB format
    txVector.SetPreambleType(WIFI_PREAMBLE_EHT_TB);
    txVector.SetHeMuUserInfo(1, userInfoMap.at(1));
    txVector.SetHeMuUserInfo(2, userInfoMap.at(2));
    sections[WIFI_PPDU_FIELD_U_SIG] = {{ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(32)},
                                       uSigMode};
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(32),
         ppduStart + MicroSeconds(56)}, // 1 EHT-STF (@ 8 us) + 2 EHT-LTFs (@ 8 us)
        uSigMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);

    // -> EHT MU format
    txVector.SetPreambleType(WIFI_PREAMBLE_EHT_MU);
    txVector.SetEhtPpduType(0); // EHT MU transmission
    txVector.SetRuAllocation({96}, 0);
    sections[WIFI_PPDU_FIELD_U_SIG] = {{ppduStart + MicroSeconds(24), ppduStart + MicroSeconds(32)},
                                       uSigMode};
    sections[WIFI_PPDU_FIELD_EHT_SIG] = {
        {ppduStart + MicroSeconds(32), ppduStart + MicroSeconds(36)}, // only one symbol
        ehtSigMode};
    sections[WIFI_PPDU_FIELD_TRAINING] = {
        {ppduStart + MicroSeconds(36),
         ppduStart + MicroSeconds(56)}, // 1 HE-STF (@ 4 us) + 2 HE-LTFs (@ 8 us)
        ehtSigMode};
    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
    txVector.SetChannelWidth(MHz_u{160}); // shouldn't have any impact
    txVector.SetRuAllocation({96, 113, 113, 113, 113, 113, 113, 113}, 0);

    CheckPhyHeaderSections(phyEntity->GetPhyHeaderSections(txVector, ppduStart), sections);
}

/**
 * @ingroup wifi-test
 * @ingroup tests
 *
 * @brief Tx Duration Test Suite
 */
class TxDurationTestSuite : public TestSuite
{
  public:
    TxDurationTestSuite();
};

TxDurationTestSuite::TxDurationTestSuite()
    : TestSuite("wifi-devices-tx-duration", Type::UNIT)
{
    AddTestCase(new TxDurationTest, TestCase::Duration::QUICK);

    AddTestCase(new PhyHeaderSectionsTest, TestCase::Duration::QUICK);

    // 20 MHz band, OFDMA, even number of users in HE-SIG-B content channel
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1},
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}},
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96},
                               std::make_pair(2, 0), // both users in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol;
        TestCase::Duration::QUICK);

    // 40 MHz band, OFDMA, even number of users per HE-SIG-B content channel
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1}, // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}, // CC1
                                {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 4, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 7, true}, 5, 3},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 6, 2}},  // CC2
                               VhtPhy::GetVhtMcs4(),
                               MHz_u{40},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 112},
                               std::make_pair(2, 4), // two users in HE-SIG-B content channel 1 and
                                                     // four users in HE-SIG-B content channel 2
                               MicroSeconds(4)),     // one OFDM symbol;
        TestCase::Duration::QUICK);

    // 40 MHz band, OFDMA, odd number of users in second HE-SIG-B content channel
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1}, // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}, // CC1
                                {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 4, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 7, true}, 5, 3},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 3, 1}}, // CC2
                               VhtPhy::GetVhtMcs3(),
                               MHz_u{40},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 15},
                               std::make_pair(2, 5), // two users in HE-SIG-B content channel 1 and
                                                     // five users in HE-SIG-B content channel 2
                               MicroSeconds(8)),     // two OFDM symbols
        TestCase::Duration::QUICK);

    // 80 MHz band, OFDMA
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1}, // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}, // CC1
                                {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 4, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 7, true}, 5, 3},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 3, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_242_TONE, 3, true}, 1, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 4, true}, 4, 1}}, // CC2
                               VhtPhy::GetVhtMcs1(),
                               MHz_u{80},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 15, 192, 192},
                               std::make_pair(3, 6), // three users in HE-SIG-B content channel 1
                                                     // and six users in HE-SIG-B content channel 2
                               MicroSeconds(16)),    // four OFDM symbols
        TestCase::Duration::QUICK);

    // 80 MHz band, OFDMA, no central 26-tones RU
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_26_TONE, 1, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 2, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 3, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 4, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 5, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 6, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 7, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 8, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 9, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 10, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 11, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 12, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 13, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 15, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 16, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 17, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 18, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 20, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 21, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 22, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 23, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 24, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 25, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 26, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 27, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_26_TONE, 28, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 29, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 30, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 31, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 32, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 33, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 34, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 35, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 36, true}, 8, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_26_TONE, 37, true}, 8, 1}}, // CC2
                    VhtPhy::GetVhtMcs5(),
                    MHz_u{80},
                    0,
                    HeSigBDurationTest::OFDMA,
                    {0, 0, 0, 0},
                    std::make_pair(18, 18), // 18 users users in each HE-SIG-B content channel
                    MicroSeconds(12)),      // three OFDM symbols
                TestCase::Duration::QUICK);

    // 80 MHz band, OFDMA, central 26-tones RU
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_26_TONE, 1, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 2, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 3, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 4, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 5, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 6, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 7, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 8, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 9, true}, 8, 1},   // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 10, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 11, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 12, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 13, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 15, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 16, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 17, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 18, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 19, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 20, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 21, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 22, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 23, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 24, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 25, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 26, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 27, true}, 8, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 28, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 29, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 30, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 31, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 32, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 33, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 34, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 35, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 36, true}, 8, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_26_TONE, 37, true}, 8, 1}}, // CC2
            VhtPhy::GetVhtMcs5(),
            MHz_u{80},
            0,
            HeSigBDurationTest::OFDMA,
            {0, 0, 0, 0},
            std::make_pair(19,
                           18), // 19 users (18 users + 1 central tones-RU user) in HE-SIG-B content
                                // channel 1 and 18 users user in HE-SIG-B content channel 2
            MicroSeconds(12)),  // three OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, no central 26-tones RU
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1}, // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}, // CC1
                                {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 4, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 7, true}, 5, 3},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 3, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_242_TONE, 3, true}, 1, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 4, true}, 4, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_996_TONE, 1, false},
                                 1,
                                 1}}, // CC1 or CC2 => CC1 for better split
                               VhtPhy::GetVhtMcs1(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 15, 192, 192, 208, 115, 208, 115},
                               std::make_pair(4, 6), // four users in HE-SIG-B content channel 1 and
                                                     // seven users in HE-SIG-B content channel 2
                               MicroSeconds(16)),    // four OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, central 26-tones RU in low 80 MHz
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1}, // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4}, // CC1
                                {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 4, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 7, true}, 5, 3},   // CC2
                                {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 6, 2},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 3, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, true}, 8, 2},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 3, true}, 1, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 4, true}, 4, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_996_TONE, 1, false},
                                 1,
                                 1}}, // CC1 or CC2 => CC1 for better split
                               VhtPhy::GetVhtMcs1(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 15, 192, 192, 208, 115, 208, 115},
                               std::make_pair(5, 6), // five users in HE-SIG-B content channel 1 and
                                                     // seven users in HE-SIG-B content channel 2
                               MicroSeconds(16)),    // four OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, central 26-tones RU in high 80 MHz
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4},  // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 3, true}, 11, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_106_TONE, 4, true}, 10, 4},  // CC2
                                {HeRu::RuSpec{RuType::RU_242_TONE, 3, true}, 10, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 4, true}, 11, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_484_TONE, 1, false}, 7, 1},  // CC1 or CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, false}, 8, 2},  // CC2
                                {HeRu::RuSpec{RuType::RU_484_TONE, 2, false}, 9, 1}}, // CC1 or CC2
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 96, 192, 192, 200, 114, 114, 200},
                               std::make_pair(4, 5), // two users in HE-SIG-B content channel 1 and
                                                     // one user in HE-SIG-B content channel 2
                               MicroSeconds(4)),     // two OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, central 26-tones RU in both 80 MHz
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_106_TONE, 1, true}, 11, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 2, true}, 10, 4},  // CC1
                                {HeRu::RuSpec{RuType::RU_106_TONE, 3, true}, 11, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_106_TONE, 4, true}, 10, 4},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, true}, 8, 2},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 3, true}, 10, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 4, true}, 11, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_484_TONE, 1, false}, 7, 1},  // CC1 or CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, false}, 8, 2},  // CC2
                                {HeRu::RuSpec{RuType::RU_484_TONE, 2, false}, 9, 1}}, // CC1 or CC2
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {96, 96, 192, 192, 200, 114, 114, 200},
                               std::make_pair(5, 5), // two users in HE-SIG-B content channel 1 and
                                                     // one user in HE-SIG-B content channel 2
                               MicroSeconds(4)),     // two OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, maximum number of users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_26_TONE, 1, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 2, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 3, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 4, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 5, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 6, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 7, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 8, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 9, true}, 8, 1},    // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 10, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 11, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 12, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 13, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 15, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 16, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 17, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 18, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 20, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 21, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 22, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 23, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 24, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 25, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 26, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 27, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 28, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 29, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 30, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 31, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 32, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 33, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 34, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 35, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 36, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 37, true}, 8, 1},   // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 1, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 2, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 3, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 4, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 5, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 6, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 7, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 8, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 9, false}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 10, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 11, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 12, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 13, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 14, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 15, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 16, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 17, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 18, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 19, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 20, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 21, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 22, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 23, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 24, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 25, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 26, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 27, false}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_26_TONE, 28, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 29, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 30, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 31, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 32, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 33, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 34, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 35, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 36, false}, 8, 1},  // CC2
                                {HeRu::RuSpec{RuType::RU_26_TONE, 37, false}, 8, 1}}, // CC2
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {0, 0, 0, 0, 0, 0, 0, 0},
                               std::make_pair(37,
                                              37), // 37 users (36 users + 1 central tones-RU user)
                                                   // in each HE-SIG-B content channel
                               MicroSeconds(20)),  // five OFDM symbols
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, single-user using 2x996 tones RU
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 8, 1}}, // CC1
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{160},
                               0,
                               HeSigBDurationTest::OFDMA,
                               {208, 208, 208, 208, 208, 208, 208, 208},
                               std::make_pair(1, 0), // one user in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol;
        TestCase::Duration::QUICK);

    // 160 MHz band, OFDMA, primary80 is in the high 80 MHz band
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_996_TONE, 1, false}, 8, 1}, // CC2
                                {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 8, 1}}, // CC1
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{160},
                               4,
                               HeSigBDurationTest::OFDMA,
                               {208, 115, 208, 115, 115, 208, 115, 208},
                               std::make_pair(1, 1), // one user in each HE-SIG-B content channel
                               MicroSeconds(4)),     // one OFDM symbol;
        TestCase::Duration::QUICK);

    // 20 MHz band, OFDMA, one unallocated RU at the middle
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_26_TONE, 1, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 2, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 3, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 4, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 6, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 7, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 8, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_26_TONE, 9, true}, 11, 1}}, // CC1
            VhtPhy::GetVhtMcs5(),
            MHz_u{20},
            0,
            HeSigBDurationTest::OFDMA,
            {0},
            std::make_pair(9, 0), // 9 users (8 users + 1 empty user) in HE-SIG-B content channel 1
            MicroSeconds(8)),     // two OFDM symbols
        TestCase::Duration::QUICK);

    // 40 MHz band, OFDMA, unallocated RUs at the begin and at the end of the
    // first 20 MHz subband and in the middle of the second 20 MHz subband
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_52_TONE, 2, true}, 10, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_52_TONE, 3, true}, 10, 2},  // CC1
             {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 11, 1},  // CC2
             {HeRu::RuSpec{RuType::RU_52_TONE, 8, true}, 11, 2}}, // CC2
            VhtPhy::GetVhtMcs5(),
            MHz_u{40},
            0,
            HeSigBDurationTest::OFDMA,
            {112, 112},
            std::make_pair(4,
                           4), // 4 users (2 users + 2 empty users) in each HE-SIG-B content channel
            MicroSeconds(4)),  // one OFDM symbol
        TestCase::Duration::QUICK);

    // 40 MHz band, OFDMA, one unallocated RUs in the first 20 MHz subband and
    // two unallocated RUs in second 20 MHz subband
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_52_TONE, 1, true}, 10, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_52_TONE, 2, true}, 10, 2},  // CC1
             {HeRu::RuSpec{RuType::RU_52_TONE, 3, true}, 11, 1},  // CC1
             {HeRu::RuSpec{RuType::RU_52_TONE, 5, true}, 11, 2},  // CC2
             {HeRu::RuSpec{RuType::RU_52_TONE, 6, true}, 11, 3}}, // CC2
            VhtPhy::GetVhtMcs5(),
            MHz_u{40},
            0,
            HeSigBDurationTest::OFDMA,
            {112, 112},
            std::make_pair(4,
                           4), // 4 users (3 users + 1 empty user) in HE-SIG-B content channel 1 and
                               // 4 users (2 users + 2 empty users) in HE-SIG-B content channel 2
            MicroSeconds(4)),  // one OFDM symbol
        TestCase::Duration::QUICK);

    // 40 MHz band, OFDMA, first 20 MHz is punctured
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 2, true}, 11, 1}}, // CC2
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{40},
                               1,
                               HeSigBDurationTest::OFDMA,
                               {113, 192},
                               std::make_pair(0, 1), // one user in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol;
        TestCase::Duration::QUICK);

    // 20 MHz band, MU-MIMO, 2 users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 11, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 10, 4}}, // CC1
                               VhtPhy::GetVhtMcs5(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::MU_MIMO,
                               {192},
                               std::make_pair(2, 0), // both users in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 20 MHz band, MU-MIMO, 3 users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 4, 3},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 5, 2},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 6, 1}}, // CC1
                               VhtPhy::GetVhtMcs4(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::MU_MIMO,
                               {192},
                               std::make_pair(3, 0), // all users in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 20 MHz band, MU-MIMO, 4 users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 4, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 5, 2},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 6, 3},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 7, 2}}, // CC1
                               VhtPhy::GetVhtMcs4(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::MU_MIMO,
                               {192},
                               std::make_pair(4, 0), // all users in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 20 MHz band, MU-MIMO, 6 users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 4, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 5, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 6, 2},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 7, 2},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 8, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 9, 1}}, // CC1
                               VhtPhy::GetVhtMcs4(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::MU_MIMO,
                               {192},
                               std::make_pair(6, 0), // all users in HE-SIG-B content channel 1
                               MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 20 MHz band, MU-MIMO, 8 users
    AddTestCase(
        new HeSigBDurationTest({{HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 4, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 5, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 6, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 7, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 8, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 9, 1},   // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 10, 1},  // CC1
                                {HeRu::RuSpec{RuType::RU_242_TONE, 1, true}, 11, 1}}, // CC1
                               VhtPhy::GetVhtMcs4(),
                               MHz_u{20},
                               0,
                               HeSigBDurationTest::MU_MIMO,
                               {192},
                               std::make_pair(8, 0), // all users in HE-SIG-B content channel 1
                               MicroSeconds(8)),     // two OFDM symbols
        TestCase::Duration::QUICK);

    // 40 MHz band, MU-MIMO, 2 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 11, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 10, 4}}, // CC2
                    VhtPhy::GetVhtMcs5(),
                    MHz_u{40},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {200, 200},
                    std::make_pair(1, 1), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 40 MHz band, MU-MIMO, 3 users
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 4, 3},  // CC1
             {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 5, 2},  // CC2
             {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 6, 1}}, // CC1
            VhtPhy::GetVhtMcs4(),
            MHz_u{40},
            0,
            HeSigBDurationTest::MU_MIMO,
            {200, 200},
            std::make_pair(2, 1), // 2 users in content channel 1 and 1 user in content channel 2
            MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 40 MHz band, MU-MIMO, 4 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 5, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 6, 3},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 7, 2}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{40},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {200, 200},
                    std::make_pair(2, 2), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 40 MHz band, MU-MIMO, 6 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 5, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 6, 2},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 7, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 9, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{40},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {200, 200},
                    std::make_pair(3, 3), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 40 MHz band, MU-MIMO, 8 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 4, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 5, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 6, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 7, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 9, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 10, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_484_TONE, 1, true}, 11, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{40},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {200, 200},
                    std::make_pair(4, 4), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 80 MHz band, MU-MIMO, 2 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 11, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 10, 4}}, // CC2
                    VhtPhy::GetVhtMcs5(),
                    MHz_u{80},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208},
                    std::make_pair(1, 1), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 80 MHz band, MU-MIMO, 3 users
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 4, 3},  // CC1
             {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 5, 2},  // CC2
             {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 6, 1}}, // CC1
            VhtPhy::GetVhtMcs4(),
            MHz_u{80},
            0,
            HeSigBDurationTest::MU_MIMO,
            {208, 208, 208, 208},
            std::make_pair(2, 1), // 2 users in content channel 1 and 1 user in content channel 2
            MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 80 MHz band, MU-MIMO, 4 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 5, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 6, 3},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 7, 2}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{80},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208},
                    std::make_pair(2, 2), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 80 MHz band, MU-MIMO, 6 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 5, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 6, 2},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 7, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 9, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{80},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208},
                    std::make_pair(3, 3), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 80 MHz band, MU-MIMO, 8 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 4, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 5, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 6, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 7, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 9, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 10, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_996_TONE, 1, true}, 11, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{80},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208},
                    std::make_pair(4, 4), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 160 MHz band, MU-MIMO, 2 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 11, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 10, 4}}, // CC2
                    VhtPhy::GetVhtMcs5(),
                    MHz_u{160},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208, 208, 208, 208, 208},
                    std::make_pair(1, 1), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 160 MHz band, MU-MIMO, 3 users
    AddTestCase(
        new HeSigBDurationTest(
            {{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 4, 3},  // CC1
             {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 5, 2},  // CC2
             {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 6, 1}}, // CC1
            VhtPhy::GetVhtMcs4(),
            MHz_u{160},
            0,
            HeSigBDurationTest::MU_MIMO,
            {208, 208, 208, 208, 208, 208, 208, 208},
            std::make_pair(2, 1), // 2 users in content channel 1 and 1 user in content channel 2
            MicroSeconds(4)),     // one OFDM symbol
        TestCase::Duration::QUICK);

    // 160 MHz band, MU-MIMO, 4 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 5, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 6, 3},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 7, 2}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{160},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208, 208, 208, 208, 208},
                    std::make_pair(2, 2), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 160 MHz band, MU-MIMO, 6 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 4, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 5, 1},  // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 6, 2},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 7, 2},  // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 8, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 9, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{160},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208, 208, 208, 208, 208},
                    std::make_pair(3, 3), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);

    // 160 MHz band, MU-MIMO, 8 users
    AddTestCase(new HeSigBDurationTest(
                    {{HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 4, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 5, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 6, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 7, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 8, 1},   // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 9, 1},   // CC2
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 10, 1},  // CC1
                     {HeRu::RuSpec{RuType::RU_2x996_TONE, 1, true}, 11, 1}}, // CC2
                    VhtPhy::GetVhtMcs4(),
                    MHz_u{160},
                    0,
                    HeSigBDurationTest::MU_MIMO,
                    {208, 208, 208, 208, 208, 208, 208, 208},
                    std::make_pair(4, 4), // users equally split between the two content channels
                    MicroSeconds(4)),     // one OFDM symbol
                TestCase::Duration::QUICK);
}

static TxDurationTestSuite g_txDurationTestSuite; ///< the test suite
