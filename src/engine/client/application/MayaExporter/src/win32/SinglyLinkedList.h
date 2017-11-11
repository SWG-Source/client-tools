// ======================================================================
//
// SinglyLinkedList.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef SINGLY_LINKED_LIST_H
#define SINGLY_LINKED_LIST_H

// ======================================================================

template<class ElementType>
class SinglyLinkedList
{
	//lint -esym(1727, SinglyLinkedList*) // inline function X not previously defined inline

protected:

	struct Node
	{
		ElementType  element;
		Node        *next;

	private:
	
		// disabled
		Node(void);
		Node(const Node&);
		Node &operator =(const Node&);

	public:

		explicit Node(const ElementType &valueToCopy) : element(valueToCopy), next(0) {}
		~Node(void) { next = 0; }  //lint !e1540 // element neither freed nor zero'ed by destructor // okay, not always a pointer, semantics are that only the contained object is destroyed

	};

public:

	// ----------------------------------------------------------------------

	class Iterator
	{
		friend class SinglyLinkedList<ElementType>;

	protected:

		Node *node;

	private:

		// disabled
		Iterator(void);

	protected:

		explicit Iterator(Node *newNode):	node(newNode) {}

	public:

		~Iterator(void) { node = 0; }

		// iterator motion
		Iterator &operator ++(void) { DEBUG_FATAL(node == 0, ("attempted ++iterator at end of list")); node = node->next; return *this; }

		// iterator comparison
		bool operator ==(const Iterator &rhs) const { return (rhs.node == node); }
		bool operator !=(const Iterator &rhs) const { return !(rhs == *this); }

		// element access
		ElementType &operator *(void) {	DEBUG_FATAL(node == 0, ("attempted to access element at end-of-list iterator")); return node->element; }

	};

	// ----------------------------------------------------------------------

	class ConstIterator
	{
		friend class SinglyLinkedList<ElementType>;

	protected:

		const Node *node;

	private:

		// disabled
		ConstIterator(void);

	protected:

		explicit ConstIterator(const Node *newNode) :	node(newNode) {}

	public:

		~ConstIterator(void) { node = 0; }

		// iterator motion
		ConstIterator &operator ++(void) { DEBUG_FATAL(node == 0, ("attempted ++iterator at end of list")); node = node->next; return *this; }

		// iterator comparison
		bool operator ==(const ConstIterator &rhs) const { return (rhs.node == node); }
		bool operator !=(const ConstIterator &rhs) const { return !(rhs == *this); }

		// element access
		const ElementType &operator *(void) const {	DEBUG_FATAL(node == 0, ("attempted to access element at end-of-list iterator")); return node->element; }

	};

	// ----------------------------------------------------------------------

	friend class Iterator;
	friend class ConstIterator;

protected:

	Node *first;
	Node *last;
	int   elementCount;

private:

	// disabled
	SinglyLinkedList(const SinglyLinkedList&);
	SinglyLinkedList &operator =(const SinglyLinkedList&);

public:

	SinglyLinkedList(void);
	~SinglyLinkedList(void);

	void pushBack(ElementType elementToCopy = ElementType());
	void pushFront(ElementType elementToCopy = ElementType());

	ConstIterator  begin(void) const;
	Iterator       begin(void);

	ConstIterator  end(void) const;
	Iterator       end(void);

	int            size(void) const;
};

// ======================================================================

template<class ElementType>
inline SinglyLinkedList<ElementType>::SinglyLinkedList(void)
:	first(0),
	last(0),
	elementCount(0)
{
}

// ----------------------------------------------------------------------

template<class ElementType>
inline SinglyLinkedList<ElementType>::~SinglyLinkedList(void)
{
	while (first)
	{
		Node *const deadNode = first;
		first                = first->next;
		delete deadNode;
	}
	last = 0;
}

// ----------------------------------------------------------------------

template<class ElementType>
inline void SinglyLinkedList<ElementType>::pushBack(ElementType elementToCopy)
{
	Node *newNode = new Node(elementToCopy);

	if (last)
		last->next = newNode;
	else
		first = newNode;
	last = newNode;

	++elementCount;
}

// ----------------------------------------------------------------------

template<class ElementType>
inline void SinglyLinkedList<ElementType>::pushFront(ElementType elementToCopy)
{
	Node *newNode = new Node(elementToCopy);
	newNode->next = first;
	first         = newNode;
	if (!last)
		last = newNode;

	++elementCount;
}

// ----------------------------------------------------------------------

template<class ElementType>
inline SinglyLinkedList<ElementType>::ConstIterator SinglyLinkedList<ElementType>::begin(void) const
{
	return ConstIterator(first);
}

// ----------------------------------------------------------------------

template<class ElementType>
inline SinglyLinkedList<ElementType>::Iterator SinglyLinkedList<ElementType>::begin(void)
{
	return Iterator(first);
}

// ----------------------------------------------------------------------

template<class ElementType>
inline SinglyLinkedList<ElementType>::ConstIterator SinglyLinkedList<ElementType>::end(void) const
{
	return ConstIterator(0);
}

// ----------------------------------------------------------------------

template<class ElementType>
inline SinglyLinkedList<ElementType>::Iterator SinglyLinkedList<ElementType>::end(void)
{
	return Iterator(0);
}

// ----------------------------------------------------------------------

template<class ElementType>
inline int SinglyLinkedList<ElementType>::size(void) const
{
	return elementCount;
}

// ======================================================================

#endif
