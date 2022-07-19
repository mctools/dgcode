////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2022 NCrystal developers                                   //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//      http://www.apache.org/licenses/LICENSE-2.0                            //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "NCrystalRel/ncrystal.h"
#include "NCrystalRel/NCRNG.hh"
#include "NCrystalRel/NCInfoBuilder.hh"
#include "NCrystalRel/NCMatCfg.hh"
#include "NCrystalRel/NCFactImpl.hh"
#include "NCrystalRel/NCFact.hh"
#include "NCrystalRel/NCPluginMgmt.hh"
#include "NCrystalRel/NCDataSources.hh"
#include "NCrystalRel/internal_NCDynInfoUtils.hh"
#include "NCrystalRel/internal_NCPlaneProvider.hh"
#include "NCrystalRel/NCDump.hh"
#include "NCrystalRel/internal_NCMath.hh"
#include "NCrystalRel/internal_NCString.hh"
#include "NCrystalRel/internal_NCAtomUtils.hh"
#include "NCrystalRel/internal_NCDebyeMSD.hh"
#include "NCrystalRel/internal_NCLatticeUtils.hh"//TODO: might not be needed eventually
#include "NCrystalRel/internal_NCEqRefl.hh"//TODO: might not be needed eventually
#include "NCrystalRel/internal_NCAtomDB.hh"
#include "NCrystalRel/internal_NCVDOSEval.hh"
#include "NCrystalRel/NCParseNCMAT.hh"
#include <cstdio>
#include <cstdlib>

namespace NCrystalRel {

  namespace NCCInterface {

    using ObjectTypeID = uint32_t;

    class AtomDataObj : private MoveOnly {
      //Hold AtomData and related info. This wrapper class is associated with a
      //particular Info instance, and therefore also knows the displayLabel
      //(unless a sub-component). It also caches some strings which would
      //otherwise be temporary objects, making it safe to return their values to
      //C code. Note that the displayLabels are only valid in connection with a
      //particular Info object, and only "top-level" atoms, the ones with an
      //AtomIndex on the Info object, have a displayLabel.
      shared_obj<const AtomData> m_atomDataSO;
      std::unique_ptr<std::string> m_displayLabel_ptr;
      std::unique_ptr<std::string> m_description_ptr;

    public:
      AtomDataObj( AtomDataSP ad, std::string displayLabel = "" )
        : m_atomDataSO(std::move(ad)),
          m_displayLabel_ptr( displayLabel.empty() ? nullptr : std::make_unique<std::string>(std::move(displayLabel)) ),
          m_description_ptr([this]() -> decltype(m_description_ptr)
          {
            std::string descr = m_atomDataSO->description(false);
            if ( m_displayLabel_ptr != nullptr && descr == *m_displayLabel_ptr )
              return nullptr;
            return std::make_unique<std::string>(std::move(descr));
          }())
      {
      }

      ncconstexpr17 const AtomData& atomData() const noexcept { return m_atomDataSO; }

      const std::string& displayLabel() const noexcept
      {
        static const std::string s_empty;
        return !m_displayLabel_ptr ? s_empty : *m_displayLabel_ptr;
      }
      const std::string& description() const ncnoexceptndebug
      {
        nc_assert( m_description_ptr != nullptr || m_displayLabel_ptr != nullptr );
        return m_description_ptr!=nullptr ? *m_description_ptr : *m_displayLabel_ptr;
      }

    };

    ObjectTypeID extractObjectTypeID(void*addr)
    {
      return *reinterpret_cast<ObjectTypeID*>(addr);
    }

    template<class TWrappedClass>
    TWrappedClass* tryCastWrapper(void* addr)
    {
      if (!addr)
        NCRYSTAL_THROW2(LogicError, "Could not extract " << TWrappedClass::wrapped_def::name()
                        << " object from provided handle in the C-interfaces. The provided"
                        << " handle was invalid (the internal state was a null pointer).");
      if ( extractObjectTypeID(addr) == TWrappedClass::wrapped_def::object_typeid )
        return reinterpret_cast<typename TWrappedClass::FingerPrint*>(addr)->wrapped;
      return nullptr;
    }

    template<class TWrappedClass>
    TWrappedClass& forceCastWrapper(void* addr)
    {
      TWrappedClass* t = tryCastWrapper<TWrappedClass>(addr);
      if (!t)
        NCRYSTAL_THROW2(LogicError, "Could not extract " << TWrappedClass::wrapped_def::name()
                        << " object from provided handle in the C-interfaces. Likely this is a"
                        << " sign of passing the wrong type of object handle to a function.");
      return *t;
    }

    template<class TWrappedClass>
    TWrappedClass& extractWrapperImpl(typename TWrappedClass::wrapped_def::c_handle_type h) {
      return forceCastWrapper<TWrappedClass>(h.internal);
    }

    template <class WrappedDef>
    struct Wrapped : private NoCopyMove {
    public:
      using object_type = typename WrappedDef::object_type;
      using c_handle_type = typename WrappedDef::c_handle_type;
      using wrapped_def = WrappedDef;
      static constexpr ObjectTypeID object_typeid() { return WrappedDef::object_typeid; };

      template<typename ...Args>
      Wrapped( Args&& ...args ) : m_obj(std::forward<Args>(args)...)
      {
        m_fingerprint.wrapped = this;
      }

      object_type& obj() { return m_obj; }
      const object_type& obj() const { return m_obj; }

      void ref() noexcept { ++m_refcount; }
      uint_fast32_t refCount() const noexcept { return m_refcount.load(); }
      bool unref() noexcept
      {
        //returns true if unref leads to deletion
        if ( m_refcount.fetch_sub(1) == 1 ) {
          delete this;
          return true;
        }
        return false;
      }

      struct FingerPrint {
        ObjectTypeID m_typecheck_value = WrappedDef::object_typeid;//simple trick to try to detect wrong usage of the C interfaces.
        Wrapped * wrapped;
        //This Fingerprint struct is "standard layout", even though the Wrapped
        //class itself might not be. This is important because we can then
        //access the m_typecheck_value field below through a pointer to the
        //struct itself, via a cast:
        //
        //  section 9.2 [class.mem]):
        //  A pointer to a standard-layout struct object, suitably converted using
        //  a reinterpret_cast, points to its initial member (or if that member is
        //  a bit-field, then to the unit in which it resides) and vice versa.
        //
        //So in order to follow the standard (and thus be more portable) we add
        //an additional layer of indirection through the fingerprint
        //struct. However, as m_fingerprint resides on Wrapped, the indirection
        //is with highest level of cache-locality.
      };
      static_assert(std::is_standard_layout<FingerPrint>::value,"");
      void * addressForCHandle() { return &m_fingerprint; }
    private:
      FingerPrint m_fingerprint;
      std::atomic<uint_fast32_t> m_refcount = {1};
      object_type m_obj;
      ~Wrapped() = default;
    };

    template<class TWrappedClass,typename ...Args>
    typename TWrappedClass::wrapped_def::c_handle_type createNewCHandle( Args&& ...args )
    {
      typename TWrappedClass::wrapped_def::c_handle_type handle;
      auto w = new TWrappedClass( std::forward<Args>(args)... );
      handle.internal = w->addressForCHandle();
      return handle;
    }

    ////////////////////////////////////////////////////////////////////////////////////
    //Info objects:
    struct WrappedDef_Info {
      using object_type = shared_obj<const Info>;
      using c_handle_type = ncrystalrel_info_t;
      static constexpr ObjectTypeID object_typeid = 0xcac4c93f;//randomly generated 32 bits
      static constexpr const char * name() { return "Info"; }
    };
    using Wrapped_Info = Wrapped<WrappedDef_Info>;
    Wrapped_Info& extractWrapper(Wrapped_Info::c_handle_type h) { return extractWrapperImpl<Wrapped_Info>(h); }
    Wrapped_Info::object_type& extract(Wrapped_Info::c_handle_type h) { return extractWrapperImpl<Wrapped_Info>(h).obj(); }

    ////////////////////////////////////////////////////////////////////////////////////
    //Scatter objects:
    struct WrappedDef_Scatter {
      using object_type = Scatter;
      using c_handle_type = ncrystalrel_scatter_t;
      static constexpr ObjectTypeID object_typeid = 0x7d6b0637;//randomly generated 32 bits
      static constexpr const char * name() { return "Scatter"; }
    };
    using Wrapped_Scatter = Wrapped<WrappedDef_Scatter>;
    Wrapped_Scatter& extractWrapper(Wrapped_Scatter::c_handle_type h) { return extractWrapperImpl<Wrapped_Scatter>(h); }
    Wrapped_Scatter::object_type& extract(Wrapped_Scatter::c_handle_type h) { return extractWrapperImpl<Wrapped_Scatter>(h).obj(); }

