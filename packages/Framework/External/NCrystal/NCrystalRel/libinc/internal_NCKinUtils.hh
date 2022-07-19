#ifndef NCrystalRel_KinUtils_hh
#define NCrystalRel_KinUtils_hh

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

//Various utilities related to kinematics, including those related to the
//S(alpha,beta) formalism.

namespace NCrystalRel {

  //Get deltaE and mu=cos(scattering_angle) from (alpha,beta). It is recommended
  //to check muIsotropicAtBeta before calling this (for numerical safety, but it
  //might also save the effort of sampling alpha).
  PairDD convertAlphaBetaToDeltaEMu(double alpha, double beta, NeutronEnergy ekin, double kT );
  PairDD convertAlphaBetaToDeltaEMu(PairDD alphabeta, NeutronEnergy ekin, double kT );

  //Get kinematically accessible alpha region for given ekin/kT and beta:
  double getAlphaMinus( double ekin_div_kT, double beta );
  double getAlphaPlus( double ekin_div_kT, double beta );

  //It is faster to get both (alpha-,alpha+) at once (returns (1.0,-1.0) if not
  //kinematically accessible, i.e. beta<-E/kT):
  PairDD getAlphaLimits( double ekin_div_kT, double beta );

  //Get { alpha-, alpha+, and alpha+ - alpha- } in numerically stable manner:
  struct AlphaLimitsWithDiff { double aminus, aplus, adiff; };
  AlphaLimitsWithDiff getAlphaLimitsWithDiff( double ekin_div_kT, double beta );

  //Get kinematically accessible beta region for given ekin/kT and alpha:
  double getBetaMinus( double ekin_div_kT, double alpha );
  double getBetaPlus( double ekin_div_kT, double alpha );

  //It is again faster to get both (beta-,beta+) at once:
  PairDD getBetaLimits( double ekin_div_kT, double alpha );

  //Near kinematic endpoint. alpha-, alpha, and alpha+ might become numerically
  //indistinguishable at floating point precision, but mu=cos(scattering_angle)
  //should be sampled isotropically in this limit (since
  //S(alpha-,beta)~=S(alpha+,beta)). For consistency, always use this function
  //to detect this degenerate case:
  inline bool muIsotropicAtBeta( double beta, double ekin_div_kT )
  {
    nc_assert(beta>=-ekin_div_kT);
    nc_assert(ekin_div_kT>=0.0);
    constexpr double lim(-1.0+1e-14);
    return beta <= ekin_div_kT*lim;
  }


}


////////////////////////////
// Inline implementations //
////////////////////////////

namespace NCrystalRel {
  inline PairDD convertAlphaBetaToDeltaEMu( PairDD alphabeta, NeutronEnergy ekin, double kT )
  {
    return convertAlphaBetaToDeltaEMu(alphabeta.first,alphabeta.second,ekin,kT);
  }

  namespace detail {
    inline bool alphaMinusNeedsTaylor( double ekin_div_kT, double beta )
    {
      return std::abs(beta) < 0.01*ekin_div_kT;
    }
    inline double alphaMinusTaylor( double ekin_div_kT, double beta ) {
      nc_assert( ekin_div_kT > 0.0 );
      nc_assert( std::abs(beta) < 0.011*ekin_div_kT );
      const double x = beta / ekin_div_kT;
      constexpr double c9 = -715./32768.;
      constexpr double c8 = 429./16384.;
      constexpr double c7 = -33./1024.;
      constexpr double c6 = 21./512.;
      constexpr double c5 = -7./128.;
      constexpr double c4 = 5./64.;
      constexpr double c3 = - 1./8.;
      constexpr double c2 = 1./4.;
      return beta*x*(c2+x*(c3+x*(c4+x*(c5+x*(c6+x*(c7+x*(c8+x*c9)))))));
    }
  }

  //Get kinematically accessible alpha region for given ekin/kT and beta:
  inline PairDD getAlphaLimits( double ekin_div_kT, double beta )
  {
    nc_assert( ekin_div_kT >= 0.0 );
    nc_assert( !std::isnan(beta) );
    const double kk = ekin_div_kT + beta;
    if ( !(kk >= 0.0) ) {
      //kinematically forbidden
      return std::make_pair(1.0,-1.0);
    }
    const double a = kk + ekin_div_kT;
    const double b = 2.0*std::sqrt( ekin_div_kT * kk );
    const double aminus = ( detail::alphaMinusNeedsTaylor( ekin_div_kT, beta )
                            ? detail::alphaMinusTaylor( ekin_div_kT, beta )
                            : std::max(0.0,a - b) );
    nc_assert( a+b >= aminus );
    return std::make_pair( aminus, a + b );
  }

