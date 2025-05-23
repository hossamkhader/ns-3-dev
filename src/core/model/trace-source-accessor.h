/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef TRACE_SOURCE_ACCESSOR_H
#define TRACE_SOURCE_ACCESSOR_H

#include "callback.h"
#include "ptr.h"
#include "simple-ref-count.h"

#include <stdint.h>

/**
 * @file
 * @ingroup tracing
 * ns3::TraceSourceAccessor and ns3::MakeTraceSourceAccessor declarations.
 */

namespace ns3
{

class ObjectBase;

/**
 * @ingroup tracing
 *
 * @brief Control access to objects' trace sources.
 *
 * This class abstracts the kind of trace source to which we want to connect
 * and provides services to Connect and Disconnect a sink to a trace source.
 */
class TraceSourceAccessor : public SimpleRefCount<TraceSourceAccessor>
{
  public:
    /** Constructor. */
    TraceSourceAccessor();
    /** Destructor. */
    virtual ~TraceSourceAccessor();

    /**
     * Connect a Callback to a TraceSource (without context.)
     *
     * @param [in] obj The object instance which contains the target trace source.
     * @param [in] cb The callback to connect to the target trace source.
     * @return \c true unless the connection could not be made, typically because
     *         the \c obj couldn't be cast to the correct type.
     */
    virtual bool ConnectWithoutContext(ObjectBase* obj, const CallbackBase& cb) const = 0;
    /**
     * Connect a Callback to a TraceSource with a context string.
     *
     * The context string will be provided as the first argument to the
     * Callback function.
     *
     * @param [in] obj The object instance which contains the target trace source.
     * @param [in] context The context to bind to the user callback.
     * @param [in] cb The callback to connect to the target trace source.
     * @return \c true unless the connection could not be made, typically because
     *         the \c obj couldn't be cast to the correct type.
     */
    virtual bool Connect(ObjectBase* obj, std::string context, const CallbackBase& cb) const = 0;
    /**
     * Disconnect a Callback from a TraceSource (without context).
     *
     * @param [in] obj The object instance which contains the target trace source.
     * @param [in] cb The callback to disconnect from the target trace source.
     * @return \c true unless the connection could not be made, typically because
     *         the \c obj couldn't be cast to the correct type.
     */
    virtual bool DisconnectWithoutContext(ObjectBase* obj, const CallbackBase& cb) const = 0;
    /**
     * Disconnect a Callback from a TraceSource with a context string.
     *
     * The context string will be provided as the first argument to the
     * Callback function.
     *
     * @param [in] obj the object instance which contains the target trace source.
     * @param [in] context the context which was bound to the user callback.
     * @param [in] cb the callback to disconnect from the target trace source.
     * @return \c true unless the connection could not be made, typically because
     *         the \c obj couldn't be cast to the correct type.
     */
    virtual bool Disconnect(ObjectBase* obj, std::string context, const CallbackBase& cb) const = 0;
};

/**
 * @ingroup tracing
 *
 * Create a TraceSourceAccessor which will control access to the underlying
 * trace source.
 *
 * This helper template method assumes that the underlying
 * type implements a statically-polymorphic set of Connect and Disconnect
 * methods and creates a dynamic-polymorphic class to wrap the underlying
 * static-polymorphic class.  This functionality is typically provided
 * by wrapping an object data member in a TracedCallback or TracedValue.
 *
 * @param [in] a The trace source
 * @returns The TraceSourceAccessor
 */
template <typename T>
Ptr<const TraceSourceAccessor> MakeTraceSourceAccessor(T a);

/**
 * @ingroup tracing
 *
 * Create an empty TraceSourceAccessor.
 *
 * @returns The empty TraceSourceAccessor (runtime exception if used)
 */
static inline Ptr<const TraceSourceAccessor>
MakeEmptyTraceSourceAccessor()
{
    return Ptr<const TraceSourceAccessor>(nullptr);
}

} // namespace ns3

/********************************************************************
 *  Implementation of the templates declared above.
 ********************************************************************/

namespace ns3
{

/**
 * @ingroup tracing
 * MakeTraceSourceAccessor() implementation.
 *
 * @tparam T \deduced Class type of the TracedCallback
 * @tparam SOURCE \deduced Type of the underlying value.
 * @param [in] a The underlying data value.
 * @returns The TraceSourceAccessor
 */
// clang-format off
// Clang-format guard needed for versions <= 18
template <typename T, typename SOURCE>
Ptr<const TraceSourceAccessor>
DoMakeTraceSourceAccessor(SOURCE T::* a)
// clang-format on
{
    struct Accessor : public TraceSourceAccessor
    {
        bool ConnectWithoutContext(ObjectBase* obj, const CallbackBase& cb) const override
        {
            T* p = dynamic_cast<T*>(obj);
            if (p == nullptr)
            {
                return false;
            }
            (p->*m_source).ConnectWithoutContext(cb);
            return true;
        }

        bool Connect(ObjectBase* obj, std::string context, const CallbackBase& cb) const override
        {
            T* p = dynamic_cast<T*>(obj);
            if (p == nullptr)
            {
                return false;
            }
            (p->*m_source).Connect(cb, context);
            return true;
        }

        bool DisconnectWithoutContext(ObjectBase* obj, const CallbackBase& cb) const override
        {
            T* p = dynamic_cast<T*>(obj);
            if (p == nullptr)
            {
                return false;
            }
            (p->*m_source).DisconnectWithoutContext(cb);
            return true;
        }

        bool Disconnect(ObjectBase* obj, std::string context, const CallbackBase& cb) const override
        {
            T* p = dynamic_cast<T*>(obj);
            if (p == nullptr)
            {
                return false;
            }
            (p->*m_source).Disconnect(cb, context);
            return true;
        }

        // clang-format off
        // Clang-format guard needed for versions <= 18
        SOURCE T::* m_source;
        // clang-format on
    }* accessor = new Accessor();

    accessor->m_source = a;
    return Ptr<const TraceSourceAccessor>(accessor, false);
}

template <typename T>
Ptr<const TraceSourceAccessor>
MakeTraceSourceAccessor(T a)
{
    return DoMakeTraceSourceAccessor(a);
}

} // namespace ns3

#endif /* TRACE_SOURCE_ACCESSOR_H */
