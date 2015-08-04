/*
*   SmartPointer declaration and implementation
*
*   Author: Richard Jayne
*
*   (C) 2008 Sony Online Entertainment, LLC
*/

#ifndef SMART_SMARTPTR_H
#define SMART_SMARTPTR_H

namespace Smart
{

template<typename T>
class DeleteFunc
{
public:
    void operator()(T* p)
    {
        delete p;
    }
};

template<typename T>
class ReleaseFunc
{
public:
    void operator()(T* p)
    {
        p->Release();
    }
};

template<typename T, typename ReleaseFunc = DeleteFunc<T> >
class SmartPtrT
{
public:
    SmartPtrT(): m_p(0)
    {
    }
    SmartPtrT(T* p): m_p(p)
    {
    }
    ~SmartPtrT()
    {
        if(m_p)
        {
            m_releaseFunc(m_p);
        }
    }
    operator bool() const
    {
        return (0 != m_p);
    }
    operator T*() const
    {
        return m_p;
    }
    T* operator ->() const
    {
        return m_p;
    }
    T* Ptr() const
    {
        return m_p;
    }
    const T* CPtr() const
    {
        return m_p;
    }
    void Attach(T* p)
    {
        Release();
        m_p = p;
    }
    T* Detach()
    {
        T* tmp = m_p;
        m_p = 0;
        return tmp;
    }
    void Swap(SmartPtrT& rhs)
    {
        T* tmp = m_p;
        m_p = rhs.m_p;
        rhs.m_p = tmp;
    }
    void Release()
    {
        if(m_p)
        {
            m_releaseFunc(m_p);
            m_p = 0;
        }
    }
private:
    SmartPtrT(const SmartPtrT&);
    SmartPtrT& operator =(const SmartPtrT&);
    T* m_p;
    ReleaseFunc m_releaseFunc;
};

} // Smart

#endif // SMART_SMARTPTR_H
