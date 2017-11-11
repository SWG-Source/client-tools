// UISmartPointer.h

#ifndef INCLUDED_UISmartPointer_H
#define INCLUDED_UISmartPointer_H

template <typename T>
class UISmartPointer
{
public:

	//----------------------------------------------------------------------
	
	explicit UISmartPointer(T * const object = NULL) : m_object(object)
	{
		T::Lock(m_object);
	}
	
	//----------------------------------------------------------------------
	
	UISmartPointer(UISmartPointer<T> const & rhs) : m_object(rhs.m_object)
	{
		T::Lock(m_object);
	}
	
	//----------------------------------------------------------------------
	
	~UISmartPointer()
	{
		T::Unlock(m_object);

		m_object = 0;
	}

	//----------------------------------------------------------------------
	
	UISmartPointer<T> & operator = (T * const object)
	{
		if (m_object != object) 
		{
			T::Lock(object);
			T::Unlock(m_object);
			m_object = object;
		}

		return *this;
	}

	//----------------------------------------------------------------------
	
	UISmartPointer<T> & operator = (UISmartPointer<T> const & rhs)
	{
		if (m_object != rhs.m_object)
		{
			T::Lock(rhs.m_object);
			T::Unlock(m_object);
			m_object = rhs.m_object;
		}

		return *this;
	}

	//----------------------------------------------------------------------
	
	bool operator == ( T const * const object ) const
	{
		return m_object == object;
	}
	
	//----------------------------------------------------------------------
	
	bool operator <  ( T const * const object ) const
	{
		return m_object < object;
	}
	
	//----------------------------------------------------------------------
	
	bool operator == ( UISmartPointer<T> const & rhs ) const
	{
		return m_object == rhs.m_object;
	}
	
	//----------------------------------------------------------------------
	
	bool operator <  ( UISmartPointer<T> const & rhs ) const
	{
		return m_object < rhs.m_object;
	}
	
	//----------------------------------------------------------------------
	
	T * pointer()
	{
		return m_object;
	}

	//----------------------------------------------------------------------
	
	operator T*()
	{
		return m_object;
	}
	
	//----------------------------------------------------------------------
	
	T * operator ->()
	{
		return m_object;
	}
	
	//----------------------------------------------------------------------
	
	T const * pointer() const
	{
		return m_object;
	}
	
	//----------------------------------------------------------------------
	
	operator T const *() const
	{
		return m_object;
	}
	
	//----------------------------------------------------------------------
	
	T const * operator ->() const
	{
		return m_object;
	}

	//----------------------------------------------------------------------
	
	operator bool() const
	{
		return m_object != NULL;
	}
	

private:
	T * m_object;
};

#endif
