// ======================================================================
//
// UiMemoryBlockManager.h
//
// ======================================================================

#ifndef INCLUDED_UiMemoryBlockManager_H
#define INCLUDED_UiMemoryBlockManager_H

// ======================================================================

// Efficiently handle memory mangagement for rapidly allocated and deleted objects.

#include <unordered_map>
#include <unordered_set>

class UiMemoryBlockManager
{
public:

	UiMemoryBlockManager(char const * name, size_t granularity /*bytes*/);
	~UiMemoryBlockManager();

	char const * getName() const;

	void *allocMem(size_t size, bool returnNullOnFailure=false);
	void  freeMem(void *pointer);

#ifdef _DEBUG
	void  debugDump() const;
#endif

private:

	// disable these routines
	UiMemoryBlockManager();
	UiMemoryBlockManager(const UiMemoryBlockManager &);
	UiMemoryBlockManager & operator =(const UiMemoryBlockManager &);

	void addPool();
	void * poolAllocate(size_t const size);

private:
	// name of this UiMemoryBlockManager
	char * m_name;

	template<class T>
	struct HashPointer
	{
		size_t operator()(const T & source) const
		{
			return (size_t)source >> 4;
		};
	};

#ifdef _DEBUG
	typedef std::unordered_set<void * /*data*/, HashPointer<void *> > MemorySet;
	int m_bytesAllocated;
	MemorySet * m_allAllocations;
#endif

	// convert to hash multi-map.
	typedef std::unordered_multimap<int /*elementSize*/, void * /*data*/> MemoryMap;
	MemoryMap * m_memoryMap;

	// Memory pool.
	class MemoryPool
	{
	public:
		typedef std::vector<MemoryPool*> Array;

		MemoryPool(size_t const size);
		~MemoryPool();

		size_t remainingBytes() const;
		
		void * allocate(size_t const size);

	private:
		MemoryPool();
		MemoryPool(MemoryPool const &);
		MemoryPool & operator=(MemoryPool const &);

		size_t m_size;
		void * m_memoryBase;
		void * m_memoryCurrent;
	};

	MemoryPool::Array * m_memoryPool;
	size_t m_poolGranularity;
};

// ----------------------------------------------------------------------

inline const char *UiMemoryBlockManager::getName() const
{
	return m_name;
}

// ======================================================================

#endif
