/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.28 from the
 * contents of Notify.xs. Do not edit this file, edit Notify.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Notify.xs"
#include "module.h"

#line 13 "Notify.c"
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

#line 157 "Notify.c"

XS_EUPXS(XS_Purple__Notify_close); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_close)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "type, ui_handle");
    {
	Purple__NotifyType	type = (Purple__NotifyType)SvIV(ST(0))
;
	void *	ui_handle = INT2PTR(void *,SvIV(ST(1)))
;

	purple_notify_close(type, ui_handle);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Notify_close_with_handle); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_close_with_handle)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "handle");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;

	purple_notify_close_with_handle(handle);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Notify_email); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_email)
{
    dVAR; dXSARGS;
    if (items != 7)
       croak_xs_usage(cv,  "handle, subject, from, to, url, cb, user_data");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;
	const char *	subject = (const char *)SvPV_nolen(ST(1))
;
	const char *	from = (const char *)SvPV_nolen(ST(2))
;
	const char *	to = (const char *)SvPV_nolen(ST(3))
;
	const char *	url = (const char *)SvPV_nolen(ST(4))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(5)))
;
	gpointer	user_data = purple_perl_ref_object(ST(6))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_email(handle, subject, from, to, url, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_emails); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_emails)
{
    dVAR; dXSARGS;
    if (items != 9)
       croak_xs_usage(cv,  "handle, count, detailed, subjects, froms, tos, urls, cb, user_data");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;
	size_t	count = (size_t)SvUV(ST(1))
;
	gboolean	detailed = (bool)SvTRUE(ST(2))
;
	const char **	subjects = INT2PTR(const char **,SvIV(ST(3)))
;
	const char **	froms = INT2PTR(const char **,SvIV(ST(4)))
;
	const char **	tos = INT2PTR(const char **,SvIV(ST(5)))
;
	const char **	urls = INT2PTR(const char **,SvIV(ST(6)))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(7)))
;
	gpointer	user_data = purple_perl_ref_object(ST(8))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_emails(handle, count, detailed, subjects, froms, tos, urls, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_formatted); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_formatted)
{
    dVAR; dXSARGS;
    if (items != 7)
       croak_xs_usage(cv,  "handle, title, primary, secondary, text, cb, user_data");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;
	const char *	title = (const char *)SvPV_nolen(ST(1))
;
	const char *	primary = (const char *)SvPV_nolen(ST(2))
;
	const char *	secondary = (const char *)SvPV_nolen(ST(3))
;
	const char *	text = (const char *)SvPV_nolen(ST(4))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(5)))
;
	gpointer	user_data = purple_perl_ref_object(ST(6))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_formatted(handle, title, primary, secondary, text, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_userinfo); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_userinfo)
{
    dVAR; dXSARGS;
    if (items != 5)
       croak_xs_usage(cv,  "gc, who, user_info, cb, user_data");
    {
	Purple__Connection	gc = purple_perl_ref_object(ST(0))
;
	const char *	who = (const char *)SvPV_nolen(ST(1))
;
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(2))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(3)))
;
	gpointer	user_data = purple_perl_ref_object(ST(4))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_userinfo(gc, who, user_info, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_message); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_message)
{
    dVAR; dXSARGS;
    if (items != 7)
       croak_xs_usage(cv,  "handle, type, title, primary, secondary, cb, user_data");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;
	Purple__NotifyMsgType	type = (Purple__NotifyMsgType)SvIV(ST(1))
;
	const char *	title = (const char *)SvPV_nolen(ST(2))
;
	const char *	primary = (const char *)SvPV_nolen(ST(3))
;
	const char *	secondary = (const char *)SvPV_nolen(ST(4))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(5)))
