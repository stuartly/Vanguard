/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.28 from the
 * contents of Prefs.xs. Do not edit this file, edit Prefs.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Prefs.xs"
#include "module.h"
#include "../perl-handlers.h"

#line 14 "Prefs.c"
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

#line 158 "Prefs.c"

XS_EUPXS(XS_Purple__Prefs_add_bool); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_bool)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	gboolean	value = (bool)SvTRUE(ST(1))
;

	purple_prefs_add_bool(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_add_int); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_int)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	int	value = (int)SvIV(ST(1))
;

	purple_prefs_add_int(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_add_none); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_none)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;

	purple_prefs_add_none(name);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_add_string); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_string)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	value = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_add_string(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_add_string_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_string_list)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	SV *	value = ST(1)
;
#line 50 "Prefs.xs"
	GList *t_GL;
	int i, t_len;
#line 246 "Prefs.c"
#line 53 "Prefs.xs"
	t_GL = NULL;
	t_len = av_len((AV *)SvRV(value));

	for (i = 0; i <= t_len; i++)
		t_GL = g_list_append(t_GL, SvPVutf8_nolen(*av_fetch((AV *)SvRV(value), i, 0)));

	purple_prefs_add_string_list(name, t_GL);
	g_list_free(t_GL);
#line 256 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_add_path); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_path)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	value = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_add_path(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_add_path_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_add_path_list)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	SV *	value = ST(1)
;
#line 72 "Prefs.xs"
	GList *t_GL;
	int i, t_len;
#line 297 "Prefs.c"
#line 75 "Prefs.xs"
	t_GL = NULL;
	t_len = av_len((AV *)SvRV(value));

	for (i = 0; i <= t_len; i++)
		t_GL = g_list_append(t_GL, SvPVutf8_nolen(*av_fetch((AV *)SvRV(value), i, 0)));

	purple_prefs_add_path_list(name, t_GL);
	g_list_free(t_GL);
#line 307 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_destroy); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_destroy)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {

	purple_prefs_destroy();
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_connect_callback); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_connect_callback)
{
    dVAR; dXSARGS;
    if (items < 3 || items > 4)
       croak_xs_usage(cv,  "plugin, name, callback, data = 0");
    {
	Purple__Plugin	plugin = purple_perl_ref_object(ST(0))
;
	const char *	name = (const char *)SvPV_nolen(ST(1))
;
	SV *	callback = ST(2)
;
	SV *	data;
	guint	RETVAL;
	dXSTARG;

	if (items < 4)
	    data = 0;
	else {
	    data = ST(3)
;
	}
#line 94 "Prefs.xs"
	RETVAL = purple_perl_prefs_connect_callback(plugin, name, callback, data);
#line 353 "Prefs.c"
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_disconnect_by_handle); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_disconnect_by_handle)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "plugin");
    {
	Purple__Plugin	plugin = purple_perl_ref_object(ST(0))
;
#line 102 "Prefs.xs"
	purple_perl_pref_cb_clear_for_plugin(plugin);
#line 371 "Prefs.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_disconnect_callback); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_disconnect_callback)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "callback_id");
    {
	guint	callback_id = (guint)SvIV(ST(0))
;
#line 108 "Prefs.xs"
	purple_perl_prefs_disconnect_callback(callback_id);
#line 388 "Prefs.c"
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_exists); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_exists)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	gboolean	RETVAL;

	RETVAL = purple_prefs_exists(name);
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_path); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_path)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	RETVAL;
	dXSTARG;

	RETVAL = purple_prefs_get_path(name);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_path_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_path_list)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
#line 122 "Prefs.xs"
	GList *l;
#line 444 "Prefs.c"
#line 124 "Prefs.xs"
	for (l = purple_prefs_get_path_list(name); l != NULL; l = g_list_delete_link(l, l)) {
		XPUSHs(sv_2mortal(newSVpv(l->data, 0)));
		g_free(l->data);
	}
