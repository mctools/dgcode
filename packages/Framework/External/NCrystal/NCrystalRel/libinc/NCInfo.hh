#ifndef NCrystalRel_Info_hh
#define NCrystalRel_Info_hh

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   //
//                                                                            //
//  Copyright 2015-2023 NCrystal developers                                   //
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

#include "NCrystalRel/NCInfoTypes.hh"

/////////////////////////////////////////////////////////////////////////////////
// Data class containing information (high level or derived) about a given     //
// material. Instances of the class are typically generated by dedicated       //
// factories (with createInfo(..)), based on interpretation of specified input //
// configurations. The Info objects can be queried directly, or physics models //
// in the form of for example Scatter or Absorption instances, can be          //
// indirectly initialised from them (with e.g. createScatter(..))              //
/////////////////////////////////////////////////////////////////////////////////

namespace NCrystalRel {

  class NCRYSTAL_API Info final {

  public:

    /////////////////////////////////////////////////////////////////////////////
    // Info objects might represent either multi- or single phase              //
    // materials. Multi-phase objects contain a list of phases (which might    //
    // themselves be either single or multi-phase objects). Most other fields  //
    // (structure, hkl lists, dynamics, etc.)  will be unavailable on          //
    // multi-phase objects. Exceptions are phase-structure information which   //
    // is only available on multi-phase objects, and fields which are          //
    // available for both multi- and single-phase objects: uid, density,       //
    // composition, and (if well-defined) temperature and state-of-matter.     //
    /////////////////////////////////////////////////////////////////////////////

    //The single/multi-phase nature of an object can of course always be
    //queried:

    bool isSinglePhase() const;
    bool isMultiPhase() const;


    //////////////////////////////////////////////////////////////////////
    //////////////////// Both single- and multi-phase ////////////////////
    //////////////////////////////////////////////////////////////////////

    /////////
    // UID //
    /////////

    UniqueIDValue getUniqueID() const;

    /////////////////////
    // Density [g/cm^3] //
    /////////////////////

    //Both values here are guaranteed to be positive (non-zero) and
    //non-infinite.
    Density getDensity() const;//Unit is g/cm^3
    NumberDensity getNumberDensity() const;//Unit is atoms/Aa^3

    //////////////////////////
    // Basic composition () //
    //////////////////////////

    //On a singlephase Info object, the composition is always consistent with
    //AtomInfo/DynInfo if present, and atoms will have valid indices (useful for
    //passing to the displayLabel method below).

    //On a multiphase object, the composition exists exclusively to provide the
    //basic breakdown of elements and isotopes (i.e. the indices in the
    //IndexedAtomData objects will be invalid).

    struct NCRYSTAL_API CompositionEntry {
      double fraction = -1.0;//Molar fraction (aka by-number-of-atoms fraction)
      IndexedAtomData atom;//NB: will contain invalid index on multiphase object
      CompositionEntry(double fr,IndexedAtomData);//constructor needed for C++11
    };
    typedef std::vector<CompositionEntry> Composition;
    const Composition& getComposition() const;

    //Convenience methods calculating quantities (simple weighted averages over
    //atomic composition):
    SigmaAbsorption getXSectAbsorption() const;
    SigmaFree getXSectFree() const;
    AtomMass getAverageAtomMass() const;
    ScatLenDensity getSLD() const;//Depends on both compostion and numberDensity

    ///////////////////////////
    // Temperature [kelvin]  //
    ///////////////////////////

    //Temperature is optional. It will be unavailable on a multiphase object,
    //unless all phases have temperature available and with identical values.

    bool hasTemperature() const;
    Temperature getTemperature() const;

    /////////////////////
    // State of matter //
    /////////////////////

    //Will be "Unknown" on a multiphase object, unless all phases have the same
    //value (e.g. a multiphase object will be "Solid" if all daughter phases are
    //"Solid").

    enum class NCRYSTAL_API StateOfMatter { Unknown = 0, Solid = 1, Gas = 2, Liquid = 3 };
    StateOfMatter stateOfMatter() const noexcept;
    static std::string toString(StateOfMatter);

    ////////////////////////
    // Configuration data //
    ////////////////////////

