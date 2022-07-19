/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2013 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ifndef MiniZLib_ZCONF_H
#define MiniZLib_ZCONF_H

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#if 1     /* may be set to #if 1 by ./configure */
#  define Z_PREFIX_SET

/* all linked symbols */
#  define _dist_code            minizlib__dist_code
#  define _length_code          minizlib__length_code
#  define _tr_align             minizlib__tr_align
#  define _tr_flush_bits        minizlib__tr_flush_bits
#  define _tr_flush_block       minizlib__tr_flush_block
#  define _tr_init              minizlib__tr_init
#  define _tr_stored_block      minizlib__tr_stored_block
#  define _tr_tally             minizlib__tr_tally
#  define adler32               minizlib_adler32
#  define adler32_combine       minizlib_adler32_combine
#  define adler32_combine64     minizlib_adler32_combine64
#  ifndef Z_SOLO
#    define compress              minizlib_compress
#    define compress2             minizlib_compress2
#    define compressBound         minizlib_compressBound
#  endif
#  define crc32                 minizlib_crc32
#  define crc32_combine         minizlib_crc32_combine
#  define crc32_combine64       minizlib_crc32_combine64
#  define deflate               minizlib_deflate
#  define deflateBound          minizlib_deflateBound
#  define deflateCopy           minizlib_deflateCopy
#  define deflateEnd            minizlib_deflateEnd
#  define deflateInit2_         minizlib_deflateInit2_
#  define deflateInit_          minizlib_deflateInit_
#  define deflateParams         minizlib_deflateParams
#  define deflatePending        minizlib_deflatePending
#  define deflatePrime          minizlib_deflatePrime
#  define deflateReset          minizlib_deflateReset
#  define deflateResetKeep      minizlib_deflateResetKeep
#  define deflateSetDictionary  minizlib_deflateSetDictionary
#  define deflateSetHeader      minizlib_deflateSetHeader
#  define deflateTune           minizlib_deflateTune
#  define deflate_copyright     minizlib_deflate_copyright
#  define get_crc_table         minizlib_get_crc_table
#  ifndef Z_SOLO
#    define gz_error              minizlib_gz_error
#    define gz_intmax             minizlib_gz_intmax
#    define gz_strwinerror        minizlib_gz_strwinerror
#    define gzbuffer              minizlib_gzbuffer
#    define gzclearerr            minizlib_gzclearerr
#    define gzclose               minizlib_gzclose
#    define gzclose_r             minizlib_gzclose_r
#    define gzclose_w             minizlib_gzclose_w
#    define gzdirect              minizlib_gzdirect
#    define gzdopen               minizlib_gzdopen
#    define gzeof                 minizlib_gzeof
#    define gzerror               minizlib_gzerror
#    define gzflush               minizlib_gzflush
#    define gzgetc                minizlib_gzgetc
#    define gzgetc_               minizlib_gzgetc_
#    define gzgets                minizlib_gzgets
#    define gzoffset              minizlib_gzoffset
#    define gzoffset64            minizlib_gzoffset64
#    define gzopen                minizlib_gzopen
#    define gzopen64              minizlib_gzopen64
#    ifdef _WIN32
#      define gzopen_w              minizlib_gzopen_w
#    endif
#    define gzprintf              minizlib_gzprintf
#    define gzvprintf             minizlib_gzvprintf
#    define gzputc                minizlib_gzputc
#    define gzputs                minizlib_gzputs
#    define gzread                minizlib_gzread
#    define gzrewind              minizlib_gzrewind
#    define gzseek                minizlib_gzseek
#    define gzseek64              minizlib_gzseek64
#    define gzsetparams           minizlib_gzsetparams
#    define gztell                minizlib_gztell
#    define gztell64              minizlib_gztell64
#    define gzungetc              minizlib_gzungetc
#    define gzwrite               minizlib_gzwrite
#  endif
#  define inflate               minizlib_inflate
#  define inflateBack           minizlib_inflateBack
#  define inflateBackEnd        minizlib_inflateBackEnd
#  define inflateBackInit_      minizlib_inflateBackInit_
#  define inflateCopy           minizlib_inflateCopy
#  define inflateEnd            minizlib_inflateEnd
#  define inflateGetHeader      minizlib_inflateGetHeader
#  define inflateInit2_         minizlib_inflateInit2_
#  define inflateInit_          minizlib_inflateInit_
#  define inflateMark           minizlib_inflateMark
#  define inflatePrime          minizlib_inflatePrime
#  define inflateReset          minizlib_inflateReset
#  define inflateReset2         minizlib_inflateReset2
#  define inflateSetDictionary  minizlib_inflateSetDictionary
#  define inflateGetDictionary  minizlib_inflateGetDictionary
#  define inflateSync           minizlib_inflateSync
#  define inflateSyncPoint      minizlib_inflateSyncPoint
#  define inflateUndermine      minizlib_inflateUndermine
#  define inflateResetKeep      minizlib_inflateResetKeep
#  define inflate_copyright     minizlib_inflate_copyright
#  define inflate_fast          minizlib_inflate_fast
#  define inflate_table         minizlib_inflate_table
#  ifndef Z_SOLO
#    define uncompress            minizlib_uncompress
#  endif
#  define zError                minizlib_zError
#  ifndef Z_SOLO
#    define zcalloc               minizlib_zcalloc
#    define zcfree                minizlib_zcfree
#  endif
#  define zlibCompileFlags      minizlib_zlibCompileFlags
#  define zlibVersion           minizlib_zlibVersion

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  minizlib_Byte
#  define Bytef                 minizlib_Bytef
#  define alloc_func            minizlib_alloc_func
#  define charf                 minizlib_charf
#  define free_func             minizlib_free_func
#  ifndef Z_SOLO
#    define gzFile                minizlib_gzFile
#  endif
#  define gz_header             minizlib_gz_header
#  define gz_headerp            minizlib_gz_headerp
#  define in_func               minizlib_in_func
#  define intf                  minizlib_intf
#  define out_func              minizlib_out_func
#  define uInt                  minizlib_uInt
#  define uIntf                 minizlib_uIntf
#  define uLong                 minizlib_uLong
#  define uLongf                minizlib_uLongf
#  define voidp                 minizlib_voidp
#  define voidpc                minizlib_voidpc
#  define voidpf                minizlib_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  define gz_header_s           minizlib_gz_header_s
#  define internal_state        minizlib_internal_state
/* extras added by TK */
#  define distfix minizlib_distfix
#  define lenfix minizlib_lenfix
#  define lbase minizlib_lbase
#  define dbase minizlib_dbase
#  define my_version minizlib_my_version
#  define dext minizlib_dext
#  define order minizlib_order
#  define build_bl_tree minizlib_build_bl_tree
#  define copy_block minizlib_copy_block
#  define crc32_big minizlib_crc32_big
#  define crc32_little minizlib_crc32_little
#  define deflate_huff minizlib_deflate_huff
#  define deflate_rle minizlib_deflate_rle
#  define detect_data_type minizlib_detect_data_type
#  define fixedtables minizlib_fixedtables
#  define flush_pending minizlib_flush_pending
#  define gen_bitlen minizlib_gen_bitlen
#  define gf2_matrix_square minizlib_gf2_matrix_square
#  define gf2_matrix_times minizlib_gf2_matrix_times
#  define init_block minizlib_init_block
#  define lm_init minizlib_lm_init
#  define pqdownheap minizlib_pqdownheap
#  define putShortMSB minizlib_putShortMSB
#  define read_buf minizlib_read_buf
#  define scan_tree minizlib_scan_tree
#  define send_all_trees minizlib_send_all_trees
#  define syncsearch minizlib_syncsearch
#  define tr_static_init minizlib_tr_static_init
#  define updatewindow minizlib_updatewindow
#  define adler32_combine_       minizlib_adler32_combine_
#  define bi_flush       minizlib_bi_flush
#  define bi_reverse       minizlib_bi_reverse
#  define bi_windup       minizlib_bi_windup
#  define gen_codes       minizlib_gen_codes
#  define deflate_slow minizlib_deflate_slow
#  define deflate_fast minizlib_deflate_fast
#  define deflate_stored minizlib_deflate_stored
#  define gz_avail minizlib_gz_avail
#  define gz_load minizlib_gz_load
#  define gz_reset minizlib_gz_reset
#  define gz_skip minizlib_gz_skip
#  define gz_zero minizlib_gz_zero
#  define gz_comp minizlib_gz_comp
#  define gz_decomp minizlib_gz_decomp
#  define gz_fetch minizlib_gz_fetch
#  define gz_init minizlib_gz_init
#  define gz_look minizlib_gz_look
#  define gz_open minizlib_gz_open
#  define z_errmsg minizlib_z_errmsg
#  define crc32_combine_ minizlib_crc32_combine_
#  define crc_table minizlib_crc_table
#  define longest_match minizlib_longest_match
#  define fill_window minizlib_fill_window
#  define configuration_table minizlib_configuration_table
#  define send_tree minizlib_send_tree
#  define compress_block minizlib_compress_block
#  define extra_lbits minizlib_extra_lbits
#  define base_length minizlib_base_length
#  define extra_dbits minizlib_extra_dbits
#  define base_dist minizlib_base_dist
#  define build_tree minizlib_build_tree
#  define bl_order minizlib_bl_order
#  define extra_blbits minizlib_extra_blbits
#  define static_l_desc minizlib_static_l_desc
#  define static_bl_desc minizlib_static_bl_desc
#  define static_d_desc minizlib_static_d_desc
#  define static_dtree minizlib_static_dtree
#  define static_ltree minizlib_static_ltree
#endif

