#ifndef NCrystalRel_AtomData_hh
#define NCrystalRel_AtomData_hh

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

#include "NCrystalRel/NCTypes.hh"

namespace NCrystalRel {

  class AtomData;

  using AtomDataSP = shared_obj<const AtomData>;
  using OptionalAtomDataSP = std::shared_ptr<const AtomData>;

  class NCRYSTAL_API AtomData : public UniqueID {
  public:

    //Immutable data structure providing physical constants related to a
    //particular mix of isotopes. This can be used to represent elements
    //(i.e. all isotopes having same Z) in either natural or enriched form, but
    //can also be used to represent atoms in doped crystals. E.g. if a small
    //fraction (0.1%) of Cr-ions replace some Al-ions in a Al2O3 lattice, the
    //AtomData could represent a mix of 0.1% Cr and 99.9% Al.

    //////////////////////////////
    //  Cross sections and mass //
    //////////////////////////////

    //Cross sections are bound cross sections with a unit of barn, and values
    //are combined from the constituents in the appropriate manner (which is
    //averaging for mass and coherentScatLen, and a proper variance calculation
    //for incoherentXS). Note that we use sqrt(barn)=10fm for the unit of
    //scattering lengths, which is 10 times higher than the unit of fm often
    //used in the field. With these units xs=4pi*(scatlen)^2.

    AtomMass averageMassAMU() const;//in Daltons
    double coherentScatLen() const;//units of sqrt(barn)=10fm
    double coherentScatLenFM() const;//same in fm
    SigmaBound coherentXS() const;//barn
    SigmaBound incoherentXS() const;//barn
    SigmaBound scatteringXS() const;//barn [=coherentXS()+incoherentXS()]
    SigmaAbsorption captureXS() const;//barn, value at v_neutron = 2200m/s

    //Free rather than bound cross sections. The free cross sections are
    //obtained by multiplying the bound ones with (A/(1+A))^2 where A is the
    //mass relative to the neutron mass.
    SigmaFree freeScatteringXS() const;
    SigmaFree freeCoherentXS() const;
    SigmaFree freeIncoherentXS() const;

    //////////////////////////////////////
    //  Composition-related information //
    //////////////////////////////////////

    //Type (as boolean check or as enum):
    enum class AtomDataType { NatElem, SingleIsotope, Composite };
    AtomDataType getType() const;
    bool isNaturalElement() const;
    bool isSingleIsotope() const;
    bool isComposite() const;

    //A collection of atoms is considered an element, if all atoms share the same Z value:
    bool isElement() const;

    //Only call these after checking that isElement() is true:
    std::string elementName() const;
    unsigned Z() const;

    //Only call after checking that isSingleIsotope() is true:
    unsigned A() const;

    //Only call if isComposite() is true::
    struct NCRYSTAL_API Component {
      double fraction = -1.0;
      AtomDataSP data;
      Component(double fr,AtomDataSP);//constructor needed for C++11
    };
    unsigned nComponents() const;
    const Component& getComponent(unsigned iComponent) const;

    ////////////////////////////////
    // Human-readable description //
    ////////////////////////////////

    //For human consumption only, do NOT use to determine values or behaviour of
    //algorithms!! The only guarantee is description(false) will return exactly
    //the element name for a natural element and a symbol like "H2" or "Al26"
    //for a single isotope (not aliases like "D" or "T"). However, in general
    //code should not rely on this!!:
    std::string description(bool includeValues=true) const;
    void descriptionToStream(std::ostream&,bool includeValues=true) const;

    ///////////////////////////////
    // Constructors and plumbing //
    ///////////////////////////////

    //Natural elements (provide just Z) or single isotopes (provide both Z and A):
    AtomData( SigmaBound incXS, double cohSL, SigmaAbsorption captureXS, AtomMass avrMassAMU, unsigned Z, unsigned A=0 );

    //Composite:
    using ComponentList = std::vector<Component>;
    AtomData( const ComponentList& components );

    AtomData ( AtomData && ) = default;
    AtomData & operator= ( AtomData && ) = default;
    AtomData ( const AtomData & ) = delete;
    AtomData & operator= ( const AtomData & ) = delete;
    AtomData() = delete;
    ~AtomData();
    bool operator <(const AtomData &) const;//Sorts roughly by (Z,A,description,uniqueid)

