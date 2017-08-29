/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.28 from the
 * contents of Network.xs. Do not edit this file, edit Network.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Network.xs"
#include "module.h"

#line 13 "Network.c"
#ifndef PERL_UNUSED_VAR
#  define PERL_UNUSED_VAR(var) if (0) var = var
#endif

#ifndef dVAR
#  define dVAR		dNOOP
#endif


/* This stuff is not part of the API! You have been warned. */
#ifndef PERL_VERSION_DECIMAL
#  define PERL_VERSION_DECIMAL(r,v,s) (r*1000000 + v*1000 + s)
#endif
#ifndef PERL_DECIMAL_VERSION
#  define PERL_DECIMAL_VERSION \
	  PERL_VERSION_DECIMAL(PERL_REVISION,PERL_VERSION,PERL_SUBVERSION)
#endif
#ifndef PERL_VERSION_GE
#  define PERL_VERSION_GE(r,v,s) \
	  (PERL_DECIMAL_VERSION >= PERL_VERSION_DECIMAL(r,v,s))
#endif
#ifndef PERL_VERSION_LE
#  define PERL_VERSION_LE(r,v,s) \
	  (PERL_DECIMAL_VERSION <= PERL_VERSION_DECIMAL(r,v,s))
#endif

/* XS_INTERNAL is the explicit static-linkage variant of the default
 * XS macro.
 *
 * XS_EXTERNAL is the same as XS_INTERNAL except it does not include
 * "STATIC", ie. it exports XSUB symbols. You probably don't want that
 * for anything but the BOOT XSUB.
 *
 * See XSUB.h in core!
 */


/* TODO: This might be compatible further back than 5.10.0. */
#if PERL_VERSION_GE(5, 10, 0) && PERL_VERSION_LE(5, 15, 1)
#  undef XS_EXTERNAL
#  undef XS_INTERNAL
#  if defined(__CYGWIN__) && defined(USE_DYNAMIC_LOADING)
#    define XS_EXTERNAL(name) __declspec(dllexport) XSPROTO(name)
#    define XS_INTERNAL(name) STATIC XSPROTO(name)
#  endif
#  if defined(__SYMBIAN32__)
#    define XS_EXTERNAL(name) EXPORT_C XSPROTO(name)
#    define XS_INTERNAL(name) EXPORT_C STATIC XSPROTO(name)
#  endif
#  ifndef XS_EXTERNAL
#    if defined(HASATTRIBUTE_UNUSED) && !defined(__cplusplus)
#      define XS_EXTERNAL(name) void name(pTHX_ CV* cv __attribute__unused__)
#      define XS_INTERNAL(name) STATIC void name(pTHX_ CV* cv __attribute__unused__)
#    else
#      ifdef __cplusplus
#        define XS_EXTERNAL(name) extern "C" XSPROTO(name)
#        define XS_INTERNAL(name) static XSPROTO(name)
#      else
#        define XS_EXTERNAL(name) XSPROTO(name)
#        define XS_INTERNAL(name) STATIC XSPROTO(name)
#      endif
#    endif
#  endif
#endif

/* perl >= 5.10.0 && perl <= 5.15.1 */


/* The XS_EXTERNAL macro is used for functions that must not be static
 * like the boot XSUB of a module. If perl didn't have an XS_EXTERNAL
 * macro defined, the best we can do is assume XS is the same.
 * Dito for XS_INTERNAL.
 */
#ifndef XS_EXTERNAL
#  define XS_EXTERNAL(name) XS(name)
#endif
#ifndef XS_INTERNAL
#  define XS_INTERNAL(name) XS(name)
#endif

/* Now, finally, after all this mess, we want an ExtUtils::ParseXS
 * internal macro that we're free to redefine for varying linkage due
 * to the EXPORT_XSUB_SYMBOLS XS keyword. This is internal, use
 * XS_EXTERNAL(name) or XS_INTERNAL(name) in your code if you need to!
 */

#undef XS_EUPXS
#if defined(PERL_EUPXS_ALWAYS_EXPORT)
#  define XS_EUPXS(name) XS_EXTERNAL(name)
#else
   /* default to internal */
#  define XS_EUPXS(name) XS_INTERNAL(name)
#endif

#ifndef PERL_ARGS_ASSERT_CROAK_XS_USAGE
#define PERL_ARGS_ASSERT_CROAK_XS_USAGE assert(cv); assert(params)

/* prototype to pass -Wmissing-prototypes */
STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params);

STATIC void
S_croak_xs_usage(const CV *const cv, const char *const params)
{
    const GV *const gv = CvGV(cv);

    PERL_ARGS_ASSERT_CROAK_XS_USAGE;

    if (gv) {
        const char *const gvname = GvNAME(gv);
        const HV *const stash = GvSTASH(gv);
        const char *const hvname = stash ? HvNAME(stash) : NULL;

        if (hvname)
	    Perl_croak_nocontext("Usage: %s::%s(%s)", hvname, gvname, params);
        else
	    Perl_croak_nocontext("Usage: %s(%s)", gvname, params);
    } else {
        /* Pants. I don't think that it should be possible to get here. */
	Perl_croak_nocontext("Usage: CODE(0x%"UVxf")(%s)", PTR2UV(cv), params);
    }
}
#undef  PERL_ARGS_ASSERT_CROAK_XS_USAGE

#define croak_xs_usage        S_croak_xs_usage

#endif

/* NOTE: the prototype of newXSproto() is different in versions of perls,
 * so we define a portable version of newXSproto()
 */
