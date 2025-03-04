/*
 * Copyright (c) 2006,2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef RANDOM_WALK_2D_MOBILITY_MODEL_H
#define RANDOM_WALK_2D_MOBILITY_MODEL_H

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "rectangle.h"

#include "ns3/event-id.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{

/**
 * @ingroup mobility
 * @brief 2D random walk mobility model.
 *
 * Each instance moves with a speed and direction chosen at random
 * with the user-provided random variables until
 * either a fixed distance has been walked or until a fixed amount
 * of time. If we hit one of the boundaries (specified by a rectangle),
 * of the model, we rebound on the boundary with a reflexive angle
 * and speed. This model is often identified as a brownian motion
 * model.
 *
 * The Direction random variable is used for any point strictly
 * inside the boundaries. The points on the boundary have their
 * direction chosen randomly, without considering the Direction
 * Attribute.
 */
class RandomWalk2dMobilityModel : public MobilityModel
{
  public:
    /**
     * Register this type with the TypeId system.
     * @return the object TypeId
     */
    static TypeId GetTypeId();

    ~RandomWalk2dMobilityModel() override;

    /** An enum representing the different working modes of this module. */
    enum Mode
    {
        MODE_DISTANCE,
        MODE_TIME
    };

  private:
    /**
     * @brief Performs the rebound of the node if it reaches a boundary
     * @param timeLeft The remaining time of the walk
     */
    void Rebound(Time timeLeft);
    /**
     * Walk according to position and velocity, until distance is reached,
     * time is reached, or intersection with the bounding box
     * @param timeLeft The remaining time of the walk
     */
    void DoWalk(Time timeLeft);
    /**
     * Draw a new random velocity and distance to travel, and call DoWalk()
     */
    void DrawRandomVelocityAndDistance();
    void DoDispose() override;
    void DoInitialize() override;
    Vector DoGetPosition() const override;
    void DoSetPosition(const Vector& position) override;
    Vector DoGetVelocity() const override;
    int64_t DoAssignStreams(int64_t) override;

    ConstantVelocityHelper m_helper;       //!< helper for this object
    EventId m_event;                       //!< stored event ID
    Mode m_mode;                           //!< whether in time or distance mode
    double m_modeDistance;                 //!< Change direction and speed after this distance
    Time m_modeTime;                       //!< Change current direction and speed after this delay
    Ptr<RandomVariableStream> m_speed;     //!< rv for picking speed
    Ptr<RandomVariableStream> m_direction; //!< rv for picking direction
    Rectangle m_bounds;                    //!< Bounds of the area to cruise
};

} // namespace ns3

#endif /* RANDOM_WALK_2D_MOBILITY_MODEL_H */