    ////////////////////////////////////////////////////////////////////////////////////
    //Absorption objects:
    struct WrappedDef_Absorption {
      using object_type = Absorption;
      using c_handle_type = ncrystalrel_absorption_t;
      static constexpr ObjectTypeID object_typeid = 0xede2eb9d;//randomly generated 32 bits
      static constexpr const char * name() { return "Absorption"; }
    };
    using Wrapped_Absorption = Wrapped<WrappedDef_Absorption>;
    Wrapped_Absorption& extractWrapper(Wrapped_Absorption::c_handle_type h) { return extractWrapperImpl<Wrapped_Absorption>(h); }
    Wrapped_Absorption::object_type& extract(Wrapped_Absorption::c_handle_type h) { return extractWrapperImpl<Wrapped_Absorption>(h).obj(); }

    ////////////////////////////////////////////////////////////////////////////////////
    //AtomData objects:
    struct WrappedDef_AtomData {
      using object_type = AtomDataObj;
      using c_handle_type = ncrystalrel_atomdata_t;
      static constexpr ObjectTypeID object_typeid = 0x66ece79c;//randomly generated 32 bits
      static constexpr const char * name() { return "AtomData"; }
    };
    using Wrapped_AtomData = Wrapped<WrappedDef_AtomData>;
    Wrapped_AtomData& extractWrapper(Wrapped_AtomData::c_handle_type h) { return extractWrapperImpl<Wrapped_AtomData>(h); }
    Wrapped_AtomData::object_type& extract(Wrapped_AtomData::c_handle_type h) { return extractWrapperImpl<Wrapped_AtomData>(h).obj(); }

    Process& extractProcess(ncrystalrel_process_t h)
    {
      ObjectTypeID objtypeid = h.internal ? extractObjectTypeID(h.internal) : 0x0;
      if ( objtypeid == Wrapped_Scatter::object_typeid() )
        return reinterpret_cast<Wrapped_Scatter::FingerPrint*>(h.internal)->wrapped->obj();
      if ( objtypeid != Wrapped_Absorption::object_typeid() )
        NCRYSTAL_THROW(LogicError,"Invalid ncrystalrel_process_t handle.");
      return reinterpret_cast<Wrapped_Absorption::FingerPrint*>(h.internal)->wrapped->obj();
    }

    void * & internal(void*o) {
      //Object is here a pointer to a struct like ncrystalrel_xxx_t. As these structs are
      //all standard-layout, it is guaranteed by the standard that we can
      //reinterpret cast to a pointer to the first data member (which is a
      //"void*").:
      static_assert(std::is_standard_layout<ncrystalrel_scatter_t>::value,"");
      static_assert(std::is_standard_layout<ncrystalrel_process_t>::value,"");
      static_assert(std::is_standard_layout<ncrystalrel_absorption_t>::value,"");
      static_assert(std::is_standard_layout<ncrystalrel_atomdata_t>::value,"");
      static_assert(std::is_standard_layout<ncrystalrel_info_t>::value,"");
      return *reinterpret_cast<void**>(o);
    }

    void throwInvalidHandleType(const char * fctname)
    {
      NCRYSTAL_THROW2(LogicError, "Invalid object handle type passed to " << fctname
                      << ". Note that you must provide the address of the handle and"
                      " not the handle itself.");
    }

    static int quietonerror = 0;
    static int haltonerror = 1;
    static int waserror = 0;
    static char errmsg[512];
    static char errtype[64];
    static void (*custom_error_handler)(char *,char*) = 0;

    void setError(const char *msg, const char * etype = 0) throw() {
      if (!etype)
        etype="ncrystalrel_c-interface";
      strncpy(errmsg,msg,sizeof(errmsg)-1);
      strncpy(errtype,etype,sizeof(errtype)-1);
      //Ensure final null-char in case of very long input strings:
      errmsg[sizeof(errmsg)-1]='\0';
      errtype[sizeof(errtype)-1]='\0';
      if (custom_error_handler) {
        (*custom_error_handler)(errtype,errmsg);
      }
      waserror = 1;
      if (!quietonerror)
        printf("NCrystal ERROR [%s]: %s\n",errtype,errmsg);
      if (haltonerror) {
        printf("NCrystal terminating due to ERROR\n");
        exit(1);
      }
    }

    void handleError(const std::exception &e) throw() {
      const Error::Exception* nce = dynamic_cast<const Error::Exception*>(&e);
      if (nce) {
        setError(nce->what(),nce->getTypeName());
        return;
      }
      const std::runtime_error* stdrte = dynamic_cast<const std::runtime_error*>(&e);
      if (stdrte)
        setError(stdrte->what(),"std::runtime_error");
      else
        setError("<unknown>","std::exception");
    }

  }
}

namespace ncc = NCrystalRel::NCCInterface;
namespace NC = NCrystalRel;

void ncrystalrel_seterrhandler(void (*handler)(char*,char*))
{
  ncc::custom_error_handler = handler;
}

int ncrystalrel_error()
{
  return ncc::waserror;
}

const char * ncrystalrel_lasterror()
{
  return ncc::waserror ? ncc::errmsg : 0;
}

const char * ncrystalrel_lasterrortype()
{
  return ncc::waserror ? ncc::errtype : 0;
}

void ncrystalrel_clearerror()
{
  ncc::waserror = 0;
}

int ncrystalrel_setquietonerror(int q)
{
  int old = ncc::quietonerror;
  ncc::quietonerror = q;
  return old;
}

int ncrystalrel_sethaltonerror(int h)
{
  int old = ncc::haltonerror;
  ncc::haltonerror = h;
  return old;
}

int ncrystalrel_valid(void* object)
{
  if (!object)
    return 0;
  void *& i = ncc::internal(object);
  return i ? 1 : 0;
}

#define NCCATCH catch (std::exception& e) { ncc::handleError(e); }

