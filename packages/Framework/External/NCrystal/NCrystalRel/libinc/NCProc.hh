#ifndef NCrystalRel_Proc_hh
#define NCrystalRel_Proc_hh

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

#include "NCrystalRel/NCProcImpl.hh"
#include "NCrystalRel/NCRNG.hh"

namespace NCrystalRel {

  //////////////////////////////////////////////////////////////////////////////
  //                                                                          //
  // Physics processes representing neutron scattering or absorption. The     //
  // interfaces support various strategies related to caching and random      //
  // number streams, which client code can use as desired to balance          //
  // convenience, multi-thread-safety, etc.                                   //
  //                                                                          //
  // The processes are essentially thin convenience wrappers around the       //
  // process implementation classes provided by the NCProcImpl.hh and         //
  // NCFactImpl.hh files, managing also process caches and RNG streams.       //
  // Applications wishing to take direct control of caching and random number //
  // streams can consider using those underlying interfaces instead. If       //
  // implementing multi-threaded applications using the managed classes       //
  // declared here, it is important to use their clone methods to ensure each //
  // thread has it's own clone of Absorption and Scatter objects.             //
  //                                                                          //
  // The classes declared here are most typically created using the factory   //
  // methods from NCFact.hh.                                                  //
  //                                                                          //
  // Refer to the ProcImpl.hh file for further documentation concerning the   //
  // meaning of the classes and methods.                                      //
  //                                                                          //
  //////////////////////////////////////////////////////////////////////////////

  class Scatter;
  class Absorption;

  class NCRYSTAL_API Process : private MoveOnly {
  public:

    MaterialType materialType() const noexcept;
    ProcessType processType() const noexcept;
    const char * name() const noexcept;
    bool isOriented() const noexcept;
    EnergyDomain domain() const noexcept;
    bool isNull() const noexcept;

    CrossSect crossSection( NeutronEnergy, const NeutronDirection& );
    CrossSect crossSectionIsotropic( NeutronEnergy );

    void clearCache();
    ProcImpl::ProcPtr underlyingPtr() const;
    const ProcImpl::Process& underlying() const;

    virtual ~Process() = default;

  protected:
    //Allow move semantics only on derived classes (no slicing):
    Process( Process&& ) = default;
    Process& operator=( Process&& ) = default;
    ProcImpl::ProcPtr m_proc;
    CachePtr m_cachePtr;
  private:
    //Only allow Scatter + Absorption to implement:
    Process( ProcImpl::ProcPtr );
    friend class Scatter;
    friend class Absorption;
  };

  /////////////////////////////////////////////////////////////////////////////////
  class NCRYSTAL_API Absorption final : public Process {
  public:
    //NB: Methods related to e.g. cross section are inherited from Process.

    //Multi-threaded applications should clone the object and work
    //with one cloned object per thread:
    Absorption clone() const;

    //Allow move-semantics:
    Absorption( Absorption&& ) = default;
    Absorption& operator=( Absorption&& ) = default;

    //Constructor (NB: Most users will instead use the global
    //createAbsorption function from NCFact.hh):
    Absorption( ProcImpl::ProcPtr );

  };

  class NCRYSTAL_API Scatter final : public Process {
  public:

    //NB: Methods related to e.g. cross section are inherited from Process.

    //Sample scatterings.
    ScatterOutcome sampleScatter( NeutronEnergy, const NeutronDirection& );
    ScatterOutcomeIsotropic sampleScatterIsotropic( NeutronEnergy );

    //Multi-threaded applications should clone the object and work
    //with one cloned object per thread (will use equivalently named
    //RNGProducer::produceXXX methods to produce new RNG stream for
    //cloned object):
    Scatter clone();
    Scatter cloneByIdx( RNGStreamIndex rngstreamid );
    Scatter cloneForCurrentThread();
    //Other esoteric cloning methods:
    Scatter cloneWithIdenticalRNGSettings();
    Scatter clone( shared_obj<RNGProducer>, shared_obj<RNG> );

