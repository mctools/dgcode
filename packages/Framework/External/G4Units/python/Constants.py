"""Python module providing CLHEP/Geant4 constants. See libinc/Constants.hh for more details."""
from __future__ import division

import Core.Units as Units

pi  = 3.14159265358979323846
twopi  = 2*pi
halfpi  = pi/2
pi2 = pi*pi
Avogadro = 6.02214179e+23/Units.mole
c_light   = 2.99792458e+8 * Units.m/Units.s
c_squared = c_light * c_light
h_Planck      = 6.62606896e-34 * Units.joule*Units.s
hbar_Planck   = h_Planck/twopi
hbarc         = hbar_Planck * c_light
hbarc_squared = hbarc * hbarc
electron_charge = - Units.eplus
e_squared = Units.eplus * Units.eplus
electron_mass_c2 = 0.510998910 * Units.MeV
proton_mass_c2 = 938.272013 * Units.MeV
neutron_mass_c2 = 939.56536 * Units.MeV
amu_c2 = 931.494028 * Units.MeV
amu = amu_c2/c_squared
mu0      = 4*pi*1.e-7 * Units.henry/Units.m
epsilon0 = 1./(c_squared*mu0)
elm_coupling           = e_squared/(4.0*pi*epsilon0)
fine_structure_const   = elm_coupling/hbarc
classic_electr_radius  = elm_coupling/electron_mass_c2
electron_Compton_length = hbarc/electron_mass_c2
Bohr_radius = electron_Compton_length/fine_structure_const
alpha_rcl2 = fine_structure_const*classic_electr_radius*classic_electr_radius
twopi_mc2_rcl2 = twopi*electron_mass_c2*classic_electr_radius*classic_electr_radius
k_Boltzmann = 8.617343e-11 * Units.MeV/Units.kelvin
STP_Temperature = 273.15*Units.kelvin
STP_Pressure    = 1.*Units.atmosphere
kGasThreshold   = 10.*Units.mg/Units.cm3
universe_mean_density = 1.e-25*Units.g/Units.cm3
nuclearMagneton = Units.eplus*hbar_Planck/2./(proton_mass_c2 /c_squared)