    //In order to support advanced material configuration, Info objects are
    //required to keep track of non-Info object related configuration
    //variables. When the Info object is later used to produce Scatter or
    //Absorption objects, these variables are then used to provide defaults. Of
    //course, the caching of underlying costly parts of the Info objects do not
    //involve these variables.
    //
    //NOTE: In case of a multiphase Info object, the CfgData returned will only
    //represent those parameters which are present and set to the same value in
    //all child phases. For a full picture, it is therefore necessary to
    //investigate the cfg data on the child phases as well.
    const Cfg::CfgData& getCfgData() const;

    //Data source name - intended only to be used to make error/verbose messages
    //more meaningful to users.
    const DataSourceName& getDataSourceName() const;

    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// Multiphase /////////////////////////////
    //////////////////////////////////////////////////////////////////////

    //List of Info objects of daughter phases and the (by-volume) fractions of
    //the material:

    using Phase = std::pair<double,shared_obj<const Info>>;
    using PhaseList = std::vector<Phase>;
    const PhaseList& getPhases() const;//empty list if isSinglePhase.

    /////////////////////
    // Phase structure //
    /////////////////////

    //TODO in a future release.



    //////////////////////////////////////////////////////////////////////
    ///////////////////////////// Singlephase ////////////////////////////
    //////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////
    // Information related to crystalline phases //
    ///////////////////////////////////////////////

    // For flexibility, three main pieces of information are available for
    // crystalline phases:
    //
    //   * Structure info (unit cell parameters and spacegroup)
    //   * Atom lists (atoms in unit cell, their positions and optional thermal displacement info)
    //   * HKL lists (list of reflection planes with dspacing and structure factors).
    //
    // All crystalline phases will have HKL lists available (although they might
    // be empty). If Atom lists are available, Structure info will also be available.

    bool isCrystalline() const;

    /////////////////////////////////////////////
    // Crystalline phases: Unit cell structure //
    /////////////////////////////////////////////

    bool hasStructureInfo() const;
    const StructureInfo& getStructureInfo() const;

    //Convenience method, calculating the d-spacing of a given Miller
    //index. Calling this incurs the overhead of creating a reciprocal lattice
    //matrix from the structure info:
    double dspacingFromHKL( int h, int k, int l ) const;

    //////////////////////////////////////////////////////////
    // Crystalline phases: layout of atoms in the unit cell //
    //////////////////////////////////////////////////////////

    bool hasAtomInfo() const;
    const AtomInfoList& getAtomInfos() const;

    //NB: it is ok to call getAtomInfos() list even if hasAtomInfo()==false (the
    //list will simply be empty and the next three hasXXX methods below will
    //return false):

    //Convenience methods which tests for availability of certain fields on the
    //entries in the list of atoms returned by getAtomInfos():

    //1) Whether AtomInfo objects have mean-square-displacements
    //   (a.k.a. "U_iso") available (they will either all have them available,
    //   or none of them will have them available):
    bool hasAtomMSD() const;

    //2) Whether AtomInfo objects have Debye temperatures available (they will
    //   either all have them available, or none of them will have them available):
    bool hasAtomDebyeTemp() const;
    bool hasDebyeTemperature() const { return hasAtomDebyeTemp(); }//Alias

    /////////////////////////////////////////
    // Crystalline phases: HKL Information //
    /////////////////////////////////////////

    bool hasHKLInfo() const;//same as isCrystalline() since we guarantee HKL
                            //planes in all crystalline materials.
    const HKLList& hklList() const;

    //NB: it is ok to access hklList() even if hasHKLInfo()==false (the results
    //will simply be an empty list). However, the converse is not true: an empty
    //list might simply be a result of no valid planes existing in the d-spacing
    //range requested when initialising the material. So it is possible to have
    //the situation where hasHKLInfo() returns true but hklList() is empty.

    //The dspacing limits considered when setting up the material:
    double hklDLower() const;
    double hklDUpper() const;

    //The largest/smallest dspacings in the hklList() (both returns infinity if
    //hklList() is empty):
    double hklDMinVal() const;
    double hklDMaxVal() const;

    //Access just the Bragg threshold. In principle this method returns the same
    //value as 2*hklDMaxVal(), but with two differences: Firstly, it is safe to
    //call even if hasHKL()==false, returning NullOpt (it also returns NullOpt
    //if hasHKL()==true but there are no planes). Secondly, if on-demand HKL
    //lists are not already initialised, this will attempt to calculate them
    //WITHOUT doing a full initialisation. This is done by trying to invoke
    //hklListPartialCalc for suitable large lower dspacing cutoffs:
    Optional<NeutronWavelength> getBraggThreshold() const;

