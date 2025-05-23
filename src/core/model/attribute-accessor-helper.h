/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef ATTRIBUTE_ACCESSOR_HELPER_H
#define ATTRIBUTE_ACCESSOR_HELPER_H

#include "attribute.h"

/**
 * @file
 * @ingroup attributeimpl
 * ns3::MakeAccessorHelper declarations and template implementations.
 */

namespace ns3
{

/**
 * @ingroup attributeimpl
 *
 * Create an AttributeAccessor for a class data member,
 * or a lone class get functor or set method.
 *
 * The get functor method should have a signature like
 * @code
 *   typedef U (T::*getter)() const
 * @endcode
 * where \pname{T} is the class and \pname{U} is the type of
 * the return value.
 *
 * The set method should have one of these signatures:
 * @code
 *   typedef void (T::*setter)(U)
 *   typedef bool (T::*setter)(U)
 * @endcode
 * where \pname{T} is the class and \pname{U} is the type of the value to set
 * the attribute to, which should be compatible with the
 * specific AttributeValue type \pname{V} which holds the value
 * (or the type implied by the name \c Make<V>Accessor of this function.)
 * In the case of a \pname{setter} returning \pname{bool,} the return value
 * should be \c true if the value could be set successfully.
 *
 * @tparam V  \explicit (If present) The specific AttributeValue type
 *            to use to represent the Attribute.  (If not present,
 *            the type \pname{V} is implicit in the name of this function,
 *            as "Make<V>Accessor"
 * @tparam T1 \deduced The type of the class data member,
 *            or the type of the class get functor or set method.
 * @param [in] a1 The address of the data member,
 *            or the get or set method.
 * @returns   The AttributeAccessor
 */
template <typename V, typename T1>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1);

/**
 * @ingroup attributeimpl
 *
 * Create an AttributeAccessor using a pair of get functor
 * and set methods from a class.
 *
 * The get functor method should have a signature like
 * @code
 *   typedef U (T::*getter)() const
 * @endcode
 * where \pname{T} is the class and \pname{U} is the type of
 * the return value.
 *
 * The set method should have one of these signatures:
 * @code
 *   typedef void (T::*setter)(U)
 *   typedef bool (T::*setter)(U)
 * @endcode
 * where \pname{T} is the class and \pname{U} is the type of the value to set
 * the attribute to, which should be compatible with the
 * specific AttributeValue type \pname{V} which holds the value
 * (or the type implied by the name \c Make<V>Accessor of this function.)
 * In the case of a \pname{setter} returning \pname{bool,} the return value
 * should be true if the value could be set successfully.
 *
 * In practice the setter and getter arguments can appear in either order,
 * but setter first is preferred.
 *
 * @tparam V  \explicit (If present) The specific AttributeValue type to use to represent
 *            the Attribute.  (If not present, the type \pname{V} is implicit
 *            in the name of this function as "Make<V>Accessor"
 * @tparam T1 \deduced The type of the class data member,
 *            or the type of the class get functor or set method.
 *
 * @tparam T2 \deduced The type of the getter class functor method.
 * @param [in] a2 The address of the class method to set the attribute.
 * @param [in] a1 The address of the data member,
 *            or the get or set method.
 * @returns   The AttributeAccessor
 */
template <typename V, typename T1, typename T2>
inline Ptr<const AttributeAccessor> MakeAccessorHelper(T1 a1, T2 a2);

} // namespace ns3

/***************************************************************
 *  Implementation of the templates declared above.
 ***************************************************************/

#include <type_traits>

namespace ns3
{

/**
 * @ingroup attributeimpl
 *
 * The non-const and non-reference type equivalent to \pname{T}.
 *
 * @tparam T \explicit The original (possibly qualified) type.
 */
template <typename T>
struct AccessorTrait
{
    /** The non-const, non reference type. */
    using Result = std::remove_cvref_t<T>;
};

/**
 * @ingroup attributeimpl
 *
 * Basic functionality for accessing class attributes via
 * class data members, or get functor/set methods.
 *
 * @tparam T \explicit Class of object holding the attribute.
 * @tparam U \explicit AttributeValue type for the underlying class member
 *           which is an attribute.
 */
template <typename T, typename U>
class AccessorHelper : public AttributeAccessor
{
  public:
    /** Constructor */
    AccessorHelper()
    {
    }