#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif
#if (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#endif
#if defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#endif
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#endif
#if (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  if !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16BIT
#      define SYS16BIT
#    endif
#  endif
#endif

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ifdef SYS16BIT
#  define MAXSEG_64K
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#ifdef __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  if __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#endif
#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#endif
#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#endif
#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#endif
#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#endif

#if defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#endif

#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const       /* note: need a more gentle solution here */
#  endif
#endif

#if defined(ZLIB_CONST) && !defined(z_const)
#  define z_const const
#else
#  define z_const
#endif

/* Some Mac compilers merge all .h files incorrectly: */
#if defined(__MWERKS__)||defined(applec)||defined(THINK_C)||defined(__SC__)
#  define NO_DUMMY_DECL
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

#ifndef Z_ARG /* function prototypes for stdarg */
#  if defined(STDC) || defined(Z_HAVE_STDARG_H)
#    define Z_ARG(args)  args
#  else
#    define Z_ARG(args)  ()
#  endif
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ifdef SYS16BIT
#  if defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#  if (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#endif

#if defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_INTERNAL
#        define ZEXTERN extern __declspec(dllexport)
#      else
#        define ZEXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif  /* ZLIB_DLL */
   /* If building or using zlib with the WINAPI/WINAPIV calling convention,
    * define ZLIB_WINAPI.
    * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
    */