    //What kind of information about plane normals and Miller indices are
    //available in the hklList(). It is guaranteed to be the same for all
    //HKLInfo entries, and will return "Minimal" when hklList() is present but
    //empty. Like getBraggThreshold(), calling this method will not necessarily
    //trigger a full initialisation of the hklList().
    HKLInfoType hklInfoType() const;

    ////////////////////////////////////////////////////////////
    // Crystalline phases: HKL Information - advanced control //
    ////////////////////////////////////////////////////////////

    //As HKL list initialisation can be computationally expensive, it might be
    //the case that HKL lists internally are only created on-demand. Invoking
    //hklList(), hklDMinVal(), or hklDMaxVal(), will trigger this
    //initialisation. This method can be used to check if the initialisation is
    //fully done:
    bool hklListIsFullyInitialised() const;

    //Additionally (for advanced usage such as fast browsing), the following
    //method can be used to directly create HKL lists covering a shorter
    //dspacing range. If the factory does not support on-demand creation,
    //NullOpt is returned:
    Optional<HKLList> hklListPartialCalc( Optional<double> dlower = NullOpt,
                                          Optional<double> dupper = NullOpt ) const;

    /////////////////////////////////////////
    // Information about material dynamics //
    /////////////////////////////////////////

    bool hasDynamicInfo() const;
    const DynamicInfoList& getDynamicInfoList() const;

    ///////////////////////////
    // Atomic display labels //
    ///////////////////////////

    // The same atom might play different roles in single-phase objects, which
    // is why they are identified not only by their atom data, but also by an
    // additional index. To distinguish them in textual output, the following
    // methods can be used to provide them unique display labels (e.g. "Al(a)"
    // and "Al(b)" for a material with two roles of aluminium).

    const std::string& displayLabel(const AtomIndex& ai) const;


    /////////////////////////////////////////////////////////////////////////////////
    // Provides calculation of "background" (non-Bragg diffraction) cross-sections //
    /////////////////////////////////////////////////////////////////////////////////

    //NB: Mostly for reference, not intended for general usage.
    bool providesNonBraggXSects() const;
    CrossSect xsectScatNonBragg(NeutronEnergy) const;


    //////////////////////////////////////////////////////
    // All AtomData instances connected to object, by   //
    // index (allows efficient AtomDataSP<->index map.  //
    // (this is used to build the C/Python interfaces). //
    //////////////////////////////////////////////////////

    AtomDataSP atomDataSP( const AtomIndex& ai ) const;
    const AtomData& atomData( const AtomIndex& ai ) const;
    IndexedAtomData indexedAtomData( const AtomIndex& ai ) const;

    ////////////////////////////////////////////////////////////////////////////
    // Custom information for which the core NCrystal code does not have any  //
    // specific treatment. This is primarily intended as a place to put extra //
    // data needed while developing new physics models (e.g. in plugins). The //
    // core NCrystal code should in principle never make use of such custom   //
    // data, although from time to time we might introduce quick workarounds  //
    // based on such custom sections, but such workarounds are likely to stop //
    // working in new releases.                                               //
    ////////////////////////////////////////////////////////////////////////////

    //Data is stored as an ordered list of named "sections", with each section
    //containing "lines" which can consist of 1 or more "words" (strings).
    using CustomLine = VectS;
    using CustomSectionData = std::vector<CustomLine> ;
    using CustomSectionName = std::string;
    using CustomData = std::vector<std::pair<CustomSectionName,CustomSectionData>> ;
    const CustomData& getAllCustomSections() const;

    //Convenience (count/access specific section):
    unsigned countCustomSections(const CustomSectionName& sectionname ) const;
    const CustomSectionData& getCustomSection( const CustomSectionName& name,
                                               unsigned index=0 ) const;

    ////////////////////////////////////////////////////////////////////////
    // Copy/move semantics are as cheap as for one or two shared pointers //
    ////////////////////////////////////////////////////////////////////////

    Info( const Info& ) = default;
    Info& operator=( const Info& ) = default;
    Info( Info&& ) = default;
    Info& operator=( Info&& ) = default;

    ////////////////////////////////////////////////////////////////////////////
    // Info objects can only be directly constructed with the help of the     //
    // interface provided in NCInfoBuilder.hh. However, most NCrystal users   //
    // will not directly construct Info objects, but will instead access them //
    // via a call to createInfo from NCFact.hh.                               //
    ////////////////////////////////////////////////////////////////////////////

