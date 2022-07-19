#ifndef Utils_RefCountBase_hh
#define Utils_RefCountBase_hh

#include <cassert>

namespace Utils {

  //NOTE: Destructor of derived classes must not be public!

  class RefCountBase {
  public:
    unsigned refCount() const { return m_refCount; }
    void ref() const { ++m_refCount; }
    void unref() const
    {
      assert(m_refCount>0);
      if (m_refCount==1)
        delete this;
      else
        --m_refCount;
    }
    void unrefNoDelete() const
    {
      assert(m_refCount>0);
      --m_refCount;
    }
  protected:
    RefCountBase() : m_refCount(0) {}
    virtual ~RefCountBase(){}
  private:
    RefCountBase( const RefCountBase & );
    RefCountBase & operator= ( const RefCountBase & );
    mutable unsigned m_refCount;
  };

}

//The following RefCountHolder is useful for exposing ref-counted classes with
//to boost::python (for some reason, it only works if not in the Utils
//namespace):
template< class T >
struct RefCountHolder
{
  typedef T element_type;
  explicit RefCountHolder( T* t ) : obj( t ) { if ( obj ) obj->ref(); }
  ~RefCountHolder() { if ( obj ) obj->unref(); }
  RefCountHolder( const RefCountHolder & o ) : obj(o.obj) { if (obj) obj->ref(); }
  RefCountHolder & operator= ( const RefCountHolder & o)
  {
    if (o.obj!=obj) {
      T* old = obj;
      obj = o.obj;
      if (obj)
        obj->ref();
      if (old)
        old->unref();
    }
  }
  T* obj;
};
template< class T >
T* get_pointer(const RefCountHolder<T>& r) { return r.obj; }

#endif