    ///////////////////////////////////////////////////////////////////////////////
    // Check if instance has values and composition identical to that of another //
    // instance (within indicated relative and absolute tolerances in case of    //
    // floating point numbers. In case of components, their order matter.        //
    ///////////////////////////////////////////////////////////////////////////////
    bool sameValuesAs(const AtomData&, double rtol=1.0e-10, double atol=1.0e-10) const;

    //Hash value:
    std::size_t hash() const;

    /////////////////////////////
    // Z<->elementName mapping //
    /////////////////////////////

    static const std::string& elementZToName(unsigned);//returns empty string if invalid
    static unsigned elementNameToZ(const std::string&);//returns 0 if invalid


  private:
    AtomMass m_m;
    double m_ixs, m_csl, m_axs;
    Component * m_components = nullptr;
    int16_t m_classify;//>=1: 1 isotope with that A, 0: nat. elem., <0: composite with -m_classify parts.
    uint16_t m_z;//0 if not element
    struct Impl;
    friend struct Impl;
  };

  inline std::ostream& operator<< (std::ostream& os, const AtomData& data)
  {
    return data.descriptionToStream(os), os;
  }

}


////////////////////////////
// Inline implementations //
////////////////////////////

inline bool NCrystalRel::AtomData::isElement() const
{
  return m_z>0;
}

inline unsigned NCrystalRel::AtomData::Z() const
{
  nc_assert(isElement());
  return m_z;
}

inline unsigned NCrystalRel::AtomData::A() const
{
  nc_assert(isSingleIsotope());
  return m_classify;
}

inline NCrystalRel::AtomData::AtomDataType NCrystalRel::AtomData::getType() const
{
  if (isNaturalElement())
    return NCrystalRel::AtomData::AtomDataType::NatElem;
  return isComposite() ? NCrystalRel::AtomData::AtomDataType::Composite : NCrystalRel::AtomData::AtomDataType::SingleIsotope;
}

inline bool NCrystalRel::AtomData::isNaturalElement() const
{
  return m_classify==0;
}

inline bool NCrystalRel::AtomData::isSingleIsotope() const
{
  return m_classify>0;
}

inline bool NCrystalRel::AtomData::isComposite() const
{
  return m_classify<0;
}

inline NCrystalRel::AtomMass NCrystalRel::AtomData::averageMassAMU() const
{
  return m_m;
}

inline NCrystalRel::SigmaBound NCrystalRel::AtomData::incoherentXS() const
{
  return NCrystalRel::SigmaBound{m_ixs};
}

inline double NCrystalRel::AtomData::coherentScatLen() const
{
  return m_csl;
}

inline double NCrystalRel::AtomData::coherentScatLenFM() const
{
  return m_csl*10.0;//10 is sqrt(barn)/fm
}

inline NCrystalRel::SigmaBound NCrystalRel::AtomData::coherentXS() const
{
  return NCrystalRel::SigmaBound{m_csl*m_csl*k4Pi};
}

inline NCrystalRel::SigmaBound NCrystalRel::AtomData::scatteringXS() const
{
  return NCrystalRel::SigmaBound{m_ixs + coherentXS().get()};
}

inline NCrystalRel::SigmaAbsorption NCrystalRel::AtomData::captureXS() const
{
  return SigmaAbsorption{ m_axs };
}
inline NCrystalRel::SigmaFree NCrystalRel::AtomData::freeScatteringXS() const
{
  return scatteringXS().free(m_m);
}

inline NCrystalRel::SigmaFree NCrystalRel::AtomData::freeCoherentXS() const
{
  return coherentXS().free(m_m);
}

inline NCrystalRel::SigmaFree NCrystalRel::AtomData::freeIncoherentXS() const
{
  return incoherentXS().free(m_m);
}

inline NCrystalRel::AtomData::Component::Component(double fr,AtomDataSP sp)
  : fraction(fr), data(std::move(sp))
{
}

inline unsigned NCrystalRel::AtomData::nComponents() const
{
  return ( m_classify < 0 ? static_cast<unsigned>(-m_classify) : 0 );
}

inline const NCrystalRel::AtomData::Component& NCrystalRel::AtomData::getComponent(unsigned iComponent) const
{
  if ( ! ( m_classify < 0 && iComponent < static_cast<unsigned>(-m_classify) && m_components!=nullptr ) )
    NCRYSTAL_THROW(BadInput,"AtomData::getComponent invalid index requested (out of bounds)");
  return m_components[iComponent];
}
#endif
