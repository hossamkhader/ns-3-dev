/*
 * Copyright (c) 2008 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include "ptr.h"
#include "simple-ref-count.h"

#include <stdint.h>
#include <string>

/**
 * @file
 * @ingroup attributes
 * ns3::AttributeValue, ns3::AttributeAccessor and
 * ns3::AttributeChecker declarations.
 */

namespace ns3
{

class AttributeAccessor;
class AttributeChecker;
class Attribute;
class ObjectBase;

/**
 *
 * @ingroup core
 * @defgroup attributes Attributes
 *
 * The \c ns-3 attribute system is the mechanism used in \c ns-3 to
 * organize, document, and modify the *values* used by the various
 * component models.
 *
 * Attributes also enable the tracing and statistics gathering
 * in the simulator.
 *
 * See \ref attributehelper for macros to ease the declaration
 * and definition of Attributes.
 */

/**
 *
 * @ingroup attributes
 *
 * @brief Hold a value for an Attribute.
 *
 * Instances of this class should always be wrapped into an Attribute object.
 * Most subclasses of this base class are implemented by the
 * ATTRIBUTE_HELPER_* macros.
 */
class AttributeValue : public SimpleRefCount<AttributeValue>
{
  public:
    AttributeValue();
    virtual ~AttributeValue();

    /**
     * @returns a deep copy of this class, wrapped into an Attribute object.
     */
    virtual Ptr<AttributeValue> Copy() const = 0;
    /**
     * @param [in] checker The checker associated to the attribute
     * @returns A string representation of this value.
     *
     * In most cases, this method will not make any use of the checker argument.
     * However, in a very limited set of cases, the checker argument is needed to
     * perform proper serialization. A nice example of code which needs it is
     * the EnumValue::SerializeToString code.
     */
    virtual std::string SerializeToString(Ptr<const AttributeChecker> checker) const = 0;
    /**
     * @param [in] value A string representation of the value
     * @param [in] checker A pointer to the checker associated to the attribute.
     * @returns true if the input string was correctly-formatted and could be
     *          successfully deserialized, false otherwise.
     *
     * Upon return of this function, this AttributeValue instance contains
     * the deserialized value.
     * In most cases, this method will not make any use of the checker argument.
     * However, in a very limited set of cases, the checker argument is needed to
     * perform proper serialization. A nice example of code which needs it is
     * the EnumValue::SerializeToString code.
     */
    virtual bool DeserializeFromString(std::string value, Ptr<const AttributeChecker> checker) = 0;
};

/**
 * @brief allow setting and getting the value of an attribute.
 *
 * @ingroup attributes
 *
 * The goal of this class is to hide from the user how an attribute
 * is actually set or get to or from a class instance. Implementations
 * of this base class are usually provided through the MakeAccessorHelper
 * template functions, hidden behind an ATTRIBUTE_HELPER_* macro.
 */
class AttributeAccessor : public SimpleRefCount<AttributeAccessor>
{
  public:
    AttributeAccessor();
    virtual ~AttributeAccessor();

    /**
     * @param [in,out] object The object instance to set the value in
     * @param [in] value The value to set
     * @returns true if the value could be set successfully, false otherwise.
     *
     * This method expects that the caller has checked that the input value is
     * valid with AttributeChecker::Check.
     */
    virtual bool Set(ObjectBase* object, const AttributeValue& value) const = 0;
    /**
     * @param [in,out] object The object instance to get the value from
     * @param [out] attribute A pointer to where the value should be set.
     * @returns true if the value could be read successfully, and
     *          stored in the input value, false otherwise.
     *
     * This method expects that the caller has checked that the input value is
     * valid with AttributeChecker::Check.
     */
    virtual bool Get(const ObjectBase* object, AttributeValue& attribute) const = 0;

    /**
     * @return true if this accessor supports the Get operation, false
     *         otherwise.
     */
    virtual bool HasGetter() const = 0;
    /**
     * @return true if this accessor supports the Set operation, false
     *         otherwise.
     */
    virtual bool HasSetter() const = 0;
};

/**
 * @brief Represent the type of an attribute
 *
 * @ingroup attributes
 *
 * Each type of attribute has an associated unique AttributeChecker
 * subclass. The type of the subclass can be safely used by users
 * to infer the type of the associated attribute. i.e., we expect
 * binding authors to use the checker associated to an attribute
 * to detect the type of the associated attribute.
 *
 * Most subclasses of this base class are implemented by the
 * ATTRIBUTE_HELPER_HEADER and ATTRIBUTE_HELPER_CPP macros.
 */
class AttributeChecker : public SimpleRefCount<AttributeChecker>
{
  public:
    AttributeChecker();
    virtual ~AttributeChecker();