    Info() = delete;

    //////////////////////////////////////////////////////////////
    // Internal infrastructure only accessible to NCInfoBuilder //
    //////////////////////////////////////////////////////////////

    struct InternalState;
    Info( InternalState&& );
    InternalState copyInternalState() const;

    //////////////////////////////////////////////////////////////////
    // Obsolete functions which will be removed in a future release //
    //////////////////////////////////////////////////////////////////

    bool hasAtomPositions() const { return hasAtomInfo(); }
    bool hasAnyDebyeTemperature() const { return hasAtomDebyeTemp(); };
    bool hasXSectFree() const { return true; }
    bool hasXSectAbsorption() const { return true; }
    bool hasDensity() const { return true; }
    bool hasNumberDensity() const { return true; }
    unsigned nHKL() const;
    HKLList::const_iterator hklBegin() const;
    HKLList::const_iterator hklLast() const;
    HKLList::const_iterator hklEnd() const;
    AtomInfoList::const_iterator atomInfoBegin() const;
    AtomInfoList::const_iterator atomInfoEnd() const;
    bool hasComposition() const { return true; }

    //////////////////////////////////////////////////////////////////////////////////////
    // Internals. Note that private members are just two shared ptrs, the second of     //
    // which is usually a nullptr (only active when densities/cfgdata are overridden).  //
    //////////////////////////////////////////////////////////////////////////////////////

    //access "underlying" object, i.e. with density/cfgdata overrides.
    bool detail_hasDifferentUnderlying() const noexcept;
    static shared_obj<const Info> detail_copyUnderlying( const shared_obj<const Info>& );
    Info detail_accessUnderlying() const;
    UniqueIDValue detail_getUnderlyingUniqueID() const;
    //phase list shr ptr (if multiphase):
    shared_obj<const PhaseList> detail_getPhasesSP() const;

  public:
    struct Data;
    struct OverrideableData;
    struct OverrideableDataFields;
    const OverrideableDataFields& ovrData() const noexcept;
  private:
    struct clone_underlying_t {};
    Info( clone_underlying_t, const Info& );
    shared_obj<const Data> m_data;
    optional_shared_obj<const OverrideableData> m_oData;
    void singlePhaseOnly(const char*) const;
    void singlePhaseOnlyRaiseError(const char*) const;
  };

  using InfoPtr = shared_obj<const Info>;
  using OptionalInfoPtr = optional_shared_obj<const Info>;
}


////////////////////////////
// Inline implementations //
////////////////////////////

namespace NCrystalRel {

  struct NCRYSTAL_API Info::OverrideableDataFields final {
    //NB: It is not trivial to add more fields to this structs. Adding fields
    //would require NCInfoBuilder.cc code to be carefully updated, and
    //assumptions in the ProcessRequestBase(internal_t,...) constructor should
    //be revisited.
    Density density;
    NumberDensity numberdensity;
    Cfg::CfgData cfgData;
    optional_shared_obj<const PhaseList> phases;
  };

  struct NCRYSTAL_API Info::OverrideableData final : private NoCopyMove {
    //Fields + UID
    UniqueID uid;
    OverrideableDataFields fields;
  };

  struct NCRYSTAL_API Info::Data final : private NoCopyMove {
    //Single-phase only:
    Optional<StructureInfo> structinfo;
    AtomInfoList atomlist;
    DynamicInfoList dyninfolist;
    std::function<CrossSect(NeutronEnergy)> xsectprovider;
    CustomData custom;
    std::vector<AtomDataSP> atomDataSPs;
    VectS displayLabels;
    //Single-phase only HKLLists:
    Optional<PairDD> hkl_dlower_and_dupper;//dspacing range. No value means HKL lists unavailable.
    std::function<HKLList(PairDD)> hkl_ondemand_fct;
    mutable std::atomic<bool> detail_hkllist_needs_init;
    mutable HKLList detail_hklList;
    mutable std::atomic<double> detail_braggthreshold;//-1: needs init, =0: N/A, >0: the value

