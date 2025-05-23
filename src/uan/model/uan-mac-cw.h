/*
 * Copyright (c) 2009 University of Washington
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Leonard Tracy <lentracy@gmail.com>
 */

#ifndef UAN_MAC_CW_H
#define UAN_MAC_CW_H

#include "uan-mac.h"
#include "uan-phy.h"
#include "uan-tx-mode.h"

#include "ns3/mac8-address.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"

namespace ns3
{

/**
 * @ingroup uan
 *
 * CW-MAC protocol, similar in idea to the 802.11 DCF with
 * constant backoff window
 *
 * For more information on this MAC protocol, see:
 * Parrish, N.; Tracy, L.; Roy, S.; Arabshahi, P.; Fox, W.,
 * "System Design Considerations for Undersea Networks: Link and
 * Multiple Access Protocols," Selected Areas in Communications,
 * IEEE Journal on , vol.26, no.9, pp.1720-1730, December 2008
 */
class UanMacCw : public UanMac, public UanPhyListener
{
  public:
    /** Default constructor */
    UanMacCw();
    /** Dummy destructor, DoDispose. */
    ~UanMacCw() override;
    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Set the contention window size.
     *
     * @param cw Contention window size.
     */
    virtual void SetCw(uint32_t cw);
    /**
     * Set the slot time duration.
     *
     * @param duration Slot time duration.
     */
    virtual void SetSlotTime(Time duration);
    /**
     * Get the contention window size.
     *
     * @return Contention window size.
     */
    virtual uint32_t GetCw();
    /**
     * Get the slot time duration.
     *
     * @return Slot time duration.
     */
    virtual Time GetSlotTime();

    // Inherited methods from UanMac
    bool Enqueue(Ptr<Packet> pkt, uint16_t protocolNumber, const Address& dest) override;
    void SetForwardUpCb(Callback<void, Ptr<Packet>, uint16_t, const Mac8Address&> cb) override;
    void AttachPhy(Ptr<UanPhy> phy) override;
    void Clear() override;
    int64_t AssignStreams(int64_t stream) override;

    // Inherited methods from UanPhyListener
    void NotifyRxStart() override;
    void NotifyRxEndOk() override;
    void NotifyRxEndError() override;
    void NotifyCcaStart() override;
    void NotifyCcaEnd() override;
    void NotifyTxStart(Time duration) override;
    void NotifyTxEnd() override;

    /**
     *  TracedCallback signature for enqueue/dequeue of a packet.
     *
     * @param [in] packet The Packet being received.
     * @param [in] proto The protocol number.
     */
    typedef void (*QueueTracedCallback)(Ptr<const Packet> packet, uint16_t proto);

  private:
    /** Enum defining possible Phy states. */
    enum State
    {
        IDLE,    //!< Idle state.
        CCABUSY, //!< Channel busy.
        RUNNING, //!< Delay timer running.
        TX       //!< Transmitting.
    };

    /** Forwarding up callback. */
    Callback<void, Ptr<Packet>, uint16_t, const Mac8Address&> m_forwardUpCb;
    /** PHY layer attached to this MAC. */
    Ptr<UanPhy> m_phy;
    /** A packet destined for this MAC was received. */
    TracedCallback<Ptr<const Packet>, UanTxMode> m_rxLogger;
    /** A packet arrived at the MAC for transmission. */
    TracedCallback<Ptr<const Packet>, uint16_t> m_enqueueLogger;
    /** A packet was passed down to the PHY from the MAC. */
    TracedCallback<Ptr<const Packet>, uint16_t> m_dequeueLogger;

    // Mac parameters
    uint32_t m_cw;   //!< Contention window size.
    Time m_slotTime; //!< Slot time duration.

    // State variables
    /** Time to send next packet. */
    Time m_sendTime;
    /** Remaining delay until next send. */
    Time m_savedDelayS;
    /** Next packet to send. */
    Ptr<Packet> m_pktTx;
    /** Next packet protocol number (usage varies by MAC). */
    uint16_t m_pktTxProt;
    /** Scheduled SendPacket event. */
    EventId m_sendEvent;
    /** Tx is ongoing */
    bool m_txOngoing;
    /** Current state. */
    State m_state;

    /** Flag when we've been cleared */
    bool m_cleared;

    /** Provides uniform random variable for contention window. */
    Ptr<UniformRandomVariable> m_rv;

    /**
     * Receive packet from lower layer (passed to PHY as callback).
     *
     * @param packet Packet being received.
     * @param sinr SINR of received packet.
     * @param mode Mode of received packet.
     */
    void PhyRxPacketGood(Ptr<Packet> packet, double sinr, UanTxMode mode);
    /**
     * Packet received at lower layer in error.
     *
     * @param packet Packet received in error.
     * @param sinr SINR of received packet.
     */
    void PhyRxPacketError(Ptr<Packet> packet, double sinr);
    /** Cancel SendEvent and save remaining delay. */
    void SaveTimer();
    /** Schedule SendPacket after delay. */
    void StartTimer();
    /** Send packet on PHY. */
    void SendPacket();
    /** End TX state. */
    void EndTx();

  protected:
    void DoDispose() override;
};

} // namespace ns3

#endif /* UAN_MAC_CW_H */
