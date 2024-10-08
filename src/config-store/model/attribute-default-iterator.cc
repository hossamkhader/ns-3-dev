/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Faker Moatamri <faker.moatamri@sophia.inria.fr>
 *          Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "attribute-default-iterator.h"

#include "ns3/attribute.h"
#include "ns3/callback.h"
#include "ns3/global-value.h"
#include "ns3/object-ptr-container.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{

AttributeDefaultIterator::~AttributeDefaultIterator()
{
}

void
AttributeDefaultIterator::Iterate()
{
    for (uint32_t i = 0; i < TypeId::GetRegisteredN(); i++)
    {
        TypeId tid = TypeId::GetRegistered(i);
        if (tid.MustHideFromDocumentation())
        {
            continue;
        }
        bool calledStart = false;
        for (uint32_t j = 0; j < tid.GetAttributeN(); j++)
        {
            TypeId::AttributeInformation info = tid.GetAttribute(j);
            if (!(info.flags & TypeId::ATTR_CONSTRUCT))
            {
                // we can't construct the attribute, so, there is no
                // initial value for the attribute
                continue;
            }
            // No accessor, go to next attribute
            if (!info.accessor)
            {
                continue;
            }
            if (!info.accessor->HasSetter())
            {
                // skip this attribute it doesn't have an setter
                continue;
            }
            if (!info.checker)
            {
                // skip, it doesn't have a checker
                continue;
            }
            if (!info.initialValue)
            {
                // No value, check next attribute
                continue;
            }
            Ptr<const ObjectPtrContainerValue> vector =
                DynamicCast<const ObjectPtrContainerValue>(info.initialValue);
            if (vector)
            {
                // a vector value, won't take it
                continue;
            }
            Ptr<const PointerValue> pointer = DynamicCast<const PointerValue>(info.initialValue);
            if (pointer)
            {
                // pointer value, won't take it
                continue;
            }
            Ptr<const CallbackValue> callback = DynamicCast<const CallbackValue>(info.initialValue);
            if (callback)
            {
                // callback value, won't take it
                continue;
            }
            // We take only values, no pointers or vectors or callbacks
            if (!calledStart)
            {
                StartVisitTypeId(tid.GetName());
            }
            VisitAttribute(tid, info.name, info.initialValue->SerializeToString(info.checker), j);
            calledStart = true;
        }
        if (calledStart)
        {
            EndVisitTypeId();
        }
    }
}

void
AttributeDefaultIterator::StartVisitTypeId(std::string name)
{
}

void
AttributeDefaultIterator::EndVisitTypeId()
{
}

void
AttributeDefaultIterator::DoVisitAttribute(std::string name, std::string defaultValue)
{
}

void
AttributeDefaultIterator::VisitAttribute(TypeId tid,
                                         std::string name,
                                         std::string defaultValue,
                                         uint32_t index)
{
    DoVisitAttribute(name, defaultValue);
}

} // namespace ns3