    //HKLInfoType as atomic integer (using hKLInfoTypeInt_unsetval if needs init):
    static constexpr std::underlying_type<HKLInfoType>::type hKLInfoTypeInt_unsetval = 9999;
    mutable std::atomic<unsigned> detail_hklInfoType;
    static_assert( enumAsInt(HKLInfoType::SymEqvGroup) != hKLInfoTypeInt_unsetval, "" );
    static_assert( enumAsInt(HKLInfoType::ExplicitHKLs) != hKLInfoTypeInt_unsetval, "" );
    static_assert( enumAsInt(HKLInfoType::ExplicitNormals) != hKLInfoTypeInt_unsetval, "" );
    static_assert( enumAsInt(HKLInfoType::Minimal) != hKLInfoTypeInt_unsetval, "" );


    void doInitHKLList() const;
    const HKLList& hklList() const
    {
      if (!hkl_dlower_and_dupper.has_value())
        NCRYSTAL_THROW(LogicError,"Do not access hklList() on Info object which does not represent a crystalline material");
      if ( detail_hkllist_needs_init.load() )
        doInitHKLList();
      return detail_hklList;
    }

    //Both single and multi-phase:
    DataSourceName dataSourceName;
    StateOfMatter stateOfMatter = StateOfMatter::Unknown;
    Optional<Temperature> temp;
    Composition composition;
    OverrideableData oData;

    //MT-safe cache (to be manipulated only by the InfoBuilder code. This gives
    //a way to override data while always getting same UID for same overridden
    //fields. This cache is kept here, since it should have lifetime no greater
    //than the associated Data object:
    struct ODataCache {
      std::mutex mtx;
      std::vector<shared_obj<const OverrideableData>> entries;
    };
    mutable ODataCache oDataCache;//NB: Mutable! (always lock the mutex inside before accessing)

  };

  inline const DataSourceName& Info::getDataSourceName() const { return m_data->dataSourceName; }

  inline const Info::OverrideableDataFields& Info::ovrData() const noexcept
  {
    return m_oData == nullptr ? m_data->oData.fields : m_oData->fields;
  }

  inline UniqueIDValue Info::getUniqueID() const {
    return ( m_oData == nullptr ? m_data->oData.uid.getUniqueID() : m_oData->uid.getUniqueID() );
  }

  inline UniqueIDValue Info::detail_getUnderlyingUniqueID() const { return m_data->oData.uid.getUniqueID(); }
  inline bool Info::isSinglePhase() const { return ovrData().phases == nullptr; }
  inline bool Info::isMultiPhase() const { return ovrData().phases != nullptr; }
  namespace detail {
    const Info::PhaseList& getEmptyPL();
  }
  inline const Info::PhaseList& Info::getPhases() const { auto ph = ovrData().phases; return ph == nullptr ? detail::getEmptyPL() : *ph; }
  inline shared_obj<const Info::PhaseList> Info::detail_getPhasesSP() const
  {
    const auto& phs = ovrData().phases;
    nc_assert( phs != nullptr );
    return phs;
  }
  inline Info::StateOfMatter Info::stateOfMatter() const noexcept { return m_data->stateOfMatter; }
  inline bool Info::isCrystalline() const { singlePhaseOnly(__func__); return hasHKLInfo(); }
  inline bool Info::hasStructureInfo() const { singlePhaseOnly(__func__); return m_data->structinfo.has_value(); }
  inline const StructureInfo& Info::getStructureInfo() const  { singlePhaseOnly(__func__); nc_assert(hasStructureInfo()); return m_data->structinfo.value(); }
  inline bool Info::providesNonBraggXSects() const { singlePhaseOnly(__func__); return !!m_data->xsectprovider; }
  inline CrossSect Info::xsectScatNonBragg(NeutronEnergy ekin) const  { singlePhaseOnly(__func__); nc_assert(!!m_data->xsectprovider); return m_data->xsectprovider(ekin); }
  inline bool Info::hasTemperature() const { return m_data->temp.has_value(); }
  inline const AtomInfoList& Info::getAtomInfos() const { singlePhaseOnly(__func__); return m_data->atomlist; }
  inline Temperature Info::getTemperature() const { nc_assert(hasTemperature()); return m_data->temp.value(); }
  inline bool Info::hasAtomMSD() const { singlePhaseOnly(__func__); return hasAtomInfo() && m_data->atomlist.front().msd().has_value(); }
  inline bool Info::hasAtomDebyeTemp() const { singlePhaseOnly(__func__); return hasAtomInfo() && m_data->atomlist.front().debyeTemp().has_value(); }
  inline bool Info::hasAtomInfo() const  { singlePhaseOnly(__func__); return !m_data->atomlist.empty(); }
  inline AtomInfoList::const_iterator Info::atomInfoBegin() const { singlePhaseOnly(__func__); return m_data->atomlist.begin(); }
  inline AtomInfoList::const_iterator Info::atomInfoEnd() const { singlePhaseOnly(__func__); return m_data->atomlist.end(); }
  inline bool Info::hasHKLInfo() const { singlePhaseOnly(__func__); return m_data->hkl_dlower_and_dupper.has_value(); }
  inline const HKLList& Info::hklList() const { singlePhaseOnly(__func__); return m_data->hklList(); }
  inline bool Info::hklListIsFullyInitialised() const
  {
    singlePhaseOnly(__func__);
    nc_assert(hasHKLInfo());
    return !m_data->detail_hkllist_needs_init.load();
  }
  inline unsigned Info::nHKL() const { singlePhaseOnly(__func__); return m_data->hklList().size(); }
  inline HKLList::const_iterator Info::hklBegin() const { singlePhaseOnly(__func__); return m_data->hklList().begin(); }
  inline HKLList::const_iterator Info::hklLast() const
  {
    singlePhaseOnly(__func__);
    auto& hklList = m_data->hklList();
    return hklList.empty() ? hklList.end() : std::prev(hklList.end());
  }
  inline HKLList::const_iterator Info::hklEnd() const { singlePhaseOnly(__func__); return m_data->hklList().end(); }
  inline double Info::hklDLower() const { singlePhaseOnly(__func__); nc_assert(hasHKLInfo()); return m_data->hkl_dlower_and_dupper.value().first; }
  inline double Info::hklDUpper() const { singlePhaseOnly(__func__); nc_assert(hasHKLInfo()); return m_data->hkl_dlower_and_dupper.value().second; }
  inline Density Info::getDensity() const { return ovrData().density; }
  inline NumberDensity Info::getNumberDensity() const { return ovrData().numberdensity; }
  inline const Cfg::CfgData& Info::getCfgData() const { return ovrData().cfgData; }

