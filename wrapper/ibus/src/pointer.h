#ifndef POINTER_H
#define POINTER_H

template <typename T>
class Pointer
{
public:
    Pointer(T *p = 0) 
        : m_p(0)
    {
        set(p);
    }

    ~Pointer()
    {
        set(0);
    }
    
    Pointer(const Pointer& p)
        : m_p(0)
    {
        set(p.m_p);
    }

    Pointer& operator=(T *object)
    {
        set(object);
        return *this;
    }

    Pointer& operator=(const Pointer<T>& src)
    {
        set(src.m_p);
        return *this;
    }

    const T* operator->() const
    {
        return m_p;
    }

    T* operator->()
    {
        return m_p;
    }

    operator T* () const 
    {
        return m_p;
    }

    operator bool () const
    {
        return m_p != 0;
    }
    
private:
    
    T *m_p;

    void set(T *p)
    {
        if (m_p) {
            g_object_unref(m_p);
        }
        m_p = p;
        
        if (p) {
            g_object_ref_sink(p);
        }
    }
};

#endif// POINTER_H