#  ifdef ZLIB_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    else
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_INTERNAL
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    else
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef FAR
#  define FAR
#endif

#if !defined(__MACTYPES__)
typedef unsigned char  Byte;  /* 8 bits */
#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

#ifdef SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void const *voidpc;
   typedef void FAR   *voidpf;
   typedef void       *voidp;
#else
   typedef Byte const *voidpc;
   typedef Byte FAR   *voidpf;
   typedef Byte       *voidp;
#endif

#if !defined(Z_U4) && !defined(Z_SOLO) && defined(STDC)
#  include <limits.h>
#  if (UINT_MAX == 0xffffffffUL)
#    define Z_U4 unsigned
#  elif (ULONG_MAX == 0xffffffffUL)
#    define Z_U4 unsigned long
#  elif (USHRT_MAX == 0xffffffffUL)
#    define Z_U4 unsigned short
#  endif
#endif

#ifdef Z_U4
   typedef Z_U4 z_crc_t;
#else
   typedef unsigned long z_crc_t;
#endif

#ifdef HAVE_UNISTD_H    /* may be set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#endif

#ifdef HAVE_STDARG_H    /* may be set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#endif

#ifdef STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#endif

#if defined(STDC) || defined(Z_HAVE_STDARG_H)
#  ifndef Z_SOLO
#    include <stdarg.h>         /* for va_list */
#  endif
#endif

#ifdef _WIN32
#  ifndef Z_SOLO
#    include <stddef.h>         /* for wchar_t */
#  endif
#endif

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#if defined(_LARGEFILE64_SOURCE) && -_LARGEFILE64_SOURCE - -1 == 1
#  undef _LARGEFILE64_SOURCE
#endif

#if defined(__WATCOMC__) && !defined(Z_HAVE_UNISTD_H)
#  define Z_HAVE_UNISTD_H
#endif
#ifndef Z_SOLO
#  if defined(Z_HAVE_UNISTD_H) || defined(_LARGEFILE64_SOURCE)
#    include <unistd.h>         /* for SEEK_*, off_t, and _LFS64_LARGEFILE */
#    ifdef VMS
#      include <unixio.h>       /* for off_t */
#    endif
#    ifndef z_off_t
#      define z_off_t off_t
#    endif
#  endif
#endif

#if defined(_LFS64_LARGEFILE) && _LFS64_LARGEFILE-0
#  define Z_LFS64
#endif

#if defined(_LARGEFILE64_SOURCE) && defined(Z_LFS64)
#  define Z_LARGE64
#endif

#if defined(_FILE_OFFSET_BITS) && _FILE_OFFSET_BITS-0 == 64 && defined(Z_LFS64)
#  define Z_WANT64
#endif

#if !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif

#ifndef z_off_t
#  define z_off_t long
#endif

#if !defined(_WIN32) && defined(Z_LARGE64)
#  define z_off64_t off64_t
#else
#  if defined(_WIN32) && !defined(__GNUC__) && !defined(Z_SOLO)
#    define z_off64_t __int64
#  else
#    define z_off64_t z_off_t
#  endif
#endif

/* MVS linker does not support external names larger than 8 bytes */
#if defined(__MVS__)
  #pragma map(deflateInit_,"DEIN")
  #pragma map(deflateInit2_,"DEIN2")
  #pragma map(deflateEnd,"DEEND")
  #pragma map(deflateBound,"DEBND")
  #pragma map(inflateInit_,"ININ")
  #pragma map(inflateInit2_,"ININ2")
  #pragma map(inflateEnd,"INEND")
  #pragma map(inflateSync,"INSY")
  #pragma map(inflateSetDictionary,"INSEDI")
  #pragma map(compressBound,"CMBND")
  #pragma map(inflate_table,"INTABL")
  #pragma map(inflate_fast,"INFA")
  #pragma map(inflate_copyright,"INCOPY")
#endif

#endif /* ZCONF_H */
