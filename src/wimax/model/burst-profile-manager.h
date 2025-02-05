/*
 * Copyright (c) 2007,2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Jahanzeb Farooq <jahanzeb.farooq@sophia.inria.fr>
 */

#ifndef BURST_PROFILE_MANAGER_H
#define BURST_PROFILE_MANAGER_H

#include "cid.h"
#include "wimax-net-device.h"
#include "wimax-phy.h"

#include <stdint.h>

namespace ns3
{

class SSRecord;
class RngReq;

/**
 * @ingroup wimax
 *
 * Profile manager for burst communications
 */
class BurstProfileManager : public Object
{
  public:
    /**
     * @brief Get the type ID.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * Constructor
     *
     * @param device WIMAX device
     */
    BurstProfileManager(Ptr<WimaxNetDevice> device);
    ~BurstProfileManager() override;

    // Delete copy constructor and assignment operator to avoid misuse
    BurstProfileManager(const BurstProfileManager&) = delete;
    BurstProfileManager& operator=(const BurstProfileManager&) = delete;

    void DoDispose() override;
    /**
     * @returns the number of available burst profile
     */
    uint16_t GetNrBurstProfilesToDefine();

    /**
     * @brief returns the modulation type of a given iuc
     * @param direction should be uplink or downlink
     * @param iuc the iuc
     * @returns the modulation type of the selected iuc
     */
    WimaxPhy::ModulationType GetModulationType(uint8_t iuc,
                                               WimaxNetDevice::Direction direction) const;

    /**
     * @brief returns the burst profile
     * @param modulationType
     * @param direction should be uplink or downlink
     * @returns the modulation type of the selected iuc
     */
    uint8_t GetBurstProfile(WimaxPhy::ModulationType modulationType,
                            WimaxNetDevice::Direction direction) const;

    /**
     * @brief Get burst profile for SS
     * @param modulationType
     * @param ssRecord
     * @param rngreq
     * @returns the burst profile for SS
     */
    uint8_t GetBurstProfileForSS(const SSRecord* ssRecord,
                                 const RngReq* rngreq,
                                 WimaxPhy::ModulationType& modulationType) const;
    /**
     * @brief Get module ation type for SS
     * @param ssRecord
     * @param rngreq
     * @returns the burst profile for SS
     */
    WimaxPhy::ModulationType GetModulationTypeForSS(const SSRecord* ssRecord,
                                                    const RngReq* rngreq) const;
    /**
     * @brief Get burst profile to request
     * @returns the burst profile for SS
     */
    uint8_t GetBurstProfileToRequest();

  private:
    Ptr<WimaxNetDevice> m_device; ///< the device
};

} // namespace ns3

#endif /* BURST_PROFILE_MANAGER_H */