  inline bool Info::hasDynamicInfo() const { singlePhaseOnly(__func__); return !m_data->dyninfolist.empty(); }
  inline const DynamicInfoList& Info::getDynamicInfoList() const  { singlePhaseOnly(__func__); return m_data->dyninfolist; }
  inline const Info::CustomData& Info::getAllCustomSections() const { singlePhaseOnly(__func__); return m_data->custom; }
  inline const Info::Composition& Info::getComposition() const { return m_data->composition; }
  inline const std::string& Info::displayLabel(const AtomIndex& ai) const
  {
    singlePhaseOnly(__func__);
    nc_assert(ai.get()<m_data->displayLabels.size());
    return m_data->displayLabels[ai.get()];
  }

  inline AtomDataSP Info::atomDataSP( const AtomIndex& ai ) const
  {
    singlePhaseOnly(__func__);
    nc_assert( ai.get() < m_data->atomDataSPs.size());
    return m_data->atomDataSPs[ai.get()];
  }

  inline const AtomData& Info::atomData( const AtomIndex& ai ) const
  {
    singlePhaseOnly(__func__);
    nc_assert( ai.get() < m_data->atomDataSPs.size());
    return m_data->atomDataSPs[ai.get()];
  }
  inline IndexedAtomData Info::indexedAtomData( const AtomIndex& ai ) const
  {
    singlePhaseOnly(__func__);
    nc_assert( ai.get() < m_data->atomDataSPs.size());
    return IndexedAtomData{m_data->atomDataSPs[ai.get()],{ai}};
  }
  inline Info::CompositionEntry::CompositionEntry(double fr,IndexedAtomData iad)
    : fraction(fr), atom(std::move(iad))
  {
  }
  inline void Info::singlePhaseOnly(const char*fctname) const
  {
    if ( isMultiPhase() )
      singlePhaseOnlyRaiseError(fctname);
  }
  inline bool Info::detail_hasDifferentUnderlying() const noexcept
  {
    return m_oData != nullptr;
  }

  inline Info::Info( clone_underlying_t, const Info& o )
    : m_data( o.m_data )
  {
  }

  inline InfoPtr Info::detail_copyUnderlying( const InfoPtr& infoptr )
  {
    if ( infoptr->m_oData == nullptr )
      return infoptr;
    else
      return makeSO<Info>( infoptr->detail_accessUnderlying() );
  }

  inline Info Info::detail_accessUnderlying() const
  {
    return Info( clone_underlying_t(), *this );
  }
}

#endif