    //Access and manipulate RNG state:
    RNG& rng();
    shared_obj<RNG> rngSO();
    RNGProducer& rngproducer();
    shared_obj<RNGProducer> rngproducerSO();
    void replaceRNG( shared_obj<RNG>, shared_obj<RNGProducer> );
    void replaceRNGAndUpdateProducer( shared_obj<RNGStream> );//will reinit current producer (potentially affecting other objects!)

    //Allow move-semantics:
    Scatter( Scatter&& ) = default;
    Scatter &operator=(Scatter &&) = default;

    //Constructor (NB: Most users will instead use the global
    //createScatter function from NCFact.hh):
    Scatter( shared_obj<RNGProducer>, shared_obj<RNG>, ProcImpl::ProcPtr );
  private:
    shared_obj<RNG> m_rng;
    shared_obj<RNGProducer> m_rngproducer;
  };

}


////////////////////////////
// Inline implementations //
////////////////////////////

inline NCrystalRel::Process::Process( ProcImpl::ProcPtr pp ) : m_proc(std::move(pp)) {}
inline NCrystalRel::Absorption::Absorption( ProcImpl::ProcPtr pp) : Process(std::move(pp)) {}
inline NCrystalRel::Scatter::Scatter( shared_obj<RNGProducer> rp, shared_obj<RNG> r, ProcImpl::ProcPtr pp )
  : Process(std::move(pp)), m_rng(std::move(r)), m_rngproducer(std::move(rp)) {}
inline NCrystalRel::ProcImpl::ProcPtr NCrystalRel::Process::underlyingPtr() const { return m_proc; }
inline const NCrystalRel::ProcImpl::Process& NCrystalRel::Process::underlying() const { return *m_proc; }
inline NCrystalRel::MaterialType NCrystalRel::Process::materialType() const noexcept { return m_proc->materialType(); }
inline NCrystalRel::ProcessType NCrystalRel::Process::processType() const noexcept { return m_proc->processType(); }
inline const char * NCrystalRel::Process::name() const noexcept { return m_proc->name(); }
inline bool NCrystalRel::Process::isOriented() const noexcept { return m_proc->isOriented(); }
inline NCrystalRel::EnergyDomain NCrystalRel::Process::domain() const noexcept { return m_proc->domain(); }
inline bool NCrystalRel::Process::isNull() const noexcept { return m_proc->isNull(); }
inline void NCrystalRel::Process::clearCache() { m_cachePtr.reset(); }
inline NCrystalRel::CrossSect NCrystalRel::Process::crossSection( NeutronEnergy ekin, const NeutronDirection& dir )
{ return m_proc->crossSection(m_cachePtr,ekin,dir); }
inline NCrystalRel::CrossSect NCrystalRel::Process::crossSectionIsotropic( NeutronEnergy ekin )
{ return m_proc->crossSectionIsotropic(m_cachePtr,ekin); }
inline NCrystalRel::shared_obj<NCrystalRel::RNG> NCrystalRel::Scatter::rngSO() { return m_rng; }
inline NCrystalRel::RNG& NCrystalRel::Scatter::rng() { return m_rng; }
inline NCrystalRel::shared_obj<NCrystalRel::RNGProducer> NCrystalRel::Scatter::rngproducerSO() { return m_rngproducer; }
inline NCrystalRel::RNGProducer& NCrystalRel::Scatter::rngproducer() { return m_rngproducer; }
inline NCrystalRel::ScatterOutcome NCrystalRel::Scatter::sampleScatter( NeutronEnergy ekin, const NeutronDirection& dir )
{ return m_proc->sampleScatter(m_cachePtr,m_rng,ekin,dir); }
inline NCrystalRel::ScatterOutcomeIsotropic NCrystalRel::Scatter::sampleScatterIsotropic( NeutronEnergy ekin )
{ return m_proc->sampleScatterIsotropic(m_cachePtr,m_rng,ekin); }

#endif