    /**
     * Create a valid value from the argument value,
     * or reinterpret the argument as a string.
     *
     * @param [in] value The AttributeValue to check
     * @return Ptr to a valid value
     */
    Ptr<AttributeValue> CreateValidValue(const AttributeValue& value) const;
    /**
     * @param [in] value A pointer to the value to check
     * @returns true if the input value is both of the right type
     *          and if its value is within the requested range. Returns
     *          false otherwise.
     */
    virtual bool Check(const AttributeValue& value) const = 0;
    /**
     * @returns the c++ fully-qualified typename of the subclass
     *          of the ns3::AttributeValue base class which is associated
     *          to this checker.
     *
     * A typical return value here is FooValue where Foo is the name of the
     * type being wrapped.
     */
    virtual std::string GetValueTypeName() const = 0;
    /**
     * @returns true if this checker has information about the underlying
     *          C++ type, false otherwise.
     *
     * If this method returns false, the return value of the GetUnderlyingTypeInformation
     * method cannot be relied upon.
     */
    virtual bool HasUnderlyingTypeInformation() const = 0;
    /**
     * @returns a human-readable representation of information about
     *          the underlying C++ type.
     */
    virtual std::string GetUnderlyingTypeInformation() const = 0;
    /**
     * @returns a new instance of an AttributeValue (wrapper in an Attribute
     *          instance) which matches the type of the underlying attribute.
     *
     * This method is typically used to create a temporary variable prior
     * to calling Attribute::DeserializeFromString.
     */
    virtual Ptr<AttributeValue> Create() const = 0;
    /**
     * Copy the source to the destination

     * @param [in] source Source AttributeValue
     * @param [out] destination Destination AttributeValue
     * @return true if copy was successful
     */
    virtual bool Copy(const AttributeValue& source, AttributeValue& destination) const = 0;
};

/**
 * @ingroup attributes
 * @defgroup attribute_EmptyAttribute EmptyAttribute Attribute
 * AttributeValue implementation for EmptyAttribute
 */

/**
 * @brief A class for an empty attribute value.
 *
 * @ingroup attribute_EmptyAttribute
 *
 * @see AttributeValue
 */
class EmptyAttributeValue : public AttributeValue
{
  public:
    /** Default constructor. */
    EmptyAttributeValue();

  private:
    /**
     * @returns a deep copy of this class, wrapped into an Attribute object.
     */
    Ptr<AttributeValue> Copy() const override;
    /**
     * @param [in] checker The checker associated to the attribute
     * @returns a string representation of this value.
     *
     * In the EmptyAttributeValue case, the string returned will be simply ""
     */
    std::string SerializeToString(Ptr<const AttributeChecker> checker) const override;
    /**
     * @param [in] value A string representation of the value
     * @param [in] checker A pointer to the checker associated to the attribute.
     * @returns true if the input string was correctly-formatted and could be
     *          successfully deserialized, false otherwise.
     *
     * In the trivial case of EmptyAttributeValue, this should always return true
     */
    bool DeserializeFromString(std::string value, Ptr<const AttributeChecker> checker) override;
};

/**
 * @brief An accessor for EmptyAttributeValue
 *
 * @ingroup attribute_EmptyAttribute
 *
 * Does nothing, since every EmptyAttributeValue is the same.
 */
class EmptyAttributeAccessor : public AttributeAccessor
{
  public:
    EmptyAttributeAccessor();
    ~EmptyAttributeAccessor() override;
    bool Set(ObjectBase* object, const AttributeValue& value) const override;
    bool Get(const ObjectBase* object, AttributeValue& attribute) const override;
    bool HasGetter() const override;
    bool HasSetter() const override;
};

/**
 * @ingroup attribute_EmptyAttribute
 *
 * @brief Create an empty AttributeAccessor.
 *
 * @returns The empty AttributeAccessor (runtime exception if used)
 */
static inline Ptr<const AttributeAccessor>
MakeEmptyAttributeAccessor()
{
    return Ptr<const AttributeAccessor>(new EmptyAttributeAccessor(), false);
}

/**
 * @brief A checker for EmptyAttributeValue
 *
 * @ingroup attribute_EmptyAttribute
 *
 * Does nothing, since every EmptyAttributeValue does not contain anything and
 * is, of course, valid.
 */
class EmptyAttributeChecker : public AttributeChecker
{
  public:
    EmptyAttributeChecker();
    ~EmptyAttributeChecker() override;
    bool Check(const AttributeValue& value) const override;
    std::string GetValueTypeName() const override;
    bool HasUnderlyingTypeInformation() const override;
    std::string GetUnderlyingTypeInformation() const override;
    Ptr<AttributeValue> Create() const override;
    bool Copy(const AttributeValue& source, AttributeValue& destination) const override;
};

/**
 * @ingroup attribute_EmptyAttribute
 *
 * @brief Create an empty AttributeChecker.
 *
 * @returns The empty AttributeChecker (runtime exception if used)
 */
static inline Ptr<AttributeChecker>
MakeEmptyAttributeChecker()
{
    return Ptr<AttributeChecker>(new EmptyAttributeChecker(), false);
}

} // namespace ns3

#endif /* ATTRIBUTE_H */
