#include "ExprParser/ASTStdPhys.hh"
#include "Core/Units.hh"
#include "Core/Constants.hh"

namespace ExprParser {

  ExprEntityPtr create_standard_unit(const str_type& name)
  {
    if (name.empty())
      return ExprEntityPtr();

    switch (name[0]) {
#     define UNIT(u) { if (name==#u) return create_constant(Units::u); };
    case 'A':
      UNIT(Aa); break;
    case 'a':
      UNIT(ampere);
      UNIT(angstrom);
      UNIT(atmosphere); break;
    case 'b':
      UNIT(bar);
      UNIT(barn);
      UNIT(becquerel); break;
    case 'c':
      UNIT(candela);
      UNIT(centimeter);
      UNIT(centimeter2);
      UNIT(centimeter3);
      UNIT(cm);
      UNIT(cm2);
      UNIT(cm3);
      UNIT(coulomb);
      UNIT(curie); break;
    case 'd':
      UNIT(deg);
      UNIT(degree); break;
    case 'e':
      UNIT(electronvolt);
      UNIT(eplus);
      UNIT(e_SI);
      UNIT(eV); break;
    case 'f':
      UNIT(farad);
      UNIT(fermi); break;
    case 'g':
      UNIT(g);
      UNIT(gauss);
      UNIT(gigaelectronvolt);
      UNIT(gram);
      UNIT(gray); break;
    case 'G':
      UNIT(GeV); break;
    case 'h':
      UNIT(henry);
      UNIT(hertz); break;
    case 'j':
      UNIT(joule); break;
    case 'k':
      UNIT(kelvin);
      UNIT(keV);
      UNIT(kg);
      UNIT(kiloelectronvolt);
      UNIT(kilogauss);
      UNIT(kilogram);
      UNIT(kilogray);
      UNIT(kilohertz);
      UNIT(kilometer);
      UNIT(kilometer2);
      UNIT(kilometer3);
      UNIT(kilovolt);
      UNIT(km);
      UNIT(km2);
      UNIT(km3); break;
    case 'l':
      UNIT(lumen);
      UNIT(lux); break;
    case 'm':
      UNIT(m);
      UNIT(m2);
      UNIT(m3);
      UNIT(megaelectronvolt);
      UNIT(megahertz);
      UNIT(megavolt);
      UNIT(meter);
      UNIT(meter2);
      UNIT(meter3);
      UNIT(meV);
      UNIT(mg);
      UNIT(microampere);
      UNIT(microbarn);
      UNIT(microfarad);
      UNIT(microgray);
      UNIT(micrometer);
      UNIT(microsecond);
      UNIT(milliampere);
      UNIT(millibarn);
      UNIT(millielectronvolt);
      UNIT(millifarad);
      UNIT(milligram);
      UNIT(milligray);
      UNIT(millimeter);
      UNIT(millimeter2);
      UNIT(millimeter3);
      UNIT(milliradian);
      UNIT(millisecond);
      UNIT(mm);
      UNIT(mm2);
      UNIT(mm3);
      UNIT(mole);
      UNIT(mrad);
      UNIT(ms); break;
    case 'M':
      UNIT(MeV); break;
    case 'n':
      UNIT(nanoampere);
      UNIT(nanobarn);
      UNIT(nanofarad);
      UNIT(nanometer);
      UNIT(nanosecond);
      UNIT(newton);
      UNIT(nm);
      UNIT(ns); break;
    case 'o':
      UNIT(ohm); break;
    case 'p':
      UNIT(parsec);
      UNIT(pascal);
      UNIT(pc);
      UNIT(perCent);
      UNIT(perMillion);
      UNIT(perThousand);
      UNIT(petaelectronvolt);
      UNIT(picobarn);
      UNIT(picofarad);
      UNIT(picosecond); break;
    case 'P':
      UNIT(PeV); break;
    case 'r':
      UNIT(rad);
      UNIT(radian); break;
    case 's':
      UNIT(s);
      UNIT(second);
      UNIT(sr);
      UNIT(steradian); break;
    case 't':
      UNIT(teraelectronvolt);
      UNIT(tesla); break;
    case 'T':
      UNIT(TeV); break;
    case 'u':
      UNIT(um); break;
    case 'v':
      UNIT(volt); break;
    case 'w':
      UNIT(watt);
      UNIT(weber); break;
    }
    return ExprEntityPtr();
  }

  ExprEntityPtr create_standard_constant(const str_type& name)
  {
    if (name.empty())
      return ExprEntityPtr();

    switch (name[0]) {
#     define CONSTANT(u) { if (name==#u) return create_constant(Constants::u); };
    case 'a':
      CONSTANT(alpha_rcl2);
      CONSTANT(amu);
      CONSTANT(amu_c2); break;
    case 'A':
      CONSTANT(Avogadro); break;
    case 'B':
      CONSTANT(Bohr_radius); break;
    case 'c':
      CONSTANT(classic_electr_radius);
      CONSTANT(c_light);
      CONSTANT(c_squared); break;
    case 'e':
      CONSTANT(electron_charge);
      CONSTANT(electron_Compton_length);
      CONSTANT(electron_mass_c2);
      CONSTANT(elm_coupling);
      CONSTANT(epsilon0);
      CONSTANT(e_squared); break;
    case 'f':
      CONSTANT(fine_structure_const); break;
    case 'h':
      CONSTANT(halfpi);
      CONSTANT(hbarc);
      CONSTANT(hbarc_squared);
      CONSTANT(hbar_Planck);
      CONSTANT(h_Planck); break;
    case 'k':
      CONSTANT(k_Boltzmann);
      CONSTANT(kGasThreshold); break;
    case 'm':
      CONSTANT(mu0); break;
    case 'n':
      CONSTANT(neutron_mass_c2);
      CONSTANT(nuclearMagneton); break;
    case 'p':
      CONSTANT(pi);
      CONSTANT(pi2);
      CONSTANT(proton_mass_c2); break;
    case 'S':
      CONSTANT(STP_Pressure);
      CONSTANT(STP_Temperature); break;
    case 't':
      CONSTANT(twopi);
      CONSTANT(twopi_mc2_rcl2); break;
    case 'u':
      CONSTANT(universe_mean_density); break;
    }
    return ExprEntityPtr();
  }

  ExprEntityPtr create_standard_unit_or_constant(const str_type& name)
  {
    auto p = create_standard_unit(name);
    return p ? p : create_standard_constant(name);
  }

}