  inline AlphaLimitsWithDiff getAlphaLimitsWithDiff( double ekin_div_kT, double beta )
  {
    nc_assert( ekin_div_kT >= 0.0 );
    nc_assert( !std::isnan(beta) );
    nc_assert( beta >= -ekin_div_kT );
    const double kk = ekin_div_kT + beta;
    const double a = kk + ekin_div_kT;
    const double b = 2.0*std::sqrt( ekin_div_kT * kk );
    const double aminus = ( detail::alphaMinusNeedsTaylor( ekin_div_kT, beta )
                            ? detail::alphaMinusTaylor( ekin_div_kT, beta )
                            : std::max(0.0,a - b) );
    return { aminus, a+b, 2.0*b };
  }

  inline double getAlphaMinus( double ekin_div_kT, double beta ) {
    nc_assert(ekin_div_kT >= 0.0);
    if ( detail::alphaMinusNeedsTaylor( ekin_div_kT, beta ) )
      return detail::alphaMinusTaylor( ekin_div_kT, beta );
    nc_assert( ekin_div_kT >= 0.0 );
    nc_assert( !std::isnan(beta) );
    const double kk = ekin_div_kT + beta;
    nc_assert( kk >= 0.0 );
    const double a = kk + ekin_div_kT;
    const double b = 2.0*std::sqrt( ekin_div_kT * kk );
    return std::max(0.0,a - b);
  }

  inline double getAlphaPlus( double ekin_div_kT, double beta )
  {
    nc_assert(ekin_div_kT >= 0.0);
    nc_assert( ekin_div_kT >= 0.0 );
    nc_assert( !std::isnan(beta) );
    const double kk = ekin_div_kT + beta;
    nc_assert( kk >= 0.0 );
    const double a = kk + ekin_div_kT;
    const double b = 2.0*std::sqrt( ekin_div_kT * kk );
    return a + b;
  }

  namespace detail {
    inline bool betaMinusNeedsTaylor( double ekin_div_kT, double alpha )
    {
      return std::abs( alpha - 4.0*ekin_div_kT) < 0.05*ekin_div_kT;
    }

    inline double betaMinusTaylor( double ekin_div_kT, double alpha ) {
      const double x = alpha / ekin_div_kT - 4.0;
      nc_assert( std::abs(x) < 0.051 );
      constexpr double c1 = 0.5;
      constexpr double c2 = 1./32.;
      constexpr double c3 = -1./256.;
      constexpr double c4 = 5./8192.;
      constexpr double c5 = -7./65536.;
      constexpr double c6 = 21./1048576.;
      constexpr double c7 = -33./8388608.;
      constexpr double c8 = 429./536870912.;
      return ekin_div_kT*(x*(c1+x*(c2+x*(c3+x*(c4+x*(c5+x*(c6+x*(c7+x*c8))))))));
    }
  }

  inline double getBetaMinus( double ekin_div_kT, double alpha )
  {
    nc_assert(ekin_div_kT*alpha>=0);
    return ( detail::betaMinusNeedsTaylor( ekin_div_kT, alpha )
             ? detail::betaMinusTaylor( ekin_div_kT, alpha )
             : ( alpha - 2*std::sqrt(ekin_div_kT*alpha) ) );
  }

  inline double getBetaPlus( double ekin_div_kT, double alpha )
  {
    nc_assert(ekin_div_kT*alpha>=0);
    return alpha + 2*std::sqrt(ekin_div_kT*alpha);
  }

  inline PairDD getBetaLimits( double ekin_div_kT, double alpha )
  {
    nc_assert(ekin_div_kT>=0);
    nc_assert(alpha>=0);
    const double k = 2*std::sqrt(ekin_div_kT*alpha);
    return { ( detail::betaMinusNeedsTaylor( ekin_div_kT, alpha )
               ? detail::betaMinusTaylor( ekin_div_kT, alpha )
               : alpha - k ), alpha + k };
  }
}

#endif
