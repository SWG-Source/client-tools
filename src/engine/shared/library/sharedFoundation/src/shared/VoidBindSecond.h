// ======================================================================
//
// VoidBindSecond.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include <functional>

// ======================================================================

template <class Operation>
class VoidBinderSecond
{
private:
	using FirstArgType = typename Operation::first_argument_type;
	using SecondArgType = typename Operation::second_argument_type;

	SecondArgType  m_boundValue;
	Operation      m_operation;

public:
	VoidBinderSecond(const Operation& operation, const SecondArgType& boundValue)
		: m_operation(operation), m_boundValue(boundValue)
	{
	}

	void operator()(const FirstArgType& argument) const
	{
		m_operation(argument, m_boundValue);
	}
};

// ======================================================================
/**
 * works just like std::bind2nd() when return value of function is void.
 *
 * this is necessary since our STL breaks under MSVC 6 when using void functions
 * with bind2nd.
 */


template <class Operation, typename BindArgumentType>
inline VoidBinderSecond<Operation> VoidBindSecond(const Operation& operation, const BindArgumentType &bindArgument) 
{
  typedef typename Operation::second_argument_type  SecondArgumentType;

  return VoidBinderSecond<Operation>(operation, SecondArgumentType(bindArgument));
}

// ======================================================================