;
	gpointer	user_data = purple_perl_ref_object(ST(6))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_message(handle, type, title, primary, secondary, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_searchresults); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_searchresults)
{
    dVAR; dXSARGS;
    if (items != 7)
       croak_xs_usage(cv,  "gc, title, primary, secondary, results, cb, user_data");
    {
	Purple__Connection	gc = purple_perl_ref_object(ST(0))
;
	const char *	title = (const char *)SvPV_nolen(ST(1))
;
	const char *	primary = (const char *)SvPV_nolen(ST(2))
;
	const char *	secondary = (const char *)SvPV_nolen(ST(3))
;
	Purple__NotifySearchResults	results = purple_perl_ref_object(ST(4))
;
	Purple__NotifyCloseCallback	cb = INT2PTR(Purple__NotifyCloseCallback,SvIV(ST(5)))
;
	gpointer	user_data = purple_perl_ref_object(ST(6))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_searchresults(gc, title, primary, secondary, results, cb, user_data);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Notify_uri); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Notify_uri)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "handle, uri");
    {
	void *	handle = INT2PTR(void *,SvIV(ST(0)))
;
	const char *	uri = (const char *)SvPV_nolen(ST(1))
;
	void *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_uri(handle, uri);
	XSprePUSH; PUSHi(PTR2IV(RETVAL));
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__NotifyUserInfo_new); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_new)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "class");
    {
	Purple__NotifyUserInfo	RETVAL;

	RETVAL = purple_notify_user_info_new(/* void */);
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::NotifyUserInfo");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__NotifyUserInfo_destroy); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_destroy)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;

	purple_notify_user_info_destroy(user_info);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_get_entries); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_get_entries)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;
#line 136 "Notify.xs"
	GList *l;
#line 450 "Notify.c"
#line 138 "Notify.xs"
	l = purple_notify_user_info_get_entries(user_info);
	for (; l != NULL; l = l->next) {
		XPUSHs(sv_2mortal(purple_perl_bless_object(l->data, "Purple::NotifyUserInfoEntry")));
	}
#line 456 "Notify.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__NotifyUserInfo_get_text_with_newline); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_get_text_with_newline)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "user_info, newline");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;
	const char *	newline = (const char *)SvPV_nolen(ST(1))
;
	gchar_own *	RETVAL;

	RETVAL = purple_notify_user_info_get_text_with_newline(user_info, newline);
	{
	    SV * RETVALSV;
	    RETVALSV = sv_newmortal();
	    /* used when we can directly own the returned string. */
	    /* we have to copy in the case when perl's malloc != gtk's malloc,
	     * so best copy all the time. */
	    sv_setpv ((SV*)RETVALSV, RETVAL);
	    SvUTF8_on (RETVALSV);
	    g_free (RETVAL);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__NotifyUserInfo_add_pair); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_add_pair)
{
    dVAR; dXSARGS;
    if (items != 3)
       croak_xs_usage(cv,  "user_info, label, value");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;
	const char *	label = (const char *)SvPV_nolen(ST(1))
;
	const char *	value = (const char *)SvPV_nolen(ST(2))
;

	purple_notify_user_info_add_pair(user_info, label, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_prepend_pair); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_prepend_pair)
{
    dVAR; dXSARGS;
    if (items != 3)
       croak_xs_usage(cv,  "user_info, label, value");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;
	const char *	label = (const char *)SvPV_nolen(ST(1))
;
	const char *	value = (const char *)SvPV_nolen(ST(2))
;

	purple_notify_user_info_prepend_pair(user_info, label, value);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_add_section_break); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_add_section_break)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;

	purple_notify_user_info_add_section_break(user_info);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_add_section_header); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_add_section_header)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "user_info, label");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;
	const char *	label = (const char *)SvPV_nolen(ST(1))