#line 450 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_get_bool); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_bool)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	gboolean	RETVAL;

	RETVAL = purple_prefs_get_bool(name);
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_handle); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_handle)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {
	Purple__Handle	RETVAL;

	RETVAL = purple_prefs_get_handle();
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::Handle");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_int); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_int)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	int	RETVAL;
	dXSTARG;

	RETVAL = purple_prefs_get_int(name);
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_string); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_string)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	RETVAL;
	dXSTARG;

	RETVAL = purple_prefs_get_string(name);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_get_string_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_string_list)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
#line 148 "Prefs.xs"
	GList *l;
#line 547 "Prefs.c"
#line 150 "Prefs.xs"
	for (l = purple_prefs_get_string_list(name); l != NULL; l = g_list_delete_link(l, l)) {
		XPUSHs(sv_2mortal(newSVpv(l->data, 0)));
		g_free(l->data);
	}
#line 553 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_get_type); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_type)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	Purple__PrefType	RETVAL;
	dXSTARG;

	RETVAL = purple_prefs_get_type(name);
	XSprePUSH; PUSHi((IV)RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_load); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_load)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {
	gboolean	RETVAL;

	RETVAL = purple_prefs_load();
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Prefs_remove); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_remove)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;

	purple_prefs_remove(name);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_rename); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_rename)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "oldname, newname");
    {
	const char *	oldname = (const char *)SvPV_nolen(ST(0))
;
	const char *	newname = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_rename(oldname, newname);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_rename_boolean_toggle); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_rename_boolean_toggle)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "oldname, newname");
    {
	const char *	oldname = (const char *)SvPV_nolen(ST(0))
;
	const char *	newname = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_rename_boolean_toggle(oldname, newname);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_bool); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_bool)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	gboolean	value = (bool)SvTRUE(ST(1))
;

	purple_prefs_set_bool(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_generic); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_generic)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	gpointer	value = purple_perl_ref_object(ST(1))
;

	purple_prefs_set_generic(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_int); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_int)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	int	value = (int)SvIV(ST(1))
;

	purple_prefs_set_int(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_string); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_string)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	value = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_set_string(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_string_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_string_list)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	SV *	value = ST(1)
;
#line 201 "Prefs.xs"
	GList *t_GL;
	int i, t_len;
#line 735 "Prefs.c"
#line 204 "Prefs.xs"
	t_GL = NULL;
	t_len = av_len((AV *)SvRV(value));

	for (i = 0; i <= t_len; i++)
		t_GL = g_list_append(t_GL, SvPVutf8_nolen(*av_fetch((AV *)SvRV(value), i, 0)));

	purple_prefs_set_string_list(name, t_GL);
	g_list_free(t_GL);
#line 745 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_set_path); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_path)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	const char *	value = (const char *)SvPV_nolen(ST(1))
;

	purple_prefs_set_path(name, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_set_path_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_set_path_list)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "name, value");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
	SV *	value = ST(1)
;
#line 223 "Prefs.xs"
	GList *t_GL;
	int i, t_len;
#line 786 "Prefs.c"
#line 226 "Prefs.xs"
	t_GL = NULL;
	t_len = av_len((AV *)SvRV(value));

	for (i = 0; i <= t_len; i++)
		t_GL = g_list_append(t_GL, SvPVutf8_nolen(*av_fetch((AV *)SvRV(value), i, 0)));

	purple_prefs_set_path_list(name, t_GL);
	g_list_free(t_GL);
#line 796 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_trigger_callback); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_trigger_callback)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;

	purple_prefs_trigger_callback(name);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Prefs_get_children_names); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_get_children_names)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "name");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	const char *	name = (const char *)SvPV_nolen(ST(0))
;
#line 244 "Prefs.xs"
	GList *l;
#line 832 "Prefs.c"
#line 246 "Prefs.xs"
	for (l = purple_prefs_get_children_names(name); l != NULL; l = g_list_delete_link(l, l)) {
		XPUSHs(sv_2mortal(newSVpv(l->data, 0)));
		g_free(l->data);
	}
