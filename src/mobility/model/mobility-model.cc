/*
 * Copyright (c) 2006,2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "mobility-model.h"

#include "ns3/trace-source-accessor.h"

#include <cmath>

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED(MobilityModel);

TypeId
MobilityModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::MobilityModel")
            .SetParent<Object>()
            .SetGroupName("Mobility")
            .AddAttribute(
                "Position",
                "The current position of the mobility model.",
                TypeId::ATTR_SET | TypeId::ATTR_GET,
                VectorValue(Vector(0.0, 0.0, 0.0)),
                MakeVectorAccessor(&MobilityModel::SetPosition, &MobilityModel::GetPosition),
                MakeVectorChecker())
            .AddAttribute("Velocity",
                          "The current velocity of the mobility model.",
                          TypeId::ATTR_GET,
                          VectorValue(Vector(0.0, 0.0, 0.0)), // ignored initial value.
                          MakeVectorAccessor(&MobilityModel::GetVelocity),
                          MakeVectorChecker())
            .AddTraceSource("CourseChange",
                            "The value of the position and/or velocity vector changed",
                            MakeTraceSourceAccessor(&MobilityModel::m_courseChangeTrace),
                            "ns3::MobilityModel::TracedCallback");
    return tid;
}

MobilityModel::MobilityModel()
{
}

MobilityModel::~MobilityModel()
{
}

Vector
MobilityModel::GetPosition() const
{
    return DoGetPosition();
}

Vector
MobilityModel::GetPositionWithReference(const Vector& referencePosition) const
{
    return DoGetPositionWithReference(referencePosition);
}

// Default implementation ignores referencePosition
Vector
MobilityModel::DoGetPositionWithReference(const Vector& referencePosition) const
{
    return DoGetPosition();
}

Vector
MobilityModel::GetVelocity() const
{
    return DoGetVelocity();
}

void
MobilityModel::SetPosition(const Vector& position)
{
    DoSetPosition(position);
}

double
MobilityModel::GetDistanceFrom(Ptr<const MobilityModel> other) const
{
    Vector oPosition = other->DoGetPosition();
    Vector position = DoGetPosition();
    return CalculateDistance(position, oPosition);
}

double
MobilityModel::GetRelativeSpeed(Ptr<const MobilityModel> other) const
{
    return (GetVelocity() - other->GetVelocity()).GetLength();
}

void
MobilityModel::NotifyCourseChange() const
{
    m_courseChangeTrace(this);
}

int64_t
MobilityModel::AssignStreams(int64_t start)
{
    return DoAssignStreams(start);
}

// Default implementation does nothing
int64_t
MobilityModel::DoAssignStreams(int64_t start)
{
    return 0;
}

} // namespace ns3
