// ======================================================================
//
// CuiMediatorFactory_Constructor.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CuiMediatorFactory_Constructor_H
#define INCLUDED_CuiMediatorFactory_Constructor_H

#include "clientUserInterface/CuiMediatorFactory_ConstructorBase.h"
#include "UIPage.h"

// ======================================================================

template <typename T> class CuiMediatorFactory::Constructor :
public CuiMediatorFactory::ConstructorBase
{
public:
	explicit Constructor (const std::string & path, bool duplicateOnly = false) :
	ConstructorBase     (duplicateOnly),
	m_path              (path),
	m_mediator          (0)
	{}

	CuiMediator * get (bool create = true) 
	{
		DEBUG_FATAL (m_duplicateOnly, ("CuiMediatorFactory::Constructor [%s] is flagged duplicate only", m_path.c_str ()));

		//@ todo make this load the UIPage
		if (!m_mediator && create)
		{
			UIPage * const root = NON_NULL (UIManager::gUIManager ().GetRootPage ());
			UIPage * const page = GET_UI_OBJ ((*root), UIPage, m_path.c_str ());

			FATAL(NULL == page, ("CuiMediatorFactory_Constructor null page [%s]", m_path.c_str()));

			m_mediator = this->create (*page);
			m_mediator->fetch ();
		}

		return m_mediator;
	}
	
	CuiMediator * create (UIPage & page) const
	{
		return new T(page);
	}

	CuiMediator * createInto (UIPage & page) const;

	~Constructor ()
	{
		detachMediator ();
	}

	void detachMediator ()
	{
		if (m_mediator)
		{
			DEBUG_FATAL (m_duplicateOnly, ("CuiMediatorFactory::Constructor [%s] is flagged duplicate only", m_path.c_str ()));
			m_mediator->release ();
		}
		m_mediator = 0;
	}

	const type_info & getTypeId () const;

private:
	Constructor & operator= (const Constructor & rhs);
	Constructor (const Constructor & rhs);
	Constructor ();

	std::string   m_path;
	CuiMediator * m_mediator;
};

//----------------------------------------------------------------------

template <typename T>
inline const type_info & CuiMediatorFactory::Constructor<T>::getTypeId () const
{ 
	return typeid (T);
}

//----------------------------------------------------------------------

template <typename T>
inline CuiMediator * CuiMediatorFactory::Constructor<T>::createInto (UIPage & page) const
{
	UIPage * const dupe = NON_NULL (UIPage::DuplicateInto (page, m_path.c_str ()));
	return new T (*dupe);
}

// ======================================================================

#endif