ncrystalrel_process_t ncrystalrel_cast_scat2proc(ncrystalrel_scatter_t s)
{
  try {
    ncc::extractWrapperImpl<ncc::Wrapped_Scatter>(s);//here just to check that input is valid
    return { s.internal };//not much to do!
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_process_t ncrystalrel_cast_abs2proc(ncrystalrel_absorption_t a)
{
  try {
    ncc::extractWrapperImpl<ncc::Wrapped_Absorption>(a);//here just to check that input is valid
    return { a.internal };//not much to do!
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_scatter_t ncrystalrel_cast_proc2scat(ncrystalrel_process_t p)
{
  try {
    if ( ncc::tryCastWrapper<ncc::Wrapped_Scatter>(p.internal) != nullptr )
      return { p.internal };//was indeed an upcasted scatter object
    ncc::extractProcess(p);//check input is valid process pointer at all
    return { nullptr };//incorrect type (probably absorption)
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_absorption_t ncrystalrel_cast_proc2abs(ncrystalrel_process_t p)
{
  try {
    if ( ncc::tryCastWrapper<ncc::Wrapped_Absorption>(p.internal) != nullptr )
      return { p.internal };//was indeed an upcasted absorption object
    ncc::extractProcess(p);//check input is valid process pointer at all
    return { nullptr };//incorrect type (probably scatter)
  } NCCATCH;
  return {nullptr};
}

int ncrystalrel_refcount(void* addrhandle)
{
  auto o = ncc::internal(addrhandle);
  try {
    switch (ncc::extractObjectTypeID(o)) {
      case ncc::Wrapped_Info::object_typeid():       return static_cast<int>(ncc::forceCastWrapper<ncc::Wrapped_Info>(o).refCount());
      case ncc::Wrapped_Scatter::object_typeid():    return static_cast<int>(ncc::forceCastWrapper<ncc::Wrapped_Scatter>(o).refCount());
      case ncc::Wrapped_Absorption::object_typeid(): return static_cast<int>(ncc::forceCastWrapper<ncc::Wrapped_Absorption>(o).refCount());
      case ncc::Wrapped_AtomData::object_typeid():   return static_cast<int>(ncc::forceCastWrapper<ncc::Wrapped_AtomData>(o).refCount());
      default: ncc::throwInvalidHandleType("ncrystalrel_refcount");
    };
  } NCCATCH;
  return -999;
}

void ncrystalrel_ref(void* addrhandle)
{
  auto o = ncc::internal(addrhandle);
  try {
    switch (ncc::extractObjectTypeID(o)) {
      case ncc::Wrapped_Info::object_typeid():       return ncc::forceCastWrapper<ncc::Wrapped_Info>(o).ref();
      case ncc::Wrapped_Scatter::object_typeid():    return ncc::forceCastWrapper<ncc::Wrapped_Scatter>(o).ref();
      case ncc::Wrapped_Absorption::object_typeid(): return ncc::forceCastWrapper<ncc::Wrapped_Absorption>(o).ref();
      case ncc::Wrapped_AtomData::object_typeid():   return ncc::forceCastWrapper<ncc::Wrapped_AtomData>(o).ref();
      default: ncc::throwInvalidHandleType("ncrystalrel_ref");
    };
  } NCCATCH;
}

namespace NCrystalRel {
  namespace NCCInterface {
    template<class TWrappedClass>
    void doUnref(void*o)
    {
      void*& handle_internal = ncc::internal(o);
      if (ncc::forceCastWrapper<TWrappedClass>(handle_internal).unref()) {
        //Object was deleted, invalidate handle:
        handle_internal = nullptr;
      }
    }
  }
}

void ncrystalrel_unref(void* addrhandle)
{
  try {
    switch (ncc::extractObjectTypeID(ncc::internal(addrhandle))) {
      case ncc::Wrapped_Info::object_typeid():       return ncc::doUnref<ncc::Wrapped_Info>(addrhandle);
      case ncc::Wrapped_Scatter::object_typeid():    return ncc::doUnref<ncc::Wrapped_Scatter>(addrhandle);
      case ncc::Wrapped_Absorption::object_typeid(): return ncc::doUnref<ncc::Wrapped_Absorption>(addrhandle);
      case ncc::Wrapped_AtomData::object_typeid():   return ncc::doUnref<ncc::Wrapped_AtomData>(addrhandle);
      default: ncc::throwInvalidHandleType("ncrystalrel_unref");
    };
  } NCCATCH;
}

void ncrystalrel_invalidate(void* o)
{
  if (!ncrystalrel_valid(o))
    return;
  ncc::internal(o) = 0;
}

void ncrystalrel_dump(ncrystalrel_info_t ci) { try { NC::dump(ncc::extract(ci)); } NCCATCH; }
void ncrystalrel_dump_verbose(ncrystalrel_info_t ci, unsigned verbosity_lvl ) {
  try {
    NC::dump(ncc::extract(ci),
             ( verbosity_lvl == 0
               ? NC::DumpVerbosity::DEFAULT
               : ( verbosity_lvl == 1
                   ? NC::DumpVerbosity::VERBOSE1
                   : NC::DumpVerbosity::VERBOSE2 ) ) );
  } NCCATCH;
}

int ncrystalrel_info_getstructure( ncrystalrel_info_t ci,
                                unsigned* spacegroup,
                                double* lattice_a, double* lattice_b, double* lattice_c,
                                double* alpha, double* beta, double* gamma,
                                double* volume, unsigned* n_atoms )
{
  try {
    auto& info = ncc::extract(ci);
    if (!info->hasStructureInfo())
      return 0;
    const NC::StructureInfo& si = info->getStructureInfo();
    *spacegroup = si.spacegroup;
    *lattice_a = si.lattice_a;
    *lattice_b = si.lattice_b;
    *lattice_c = si.lattice_c;
    *alpha = si.alpha;
    *beta = si.beta;
    *gamma = si.gamma;
    *volume = si.volume;
    *n_atoms = si.n_atoms;
    return 1;
  } NCCATCH;
  *spacegroup = 9999;
  *lattice_a = *lattice_b = *lattice_c = -1.0;
  *alpha = *beta = *gamma = *volume = -1.0;
  *n_atoms = 0;
  return 0;
}

double ncrystalrel_info_gettemperature( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->hasTemperature() ? info->getTemperature().get() : -1.0;
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_getxsectabsorption( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->getXSectAbsorption().dbl();
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_getxsectfree( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->getXSectFree().dbl();
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_getdensity( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->getDensity().dbl();
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_getnumberdensity( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->getNumberDensity().dbl();
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_getsld( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->getSLD().dbl();
  } NCCATCH;
  return -1.0;
}

int ncrystalrel_info_getstateofmatter( ncrystalrel_info_t ih )
{
  try {
    return static_cast<int>(ncc::extract(ih)->stateOfMatter());
  } NCCATCH;
  return -1;
}

double ncrystalrel_info_braggthreshold( ncrystalrel_info_t ci )
{
  try {
    auto bt = ncc::extract(ci)->getBraggThreshold();
    return bt.has_value() ? bt.value().dbl() : -1.0;
  } NCCATCH;
  return -1.0;
}

int ncrystalrel_info_hklinfotype( ncrystalrel_info_t nfo )
{
  try {
    return NC::enumAsInt( ncc::extract(nfo)->hklInfoType() );
  } NCCATCH;
  return -1;
}

int ncrystalrel_info_nhkl( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->hasHKLInfo() ? info->hklList().size() : -1;
  } NCCATCH;
  return -1;
}

double ncrystalrel_info_hkl_dlower( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->hasHKLInfo() ? info->hklDLower() : -1.0;
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_info_hkl_dupper( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    return info->hasHKLInfo() ? info->hklDUpper() : -1.0;
  } NCCATCH;
  return -1.0;
}

void ncrystalrel_info_gethkl( ncrystalrel_info_t ci, int idx,
                           int* h, int* k, int* l, int* multiplicity,
                           double * dspacing, double* fsquared )
{
  try {
    auto& info = ncc::extract(ci);
    auto& hklList = info->hklList();
    NC::HKLList::const_iterator it = std::next(hklList.begin(),idx);
    nc_assert(it<hklList.end());
    *h = it->hkl.h;
    *k = it->hkl.k;
    *l = it->hkl.l;
    *multiplicity = it->multiplicity;
    *dspacing = it->dspacing;
    *fsquared = it->fsquared;
    return;
  } NCCATCH;
  *h = *k = *l = *multiplicity = -9999;
  *dspacing = *fsquared = -1.0;
}

void ncrystalrel_info_gethkl_allindices( ncrystalrel_info_t nfo, int idx,
                                      int* h, int* k, int* l )
{
  try {
    h[0] = k[0] = l[0] = 0;//signature for not possible
    auto& info = ncc::extract(nfo);
    auto& hklList = info->hklList();
    NC::HKLList::const_iterator it = std::next(hklList.begin(),idx);
    nc_assert(it<hklList.end());
    nc_assert( info != nullptr );
    auto res = NC::ExpandHKLHelper( *info );
    for ( auto& e : res.expand( *it ) ) {
      *h++ = e.h;
      *k++ = e.k;
      *l++ = e.l;
    }
    return;
  } NCCATCH;
  h[0] = k[0] = l[0] = 0;
}

unsigned ncrystalrel_info_ndyninfo( ncrystalrel_info_t ci )
{
  try {
    return static_cast<unsigned>(ncc::extract(ci)->getDynamicInfoList().size());
  } NCCATCH;
  return 0;
}


void ncrystalrel_dyninfo_base( ncrystalrel_info_t ci,
                            unsigned idyninfo,
                            double* fraction,
                            unsigned* atomdataindex,
                            double* temperature,
                            unsigned* ditypeid )
{
  try {
    auto& di = ncc::extract(ci)->getDynamicInfoList().at(idyninfo);
    *fraction = di->fraction();
    *temperature = di->temperature().dbl();
    *atomdataindex = di->atom().index.get();
    if (dynamic_cast<const NC::DI_Sterile*>(di.get()))
      *ditypeid = 0;
    else if (dynamic_cast<const NC::DI_FreeGas*>(di.get()))
      *ditypeid = 1;
    else if (dynamic_cast<const NC::DI_ScatKnlDirect*>(di.get()))
      *ditypeid = 2;
    else if (dynamic_cast<const NC::DI_VDOS*>(di.get()))
      *ditypeid = 3;
    else if (dynamic_cast<const NC::DI_VDOSDebye*>(di.get()))
      *ditypeid = 4;
    else
      *ditypeid = 99;
    return;
  } NCCATCH;
  *fraction = *temperature = -1.0;
  *atomdataindex = *ditypeid = 0;
}

void ncrystalrel_dyninfo_extract_scatknl( ncrystalrel_info_t ci,
                                       unsigned idyninfo,
                                       unsigned vdoslux,
                                       double * suggestedEmax,
                                       unsigned* negrid,
                                       unsigned* nalpha,
                                       unsigned* nbeta,
                                       const double** egrid,
                                       const double** alphagrid,
                                       const double** betagrid,
                                       const double** sab )
{
  try {
    auto& di = ncc::extract(ci)->getDynamicInfoList().at(idyninfo);
    nc_assert_always(!!di);
    std::shared_ptr<const NC::SABData> shptr_sabdata;
    NC::DI_ScatKnl::EGridShPtr shptr_egrid;
    auto di_sk = dynamic_cast<const NC::DI_ScatKnl*>(di.get());
    if (di_sk) {
      shptr_sabdata = NC::extractSABDataFromDynInfo(di_sk,vdoslux);
      shptr_egrid = di_sk->energyGrid();
      //In case the sabdata factory does not keep strong references, we must
      //keep the newly created object in shptr_sabdata alive when returning to
      //C/Python code without shared pointers. For now we do this by adding to a
      //global static array here:
      static std::vector<std::shared_ptr<const NC::SABData>> s_keepAlive;
      static std::mutex s_keepAlive_mutex;
      NCRYSTAL_LOCK_GUARD(s_keepAlive_mutex);
      s_keepAlive.push_back(shptr_sabdata);
      static bool first = true;
      if (first) {
        //Register for clearance by global clearCaches function:
        first = false;
        NC::registerCacheCleanupFunction([](){ s_keepAlive.clear(); });
      }

    }
    if (!!shptr_sabdata) {
      const auto& sabdata = *shptr_sabdata;
      unsigned na = static_cast<unsigned>(sabdata.alphaGrid().size());
      unsigned nb = static_cast<unsigned>(sabdata.betaGrid().size());
      unsigned nsab = static_cast<unsigned>(sabdata.sab().size());
      nc_assert_always(na>1&&nb>1&&na*nb==nsab);
      *nalpha = na;
      *nbeta = nb;
      *alphagrid = &sabdata.alphaGrid()[0];
      *betagrid = &sabdata.betaGrid()[0];
      *sab = &sabdata.sab()[0];
      *suggestedEmax = sabdata.suggestedEmax();
    } else {
      *nalpha = 0;
      *nbeta = 0;
      *alphagrid = nullptr;
      *betagrid = nullptr;
      *sab = nullptr;
      *suggestedEmax = 0.0;
    }
    if ( !shptr_egrid || shptr_egrid->empty() ) {
      *negrid = 0;
      //make sure egrid does at least point to valid memory (and avoid actually
      //using 0-length arrays);
      static const double dummy[1] = { 0.0 };
      *egrid = &dummy[0];
    } else {
      *negrid = shptr_egrid->size();
      *egrid = &(*shptr_egrid)[0];
    }
    return;
  } NCCATCH;
  *suggestedEmax = -1.0;
  *negrid = *nalpha = *nbeta = 0;
  *egrid = *alphagrid = *betagrid = *sab = nullptr;
}

void ncrystalrel_dyninfo_extract_vdos( ncrystalrel_info_t ci,
                                    unsigned idyninfo,
                                    double * egridMin,
                                    double * egridMax,
                                    unsigned * vdos_ndensity,
                                    const double ** vdos_density )
{
  try {
    auto& di = ncc::extract(ci)->getDynamicInfoList().at(idyninfo);
    nc_assert_always(!!di);
    auto di_vdos = dynamic_cast<const NC::DI_VDOS*>(di.get());
    if (di_vdos) {
      const auto& vdosData = di_vdos->vdosData();
      const auto& v_egrid = vdosData.vdos_egrid();
      const auto& v_density = vdosData.vdos_density();
      nc_assert_always(v_density.size()<=std::numeric_limits<unsigned>::max());
      *egridMin = v_egrid.first;
      *egridMax = v_egrid.second;
      *vdos_ndensity = static_cast<unsigned>(v_density.size());
      *vdos_density = &v_density[0];
    } else {
      static const double dummy[1] = { 0.0 };//avoid pointing to invalid mem
      *egridMin = 0;
      *egridMax = 0;
      *vdos_ndensity = 0;
      *vdos_density = &dummy[0];
    }
    return;
  } NCCATCH;
  *egridMin = *egridMax = -1;
  *vdos_ndensity = 0;
  *vdos_density = nullptr;
}

void ncrystalrel_dyninfo_extract_vdos_input( ncrystalrel_info_t ci,
                                          unsigned idyninfo,
                                          unsigned* vdos_negrid,
                                          const double ** vdos_egrid,
                                          unsigned* vdos_ndensity,
                                          const double ** vdos_density )
{
  try {
    auto& di = ncc::extract(ci)->getDynamicInfoList().at(idyninfo);
    nc_assert_always(!!di);
    auto di_vdos = dynamic_cast<const NC::DI_VDOS*>(di.get());
    static const double dummy[1] = { 0.0 };//avoid pointing to invalid mem
    *vdos_negrid = 0;
    *vdos_ndensity = 0;
    *vdos_egrid = &dummy[0];
    *vdos_density = &dummy[0];
    if (di_vdos) {
      const NC::VectD& egrid = di_vdos->vdosOrigEgrid();
      const NC::VectD& density = di_vdos->vdosOrigDensity();
      nc_assert_always(density.size()<=std::numeric_limits<unsigned>::max());
      if ( !egrid.empty() && !density.empty() ) {
        *vdos_egrid = &egrid[0];
        *vdos_density = &density[0];
        *vdos_negrid = static_cast<unsigned>(egrid.size());
        *vdos_ndensity = static_cast<unsigned>(density.size());
      }
    }
    return;
  } NCCATCH;
  *vdos_negrid = *vdos_ndensity = 0;
  *vdos_egrid = *vdos_density = nullptr;
}

void ncrystalrel_dyninfo_extract_vdosdebye( ncrystalrel_info_t ci,
                                         unsigned idyninfo,
                                         double * debye_temp )
{
  try {
    auto& di = ncc::extract(ci)->getDynamicInfoList().at(idyninfo);
    nc_assert_always(!!di);
    auto di_vdosdebye = dynamic_cast<const NC::DI_VDOSDebye*>(di.get());
    *debye_temp = di_vdosdebye ? di_vdosdebye->debyeTemperature().dbl() : 0.0;
    return;
  } NCCATCH;
  *debye_temp = -1.0;
}


double ncrystalrel_info_dspacing_from_hkl( ncrystalrel_info_t ci, int h, int k, int l )
{
  try {
    return ncc::extract(ci)->dspacingFromHKL(h,k,l);
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_wl2ekin( double wl )
{
  return NC::wl2ekin(wl);//doesn't throw
}

double ncrystalrel_ekin2wl( double ekin )
{
  return NC::ekin2wl(ekin);//doesn't throw
}

int ncrystalrel_isnonoriented(ncrystalrel_process_t o)
{
  try {
    return ncc::extractProcess(o).isOriented() ? 0 : 1;
  } NCCATCH;
  return 0;
}

const char * ncrystalrel_name(ncrystalrel_process_t o)
{
  try {
    return ncc::extractProcess(o).name();
  } NCCATCH;
  return 0;
}

void ncrystalrel_domain( ncrystalrel_process_t o,
                      double* ekin_low, double* ekin_high)
{
  try {
    auto domain = ncc::extractProcess(o).domain();
    *ekin_low = domain.elow.get();
    *ekin_high = domain.ehigh.get();
    return;
  } NCCATCH;
  *ekin_low = *ekin_high = -1.0;
}

void ncrystalrel_crosssection( ncrystalrel_process_t o, double ekin, const double (*direction)[3], double* result)
{
  try {
    *result = ncc::extractProcess(o).crossSection( NC::NeutronEnergy{ekin}, NC::NeutronDirection{*direction} ).get();
    return;
  } NCCATCH;
  *result = -1.0;
}

void ncrystalrel_crosssection_nonoriented( ncrystalrel_process_t o, double ekin, double* result)
{
  try {
    *result = ncc::extractProcess(o).crossSectionIsotropic( NC::NeutronEnergy{ekin} ).get();
    return;
  } NCCATCH;
  *result = -1.0;
}

void ncrystalrel_crosssection_nonoriented_many( ncrystalrel_process_t o,
                                             const double * ekin,
                                             unsigned long n_ekin,
                                             unsigned long repeat,
                                             double* results )
{
  unsigned long repeat_orig = repeat;
  double* results_orig = results;
  try {
    auto& process = ncc::extractProcess(o);
    while (repeat--) {
      for (unsigned long i = 0; i < n_ekin; ++i)
        *results++ = process.crossSectionIsotropic(NC::NeutronEnergy{ekin[i]}).get();
    }
    return;
  } NCCATCH;
  while (repeat_orig--) {
    for (unsigned long i = 0; i < n_ekin; ++i)
      *results_orig++ = -1.0;
  }
}

void ncrystalrel_samplescatterisotropic( ncrystalrel_scatter_t o,
                                      double ekin,
                                      double* ekin_final,
                                      double* cos_scat_angle )
{
  try {
    auto& sc = ncc::extract(o);
    auto outcome = sc.sampleScatterIsotropic( NC::NeutronEnergy{ekin} );
    *ekin_final = outcome.ekin.dbl();
    *cos_scat_angle = outcome.mu.dbl();
    return;
  } NCCATCH;
  *ekin_final = -1.0;
  *cos_scat_angle = -999;
}

void ncrystalrel_samplescatter( ncrystalrel_scatter_t o,
                             double ekin,
                             const double (*direction)[3],
                             double* ekin_final,
                             double (*direction_final)[3] )
{
  try {
    auto& sc = ncc::extract(o);
    auto outcome = sc.sampleScatter(NC::NeutronEnergy{ekin}, NC::NeutronDirection{*direction} );
    *ekin_final = outcome.ekin.dbl();
    outcome.direction.applyTo(*direction_final);
    return;
  } NCCATCH;
  *ekin_final = -1.0;
  (*direction_final)[0] = (*direction_final)[1] = (*direction_final)[2] = 0.0;
}


void ncrystalrel_samplescatterisotropic_many( ncrystalrel_scatter_t o,
                                           const double * ekin,
                                           unsigned long n_ekin,
                                           unsigned long repeat,
                                           double* results_ekin,
                                           double* results_cos_scat_angle )
{
  unsigned long repeat_orig = repeat;
  double* results_ekin_orig = results_ekin;
  double* results_cos_scat_angle_orig = results_cos_scat_angle;
  try {
    auto& sc = ncc::extract(o);
    while (repeat--) {
      for (unsigned long i = 0; i < n_ekin; ++i) {
        auto outcome = sc.sampleScatterIsotropic(NC::NeutronEnergy{ekin[i]});
        *results_ekin++ = outcome.ekin.dbl();
        *results_cos_scat_angle++ = outcome.mu.dbl();
      }
    }
    return;
  } NCCATCH;
  //non-halting-error, invalidate all output:
  while (repeat_orig--) {
    for (unsigned long i = 0; i < n_ekin; ++i) {
      *results_ekin_orig++ = -1.0;
      *results_cos_scat_angle_orig++ = -999.0;
    }
  }
}

void ncrystalrel_samplescatter_many( ncrystalrel_scatter_t o,
                                  double ekin,
                                  const double (*direction)[3],
                                  unsigned long repeat,
                                  double* results_ekin,
                                  double * results_dirx,
                                  double * results_diry,
                                  double * results_dirz )
{
  unsigned long repeat_orig = repeat;
  double* results_ekin_orig = results_ekin;
  double* results_dirx_orig = results_dirx;
  double* results_diry_orig = results_diry;
  double* results_dirz_orig = results_dirz;
  try {
    NC::NeutronDirection dir{ *direction };
    auto& sc = ncc::extract(o);
    while (repeat--) {
      auto outcome = sc.sampleScatter(NC::NeutronEnergy{ekin}, dir);
      *results_ekin++ = outcome.ekin.dbl();
      *results_dirx++ = outcome.direction[0];
      *results_diry++ = outcome.direction[1];
      *results_dirz++ = outcome.direction[2];
    }
    return;
  } NCCATCH;
  //non-halting-error, invalidate all output:
  while (repeat_orig--) {
    *results_ekin_orig++ = -1.0;
    *results_dirx_orig++ = 0.0;
    *results_diry_orig++ = 0.0;
    *results_dirz_orig++ = 0.0;
  }

}

void ncrystalrel_genscatter_nonoriented( ncrystalrel_scatter_t o, double ekin, double* result_angle, double* result_dekin )
{
  //obsolete fct:
  try {
    auto& sc = ncc::extract(o);
    auto outcome = sc.sampleScatterIsotropic( NC::NeutronEnergy{ekin} );
    *result_dekin = outcome.ekin.get() - ekin;
    *result_angle = std::acos(outcome.mu.get());
    return;
  } NCCATCH;
  //nb: should set everything to some dummy values here, but the fct is obsolete anyway...
}

void ncrystalrel_genscatter_nonoriented_many( ncrystalrel_scatter_t o,
                                           const double * ekin,
                                           unsigned long n_ekin,
                                           unsigned long repeat,
                                           double* results_angle,
                                           double* results_dekin )
{
  //obsolete fct:
  try {
    auto& sc = ncc::extract(o);
    while (repeat--) {
      for (unsigned long i = 0; i < n_ekin; ++i) {
        auto outcome = sc.sampleScatterIsotropic(NC::NeutronEnergy{ekin[i]});
        *results_dekin++ = outcome.ekin.get() - ekin[i];
        *results_angle++ = std::acos(outcome.mu.get());
      }
    }
    return;
  } NCCATCH;
  //nb: should set everything to some dummy values here, but the fct is obsolete anyway...
}

void ncrystalrel_genscatter_many( ncrystalrel_scatter_t o,
                               double ekin,
                               const double (*indir)[3],
                               unsigned long repeat,
                               double * results_dirx,
                               double * results_diry,
                               double * results_dirz,
                               double * results_dekin )
{
  //obsolete fct:
  try {
    NC::NeutronDirection dir{ *indir };
    auto& sc = ncc::extract(o);
    while (repeat--) {
      auto outcome = sc.sampleScatter(NC::NeutronEnergy{ekin}, dir);
      *results_dekin++ = outcome.ekin.get() - ekin;
      *results_dirx++ = outcome.direction[0];
      *results_diry++ = outcome.direction[1];
      *results_dirz++ = outcome.direction[2];
    }
    return;
  } NCCATCH;
  //nb: should set everything to some dummy values here, but the fct is obsolete anyway...
}

void ncrystalrel_genscatter( ncrystalrel_scatter_t o, double ekin, const double (*direction)[3],
                          double (*result_direction)[3], double* result_deltaekin )
{
  //obsolete fct.
  try {
    auto& sc = ncc::extract(o);
    auto outcome = sc.sampleScatter(NC::NeutronEnergy{ekin}, NC::NeutronDirection{*direction} );
    *result_deltaekin = outcome.ekin.get() - ekin;
    outcome.direction.applyTo(*result_direction);
    return;
  } NCCATCH;
  (*result_direction)[0] = (*result_direction)[1] = (*result_direction)[2] = 0.0;
  *result_deltaekin = 0.0;
}


int ncrystalrel_info_hasatompos( ncrystalrel_info_t ci_t ) {
  //obsolete fct:
  try { return int(ncc::extract(ci_t)->hasAtomInfo()); } NCCATCH;  return 0;
}

int ncrystalrel_info_hasanydebyetemp( ncrystalrel_info_t ci_t )
{
  //obsolete fct.
  return ncrystalrel_info_hasatomdebyetemp(ci_t);
}

double ncrystalrel_info_getglobaldebyetemp( ncrystalrel_info_t ci )
{
  //obsolete fct.
  try {
    ncc::extract(ci);//trigger type checking
    return -1.0;//Always return "n/a" now
  } NCCATCH;
  return -1.0;
}
double ncrystalrel_info_getdebyetempbyelement( ncrystalrel_info_t ci,
                                            unsigned atomdataindex )
{
  //obsolete fct
  try {
    NC::AtomIndex idx{atomdataindex};
    for ( auto& ai : ncc::extract(ci)->getAtomInfos() ) {
      if ( ai.indexedAtomData().index == idx )
        return ai.debyeTemp().has_value() ? ai.debyeTemp().value().dbl() : -1.0;
    }
  } NCCATCH;
  return -1.0;
}

void ncrystalrel_multicreate_direct( const char* data,
                                  const char* dataType,
                                  const char* cfg_params,
                                  ncrystalrel_info_t* h_i,
                                  ncrystalrel_scatter_t* h_s,
                                  ncrystalrel_absorption_t* h_a )
{
  try{
    if (h_i)
      h_i->internal = nullptr;
    if (h_s)
      h_s->internal = nullptr;
    if (h_a)
      h_a->internal = nullptr;
    auto cfg = NC::MatCfg::createFromRawData( std::string(data),
                                              std::string(cfg_params?cfg_params:""),
                                              std::string(dataType?dataType:"") );
    if ( h_i )
      *h_i = ncc::createNewCHandle<ncc::Wrapped_Info>( NC::createInfo(cfg) );
    if ( h_s )
      *h_s = ncc::createNewCHandle<ncc::Wrapped_Scatter>( NC::createScatter(cfg) );
    if ( h_a )
      *h_a = ncc::createNewCHandle<ncc::Wrapped_Absorption>( NC::createAbsorption(cfg) );
  } NCCATCH;

}

int ncrystalrel_info_nphases( ncrystalrel_info_t ih )
{
  try {
    return static_cast<int>(ncc::extract(ih)->getPhases().size());
  } NCCATCH;
  return -1;
}

ncrystalrel_info_t ncrystalrel_info_getphase( ncrystalrel_info_t ih, int iphase, double* fraction )
{
  *fraction = -1.0;
  try {
    const auto& ph = ncc::extract(ih)->getPhases().at(iphase);
    *fraction = ph.first;
    return ncc::createNewCHandle<ncc::Wrapped_Info>( ph.second );
  } NCCATCH;
  *fraction = -1.0;
  return {nullptr};
}

ncrystalrel_info_t ncrystalrel_create_info( const char * cfgstr )
{
  try {
    return ncc::createNewCHandle<ncc::Wrapped_Info>( NC::createInfo(cfgstr) );
  } NCCATCH;
  return {nullptr};
}

double ncrystalrel_decodecfg_packfact( const char * cfgstr )
{
  //Obsolete, returns 1 (but we still decode the cfgstr just to help the user
  //catch errors).
  try {
    NC::MatCfg cfg(cfgstr);
    (void)cfg;
    return 1.0;
  } NCCATCH;
  return -1.0;
}

unsigned ncrystalrel_decodecfg_vdoslux( const char * cfgstr )
{
  try {
    NC::MatCfg cfg(cfgstr);
    return cfg.get_vdoslux();
  } NCCATCH;
  return 999;
}

ncrystalrel_scatter_t ncrystalrel_create_scatter( const char * cfgstr )
{
  try {
    return ncc::createNewCHandle<ncc::Wrapped_Scatter>( NC::createScatter(cfgstr) );
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_absorption_t ncrystalrel_clone_absorption( ncrystalrel_absorption_t ah )
{
  try {
    auto& absn = ncc::extract(ah);
    return ncc::createNewCHandle<ncc::Wrapped_Absorption>(absn.clone());
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_scatter_t ncrystalrel_clone_scatter( ncrystalrel_scatter_t sh )
{
  try {
    auto& sc = ncc::extract(sh);
    return ncc::createNewCHandle<ncc::Wrapped_Scatter>(sc.clone());
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_scatter_t ncrystalrel_clone_scatter_rngbyidx( ncrystalrel_scatter_t sh, unsigned long rngstreamidx )
{
  try {
    auto& sc = ncc::extract(sh);
    return ncc::createNewCHandle<ncc::Wrapped_Scatter>(sc.cloneByIdx(NC::RNGStreamIndex{(uint64_t)rngstreamidx}));
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_scatter_t ncrystalrel_clone_scatter_rngforcurrentthread( ncrystalrel_scatter_t sh )
{
  try {
    auto& sc = ncc::extract(sh);
    return ncc::createNewCHandle<ncc::Wrapped_Scatter>(sc.cloneForCurrentThread());
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_scatter_t ncrystalrel_create_scatter_builtinrng( const char * cfgstr, unsigned long seed )
{
  try {
    auto rng = NC::createBuiltinRNG( static_cast<uint64_t>(seed) );
    auto rngproducer = NC::makeSO<NC::RNGProducer>( rng );
    auto pp = NC::FactImpl::createScatter(cfgstr);
    return ncc::createNewCHandle<ncc::Wrapped_Scatter>( NC::Scatter(std::move(rngproducer), std::move(rng),std::move(pp)));
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_absorption_t ncrystalrel_create_absorption( const char * cfgstr )
{
  try {
    return ncc::createNewCHandle<ncc::Wrapped_Absorption>( NC::createAbsorption(cfgstr) );
  } NCCATCH;
  return {nullptr};
}

void ncrystalrel_clear_caches()
{
  try {
    NC::clearCaches();
  } NCCATCH;
}

void ncrystalrel_clear_info_caches()
{
  //deprecated, now simply redirects to ncrystalrel_clear_caches.
  ncrystalrel_clear_caches();
}

int ncrystalrel_has_factory( const char* name )
{
  try {
    if (NC::FactImpl::hasInfoFactory(name))
      return 1;
    if (NC::FactImpl::hasScatterFactory(name))
      return 1;
    if (NC::FactImpl::hasAbsorptionFactory(name))
      return 1;
  } NCCATCH;
  return 0;
}

int ncrystalrel_version()
{
  return NCRYSTAL_VERSION;
}

const char * ncrystalrel_version_str()
{
  return NCRYSTAL_VERSION_STR;
}

unsigned ncrystalrel_info_natominfo( ncrystalrel_info_t ci_t )
{
  try {
    auto& info = ncc::extract(ci_t);
    if (!info->hasAtomInfo())
      return 0;
    return static_cast<unsigned>(info->getAtomInfos().size());
  } NCCATCH;
  return 0;
}

int ncrystalrel_info_hasatommsd( ncrystalrel_info_t ci_t ) { try { return int(ncc::extract(ci_t)->hasAtomMSD()); } NCCATCH;  return 0; }
int ncrystalrel_info_hasatomdebyetemp( ncrystalrel_info_t ci_t ) { try { return int(ncc::extract(ci_t)->hasAtomDebyeTemp()); } NCCATCH;  return 0; }
int ncrystalrel_info_hasdebyetemp( ncrystalrel_info_t ci_t ) { return ncrystalrel_info_hasatomdebyetemp(ci_t); }//alias

void ncrystalrel_info_getatominfo( ncrystalrel_info_t ci, unsigned iatom,
                                unsigned* atomdataindex,
                                unsigned* number_per_unit_cell,
                                double* debye_temp, double* msd )
{
  try {
    auto& info = ncc::extract(ci);
    auto& atominfos = info->getAtomInfos();
    if ( iatom >= atominfos.size() )
      NCRYSTAL_THROW(BadInput,"ncrystalrel_info_getatominfo iatom is out of bounds");
    const NC::AtomInfo& ai = atominfos.at(iatom);
    *atomdataindex = ai.atom().index.get();
    *number_per_unit_cell = ai.numberPerUnitCell();
    if ( ai.debyeTemp().has_value() )
      *debye_temp = ai.debyeTemp().value().dbl();
    else
      *debye_temp = 0.0;
    *msd = ai.msd().value_or(0.0);
    return;
  } NCCATCH;

  *atomdataindex = *number_per_unit_cell = 0;
  *debye_temp = *msd = -1.0;
}

void ncrystalrel_info_getatompos( ncrystalrel_info_t ci,
                               unsigned iatom, unsigned ipos,
                               double* x, double* y, double* z )
{
  try {
    auto& info = ncc::extract(ci);
    auto& atominfos = info->getAtomInfos();
    if ( iatom >= atominfos.size() )
      NCRYSTAL_THROW(BadInput,"ncrystalrel_info_getatompos iatom is out of bounds");
    const NC::AtomInfo& ai = atominfos.at(iatom);
    const auto& poslist = ai.unitCellPositions();
    nc_assert( !poslist.empty() );//AtomInfo constructor forbids this
    if ( ! ( ipos < poslist.size() ) )
      NCRYSTAL_THROW(BadInput,"ncrystalrel_info_getatompos ipos is out of bounds");
    const auto& pos = poslist[ipos];
    *x = pos[0];
    *y = pos[1];
    *z = pos[2];
    return;
  } NCCATCH;
  *x = *y = *z = -999.0;
}

unsigned ncrystalrel_info_ncustomsections( ncrystalrel_info_t ci )
{
  try {
    return static_cast<unsigned>( ncc::extract(ci)->getAllCustomSections().size() );
  } NCCATCH;
  return 0;
}

const char* ncrystalrel_info_customsec_name( ncrystalrel_info_t ci, unsigned isection )
{
  try {
    return ncc::extract(ci)->getAllCustomSections().at(isection).first.c_str();
  } NCCATCH;
  return "";
}

unsigned ncrystalrel_info_customsec_nlines( ncrystalrel_info_t ci, unsigned isection )
{
  try {
    return static_cast<unsigned>( ncc::extract(ci)->getAllCustomSections().at(isection).second.size() );
  } NCCATCH;
  return 0;
}

unsigned ncrystalrel_info_customline_nparts( ncrystalrel_info_t ci, unsigned isection, unsigned iline )
{
  try {
    return static_cast<unsigned>( ncc::extract(ci)->getAllCustomSections().at(isection).second.at(iline).size() );
  } NCCATCH;
  return 0;
}

const char* ncrystalrel_info_customline_getpart( ncrystalrel_info_t ci, unsigned isection, unsigned iline, unsigned ipart )
{
  try {
    return ncc::extract(ci)->getAllCustomSections().at(isection).second.at(iline).at(ipart).c_str();
  } NCCATCH;
  return "";
}

void ncrystalrel_register_in_mem_file_data(const char* virtual_filename, const char* cdata)
{
  try {
    std::string s(cdata);
    if ( NC::startswith(s,"ondisk://") ) {
      if ( NC::contains(s,'\n') || NC::contains(s,'\r') )
        NCRYSTAL_THROW2(BadInput,"ncrystalrel_register_in_mem_file_data: newlines not allowed in data starting with 'ondisk://'");
      NC::DataSources::registerVirtualFileAlias( virtual_filename, s.substr(9) );
    } else {
      NC::DataSources::registerInMemoryFileData(virtual_filename,std::string(cdata));
    }
  } NCCATCH;
}

ncrystalrel_atomdata_t ncrystalrel_create_atomdata( ncrystalrel_info_t ci,
                                              unsigned atomdataindex )
{
  try {
    auto& info = ncc::extract(ci);
    return ncc::createNewCHandle<ncc::Wrapped_AtomData>( info->atomDataSP(NC::AtomIndex{atomdataindex}),
                                                         info->displayLabel(NC::AtomIndex{atomdataindex}) );
  } NCCATCH;
  return {nullptr};
}

void ncrystalrel_atomdata_getfields( ncrystalrel_atomdata_t o,
                                  const char** displaylabel,
                                  const char** description,
                                  double* mass, double *incxs,
                                  double* cohsl_fm, double* absxs,
                                  unsigned* ncomponents,
                                  unsigned* zval, unsigned* aval )
{
  try {
    auto& ad = ncc::extract(o);
    *displaylabel = ad.displayLabel().c_str();
    *description = ad.description().c_str();
    auto& data = ad.atomData();
    *mass = data.averageMassAMU().dbl();
    *cohsl_fm = data.coherentScatLenFM();
    *incxs = data.incoherentXS().dbl();
    *absxs = data.captureXS().dbl();
    *zval = data.isElement() ? data.Z() : 0;
    *aval = data.isSingleIsotope() ? data.A() : 0;
    *ncomponents = data.nComponents();
    return;
  } NCCATCH;
  *displaylabel = *description = "";
  *cohsl_fm = *absxs = -99999.0;
  *ncomponents = *zval = *aval;
}

ncrystalrel_atomdata_t ncrystalrel_create_atomdata_subcomp( ncrystalrel_atomdata_t o,
                                                      unsigned icomponent,
                                                      double* fraction )
{
  try {
    const auto& comp = ncc::extract(o).atomData().getComponent(icomponent);
    *fraction = comp.fraction;
    return ncc::createNewCHandle<ncc::Wrapped_AtomData>( comp.data );
  } NCCATCH;
  return {nullptr};
}

unsigned ncrystalrel_info_ncomponents( ncrystalrel_info_t ci )
{
  try {
    auto n = ncc::extract(ci)->getComposition().size();
    nc_assert( n>0 && n < std::numeric_limits<unsigned>::max() );
    return static_cast<unsigned>(n);
  } NCCATCH;
  return 0;
}

void ncrystalrel_info_getcomponent( ncrystalrel_info_t ci, unsigned icomponent,
                                 unsigned* atomdataindex, double* fraction )
{
  try {
    auto& info = ncc::extract(ci);
    auto n = info->getComposition().size();
    nc_assert(n>=1);
    if ( ! ( icomponent<n) )
      NCRYSTAL_THROW(BadInput,"Requested component index is out of bounds");
    const auto& comp = info->getComposition().at(icomponent);
    *atomdataindex = comp.atom.index.get();
    *fraction = comp.fraction;
    return;
  } NCCATCH;
  *atomdataindex = 999999;
  *fraction = -1.0;
}

ncrystalrel_atomdata_t ncrystalrel_create_atomdata_fromdb( unsigned z, unsigned a )
{
  try {
    auto opt_atomdatasp = NC::AtomDB::getIsotopeOrNatElem(z,a);
    if ( opt_atomdatasp != nullptr )
      return ncc::createNewCHandle<ncc::Wrapped_AtomData>( std::move(opt_atomdatasp) );
  } NCCATCH;
  return {nullptr};
}

ncrystalrel_atomdata_t ncrystalrel_create_atomdata_fromdbstr( const char* name )
{
  unsigned z(0),a(0);
  try {
    nc_assert(name);
    NC::AtomSymbol symb(name);
    if (symb.isElement()||symb.isIsotope()) {
      z = symb.Z();
      a = symb.A();
    }
    if (z) {
      auto opt_atomdatasp = NC::AtomDB::getIsotopeOrNatElem(z,a);
      if ( opt_atomdatasp != nullptr )
        return ncc::createNewCHandle<ncc::Wrapped_AtomData>( std::move(opt_atomdatasp) );
    }
  } NCCATCH;
  return {nullptr};
}

unsigned ncrystalrel_atomdatadb_getnentries()
{
  try {
    return NC::AtomDB::getAllEntriesCount();
  } NCCATCH;
  return 0;
}

void ncrystalrel_atomdatadb_getallentries( unsigned* zvals,
                                        unsigned* avals )
{
  try {
    std::vector<std::pair<unsigned,unsigned>> all = NC::AtomDB::getAllEntries();
    nc_assert( static_cast<unsigned>(all.size()) == ncrystalrel_atomdatadb_getnentries() );
    for ( auto& e : all ) {
      *zvals++ = e.first;
      *avals++ = e.second;
    }
  } NCCATCH;
}


namespace NCrystalRel {

  namespace NCCInterface {
    char * createString(const char * str_begin, const char * str_end)
    {
      //For safety, add extra byte with null char at end (in case this is
      //absent):
      auto nn = std::distance(str_begin,str_end);
      char * cs = new char[nn+1];
      std::memcpy(cs,str_begin,nn);
      cs[nn] = '\0';
      return cs;
    }

    char * createString(const std::string& ss)
    {
      auto nn = ss.size() + 1;
      char * cs = new char[nn];
      std::memcpy(cs,&ss[0],nn);
      return cs;
    }

    void createStringList(const NC::VectS& l, char*** tgt, unsigned* tgtlen)
    {
      if (l.empty()) {
        *tgt = nullptr;
        *tgtlen = 0;
        return;
      }
      nc_assert_always( l.size() < std::numeric_limits<unsigned>::max() );
      unsigned len = static_cast<unsigned>(l.size());
      char ** out = new char*[len];
      char ** it = out;
      for ( auto& e : l ) {
        nc_assert(it<(out + len));
        *it = new char[e.size()+1];
        std::memcpy(*it,&e[0],e.size()+1);
        ++it;
      }
      *tgtlen = len;
      *tgt = out;
    }
  }
}

void ncrystalrel_dealloc_stringlist( unsigned length, char** sl )
{
  if (sl) {
    nc_assert(length>0);
    for (unsigned i = 0; i < length; ++i)
      delete[] const_cast<char*>(sl[i]);
    delete[] sl;
  }
}

void ncrystalrel_dealloc_string( char* ss )
{
  if (ss)
    delete[] ss;
}

void ncrystalrel_setrandgen( double (*rg)() )
{
  try {
    if (rg)
      NC::setDefaultRNGFctForAllThreads(rg);
    else
      NC::clearDefaultRNG();
  } NCCATCH;
}

void ncrystalrel_setbuiltinrandgen()
{
  try {
    NC::setDefaultRNG( NC::createBuiltinRNG() );
  } NCCATCH;
}

void ncrystalrel_setbuiltinrandgen_withseed(unsigned long seed)
{
  try {
    NC::setDefaultRNG( NC::createBuiltinRNG( static_cast<uint64_t>(seed) ) );
  } NCCATCH;
}

void ncrystalrel_setbuiltinrandgen_withstate(const char* state)
{
  try {
    nc_assert_always(state!=nullptr);
    if ( ! NC::stateIsFromBuiltinRNG(NC::RNGStreamState{state}) )
      NCRYSTAL_THROW2(BadInput,"ncrystalrel_setbuiltinrandgen_withstate got state which is not from NCrystal's builtin RNG: "<<state);
    setDefaultRNG( NC::createBuiltinRNG(NC::RNGStreamState{state}) );
  } NCCATCH;
}

int ncrystalrel_rngsupportsstatemanip_ofscatter( ncrystalrel_scatter_t sh )
{
  try {
    auto rng = ncc::extract(sh).rngSO().tryDynCast<NC::RNGStream>();
    return ( rng != nullptr && rng->supportsStateManipulation() ) ? 1 : 0;
  } NCCATCH;
  return 0;
}

char* ncrystalrel_getrngstate_ofscatter(ncrystalrel_scatter_t sh)
{
  try {
    auto rng = ncc::extract(sh).rngSO().tryDynCast<NC::RNGStream>();
    if ( rng == nullptr || ! rng->supportsStateManipulation() )
      return nullptr;
    return ncc::createString(rng->getState().get());
  } NCCATCH;
  return nullptr;
}

char* ncrystalrel_decodecfg_json( const char * cfgstr )
{
  try {
    NC::MatCfg cfg(cfgstr);
    return ncc::createString(cfg.toJSONCfg());
  } NCCATCH;
  return nullptr;
}

char * ncrystalrel_dbg_process( ncrystalrel_process_t o )
{
  try {
    return ncc::createString(ncc::extractProcess(o).underlying().jsonDescription());
  } NCCATCH;
  return nullptr;
}

char* ncrystalrel_normalisecfg( const char * cfgstr )
{
  try {
    NC::MatCfg cfg(cfgstr);
    return ncc::createString(cfg.toStrCfg());
  } NCCATCH;
  return nullptr;
}

void ncrystalrel_setrngstate_ofscatter(ncrystalrel_scatter_t sh,const char* state_raw)
{
  try {
    nc_assert_always(state_raw!=nullptr);
    NC::RNGStreamState state{state_raw};
    auto& sc = ncc::extract(sh);
    if ( NC::stateIsFromBuiltinRNG(state) ) {
      auto rng = NC::createBuiltinRNG(state);
      sc.replaceRNGAndUpdateProducer(rng);
    } else {
      auto rng = sc.rngSO().tryDynCast<NC::RNGStream>();
      if ( rng == nullptr )
        NCRYSTAL_THROW(CalcError,"ncrystalrel_setrngstate_ofscatter ERROR: scatter has RNG source which is not actually derived from RNGStream.");
      if ( ! rng->supportsStateManipulation() )
        NCRYSTAL_THROW(CalcError,"ncrystalrel_setrngstate_ofscatter ERROR: scatter has RNG source which does not support state manipulation.");
      rng->setState(state);
      sc.replaceRNGAndUpdateProducer(rng);
    }
  } NCCATCH;
}

char** ncrystalrel_get_text_data( const char * name )
{
  try {
    //Adding: [contents, uid(as string), datasourcename,  resolvedphyspath]
    auto td = NC::FactImpl::createTextData( name );
    NC::VectS strlist;
    strlist.reserve(5);
    strlist.emplace_back(td->rawData().begin(),std::distance(td->rawData().begin(),td->rawData().end()));
    std::ostringstream suid;
    suid << td->dataUID().value() <<std::endl;
    strlist.emplace_back(suid.str());
    strlist.emplace_back(td->dataSourceName().str());
    strlist.emplace_back(td->dataType());
    if ( td->getLastKnownOnDiskLocation().has_value() )
      strlist.emplace_back( td->getLastKnownOnDiskLocation().value() );
    else
      strlist.emplace_back();
    //convert and return:
    char ** strs;
    unsigned nstrs;
    ncc::createStringList(strlist,&strs,&nstrs);
    nc_assert_always(nstrs==5);
    return strs;
  } NCCATCH;
  return nullptr;
}

void ncrystalrel_get_file_list( unsigned* nstrs, char*** strs )
{
  //Return list of: ["name", "source", "factname", "priority", "name", ...]
  //(priority is str(integer), "Unable", or "OnlyOnExplicitRequest")
  try {
    auto fl = NC::DataSources::listAvailableFiles();
    NC::VectS strlist;
    strlist.reserve(4*fl.size());
    for ( auto& e : fl ) {
      strlist.emplace_back(e.name);
      strlist.emplace_back(e.source);
      strlist.emplace_back(e.factName);
      if (!e.priority.canServiceRequest())
        strlist.emplace_back("Unable");
      else if ( e.priority.needsExplicitRequest() )
        strlist.emplace_back("OnlyOnExplicitRequest");
      else
        strlist.emplace_back(std::to_string(e.priority.priority()));
    }
    ncc::createStringList(strlist,strs,nstrs);
  } NCCATCH;
}

void ncrystalrel_get_plugin_list( unsigned* nstrs,
                               char*** strs )
{
  try {
    auto plugins = NC::Plugins::loadedPlugins();
    NC::VectS strlist;
    strlist.reserve( 3 * plugins.size() );
    for ( auto& e : plugins ) {
      nc_assert_always( e.pluginType==NC::Plugins::PluginType::Dynamic
                        || e.pluginType==NC::Plugins::PluginType::Builtin);
      std::string ptypestr( e.pluginType==NC::Plugins::PluginType::Dynamic
                            ? "dynamic" : "builtin" );
      strlist.emplace_back(e.pluginName);
      strlist.emplace_back(e.fileName);
      strlist.emplace_back(ptypestr);
    }
    ncc::createStringList(strlist,strs,nstrs);
  } NCCATCH;
}

char* ncrystalrel_get_file_contents( const char * name )
{
  try {
    auto textData = NC::FactImpl::createTextData( name );
    return ncc::createString(textData->rawData().begin(), textData->rawData().end());
  } catch ( NC::Error::FileNotFound& e ) {
    return nullptr;
  } catch ( NC::Error::DataLoadError& e ) {
    return nullptr;
  } NCCATCH;
  return nullptr;
}


void ncrystalrel_add_custom_search_dir( const char * dir )
{
  try {
    nc_assert_always(dir);
    NC::DataSources::addCustomSearchDirectory( std::string(dir) );
  } NCCATCH;
}

void ncrystalrel_remove_all_data_sources()
{
  try {
    NC::DataSources::removeAllDataSources();
  } NCCATCH;
}


void ncrystalrel_remove_custom_search_dirs( )
{
  try {
    NC::DataSources::removeCustomSearchDirectories();
  } NCCATCH;
}

void ncrystalrel_enable_abspaths( int b )
{
  try {
    NC::DataSources::enableAbsolutePaths((bool)b);
  } NCCATCH;
}

void ncrystalrel_enable_relpaths( int b )
{
  try {
    NC::DataSources::enableRelativePaths((bool)b);
  } NCCATCH;
}

void ncrystalrel_enable_stddatalib( int b, const char * dir )
{
  try {
    NC::Optional<std::string> path;
    if (dir)
      path = std::string(dir);
    NC::DataSources::enableStandardDataLibrary((bool)b,path);
  } NCCATCH;
}

void ncrystalrel_enable_stdsearchpath( int b )
{
  try {
    NC::DataSources::enableStandardSearchPath((bool)b);
  } NCCATCH;
}

double ncrystalrel_debyetemp2msd( double debyetemp, double temperature, double mass )
{
  try {
    return NC::debyeIsotropicMSD( NC::DebyeTemperature{debyetemp}, NC::Temperature{temperature}, NC::AtomMass{mass} );
  } NCCATCH;
  return -1.0;
}

double ncrystalrel_msd2debyetemp( double msd, double temperature, double mass )
{
  try {
    return NC::debyeTempFromIsotropicMSD( msd, NC::Temperature{temperature}, NC::AtomMass{mass} ).dbl();
  } NCCATCH;
  return -1.0;
}

void ncrystalrel_vdoseval( double vdos_emin, double vdos_emax,
                        unsigned vdos_ndensity, const double* vdos_density,
                        double temperature, double atom_mass_amu,
                        double* msd, double* debye_temp, double* gamma0,
                        double* temp_eff, double* origIntegral )
{
  try {
    *msd = *debye_temp = *gamma0 = *temp_eff = *origIntegral = -1.0;
    NC::VectD density;
    density.reserve(vdos_ndensity);
    for ( auto i : NC::ncrange(vdos_ndensity) )
      density.push_back( vdos_density[i] );
    NC::VDOSData vd( NC::PairDD{vdos_emin,vdos_emax},
                     std::move(density),
                     NC::Temperature{temperature},
                     NC::SigmaBound{1.0},//doesn't matter for VDOSEval
                     NC::AtomMass{atom_mass_amu} );
    NC::VDOSEval ve(vd);
    double res_oi = ve.originalIntegral();
    double res_te = ve.calcEffectiveTemperature();
    double res_g0 = ve.calcGamma0();
    double res_msd = ve.getMSD( res_g0 );
    double res_dt = NC::debyeTempFromIsotropicMSD( res_msd,
                                                   NC::Temperature{temperature},
                                                   NC::AtomMass{atom_mass_amu} ).dbl();
    *msd = res_msd;
    *debye_temp = res_dt;
    *gamma0 = res_g0;
    *temp_eff = res_te;
    *origIntegral = res_oi;
  } NCCATCH;
}

char * ncrystalrel_process_uid( ncrystalrel_process_t cproc )
{
  try {
    std::ostringstream suid;
    suid << ncc::extractProcess(cproc).underlying().getUniqueID().value;
    return ncc::createString(suid.str());
  } NCCATCH;
  return nullptr;
}

char * ncrystalrel_info_uid( ncrystalrel_info_t ci )
{
  try {
    auto& info = ncc::extract(ci);
    std::ostringstream suid;
    suid << info->getUniqueID().value;
    return ncc::createString(suid.str());
  } NCCATCH;
  return nullptr;
}

char * ncrystalrel_info_underlyinguid( ncrystalrel_info_t ci )
{
  try {
    std::ostringstream suid;
    suid << ncc::extract(ci)->detail_getUnderlyingUniqueID().value;
    return ncc::createString(suid.str());
  } NCCATCH;
  return nullptr;
}

char * ncrystalrel_gencfgstr_doc(int mode)
{
  try {
    std::ostringstream ss;
    switch (mode) {
    case 0: NC::MatCfg::genDoc(ss,NC::MatCfg::GenDocMode::TXT_FULL); break;
    case 1: NC::MatCfg::genDoc(ss,NC::MatCfg::GenDocMode::TXT_SHORT); break;
    case 2: NC::MatCfg::genDoc(ss,NC::MatCfg::GenDocMode::JSON); break;
    default:
      NCRYSTAL_THROW2(BadInput,"Invalid mode " << mode
                      << " passed to ncrystalrel_gencfgstr_doc (must be 0, 1, or 2)");
    };
    return ncc::createString(ss.str());
  } NCCATCH;
  return nullptr;
}

NCRYSTAL_API char * ncrystalrel_ncmat2json( const char * textdataname )
{
  try {
    auto textData = NC::FactImpl::createTextData( textdataname );
    auto data_mut = NC::parseNCMATData( textData,
                                        true /*doFinalValidation*/ );
    const auto& data = data_mut;
    std::ostringstream ss;
    data.toJSON(ss);
    return ncc::createString(ss.str());
  } NCCATCH;
  return nullptr;
}