    /**
     * Set the underlying member to the argument AttributeValue.
     *
     * Handle dynamic casting from generic ObjectBase and AttributeValue
     * up to desired object class and specific AttributeValue.
     *
     * Forwards to DoSet method.
     *
     * @param [in] object Generic object pointer, to upcast to \pname{T}.
     * @param [in] val Generic AttributeValue, to upcast to \pname{U}.
     * @returns true if the member was set successfully.
     */
    bool Set(ObjectBase* object, const AttributeValue& val) const override
    {
        const U* value = dynamic_cast<const U*>(&val);
        if (value == nullptr)
        {
            return false;
        }
        T* obj = dynamic_cast<T*>(object);
        if (obj == nullptr)
        {
            return false;
        }
        return DoSet(obj, value);
    }

    /**
     * Get the value of the underlying member into the AttributeValue.
     *
     * Handle dynamic casting from generic ObjectBase and AttributeValue
     * up to desired object class and specific AttributeValue.
     *
     * Forwards to DoGet method.
     *
     * @param [out] object Generic object pointer, to upcast to \pname{T}.
     * @param [out] val Generic AttributeValue, to upcast to \pname{U}.
     * @returns true if the member value could be retrieved successfully
     */
    bool Get(const ObjectBase* object, AttributeValue& val) const override
    {
        U* value = dynamic_cast<U*>(&val);
        if (value == nullptr)
        {
            return false;
        }
        const T* obj = dynamic_cast<const T*>(object);
        if (obj == nullptr)
        {
            return false;
        }
        return DoGet(obj, value);
    }

  private:
    /**
     * Setter implementation.
     *
     * @see Set()
     * @param [in] object The parent object holding the attribute.
     * @param [in] v The specific AttributeValue to set.
     * @returns true if the member was set successfully.
     */
    virtual bool DoSet(T* object, const U* v) const = 0;
    /**
     * Getter implementation.
     *
     * @see Get()
     * @param [out] object The parent object holding the attribute.
     * @param [out] v The specific AttributeValue to set.
     * @returns true if the member value could be retrieved successfully
     */
    virtual bool DoGet(const T* object, U* v) const = 0;

