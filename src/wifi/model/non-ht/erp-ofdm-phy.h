/*
 * Copyright (c) 2020 Orange Labs
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Rediet <getachew.redieteab@orange.com>
 *          Sébastien Deronne <sebastien.deronne@gmail.com> (for logic ported from wifi-phy)
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr> (for logic ported from wifi-phy)
 */

#ifndef ERP_OFDM_PHY_H
#define ERP_OFDM_PHY_H

#include "ofdm-phy.h"

/**
 * @file
 * @ingroup wifi
 * Declaration of ns3::ErpOfdmPhy class.
 */

namespace ns3
{

/**
 * @brief PHY entity for ERP-OFDM (11g)
 * @ingroup wifi
 *
 * ERP-OFDM PHY is based on OFDM PHY.
 * ERP-DSSS/CCK mode is not supported.
 *
 * Refer to IEEE 802.11-2016, clause 18.
 */
class ErpOfdmPhy : public OfdmPhy
{
  public:
    /**
     * Constructor for ERP-OFDM PHY
     */
    ErpOfdmPhy();
    /**
     * Destructor for ERP-OFDM PHY
     */
    ~ErpOfdmPhy() override;

    Ptr<WifiPpdu> BuildPpdu(const WifiConstPsduMap& psdus,
                            const WifiTxVector& txVector,
                            Time ppduDuration) override;
    uint32_t GetMaxPsduSize() const override;

    /**
     * Initialize all ERP-OFDM modes.
     */
    static void InitializeModes();
    /**
     * Return a WifiMode for ERP-OFDM
     * corresponding to the provided rate.
     *
     * @param rate the rate in bps
     * @return a WifiMode for ERP-OFDM
     */
    static WifiMode GetErpOfdmRate(uint64_t rate);

    /**
     * Return a WifiMode for ERP-OFDM at 6 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 6 Mbps
     */
    static WifiMode GetErpOfdmRate6Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 9 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 9 Mbps
     */
    static WifiMode GetErpOfdmRate9Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 12 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 12 Mbps
     */
    static WifiMode GetErpOfdmRate12Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 18 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 18 Mbps
     */
    static WifiMode GetErpOfdmRate18Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 24 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 24 Mbps
     */
    static WifiMode GetErpOfdmRate24Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 36 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 36 Mbps
     */
    static WifiMode GetErpOfdmRate36Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 48 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 48 Mbps
     */
    static WifiMode GetErpOfdmRate48Mbps();
    /**
     * Return a WifiMode for ERP-OFDM at 54 Mbps.
     *
     * @return a WifiMode for ERP-OFDM at 54 Mbps
     */
    static WifiMode GetErpOfdmRate54Mbps();

    /**
     * Return the WifiCodeRate from the ERP-OFDM mode's unique name using
     * ModulationLookupTable. This is mainly used as a callback for
     * WifiMode operation.
     *
     * @param name the unique name of the ERP-OFDM mode
     * @return WifiCodeRate corresponding to the unique name
     */
    static WifiCodeRate GetCodeRate(const std::string& name);
    /**
     * Return the constellation size from the ERP-OFDM mode's unique name using
     * ModulationLookupTable. This is mainly used as a callback for
     * WifiMode operation.
     *
     * @param name the unique name of the ERP-OFDM mode
     * @return constellation size corresponding to the unique name
     */
    static uint16_t GetConstellationSize(const std::string& name);
    /**
     * Return the PHY rate from the ERP-OFDM mode's unique name and
     * the supplied parameters. This function calls OfdmPhy::CalculatePhyRate
     * and is mainly used as a callback for WifiMode operation.
     *
     * @param name the unique name of the ERP-OFDM mode
     * @param channelWidth the considered channel width
     *
     * @return the physical bit rate of this signal in bps.
     */
    static uint64_t GetPhyRate(const std::string& name, MHz_u channelWidth);
    /**
     * Return the PHY rate corresponding to
     * the supplied TXVECTOR.
     * This function is mainly used as a callback
     * for WifiMode operation.
     *
     * @param txVector the TXVECTOR used for the transmission
     * @param staId the station ID (only here to have a common signature for all callbacks)
     * @return the physical bit rate of this signal in bps.
     */
    static uint64_t GetPhyRateFromTxVector(const WifiTxVector& txVector, uint16_t staId);
    /**
     * Return the data rate corresponding to
     * the supplied TXVECTOR.
     * This function is mainly used as a callback
     * for WifiMode operation.
     *
     * @param txVector the TXVECTOR used for the transmission
     * @param staId the station ID (only here to have a common signature for all callbacks)
     * @return the data bit rate in bps.
     */
    static uint64_t GetDataRateFromTxVector(const WifiTxVector& txVector, uint16_t staId);
    /**
     * Return the data rate from the ERP-OFDM mode's unique name and
     * the supplied parameters. This function calls OfdmPhy::CalculateDataRate
     * and is mainly used as a callback for WifiMode operation.
     *
     * @param name the unique name of the ERP-OFDM mode
     * @param channelWidth the considered channel width
     *
     * @return the data bit rate of this signal in bps.
     */
    static uint64_t GetDataRate(const std::string& name, MHz_u channelWidth);
    /**
     * Check whether the combination in TXVECTOR is allowed.
     * This function is used as a callback for WifiMode operation.
     *
     * @param txVector the TXVECTOR
     * @returns true if this combination is allowed, false otherwise.
     */
    static bool IsAllowed(const WifiTxVector& txVector);

  private:
    WifiMode GetHeaderMode(const WifiTxVector& txVector) const override;
    Time GetPreambleDuration(const WifiTxVector& txVector) const override;
    Time GetHeaderDuration(const WifiTxVector& txVector) const override;

    /**
     * Create an ERP-OFDM mode from a unique name, the unique name
     * must already be contained inside ModulationLookupTable.
     * This method binds all the callbacks used by WifiMode.
     *
     * @param uniqueName the unique name of the WifiMode
     * @param isMandatory whether the WifiMode is mandatory
     * @return the ERP-OFDM WifiMode
     */
    static WifiMode CreateErpOfdmMode(std::string uniqueName, bool isMandatory);

    static const ModulationLookupTable
        m_erpOfdmModulationLookupTable; //!< lookup table to retrieve code rate and constellation
                                        //!< size corresponding to a unique name of modulation
};

} // namespace ns3

#endif /* ERP_OFDM_PHY_H */
