/*
 * Copyright (c) 2009 IITP RAS
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Denis Fakhriev <fakhriev@iitp.ru>
 */
#ifndef STEADY_STATE_RANDOM_WAYPOINT_MOBILITY_MODEL_H
#define STEADY_STATE_RANDOM_WAYPOINT_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"

#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

/**
 * @ingroup mobility
 * @brief Steady-state random waypoint mobility model.
 *
 * This model based on random waypoint mobility (RWM) model for case when
 * speed, pause and position are uniformly distributed random variables.
 * The difference is that the initial values of this parameters are not
 * from uniform distribution but from stationary distribution of RWM model.
 * The implementation of this model is 2D-specific and with nonzero nodes speeds.
 * In the 3D ns-3 coordinate system, the movement occurs on the
 * \f$ z=\overline{Z} \f$ plane, where \f$ \overline{Z} \f$ is a constant which
 * can be configured using the Z attribute.
 *
 * Based on NS-2 implementation by Toilers Research Group -- Colorado
 *      School of Mines (http://toilers.mines.edu).
 * The papers related to this code are:
 *      W. Navidi and T. Camp, Stationary Distributions for the Random
 *      Waypoint Mobility Model, IEEE Transactions on Mobile Computing,
 *      vol. 3, no. 1, pp. 99-108, January-March 2004.
 *      W. Navidi, T. Camp, and N. Bauer, Improving the Accuracy of
 *      Random Waypoint Simulations Through Steady-State Initialization,
 *      Proceedings of the 15th International Conference on Modeling and
 *      Simulation (MS '04), pp. 319-326, March 2004.
 */
class SteadyStateRandomWaypointMobilityModel : public MobilityModel
{
  public:
    /**
     * Register this type with the TypeId system.
     * @return the object TypeId
     */
    static TypeId GetTypeId();
    SteadyStateRandomWaypointMobilityModel();
    ~SteadyStateRandomWaypointMobilityModel() override;

  protected:
    void DoInitialize() override;

  private:
    /**
     * Configure random variables based on attributes; calculate the steady
     * state probability that node is initially paused; schedule either end
     * of pause time or initial motion of the node.
     */
    void DoInitializePrivate();
    /**
     * Use provided destination to calculate travel delay, and schedule a
     * Start() event at that time.
     * @param destination the destination to move to
     */
    void SteadyStateBeginWalk(const Vector& destination);
    /**
     * Start a pause period and schedule the ending of the pause
     */
    void Start();
    /**
     * Start a motion period and schedule the ending of the motion
     */
    void BeginWalk();
    Vector DoGetPosition() const override;
    void DoSetPosition(const Vector& position) override;
    Vector DoGetVelocity() const override;
    int64_t DoAssignStreams(int64_t) override;

    ConstantVelocityHelper m_helper;            //!< helper for velocity computations
    double m_maxSpeed;                          //!< maximum speed value (m/s)
    double m_minSpeed;                          //!< minimum speed value (m/s)
    Ptr<UniformRandomVariable> m_speed;         //!< random variable for speed values
    double m_minX;                              //!< minimum x value of traveling region (m)
    double m_maxX;                              //!< maximum x value of traveling region (m)
    double m_minY;                              //!< minimum y value of traveling region (m)
    double m_maxY;                              //!< maximum y value of traveling region (m)
    double m_z;                                 //!< z value of traveling region
    Ptr<RandomBoxPositionAllocator> m_position; //!< position allocator
    double m_minPause;                          //!< minimum pause value (s)
    double m_maxPause;                          //!< maximum pause value (s)
    Ptr<UniformRandomVariable> m_pause;         //!< random variable for pause values
    EventId m_event;                            //!< current event ID
    bool alreadyStarted;                        //!< flag for starting state
    Ptr<UniformRandomVariable> m_x1_r;          //!< rv used in rejection sampling phase
    Ptr<UniformRandomVariable> m_y1_r;          //!< rv used in rejection sampling phase
    Ptr<UniformRandomVariable> m_x2_r;          //!< rv used in rejection sampling phase
    Ptr<UniformRandomVariable> m_y2_r;          //!< rv used in rejection sampling phase
    Ptr<UniformRandomVariable> m_u_r;           //!< rv used in step 5 of algorithm
    Ptr<UniformRandomVariable> m_x;             //!< rv used for position allocator
    Ptr<UniformRandomVariable> m_y;             //!< rv used for position allocator
};

} // namespace ns3

#endif /* STEADY_STATE_RANDOM_WAYPOINT_MOBILITY_MODEL_H */
