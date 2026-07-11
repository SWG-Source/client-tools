// ======================================================================
//
// VoidMemberFunction.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef VOID_MEMBER_FUNCTION_H
#define VOID_MEMBER_FUNCTION_H

// ======================================================================

#include <functional>

// ======================================================================
/**
 * Replace the broken std::mem_fun under MSVC when return type is void.
 *
 * Note: this can disappear as soon as we have class partial specialization
 *       support in all our environments.
 *
 * This should only be used when the client wants to use std::mem_fun
 * where the return value of the member function is void.  Use just lik
 * std::mem_fun().
 */

template <class Class>
class VoidConstMemberFunctionNoArg
{
private:
    using Function = void (Class::*)() const;

public:
    explicit VoidConstMemberFunctionNoArg(Function function) : m_function(function) {}

    void operator()(const Class* classArgument) const
    {
        (classArgument->*m_function)();
    }

private:
    Function m_function;
};

// ----------------------------------------------------------------------

template <class Class>
class VoidMemberFunctionNoArg
{
private:

  typedef void (Class:: *Function)(void);

public:

  explicit VoidMemberFunctionNoArg(Function function) : m_function(function) {}
  void operator ()(Class* classArgument) const { (classArgument->*m_function)(); }

private:

  Function m_function;
};

// ----------------------------------------------------------------------

template <class Class, class ArgumentType>
class VoidConstMemberFunctionOneArg
{
private:
    using Function = void (Class::*)(ArgumentType) const;

public:
    using first_argument_type = Class*;
    using second_argument_type = ArgumentType;
    using result_type = void;

    explicit VoidConstMemberFunctionOneArg(Function function)
        : m_function(function) {
    }

    void operator()(const Class* classArgument, ArgumentType argument) const
    {
        (classArgument->*m_function)(argument);
    }

private:
    Function m_function;
};

// ----------------------------------------------------------------------

template <class Class, class ArgumentType>
class VoidMemberFunctionOneArg
{
private:
    using Function = void (Class::*)(ArgumentType);

public:
    using first_argument_type = Class*;
    using second_argument_type = ArgumentType;

    using result_type = void;

    explicit VoidMemberFunctionOneArg(Function function)
        : m_function(function) {
    }

    void operator()(Class* classArgument, ArgumentType argument) const
    {
        (classArgument->*m_function)(argument);
    }

private:
    Function m_function;
};


// ======================================================================

template <class Class>
inline VoidConstMemberFunctionNoArg<Class> VoidMemberFunction(void (Class::*function)() const)
{ 
	return VoidConstMemberFunctionNoArg<Class>(function); 
}

// ----------------------------------------------------------------------

template <class Class>
inline VoidMemberFunctionNoArg<Class> VoidMemberFunction(void (Class::*function)())
{ 
	return VoidMemberFunctionNoArg<Class>(function); 
}

// ----------------------------------------------------------------------

template <class Class, typename ArgumentType>
inline VoidConstMemberFunctionOneArg<Class, ArgumentType> VoidMemberFunction(void (Class::*function)(ArgumentType) const)
{ 
	return VoidConstMemberFunctionOneArg<Class, ArgumentType>(function); 
}

// ----------------------------------------------------------------------

template <class Class, typename ArgumentType>
inline VoidMemberFunctionOneArg<Class, ArgumentType> VoidMemberFunction(void (Class::*function)(ArgumentType))
{ 
	return VoidMemberFunctionOneArg<Class, ArgumentType>(function); 
}

// ======================================================================

#endif