#line 838 "Prefs.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Prefs_update_old); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Prefs_update_old)
{
    dVAR; dXSARGS;
    if (items != 0)
       croak_xs_usage(cv,  "");
    {

	purple_prefs_update_old();
    }
    XSRETURN_EMPTY;
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Purple__Prefs); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Purple__Prefs)
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

        (void)newXSproto_portable("Purple::Prefs::add_bool", XS_Purple__Prefs_add_bool, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::add_int", XS_Purple__Prefs_add_int, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::add_none", XS_Purple__Prefs_add_none, file, "$");
        (void)newXSproto_portable("Purple::Prefs::add_string", XS_Purple__Prefs_add_string, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::add_string_list", XS_Purple__Prefs_add_string_list, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::add_path", XS_Purple__Prefs_add_path, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::add_path_list", XS_Purple__Prefs_add_path_list, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::destroy", XS_Purple__Prefs_destroy, file, "");
        (void)newXSproto_portable("Purple::Prefs::connect_callback", XS_Purple__Prefs_connect_callback, file, "$$$;$");
        (void)newXSproto_portable("Purple::Prefs::disconnect_by_handle", XS_Purple__Prefs_disconnect_by_handle, file, "$");
        (void)newXSproto_portable("Purple::Prefs::disconnect_callback", XS_Purple__Prefs_disconnect_callback, file, "$");
        (void)newXSproto_portable("Purple::Prefs::exists", XS_Purple__Prefs_exists, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_path", XS_Purple__Prefs_get_path, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_path_list", XS_Purple__Prefs_get_path_list, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_bool", XS_Purple__Prefs_get_bool, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_handle", XS_Purple__Prefs_get_handle, file, "");
        (void)newXSproto_portable("Purple::Prefs::get_int", XS_Purple__Prefs_get_int, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_string", XS_Purple__Prefs_get_string, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_string_list", XS_Purple__Prefs_get_string_list, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_type", XS_Purple__Prefs_get_type, file, "$");
        (void)newXSproto_portable("Purple::Prefs::load", XS_Purple__Prefs_load, file, "");
        (void)newXSproto_portable("Purple::Prefs::remove", XS_Purple__Prefs_remove, file, "$");
        (void)newXSproto_portable("Purple::Prefs::rename", XS_Purple__Prefs_rename, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::rename_boolean_toggle", XS_Purple__Prefs_rename_boolean_toggle, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_bool", XS_Purple__Prefs_set_bool, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_generic", XS_Purple__Prefs_set_generic, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_int", XS_Purple__Prefs_set_int, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_string", XS_Purple__Prefs_set_string, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_string_list", XS_Purple__Prefs_set_string_list, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_path", XS_Purple__Prefs_set_path, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::set_path_list", XS_Purple__Prefs_set_path_list, file, "$$");
        (void)newXSproto_portable("Purple::Prefs::trigger_callback", XS_Purple__Prefs_trigger_callback, file, "$");
        (void)newXSproto_portable("Purple::Prefs::get_children_names", XS_Purple__Prefs_get_children_names, file, "$");
        (void)newXSproto_portable("Purple::Prefs::update_old", XS_Purple__Prefs_update_old, file, "");

    /* Initialisation Section */

#line 8 "Prefs.xs"
{
	HV *stash = gv_stashpv("Purple::Pref::Type", 1);

	static const constiv *civ, const_iv[] = {
#define const_iv(name) {#name, (IV)PURPLE_PREF_##name}
		const_iv(NONE),
		const_iv(BOOLEAN),
		const_iv(INT),
		const_iv(STRING),
		const_iv(STRING_LIST),
		const_iv(PATH),
		const_iv(PATH_LIST),
	};

	for (civ = const_iv + sizeof(const_iv) / sizeof(const_iv[0]); civ-- > const_iv; )
		newCONSTSUB(stash, (char *)civ->name, newSViv(civ->iv));
}

#line 942 "Prefs.c"

    /* End of Initialisation Section */

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