;

	purple_notify_user_info_add_section_header(user_info, label);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_remove_last_item); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_remove_last_item)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info");
    {
	Purple__NotifyUserInfo	user_info = purple_perl_ref_object(ST(0))
;

	purple_notify_user_info_remove_last_item(user_info);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__NotifyUserInfo_entry_get_label); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_entry_get_label)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info_entry");
    {
	Purple__NotifyUserInfoEntry	user_info_entry = purple_perl_ref_object(ST(0))
;
	const gchar *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_user_info_entry_get_label(user_info_entry);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__NotifyUserInfo_entry_get_value); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__NotifyUserInfo_entry_get_value)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "user_info_entry");
    {
	Purple__NotifyUserInfoEntry	user_info_entry = purple_perl_ref_object(ST(0))
;
	const gchar *	RETVAL;
	dXSTARG;

	RETVAL = purple_notify_user_info_entry_get_value(user_info_entry);
	sv_setpv(TARG, RETVAL); XSprePUSH; PUSHTARG;
    }
    XSRETURN(1);
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Purple__Notify); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Purple__Notify)
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

        (void)newXSproto_portable("Purple::Notify::close", XS_Purple__Notify_close, file, "$$");
        (void)newXSproto_portable("Purple::Notify::close_with_handle", XS_Purple__Notify_close_with_handle, file, "$");
        (void)newXSproto_portable("Purple::Notify::email", XS_Purple__Notify_email, file, "$$$$$$$");
        (void)newXSproto_portable("Purple::Notify::emails", XS_Purple__Notify_emails, file, "$$$$$$$$$");
        (void)newXSproto_portable("Purple::Notify::formatted", XS_Purple__Notify_formatted, file, "$$$$$$$");
        (void)newXSproto_portable("Purple::Notify::userinfo", XS_Purple__Notify_userinfo, file, "$$$$$");
        (void)newXSproto_portable("Purple::Notify::message", XS_Purple__Notify_message, file, "$$$$$$$");
        (void)newXSproto_portable("Purple::Notify::searchresults", XS_Purple__Notify_searchresults, file, "$$$$$$$");
        (void)newXSproto_portable("Purple::Notify::uri", XS_Purple__Notify_uri, file, "$$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::new", XS_Purple__NotifyUserInfo_new, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::destroy", XS_Purple__NotifyUserInfo_destroy, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::get_entries", XS_Purple__NotifyUserInfo_get_entries, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::get_text_with_newline", XS_Purple__NotifyUserInfo_get_text_with_newline, file, "$$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::add_pair", XS_Purple__NotifyUserInfo_add_pair, file, "$$$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::prepend_pair", XS_Purple__NotifyUserInfo_prepend_pair, file, "$$$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::add_section_break", XS_Purple__NotifyUserInfo_add_section_break, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::add_section_header", XS_Purple__NotifyUserInfo_add_section_header, file, "$$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::remove_last_item", XS_Purple__NotifyUserInfo_remove_last_item, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::entry_get_label", XS_Purple__NotifyUserInfo_entry_get_label, file, "$");
        (void)newXSproto_portable("Purple::NotifyUserInfo::entry_get_value", XS_Purple__NotifyUserInfo_entry_get_value, file, "$");

    /* Initialisation Section */

#line 7 "Notify.xs"
{
	HV *type_stash = gv_stashpv("Purple::Notify::Type", 1);
	HV *msg_type_stash = gv_stashpv("Purple::Notify::Msg", 1);
	HV *user_info_stash = gv_stashpv("Purple::NotifyUserInfo::Type", 1);

	static const constiv *civ, type_const_iv[] = {
#define const_iv(name) {#name, (IV)PURPLE_NOTIFY_##name}
		const_iv(MESSAGE),
		const_iv(EMAIL),
		const_iv(EMAILS),
		const_iv(FORMATTED),
		const_iv(SEARCHRESULTS),
		const_iv(USERINFO),
		const_iv(URI),
	};
	static const constiv msg_type_const_iv[] = {
#undef const_iv
#define const_iv(name) {#name, (IV)PURPLE_NOTIFY_MSG_##name}
		const_iv(ERROR),
		const_iv(WARNING),
		const_iv(INFO),
	};
	static const constiv user_info_const_iv[] = {
#undef const_iv
#define const_iv(name) {#name, (IV)PURPLE_NOTIFY_USER_INFO_ENTRY_##name}
		const_iv(PAIR),
		const_iv(SECTION_BREAK),
		const_iv(SECTION_HEADER),
	};

	for (civ = type_const_iv + sizeof(type_const_iv) / sizeof(type_const_iv[0]); civ-- > type_const_iv; )
		newCONSTSUB(type_stash, (char *)civ->name, newSViv(civ->iv));

	for (civ = msg_type_const_iv + sizeof(msg_type_const_iv) / sizeof(msg_type_const_iv[0]); civ-- > msg_type_const_iv; )
		newCONSTSUB(msg_type_stash, (char *)civ->name, newSViv(civ->iv));

	for (civ = user_info_const_iv + sizeof(user_info_const_iv) / sizeof(user_info_const_iv[0]); civ-- > user_info_const_iv; )
		newCONSTSUB(user_info_stash, (char *)civ->name, newSViv(civ->iv));
}

#line 712 "Notify.c"

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
