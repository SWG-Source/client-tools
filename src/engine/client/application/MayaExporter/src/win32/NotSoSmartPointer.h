// ======================================================================
//
// NotSoSmartPointer.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef NOT_SO_SMART_POINTER
#define NOT_SO_SMART_POINTER

// ======================================================================

template <class T>
class NotSoSmartPointer
{
// -TRF- this warning shuts off more than I want
//lint -esym(1930, NotSoSmartPointer*::operator* ) // conversion operator found

private:
	T *pointer;

private:

	NotSoSmartPointer(void);
	NotSoSmartPointer(const NotSoSmartPointer&);
	NotSoSmartPointer &operator =(const NotSoSmartPointer&);


public:

	explicit NotSoSmartPointer(T *newPointer): pointer(newPointer) {}
	~NotSoSmartPointer(void) { delete pointer; }

	// NOTE: we do not delete whatever was previously there
	T *operator =(T *newPointer) { return pointer = newPointer; }
	
	T *operator ->(void) const { return pointer; }
	T &operator  *(void) const { FATAL (!pointer, ("attempted to dereference null pointer\n")); return *pointer; }
	operator T* (void) const { return pointer; }
};

// ======================================================================

template <class T>
class NotSoSmartArrayPointer
{
// -TRF- this warning shuts off more than I want
//lint -esym(1930, NotSoSmartArrayPointer*::operator* ) // conversion operator found

private:
	T *pointer;

private:

	NotSoSmartArrayPointer(void);
	NotSoSmartArrayPointer(const NotSoSmartArrayPointer&);
	NotSoSmartArrayPointer &operator =(const NotSoSmartArrayPointer&);


public:

	explicit NotSoSmartArrayPointer(T *newPointer): pointer(newPointer) {}
	~NotSoSmartArrayPointer(void) { delete [] pointer; }

	// NOTE: we do not delete whatever was previously there
	T *operator =(T *newPointer) { return pointer = newPointer; }
	T &operator  *(void) const { FATAL (!pointer, ("attempted to dereference null pointer\n")); return *pointer; }
	operator T* (void) const { return pointer; }
};

// ======================================================================

#endif
