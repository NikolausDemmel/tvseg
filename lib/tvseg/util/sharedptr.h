#ifndef TVSEG_UTIL_SHAREDPTR_H
#define TVSEG_UTIL_SHAREDPTR_H


#include <cassert>


namespace tvseg {

    /// boost::shared_ptr makes nvcc choke, so roll our own simple smart pointer

    template<class T>
    class SharedPtr
    {
    public:
        explicit inline SharedPtr(T *ptr) :
            p_(ptr),
            rc_(new int(1))
        {
        }

        inline SharedPtr(const SharedPtr<T> &rhs) :
            p_(rhs.p_),
            rc_(rhs.rc_)
        {
            assert(rc_);
            addref();
        }

        inline ~SharedPtr()
        {
            assert(rc_);
            assert(*rc_ >= 1);
            if (deref() == 0) {
                free();
            }
        }

        T& operator* ()
        {
            return *p_;
        }

        const T& operator* () const
        {
            return *p_;
        }

        T* operator-> ()
        {
            return p_;
        }

        const T* operator-> () const
        {
            return p_;
        }

        operator bool() const
        {
            return p_;
        }

        SharedPtr<T>& operator = (const SharedPtr<T>& rhs)
        {
            if (this != &rhs)
            {
                if(deref() == 0)
                {
                    free();
                }

                p_ = rhs.p_;
                rc_ = rhs.rc_;
                assert(rc_);
                addref();
            }
            return *this;
        }
    private:
        void free()
        {
            if (p_) {
                delete p_;
                p_ = NULL;
            }
            delete rc_;
            rc_ = NULL;
        }

        void addref()
        {
            ++(*rc_);
        }

        int deref()
        {
            return --(*rc_);
        }

    private:
        T *p_;
        int *rc_;
    };


}


#endif // TVSEG_UTIL_SHAREDPTR_H