    // end of class AccessorHelper
};

/**
 * @ingroup attributeimpl
 *
 * MakeAccessorHelper implementation for a class data member.
 *
 * @tparam V  \explicit The specific AttributeValue type to use to represent
 *            the Attribute.
 * @tparam T  \deduced The class holding the data member.
 * @tparam U  \deduced The type of the data member.
 * @param [in]  memberVariable  The address of the data member.
 * @returns The AttributeAccessor.
 */
// clang-format off
// Clang-format guard needed for versions <= 18
template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperOne(U T::* memberVariable)
// clang-format on
{
    /* AttributeAccessor implementation for a class member variable. */
    class MemberVariable : public AccessorHelper<T, V>
    {
      public:
        /*
         * Construct from a class data member address.
         * @param [in] memberVariable The class data member address.
         */
        // clang-format off
        // Clang-format guard needed for versions <= 18
        MemberVariable(U T::* memberVariable)
            // clang-format on
            : AccessorHelper<T, V>(),
              m_memberVariable(memberVariable)
        {
        }

      private:
        bool DoSet(T* object, const V* v) const override
        {
            typename AccessorTrait<U>::Result tmp;
            bool ok = v->GetAccessor(tmp);
            if (!ok)
            {
                return false;
            }
            (object->*m_memberVariable) = tmp;
            return true;
        }

        bool DoGet(const T* object, V* v) const override
        {
            v->Set(object->*m_memberVariable);
            return true;
        }

        bool HasGetter() const override
        {
            return true;
        }

        bool HasSetter() const override
        {
            return true;
        }

        // clang-format off
        // Clang-format guard needed for versions <= 18
        U T::* m_memberVariable; // Address of the class data member.
        // clang-format on
    };

    return Ptr<const AttributeAccessor>(new MemberVariable(memberVariable), false);
}

/**
 * @ingroup attributeimpl
 *
 * MakeAccessorHelper implementation for a class get functor method.
 *
 * @tparam V  \explicit The specific AttributeValue type to use to represent
 *            the Attribute.
 * @tparam T  \deduced The class holding the get functor method.
 * @tparam U  \deduced The return type of the get functor method.
 * @param [in] getter  The address of the class get functor method.
 * @returns The AttributeAccessor.
 */
template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperOne(U (T::*getter)() const)
{
    /* AttributeAccessor implementation with a class get functor method. */
    class MemberMethod : public AccessorHelper<T, V>
    {
      public:
        /*
         * Construct from a class get functor method.
         * @param [in] getter The class get functor method pointer.
         */
        MemberMethod(U (T::*getter)() const)
            : AccessorHelper<T, V>(),
              m_getter(getter)
        {
        }

      private:
        bool DoSet(T* /* object */, const V* /* v */) const override
        {
            return false;
        }

        bool DoGet(const T* object, V* v) const override
        {
            v->Set((object->*m_getter)());
            return true;
        }

        bool HasGetter() const override
        {
            return true;
        }

        bool HasSetter() const override
        {
            return false;
        }

        U (T::*m_getter)() const; // The class get functor method pointer.
    };

    return Ptr<const AttributeAccessor>(new MemberMethod(getter), false);
}

/**
 * @ingroup attributeimpl
 *
 * MakeAccessorHelper implementation for a class set method
 * returning void.
 *
 * @tparam V  \explicit The specific AttributeValue type to use to represent
 *            the Attribute.
 * @tparam T  \deduced The class holding the set method.
 * @tparam U  \deduced The argument type of the set method.
 * @param [in] setter  The address of the class set method, returning void.
 * @returns The AttributeAccessor.
 */
template <typename V, typename T, typename U>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperOne(void (T::*setter)(U))
{
    /* AttributeAccessor implementation with a class set method returning void. */
    class MemberMethod : public AccessorHelper<T, V>
    {
      public:
        /*
         * Construct from a class set method.
         * @param [in] setter The class set method pointer.
         */
        MemberMethod(void (T::*setter)(U))
            : AccessorHelper<T, V>(),
              m_setter(setter)
        {
        }

      private:
        bool DoSet(T* object, const V* v) const override
        {
            typename AccessorTrait<U>::Result tmp;
            bool ok = v->GetAccessor(tmp);
            if (!ok)
            {
                return false;
            }
            (object->*m_setter)(tmp);
            return true;
        }

        bool DoGet(const T* /* object */, V* /* v */) const override
        {
            return false;
        }

        bool HasGetter() const override
        {
            return false;
        }

        bool HasSetter() const override
        {
            return true;
        }

        void (T::*m_setter)(U); // The class set method pointer, returning void.
    };

    return Ptr<const AttributeAccessor>(new MemberMethod(setter), false);
}

/**
 * @ingroup attributeimpl
 *
 * MakeAccessorHelper implementation with a class get functor method
 * and a class set method returning \pname{void}.
 *
 * The two versions of this function differ only in argument order.
 *
 * @tparam W  \explicit The specific AttributeValue type to use to represent
 *            the Attribute.
 * @tparam T  \deduced The class holding the functor methods.
 * @tparam U  \deduced The argument type of the set method.
 * @tparam V  \deduced The return type of the get functor method.
 * @param [in] setter The address of the class set method, returning void.
 * @param [in] getter The address of the class get functor method.
 * @returns The AttributeAccessor.
 */
template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(void (T::*setter)(U), V (T::*getter)() const)
{
    /*
     * AttributeAccessor implementation with class get functor and set method,
     * returning void.
     */
    class MemberMethod : public AccessorHelper<T, W>
    {
      public:
        /*
         * Construct from class get functor and set methods.
         * @param [in] setter The class set method pointer, returning void.
         * @param [in] getter The class get functor method pointer.
         */
        MemberMethod(void (T::*setter)(U), V (T::*getter)() const)
            : AccessorHelper<T, W>(),
              m_setter(setter),
              m_getter(getter)
        {
        }

      private:
        bool DoSet(T* object, const W* v) const override
        {
            typename AccessorTrait<U>::Result tmp;
            bool ok = v->GetAccessor(tmp);
            if (!ok)
            {
                return false;
            }
            (object->*m_setter)(tmp);
            return true;
        }

        bool DoGet(const T* object, W* v) const override
        {
            v->Set((object->*m_getter)());
            return true;
        }

        bool HasGetter() const override
        {
            return true;
        }

        bool HasSetter() const override
        {
            return true;
        }

        void (T::*m_setter)(U);   // The class set method pointer, returning void.
        V (T::*m_getter)() const; // The class get functor method pointer.
    };

    return Ptr<const AttributeAccessor>(new MemberMethod(setter, getter), false);
}

/**
 * @ingroup attributeimpl
 * @copydoc DoMakeAccessorHelperTwo(void(T::*)(U),V(T::*)()const)
 */
template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(V (T::*getter)() const, void (T::*setter)(U))
{
    return DoMakeAccessorHelperTwo<W>(setter, getter);
}

/**
 * @ingroup attributeimpl
 *
 * MakeAccessorHelper implementation with a class get functor method
 * and a class set method returning \pname{bool}.
 *
 * The two versions of this function differ only in argument order.
 *
 * @tparam W  \explicit The specific AttributeValue type to use to represent
 *            the Attribute.
 * @tparam T  \deduced The class holding the functor methods.
 * @tparam U  \deduced The argument type of the set method.
 * @tparam V  \deduced The return type of the get functor method.
 * @param [in] setter The address of the class set method, returning bool.
 * @param [in] getter The address of the class get functor method.
 * @returns The AttributeAccessor.
 */
template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(bool (T::*setter)(U), V (T::*getter)() const)
{
    /*
     * AttributeAccessor implementation with class get functor and
     * set method, returning bool.
     */
    class MemberMethod : public AccessorHelper<T, W>
    {
      public:
        /*
         * Construct from class get functor and set method, returning bool.
         * @param [in] setter The class set method pointer, returning bool.
         * @param [in] getter The class get functor method pointer.
         */
        MemberMethod(bool (T::*setter)(U), V (T::*getter)() const)
            : AccessorHelper<T, W>(),
              m_setter(setter),
              m_getter(getter)
        {
        }

      private:
        bool DoSet(T* object, const W* v) const override
        {
            typename AccessorTrait<U>::Result tmp;
            bool ok = v->GetAccessor(tmp);
            if (!ok)
            {
                return false;
            }
            ok = (object->*m_setter)(tmp);
            return ok;
        }

        bool DoGet(const T* object, W* v) const override
        {
            v->Set((object->*m_getter)());
            return true;
        }

        bool HasGetter() const override
        {
            return true;
        }

        bool HasSetter() const override
        {
            return true;
        }

        bool (T::*m_setter)(U);   // The class set method pointer, returning bool.
        V (T::*m_getter)() const; // The class get functor method pointer.
    };

    return Ptr<const AttributeAccessor>(new MemberMethod(setter, getter), false);
}

/**
 * @ingroup attributeimpl
 * @copydoc ns3::DoMakeAccessorHelperTwo(bool(T::*)(U),V(T::*)()const)
 */
template <typename W, typename T, typename U, typename V>
inline Ptr<const AttributeAccessor>
DoMakeAccessorHelperTwo(V (T::*getter)() const, bool (T::*setter)(U))
{
    return DoMakeAccessorHelperTwo<W>(setter, getter);
}

template <typename V, typename T1>
inline Ptr<const AttributeAccessor>
MakeAccessorHelper(T1 a1)
{
    return DoMakeAccessorHelperOne<V>(a1);
}

template <typename V, typename T1, typename T2>
inline Ptr<const AttributeAccessor>
MakeAccessorHelper(T1 a1, T2 a2)
{
    return DoMakeAccessorHelperTwo<V>(a1, a2);
}

} // namespace ns3

#endif /* ATTRIBUTE_ACCESSOR_HELPER_H */
