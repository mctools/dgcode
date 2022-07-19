#ifndef ncrystalrelrel_h
#define ncrystalrelrel_h

/******************************************************************************/
/*                                                                            */
/*  This file is part of NCrystal (see https://mctools.github.io/ncrystal/)   */
/*                                                                            */
/*  Copyright 2015-2022 NCrystal developers                                   */
/*                                                                            */
/*  Licensed under the Apache License, Version 2.0 (the "License");           */
/*  you may not use this file except in compliance with the License.          */
/*  You may obtain a copy of the License at                                   */
/*                                                                            */
/*      http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                            */
/*  Unless required by applicable law or agreed to in writing, software       */
/*  distributed under the License is distributed on an "AS IS" BASIS,         */
/*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  */
/*  See the License for the specific language governing permissions and       */
/*  limitations under the License.                                            */
/*                                                                            */
/******************************************************************************/

/****************************/
/* C-interface for NCrystal */
/****************************/

#include "NCrystalRel/ncapi.h"

#ifdef __cplusplus
extern "C" {
#endif

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Object handle types. All are pointer-sized, thus small enough to pass    == */
  /*== around by value. The internal data-structures are reference-counted, so  == */
  /*== users should call ncrystalrel_ref/ncrystalrel_unref as appropriate if keeping  == */
  /*== such objects around. All ncrystalrel_create_xxx functions returns handles   == */
  /*== which have already have their reference counts increased, so users only  == */
  /*== need to call ncrystalrel_unref or invalidate on them after usage            == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  typedef struct { void * internal; } ncrystalrel_info_t;
  typedef struct { void * internal; } ncrystalrel_process_t;
  typedef struct { void * internal; } ncrystalrel_scatter_t;
  typedef struct { void * internal; } ncrystalrel_absorption_t;
  typedef struct { void * internal; } ncrystalrel_atomdata_t;

  NCRYSTAL_API int  ncrystalrel_refcount( void* object );
  NCRYSTAL_API void ncrystalrel_ref( void* object );
  NCRYSTAL_API void ncrystalrel_unref( void* object );/*unrefs and deletes if count reaches 0*/
  NCRYSTAL_API int  ncrystalrel_valid( void* object );
  NCRYSTAL_API void ncrystalrel_invalidate( void* object );/*invalidates handle (does not unref!)*/

  /*Casts might be needed to use shared interfaces (doesn't increase the           */
  /*ref-count of the underlying object):                                           */
  NCRYSTAL_API ncrystalrel_process_t ncrystalrel_cast_scat2proc(ncrystalrel_scatter_t);
  NCRYSTAL_API ncrystalrel_process_t ncrystalrel_cast_abs2proc(ncrystalrel_absorption_t);

  /*Can down-cast as well (returns invalid object with .internal null ptr in case  */
  /*object is not of that type:                                                    */
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_cast_proc2scat(ncrystalrel_process_t);
  NCRYSTAL_API ncrystalrel_absorption_t ncrystalrel_cast_proc2abs(ncrystalrel_process_t);

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Factory functions. The cfgstr arguments will be passed directly to the   == */
  /*== MatCfg constructor. The file NCMatCfg.hh contains more info about the    == */
  /*== format, which always starts with the name of a datafile.                 == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  NCRYSTAL_API ncrystalrel_info_t ncrystalrel_create_info( const char * cfgstr );
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_create_scatter( const char * cfgstr );
  NCRYSTAL_API ncrystalrel_absorption_t ncrystalrel_create_absorption( const char * cfgstr );

  /* Notice: ncrystalrel_scatter_t objects contain RNG streams, which a lot of other  */
  /* functions in this file are dedicated to handling.                             */

  /* Alternative creation method with new RNG stream (WARNING: Using this is       */
  /* intended for unit-tests only, as it is hard to guarantee two RNG streams are  */
  /* truly independent solely based on the seed value).                            */
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_create_scatter_builtinrng( const char * cfgstr,
                                                                      unsigned long seed );

  /* Cheaply clone scatter and absorption instances. The cloned objects will be    */
  /* using the same physics models and sharing any read-only data, but will be     */
  /* using their own copy of caches. For the case of scatter handles they will     */
  /* also get their own independent RNG stream. All in all, this means that        */
  /* the objects are safe to use concurrently in multi-threaded programming, as    */
  /* long as each thread gets its own clone. Cloned objects must still be cleaned  */
  /* up by calling ncrystalrel_unref. */
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_clone_scatter( ncrystalrel_scatter_t );
  NCRYSTAL_API ncrystalrel_absorption_t ncrystalrel_clone_absorption( ncrystalrel_absorption_t );

  /* Clone function where resulting object will use a specific rngstream index.    */
  /* All objects with the same indeed will share the same RNG state, so a sensible */
  /* strategy is to use the same index for all scatter objects which are to be     */
  /* used in the same thread:                                                      */
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_clone_scatter_rngbyidx( ncrystalrel_scatter_t,
                                                                   unsigned long rngstreamidx );

  /* Clone function where resulting object will use specific rngstream which has   */
  /* been set aside for the current thread. Thus, this function can be called      */
  /* from a given work-thread, in order to get a thread-safe scatter handle.       */
  NCRYSTAL_API ncrystalrel_scatter_t ncrystalrel_clone_scatter_rngforcurrentthread( ncrystalrel_scatter_t );

  /* Convenience function which creates objects directly from a data string        */
  /* rather than an on-disk or in-memory file. Such usage obviously precludes      */
  /* proper caching behind the scenes, and is intended for scenarios where the     */
  /* same data should not be used repeatedly. The ncrystalrel_xxx_t* arguments will   */
  /* be overriden with new handles (nullptrs results in no such object created).   */
  NCRYSTAL_API void ncrystalrel_multicreate_direct( const char* data,
                                                 const char* dataType,/*NULL => determine from data */
                                                 const char* cfg_params,/*e.g. "temp=300K;dcutoff=1"*/
                                                 ncrystalrel_info_t*,
                                                 ncrystalrel_scatter_t*,
                                                 ncrystalrel_absorption_t* );

  /* Factory availablity:                                                          */
  NCRYSTAL_API int ncrystalrel_has_factory( const char * name );

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Query ncrystal process handles (see previous section for casting         == */
  /*== scatter and absorption handles to ncrystalrel_process_t                     == */
  /*== NB: Also notice the "_many" versions of functions further below.         == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /*Name and UID of underlying ProcImpl::Process object:                           */
  NCRYSTAL_API const char * ncrystalrel_name(ncrystalrel_process_t);

  /*Determine if process is non-oriented (normally) or not (single-crystal):       */
  NCRYSTAL_API int ncrystalrel_isnonoriented(ncrystalrel_process_t);

  /*Access cross sections [barn] by neutron kinetic energy [eV]:                   */
  NCRYSTAL_API void ncrystalrel_crosssection_nonoriented( ncrystalrel_process_t,
                                                       double ekin,
                                                       double* result);
  NCRYSTAL_API void ncrystalrel_crosssection( ncrystalrel_process_t,
                                           double ekin,
                                           const double (*direction)[3],
                                           double* result );
  NCRYSTAL_API void ncrystalrel_domain( ncrystalrel_process_t,
                                     double* ekin_low, double* ekin_high);

  /*Generate random scatterings (neutron kinetic energy is in eV). The isotropic   */
  /*functions can only be called whe ncrystalrel_isnonoriented returns true (1).      */
  NCRYSTAL_API void ncrystalrel_samplescatterisotropic( ncrystalrel_scatter_t,
                                                     double ekin,
                                                     double* ekin_final,
                                                     double* cos_scat_angle );

  NCRYSTAL_API void ncrystalrel_samplescatter( ncrystalrel_scatter_t,
                                            double ekin,
                                            const double (*direction)[3],
                                            double* ekin_final,
                                            double (*direction_final)[3] );

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Query ncrystal info handles (see NCInfo.hh for more details)             == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /*Access crystal structure. Returns 0 if structure information is unavailable,   */
  /*otherwise the passed parameters are set and 1 is returned:                     */
  NCRYSTAL_API int ncrystalrel_info_getstructure( ncrystalrel_info_t,
                                               unsigned* spacegroup,
                                               double* lattice_a, double* lattice_b, double* lattice_c,
                                               double* alpha, double* beta, double* gamma,
                                               double* volume, unsigned* n_atoms );

  /*Access various scalar information:                                             */
  NCRYSTAL_API double ncrystalrel_info_getxsectabsorption( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_getxsectfree( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_getdensity( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_getnumberdensity( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_getsld( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_gettemperature( ncrystalrel_info_t );/*-1 if N/A. */

  /*State of matter (Unknown = 0, Solid = 1, Gas = 2, Liquid = 3)                  */
  NCRYSTAL_API int ncrystalrel_info_getstateofmatter( ncrystalrel_info_t );

  /* Access phase information (nphases=0 means single phase)                       */
  NCRYSTAL_API int ncrystalrel_info_nphases( ncrystalrel_info_t );
  NCRYSTAL_API ncrystalrel_info_t ncrystalrel_info_getphase( ncrystalrel_info_t,
                                                       int iphase,
                                                       double* fraction );

  /*Access HKL info:                                                               */
  NCRYSTAL_API int ncrystalrel_info_nhkl( ncrystalrel_info_t ); /* -1 when not available */
  NCRYSTAL_API double ncrystalrel_info_hkl_dlower( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_hkl_dupper( ncrystalrel_info_t );
  NCRYSTAL_API void ncrystalrel_info_gethkl( ncrystalrel_info_t, int idx,
                                          int* h, int* k, int* l, int* multiplicity,
                                          double * dspacing, double* fsquared );
  /*All HKL indices in a given group (returns first value h[0]==k[0]==l[0]==0 if not possible). */
  NCRYSTAL_API void ncrystalrel_info_gethkl_allindices( ncrystalrel_info_t, int idx,
                                                     int* h, int* k, int* l );/* arrays of length multiplicity/2 */

  NCRYSTAL_API double ncrystalrel_info_braggthreshold( ncrystalrel_info_t ); /* [Aa], -1 when not available */
  NCRYSTAL_API int ncrystalrel_info_hklinfotype( ncrystalrel_info_t ); /* integer casted value of HKLInfoType */

  /*Access AtomInfo:                                                               */
  NCRYSTAL_API unsigned ncrystalrel_info_natominfo( ncrystalrel_info_t );/* 0=unavail    */
  NCRYSTAL_API int ncrystalrel_info_hasatommsd( ncrystalrel_info_t );
  NCRYSTAL_API int ncrystalrel_info_hasatomdebyetemp( ncrystalrel_info_t );
  NCRYSTAL_API int ncrystalrel_info_hasdebyetemp( ncrystalrel_info_t );/* alias of hasatomdebyetemp */
  NCRYSTAL_API void ncrystalrel_info_getatominfo( ncrystalrel_info_t, unsigned iatom,
                                               unsigned* atomdataindex,
                                               unsigned* number_per_unit_cell,
                                               double* debye_temp, double* msd );
  NCRYSTAL_API void ncrystalrel_info_getatompos( ncrystalrel_info_t,
                                              unsigned iatom, unsigned ipos,
                                              double* x, double* y, double* z );

  /*Access dynamic info:                                                           */
  /*ditypeid: 0->nonscat, 1:freegas, 2:scatknl 3:vdos, 4:vdosdebye, 99:unknown     */
  NCRYSTAL_API unsigned ncrystalrel_info_ndyninfo( ncrystalrel_info_t );
  NCRYSTAL_API void ncrystalrel_dyninfo_base( ncrystalrel_info_t,
                                           unsigned idyninfo,
                                           double* fraction,
                                           unsigned* atomdataindex,
                                           double* temperature,
                                           unsigned* ditypeid );

  /* Extract scattering kernel for ditype 2,3,4 (vdoslux ignored for type 2).      */
  NCRYSTAL_API void ncrystalrel_dyninfo_extract_scatknl( ncrystalrel_info_t,
                                                      unsigned idyninfo,
                                                      unsigned vdoslux,
                                                      double* suggestedEmax,
                                                      unsigned* negrid,
                                                      unsigned* nalpha,
                                                      unsigned* nbeta,
                                                      const double** egrid,
                                                      const double** alphagrid,
                                                      const double** betagrid,
                                                      const double** sab );

  /* Access vdos data for ditype 3.                                                */
  NCRYSTAL_API void ncrystalrel_dyninfo_extract_vdos( ncrystalrel_info_t,
                                                   unsigned idyninfo,
                                                   double * egridMin,
                                                   double * egridMax,
                                                   unsigned * vdos_ndensity,
                                                   const double ** vdos_density );

  /* Access vdos debye temperature for ditype 4.                                   */
  NCRYSTAL_API void ncrystalrel_dyninfo_extract_vdosdebye( ncrystalrel_info_t,
                                                        unsigned idyninfo,
                                                        double * debye_temp );

  /* Access input curve ditype 3 (returns vdos_negrid=0 if not available).         */
  NCRYSTAL_API void ncrystalrel_dyninfo_extract_vdos_input( ncrystalrel_info_t,
                                                         unsigned idyninfo,
                                                         unsigned* vdos_negrid,
                                                         const double ** vdos_egrid,
                                                         unsigned* vdos_ndensity,
                                                         const double ** vdos_density );

  /* Convenience:                                                                  */
  NCRYSTAL_API double ncrystalrel_info_dspacing_from_hkl( ncrystalrel_info_t, int h, int k, int l );


  /* Composition (always >=1 component):                                           */
  NCRYSTAL_API unsigned ncrystalrel_info_ncomponents( ncrystalrel_info_t );
  NCRYSTAL_API void ncrystalrel_info_getcomponent( ncrystalrel_info_t, unsigned icomponent,
                                                unsigned* atomdataindex,
                                                double* fraction );


  /* Turn returned atomdata_idx's from calls above into actual ncrystalrel_atomdata_t */
  /* objects. The returned objects are ref-counted and the calling code should     */
  /* eventually unref them (with a call to ncrystalrel_unref) in order to prevent     */
  /* resource leaks.                                                               */
  NCRYSTAL_API ncrystalrel_atomdata_t ncrystalrel_create_atomdata( ncrystalrel_info_t,
                                                             unsigned atomdataindex );

  /* Get atom data fields. Each object falls in one of three categories:           */
  /* 1) Natural elements (ncomponents=A=0,Z>0)                                     */
  /* 2) Single isotope (ncomponents=0, Z>0, A>=Z)                                  */
  /* 3) Composite (A=0,ncomponents>1,Z>0 if all components share Z, otherwise Z=0) */
  /* Note that displaylabel=0 if atomdata object was sub-component.                */
  NCRYSTAL_API void ncrystalrel_atomdata_getfields( ncrystalrel_atomdata_t,
                                                 const char** displaylabel,
                                                 const char** description,
                                                 double* mass, double *incxs,
                                                 double* cohsl_fm, double* absxs,
                                                 unsigned* ncomponents,
                                                 unsigned* zval, unsigned* aval );

  /* Get atomdata and fraction of component:                                       */
  /* NB: Returned object should eventually be unref'ed by calling code.            */
  NCRYSTAL_API ncrystalrel_atomdata_t ncrystalrel_create_atomdata_subcomp( ncrystalrel_atomdata_t,
                                                                     unsigned icomponent,
                                                                     double* fraction );

  /* Custom data section:                                                         */
  NCRYSTAL_API unsigned ncrystalrel_info_ncustomsections( ncrystalrel_info_t );
  NCRYSTAL_API const char* ncrystalrel_info_customsec_name( ncrystalrel_info_t, unsigned isection );
  NCRYSTAL_API unsigned ncrystalrel_info_customsec_nlines( ncrystalrel_info_t, unsigned isection );
  NCRYSTAL_API unsigned ncrystalrel_info_customline_nparts( ncrystalrel_info_t, unsigned isection, unsigned iline );
  NCRYSTAL_API const char* ncrystalrel_info_customline_getpart( ncrystalrel_info_t, unsigned isection, unsigned iline, unsigned ipart );

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Error handling                                                           == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /*By default, any error encountered will result in a message printed to stdout   */
  /*and termination of the programme. This behaviour can be changed by calling     */
  /*ncrystalrel_sethaltonerror(0) and ncrystalrel_setquietonerror(1) respectively.       */
  NCRYSTAL_API int ncrystalrel_sethaltonerror(int);/* returns old value */
  NCRYSTAL_API int ncrystalrel_setquietonerror(int);/* returns old value */

  /*If not halting on error, these functions can be used to access information     */
  /*about errors encountered:                                                      */
  NCRYSTAL_API int ncrystalrel_error();/* returns 1 if an error condition occurred. */
  NCRYSTAL_API const char * ncrystalrel_lasterror();/* returns description of last error (NULL if none) */
  NCRYSTAL_API const char * ncrystalrel_lasterrortype();/* returns description of last error (NULL if none) */
  /* TODO: error file/line-no as well? */

  NCRYSTAL_API void ncrystalrel_clearerror();/* clears previous error if any */

  /*Another option is to provide a custom error handler which will be called on    */
  /*each error:                                                                    */
  NCRYSTAL_API void ncrystalrel_seterrhandler(void (*handler)(char*,char*));

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Random number stream handling                                            == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /* Register custom RNG stream (it must return numbers uniformly in [0,1)). This  */
  /* RNG will be used for any subsequent calls to ncrystalrel_create_scatter, and it  */
  /* will NOT be changed when cloning ncrystalrel_scatter_t objects. Thus, for multi- */
  /* threaded usage, the caller should ensure that the function is thread-safe and */
  /* returns numbers from different streams in each thread (through suitable usage */
  /* of thread_local objects perhaps).                                             */
  NCRYSTAL_API void ncrystalrel_setrandgen( double (*rg)() );

  /* It is also possible to (re) set the RNG to the builtin generator (optionally  */
  /* by state or integer seed) */
  NCRYSTAL_API void ncrystalrel_setbuiltinrandgen();
  NCRYSTAL_API void ncrystalrel_setbuiltinrandgen_withseed(unsigned long seed);
  NCRYSTAL_API void ncrystalrel_setbuiltinrandgen_withstate(const char*);

  /* If supported (which it will NOT be if the RNG was set using the C API and the */
  /* ncrystalrel_setrandgen function), the state of the RNG stream can be accessed    */
  /* and manipulated via the following functions (returned strings must be free'd  */
  /* by calling ncrystalrel_dealloc_string). Note that setting the rng state will     */
  /* affect all objects sharing the RNG stream with the given scatter object (and  */
  /* those subsequently cloned from any of those). Note that if the provided state */
  /* originates in (the current version of) NCrystal's builtin RNG algorithm, it   */
  /* can always be used in the ncrystalrel_setrngstate_ofscatter function, even if    */
  /* the current RNG uses a different algorithm (it will simply be replaced).      */
  /* Finally note that getrngstate returns NULL if state manip. is not supported.  */
  NCRYSTAL_API int ncrystalrel_rngsupportsstatemanip_ofscatter( ncrystalrel_scatter_t );
  NCRYSTAL_API char* ncrystalrel_getrngstate_ofscatter( ncrystalrel_scatter_t );
  NCRYSTAL_API void ncrystalrel_setrngstate_ofscatter( ncrystalrel_scatter_t, const char* );

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Data sources                                                             == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /* Access TextData. Returns a string list of length 5:                           */
  /* [contents, uid(as string), sourcename, datatype, resolvedphyspath].           */
  /* The last entry is optional and will be an empty str if absent.                */
  /* Must free list with call to ncrystalrel_dealloc_stringlist.                      */
  NCRYSTAL_API char** ncrystalrel_get_text_data( const char * name );

  /* Register in-memory file data (as a special case data can be "ondisk://<path>" */
  /* to instead create a virtual alias for an on-disk file).:                      */
  NCRYSTAL_API void ncrystalrel_register_in_mem_file_data(const char* virtual_filename,
                                                       const char* data);

  /* Browse (some) available files. Resulting string list must be deallocated by a */
  /* call to ncrystalrel_dealloc_stringlist by, and contains entries in the format    */
  /* name0,src0,fact0,priority0,name1,src1,fact1,priority1,..:                     */
  NCRYSTAL_API void ncrystalrel_get_file_list( unsigned* nstrs, char*** strs );

  /* Add directory to custom search path or clear custom search path:              */
  NCRYSTAL_API void ncrystalrel_add_custom_search_dir( const char * dir );
  NCRYSTAL_API void ncrystalrel_remove_custom_search_dirs();

  /* Enable/disable standard search mechanisms (all enabled by default). For the   */
  /* case of ncrystalrel_enable_stddatalib, provide dir=NULL if you wish to use the   */
  /* usual stddatalib path (determined by NCRYSTAL_DATA_DIR environment and        */
  /* compile-time variables: */
  NCRYSTAL_API void ncrystalrel_enable_abspaths( int doEnable );
  NCRYSTAL_API void ncrystalrel_enable_relpaths( int doEnable );
  NCRYSTAL_API void ncrystalrel_enable_stddatalib( int doEnable, const char * dir );
  NCRYSTAL_API void ncrystalrel_enable_stdsearchpath( int doEnable );

  /* Remove all current data sources (supposedly in order to subsequently enable   */
  /* sources selectively). This also removes virtual files and caches.             */
  NCRYSTAL_API void ncrystalrel_remove_all_data_sources();


  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Miscellaneous                                                            == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /* Dump info to stdout (1st fct is same as calling 2nd one with verbosity_lvl=0):*/
  NCRYSTAL_API void ncrystalrel_dump(ncrystalrel_info_t);
  NCRYSTAL_API void ncrystalrel_dump_verbose(ncrystalrel_info_t, unsigned verbosity_lvl );

  /* Utility converting between neutron wavelength [Aa] to kinetic energy [eV]:    */
  NCRYSTAL_API double ncrystalrel_wl2ekin( double wl );
  NCRYSTAL_API double ncrystalrel_ekin2wl( double ekin );

  /* Extract extra debug information about objects (as JSON string which must be   */
  /* cleaned up with ncrystalrel_dealloc_string.                                      */
  NCRYSTAL_API char * ncrystalrel_dbg_process( ncrystalrel_process_t );

  /*UID of underlying Info or ProcImpl::Process object as string (must free with   */
  /*call to ncrystalrel_dealloc_string:                                               */
  NCRYSTAL_API char * ncrystalrel_process_uid(ncrystalrel_process_t);
  NCRYSTAL_API char * ncrystalrel_info_uid(ncrystalrel_info_t);

  /*Generate cfg-str variable documentation as string (must free with call to      */
  /*ncrystalrel_dealloc_string). Mode 0 (full), 1 (short), 2 (json):                  */
  NCRYSTAL_API char * ncrystalrel_gencfgstr_doc(int mode);

  /*Underlying UID (in case density value or cfgdata  was overridden):             */
  NCRYSTAL_API char * ncrystalrel_info_underlyinguid(ncrystalrel_info_t);


  /* Access internal DB for isotopes and natural elements.                         */
  /* NB: Will return invalid handle in case lookup failed. Otherwise, the          */
  /* returned object should eventually be unref'ed by calling code:                */
  NCRYSTAL_API ncrystalrel_atomdata_t ncrystalrel_create_atomdata_fromdb( unsigned z,
                                                                    unsigned a );
  /* Version which accepts strings like "Al", "H2", "D" ...:                       */
  NCRYSTAL_API ncrystalrel_atomdata_t ncrystalrel_create_atomdata_fromdbstr( const char* );

  /* Get all (Z,A) values in internal DB (A=0 means natural element). The second   */
  /* fct accepts two preallocated arrays with length given by the first fct:       */
  NCRYSTAL_API unsigned ncrystalrel_atomdatadb_getnentries();
  NCRYSTAL_API void ncrystalrel_atomdatadb_getallentries( unsigned* zvals,
                                                       unsigned* avals );

  /* Convert between atomic mean squared displacements and Debye temperatures.     */
  /* Units are kelvin, AMU, and Angstrom^2:                                        */
  double ncrystalrel_debyetemp2msd( double debyetemp, double temperature, double mass );
  double ncrystalrel_msd2debyetemp( double msd, double temperature, double mass );

  /* Extract information from VDOS curve (see NCVDOSEval.hh for details):          */
  void ncrystalrel_vdoseval( double vdos_emin, double vdos_emax,
                          unsigned vdos_ndensity, const double* vdos_density,
                          double temperature, double atom_mass_amu,
                          double* msd, double* debye_temp, double* gamma0,
                          double* temp_eff, double* origIntegral );

  /* Extract NCMatCfg variables which can not be inferred from an ncrystalrel_info_t  */
  /* object and which might be needed in plugins (to be expanded as needed).       */
  /* Returned strings must be cleaned up with ncrystalrel_dealloc_string.             */
  NCRYSTAL_API unsigned ncrystalrel_decodecfg_vdoslux( const char * cfgstr );
  NCRYSTAL_API char* ncrystalrel_decodecfg_json( const char * cfgstr );
  /* Parse and reencode cfg (as NCrystalRel::MatCfg(cfgstr).toStrCfg()):              */
  NCRYSTAL_API char* ncrystalrel_normalisecfg( const char * cfgstr );

  /* Clear various caches employed inside NCrystal:                                */
  NCRYSTAL_API void ncrystalrel_clear_caches();

  /* Get list of plugins. Resulting string list must be deallocated by a call to   */
  /* ncrystalrel_dealloc_stringlist by, and contains entries in the format            */
  /* pluginname0,filename0,plugintype0,pluginname1,filename1,plugintype1,...:      */
  NCRYSTAL_API void ncrystalrel_get_plugin_list( unsigned* nstrs, char*** strs );

  /* Deallocate strings:                                                           */
  NCRYSTAL_API void ncrystalrel_dealloc_stringlist( unsigned len, char** );
  NCRYSTAL_API void ncrystalrel_dealloc_string( char* );

  /* NCrystal version info:                                                        */
#ifdef NCRYSTAL_VERSION_MAJOR
#  undef NCRYSTAL_VERSION_MAJOR
#endif
#ifdef NCRYSTAL_VERSION_MINOR
#  undef NCRYSTAL_VERSION_MINOR
#endif
#ifdef NCRYSTAL_VERSION_PATCH
#  undef NCRYSTAL_VERSION_PATCH
#endif
#ifdef NCRYSTAL_VERSION
#  undef NCRYSTAL_VERSION
#endif
#ifdef NCRYSTAL_VERSION_STR
#  undef NCRYSTAL_VERSION_STR
#endif
#define NCRYSTAL_VERSION_MAJOR 3
#define NCRYSTAL_VERSION_MINOR 1
#define NCRYSTAL_VERSION_PATCH 0
#define NCRYSTAL_VERSION   3001000 /* (1000000*MAJOR+1000*MINOR+PATCH)             */
#define NCRYSTAL_VERSION_STR "3.1.0"
  NCRYSTAL_API int ncrystalrel_version(); /* returns NCRYSTAL_VERSION                  */
  NCRYSTAL_API const char * ncrystalrel_version_str(); /* returns NCRYSTAL_VERSION_STR */

  /* Load raw NCMAT data into JSON structures. Must deallocate with call to        */
  /* ncrystalrel_dealloc_string as usual. (WARNING: JSON is incomplete for now!!!!!)  */
  NCRYSTAL_API char * ncrystalrel_ncmat2json( const char * textdataname );


  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Specialised array versions of some functions                             == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  NCRYSTAL_API void ncrystalrel_samplescatterisotropic_many( ncrystalrel_scatter_t,
                                                          const double * ekin,
                                                          unsigned long n_ekin,
                                                          unsigned long repeat,
                                                          double* results_ekin,
                                                          double* results_cos_scat_angle );

  NCRYSTAL_API void ncrystalrel_samplescatter_many( ncrystalrel_scatter_t,
                                                 double ekin,
                                                 const double (*direction)[3],
                                                 unsigned long repeat,
                                                 double* results_ekin,
                                                 double * results_dirx,
                                                 double * results_diry,
                                                 double * results_dirz );

  NCRYSTAL_API void ncrystalrel_crosssection_nonoriented_many( ncrystalrel_process_t,
                                                            const double * ekin,
                                                            unsigned long n_ekin,
                                                            unsigned long repeat,
                                                            double* results );

  /*============================================================================== */
  /*============================================================================== */
  /*==                                                                          == */
  /*== Various obsolete functions which are bound to be removed in future       == */
  /*== releases of NCrystal.                                                    == */
  /*==                                                                          == */
  /*============================================================================== */
  /*============================================================================== */

  /*Obsolete function which now always returns 1.0. Packing factors are now        */
  /*instead absorbed into the material densities:                                  */
  NCRYSTAL_API double ncrystalrel_decodecfg_packfact( const char * cfgstr );

  /*Obsolete function which now just is an alias for ncrystalrel_clear_caches above:  */
  NCRYSTAL_API void ncrystalrel_clear_info_caches();

  /*Obsolete function. Atom positions are now always available when                */
  /*ncrystalrel_info_natominfo returns a value greater than 0:                        */
  NCRYSTAL_API int ncrystalrel_info_hasatompos( ncrystalrel_info_t );

  /*Obsolete functions. Debye temperatures are now always available via the        */
  /*AtomInfo objects, and there is no longer a concept of a "global" Debye temp.:  */
  NCRYSTAL_API int ncrystalrel_info_hasanydebyetemp( ncrystalrel_info_t );
  NCRYSTAL_API double ncrystalrel_info_getdebyetempbyelement( ncrystalrel_info_t,
                                                           unsigned atomdataindex );
  NCRYSTAL_API double ncrystalrel_info_getglobaldebyetemp( ncrystalrel_info_t );/* -1=unavail */

  /* Obsolete function, the ncrystalrel_get_text_data function should be used instead */
  /* The returned content should be deallocated with ncrystalrel_dealloc_string.      */
  NCRYSTAL_API char* ncrystalrel_get_file_contents( const char * name );

  /*Obsolete genscatter functions. Users should use the ncrystalrel_samplescatter     */
  /*functions above instead:                                                       */
  NCRYSTAL_API void ncrystalrel_genscatter_nonoriented( ncrystalrel_scatter_t,
                                                     double ekin,
                                                     double* result_angle,
                                                     double* result_dekin );
  NCRYSTAL_API void ncrystalrel_genscatter( ncrystalrel_scatter_t,
                                         double ekin,
                                         const double (*direction)[3],
                                         double (*result_direction)[3],
                                         double* result_deltaekin );
  NCRYSTAL_API void ncrystalrel_genscatter_nonoriented_many( ncrystalrel_scatter_t,
                                                          const double * ekin,
                                                          unsigned long n_ekin,
                                                          unsigned long repeat,
                                                          double* results_angle,
                                                          double* results_dekin );
  NCRYSTAL_API void ncrystalrel_genscatter_many( ncrystalrel_scatter_t,
                                              double ekin,
                                              const double (*direction)[3],
                                              unsigned long repeat,
                                              double * results_dirx,
                                              double * results_diry,
                                              double * results_dirz,
                                              double * results_dekin );

#ifdef __cplusplus
}
#endif

#endif
