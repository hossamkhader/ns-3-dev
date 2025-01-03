/*
 * Copyright (c) 2007,2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Jahanzeb Farooq <jahanzeb.farooq@sophia.inria.fr>
 */

#ifndef SERVICE_FLOW_RECORD_H
#define SERVICE_FLOW_RECORD_H

#include "bs-net-device.h"
#include "bs-uplink-scheduler.h"

#include "ns3/nstime.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3
{

/**
 * @ingroup wimax
 * @brief this class implements a structure to manage some parameters and statistics related to a
 * service flow
 */
class ServiceFlowRecord
{
  public:
    ServiceFlowRecord();
    ~ServiceFlowRecord();

    /**
     * @brief Set the grant size (only for UGS service flows)
     * @param grantSize the grant size to set
     */
    void SetGrantSize(uint32_t grantSize);
    /**
     * @return the grant size (only for ugs service flows)
     */
    uint32_t GetGrantSize() const;
    /**
     * @brief Set the grant time stamp. Used for data allocation for ugs flows, and unicast poll
     * (bw request) for non-UGS flows
     * @param grantTimeStamp the grant time stamp to set
     */
    void SetGrantTimeStamp(Time grantTimeStamp);
    /**
     * @return the grant time stamp. Used for data allocation for ugs flows, and unicast poll (bw
     * request) for non-UGS flows
     */
    Time GetGrantTimeStamp() const;
    /**
     * @brief Set the DlTimeStamp.
     * @param dlTimeStamp time when this service flow's traffic was last sent.
     */
    void SetDlTimeStamp(Time dlTimeStamp);
    /**
     * @return the DlTimeStamp: time when this service flow's traffic was last sent
     */
    Time GetDlTimeStamp() const;
    /**
     * @brief set the number of sent packets in this service flow
     * @param pktsSent the number of sent packets
     */
    void SetPktsSent(uint32_t pktsSent);
    /**
     * @brief update the number of sent packets by adding pktsSent
     * @param pktsSent the number of sent packets to add
     */
    void UpdatePktsSent(uint32_t pktsSent);
    /**
     * @return the number of sent packet in this service flow
     */
    uint32_t GetPktsSent() const;
    /**
     * @brief Set the number of received packets
     * @param pktsRcvd The number of received packets
     */
    void SetPktsRcvd(uint32_t pktsRcvd);
    /**
     * @brief update the number of received packets by adding pktsRcvd
     * @param pktsRcvd the number of received packets to add
     */
    void UpdatePktsRcvd(uint32_t pktsRcvd);
    /**
     * @return the number of received packet
     */
    uint32_t GetPktsRcvd() const;
    /**
     * @brief Set the number of sent bytes
     * @param bytesSent the number of sent bytes
     */
    void SetBytesSent(uint32_t bytesSent);
    /**
     * @brief update the number of sent bytes by adding bytesSent
     * @param bytesSent the number of bytes to add
     */
    void UpdateBytesSent(uint32_t bytesSent);
    /**
     * @return The number of sent bytes
     */
    uint32_t GetBytesSent() const;
    /**
     * @brief Set the number of received bytes
     * @param bytesRcvd the number of received bytes
     */
    void SetBytesRcvd(uint32_t bytesRcvd);
    /**
     * @brief update the number of received bytes by adding bytesRcvd
     * @param bytesRcvd the number of bytes to add
     */
    void UpdateBytesRcvd(uint32_t bytesRcvd);
    /**
     * @return The number of received bytes
     */
    uint32_t GetBytesRcvd() const;

    /**
     * @brief set the requested bandwidth
     * @param requestedBandwidth the requested bandwidth
     */
    void SetRequestedBandwidth(uint32_t requestedBandwidth);
    /**
     * @brief update the requested bandwidth
     * @param requestedBandwidth the requested bandwidth update
     */
    void UpdateRequestedBandwidth(uint32_t requestedBandwidth);
    /**
     * @return The requested bandwidth
     */
    uint32_t GetRequestedBandwidth() const;

    /**
     * @brief set the granted bandwidth
     * @param grantedBandwidth the granted bandwidth
     */
    void SetGrantedBandwidth(uint32_t grantedBandwidth);
    /**
     * @brief update the granted bandwidth
     * @param grantedBandwidth the granted bandwidth update
     */
    void UpdateGrantedBandwidth(uint32_t grantedBandwidth);
    /**
     * @return The granted bandwidth
     */
    uint32_t GetGrantedBandwidth() const;

    /**
     * @brief set the temporary granted bandwidth
     * @param grantedBandwidthTemp the temporary granted bandwidth
     */
    void SetGrantedBandwidthTemp(uint32_t grantedBandwidthTemp);
    /**
     * @brief update the temporary granted bandwidth
     * @param grantedBandwidthTemp the temporary granted bandwidth
     */
    void UpdateGrantedBandwidthTemp(uint32_t grantedBandwidthTemp);
    /**
     * @return The temporary granted bandwidth
     */
    uint32_t GetGrantedBandwidthTemp() const;

    /**
     * @brief set BW since last expiry
     * @param bwSinceLastExpiry bandwidth since last expiry
     */
    void SetBwSinceLastExpiry(uint32_t bwSinceLastExpiry);
    /**
     * @brief update BW since last expiry
     * @param bwSinceLastExpiry bandwidth since last expiry
     */
    void UpdateBwSinceLastExpiry(uint32_t bwSinceLastExpiry);
    /**
     * @return The bandwidth since last expiry
     */
    uint32_t GetBwSinceLastExpiry() const;

    /**
     * @brief set last grant time
     * @param grantTime  grant time to set
     */
    void SetLastGrantTime(Time grantTime);
    /**
     * @return The last grant time
     */
    Time GetLastGrantTime() const;

    /**
     * @brief set backlogged
     * @param backlogged number of backlogged
     */
    void SetBacklogged(uint32_t backlogged);
    /**
     * @brief increase backlogged
     * @param backlogged the number of backlogged to update
     */
    void IncreaseBacklogged(uint32_t backlogged);
    /**
     * @return The number of backlogged
     */
    uint32_t GetBacklogged() const;

    /**
     * @brief set temporary back logged
     * @param backloggedTemp the temporary backlogged value
     */
    void SetBackloggedTemp(uint32_t backloggedTemp);
    /**
     * @brief increase temporary back logged
     * @param backloggedTemp the temporary backlogged value
     */
    void IncreaseBackloggedTemp(uint32_t backloggedTemp);
    /**
     * @return The value of temporary backlogged
     */
    uint32_t GetBackloggedTemp() const;

  private:
    uint32_t m_grantSize;  ///< only used for UGS flow
    Time m_grantTimeStamp; ///< allocation (for data) for UGS flows and unicast poll (for bandwidth
                           ///< requests) for non-UGS flows
    Time m_dlTimeStamp;    ///< time when this service flow's traffic was last sent

    // stats members
    uint32_t m_pktsSent; ///< packets sent
    uint32_t m_pktsRcvd; ///< packets received

    uint32_t m_bytesSent; ///< bytes sent
    uint32_t m_bytesRcvd; ///< bytes received

    uint32_t m_requestedBandwidth;   ///< requested bandwidth
    uint32_t m_grantedBandwidth;     ///< granted badnwidth
    uint32_t m_grantedBandwidthTemp; ///< Temporary variable used to sort list. Necessary to keep
                                     ///< original order

    /** bandwidth granted since last expiry of minimum reserved traffic rate interval,
     * only for nrtPS, to make sure minimum reserved traffic rate is maintained */
    uint32_t m_bwSinceLastExpiry;
    Time m_lastGrantTime;     ///< last grant time
    int32_t m_backlogged;     ///< back logged
    int32_t m_backloggedTemp; ///< back logged temp
};

} // namespace ns3

#endif /* SERVICE_FLOW_RECORD_H */