#ifdef newXS_flags
#define newXSproto_portable(name, c_impl, file, proto) newXS_flags(name, c_impl, file, proto, 0)
#else
#define newXSproto_portable(name, c_impl, file, proto) (PL_Sv=(SV*)newXS(name, c_impl, file), sv_setpv(PL_Sv, proto), (CV*)PL_Sv)
#endif /* !defined(newXS_flags) */

#if PERL_VERSION_LE(5, 21, 5)
#  define newXS_deffile(a,b) Perl_newXS(aTHX_ a,b,file)
#else
#  define newXS_deffile(a,b) Perl_newXS_deffile(aTHX_ a,b)
#endif

#line 157 "Network.c"

XS_EUPXS(XS_Purple__Network_get_local_system_ip); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_get_local_system_ip)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "fd");
    {
	int	fd = (int)SvIV(ST(0))
;
	const char *	RETVAL;
	dXSTARG;

	RETVAL = purple_network_get_local_system_ip(fd);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_get_my_ip); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_get_my_ip)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "fd");
    {
	int	fd = (int)SvIV(ST(0))
;
	const char *	RETVAL;
	dXSTARG;

	RETVAL = purple_network_get_my_ip(fd);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_get_port_from_fd); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_get_port_from_fd)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "fd");
    {
	int	fd = (int)SvIV(ST(0))
;
	unsigned short	RETVAL;
	dXSTARG;

	RETVAL = purple_network_get_port_from_fd(fd);
	XSprePUSH; PUSHu((UV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_get_public_ip); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_get_public_ip)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {
	const char *	RETVAL;
	dXSTARG;

	RETVAL = purple_network_get_public_ip();
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_ip_atoi); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_ip_atoi)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "ip");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	ip = (const char *)SvPV_nolen(ST(0))
;
	const unsigned char *	RETVAL;
	dXSTARG;
#line 25 "Network.xs"
	RETVAL = purple_network_ip_atoi(ip);
	sv_setpvn(TARG, (const char *)RETVAL, 4);
	XSprePUSH;
	PUSHTARG;
#line 251 "Network.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Network_listen); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_listen)
{
    dVAR; dXSARGS;
    if (items != 4)
       croak_xs_usage(cv,  "port, socket_type, cb, cb_data");
    {
	unsigned short	port = (unsigned short)SvUV(ST(0))
;
	int	socket_type = (int)SvIV(ST(1))
;
	Purple__NetworkListenCallback	cb = INT2PTR(Purple__NetworkListenCallback,SvIV(ST(2)))
;
	gpointer	cb_data = purple_perl_ref_object(ST(3))
;
	Purple__NetworkListenData	RETVAL;

	RETVAL = purple_network_listen(port, socket_type, cb, cb_data);
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::NetworkListenData");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_listen_range); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_listen_range)
{
    dVAR; dXSARGS;
    if (items != 5)
       croak_xs_usage(cv,  "start, end, socket_type, cb, cb_data");
    {
	unsigned short	start = (unsigned short)SvUV(ST(0))
;
	unsigned short	end = (unsigned short)SvUV(ST(1))
;
	int	socket_type = (int)SvIV(ST(2))
;
	Purple__NetworkListenCallback	cb = INT2PTR(Purple__NetworkListenCallback,SvIV(ST(3)))
;
	gpointer	cb_data = purple_perl_ref_object(ST(4))
;
	Purple__NetworkListenData	RETVAL;

	RETVAL = purple_network_listen_range(start, end, socket_type, cb, cb_data);
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::NetworkListenData");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Network_set_public_ip); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Network_set_public_ip)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "ip");
    {
	const char *	ip = (const char *)SvPV_nolen(ST(0))
;

	purple_network_set_public_ip(ip);
    }
    XSRETURN_EMPTY;
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Purple__Network); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Purple__Network)
{
#if PERL_VERSION_LE(5, 21, 5)
    dVAR; dXSARGS;
#else
    dVAR; dXSBOOTARGSXSAPIVERCHK;
#endif
#if (PERL_REVISION == 5 && PERL_VERSION < 9)
    char* file = __FILE__;
#else
    const char* file = __FILE__;
#endif

    PERL_UNUSED_VAR(file);

    PERL_UNUSED_VAR(cv); /* -W */
    PERL_UNUSED_VAR(items); /* -W */
#if PERL_VERSION_LE(5, 21, 5)
    XS_VERSION_BOOTCHECK;
#  ifdef XS_APIVERSION_BOOTCHECK
    XS_APIVERSION_BOOTCHECK;
#  endif
#endif

        (void)newXSproto_portable("Purple::Network::get_local_system_ip", XS_Purple__Network_get_local_system_ip, file, "$");
        (void)newXSproto_portable("Purple::Network::get_my_ip", XS_Purple__Network_get_my_ip, file, "$");
        (void)newXSproto_portable("Purple::Network::get_port_from_fd", XS_Purple__Network_get_port_from_fd, file, "$");
        (void)newXSproto_portable("Purple::Network::get_public_ip", XS_Purple__Network_get_public_ip, file, "");
        (void)newXSproto_portable("Purple::Network::ip_atoi", XS_Purple__Network_ip_atoi, file, "$");
        (void)newXSproto_portable("Purple::Network::listen", XS_Purple__Network_listen, file, "$$$$");
        (void)newXSproto_portable("Purple::Network::listen_range", XS_Purple__Network_listen_range, file, "$$$$$");
        (void)newXSproto_portable("Purple::Network::set_public_ip", XS_Purple__Network_set_public_ip, file, "$");
#if PERL_VERSION_LE(5, 21, 5)
#  if PERL_VERSION_GE(5, 9, 0)
    if (PL_unitcheckav)
        call_list(PL_scopestack_ix, PL_unitcheckav);
#  endif
    XSRETURN_YES;
#else
    Perl_xs_boot_epilog(aTHX_ ax);
#endif
}

