/*
 * This file was generated automatically by ExtUtils::ParseXS version 3.28 from the
 * contents of Roomlist.xs. Do not edit this file, edit Roomlist.xs instead.
 *
 *    ANY CHANGES MADE HERE WILL BE LOST!
 *
 */

#line 1 "Roomlist.xs"
#include "module.h"

#line 13 "Roomlist.c"
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

#line 157 "Roomlist.c"

XS_EUPXS(XS_Purple__Roomlist_cancel_get_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_cancel_get_list)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "list");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;

	purple_roomlist_cancel_get_list(list);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_expand_category); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_expand_category)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "list, category");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist__Room	category = purple_perl_ref_object(ST(1))
;

	purple_roomlist_expand_category(list, category);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_get_in_progress); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_get_in_progress)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "list");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	gboolean	RETVAL;

	RETVAL = purple_roomlist_get_in_progress(list);
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Roomlist_get_list); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_get_list)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "gc");
    {
	Purple__Connection	gc = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist	RETVAL;

	RETVAL = purple_roomlist_get_list(gc);
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::Roomlist");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Roomlist_new); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_new)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "account");
    {
	Purple__Account	account = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist	RETVAL;

	RETVAL = purple_roomlist_new(account);
	{
	    SV * RETVALSV;
	    RETVALSV = purple_perl_bless_object(RETVAL, "Purple::Roomlist");
	    RETVALSV = sv_2mortal(RETVALSV);
	    ST(0) = RETVALSV;
	}
    }
    XSRETURN(1);
}


XS_EUPXS(XS_Purple__Roomlist_ref); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_ref)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "list");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;

	purple_roomlist_ref(list);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_room_add); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_room_add)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "list, room");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist__Room	room = purple_perl_ref_object(ST(1))
;

	purple_roomlist_room_add(list, room);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_room_add_field); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_room_add_field)
{
    dVAR; dXSARGS;
    if (items != 3)
       croak_xs_usage(cv,  "list, room, field");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist__Room	room = purple_perl_ref_object(ST(1))
;
	gconstpointer	field = INT2PTR(gconstpointer,SvIV(ST(2)))
;

	purple_roomlist_room_add_field(list, room, field);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_room_join); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_room_join)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "list, room");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	Purple__Roomlist__Room	room = purple_perl_ref_object(ST(1))
;

	purple_roomlist_room_join(list, room);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_set_fields); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_set_fields)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "list, fields");
    PERL_UNUSED_VAR(ax); /* -Wall */
    SP -= items;
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	SV *	fields = ST(1)
;
#line 77 "Roomlist.xs"
	GList *t_GL;
	int i, t_len;
#line 345 "Roomlist.c"
#line 80 "Roomlist.xs"
	t_GL = NULL;
	t_len = av_len((AV *)SvRV(fields));

	for (i = 0; i <= t_len; i++)
		t_GL = g_list_append(t_GL, SvPVutf8_nolen(*av_fetch((AV *)SvRV(fields), i, 0)));

	purple_roomlist_set_fields(list, t_GL);
#line 354 "Roomlist.c"
	PUTBACK;
	return;
    }
}


XS_EUPXS(XS_Purple__Roomlist_set_in_progress); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_set_in_progress)
{
    dVAR; dXSARGS;
    if (items != 2)
       croak_xs_usage(cv,  "list, in_progress");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;
	gboolean	in_progress = (bool)SvTRUE(ST(1))
;

	purple_roomlist_set_in_progress(list, in_progress);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_show_with_account); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_show_with_account)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "account");
    {
	Purple__Account	account = purple_perl_ref_object(ST(0))
;

	purple_roomlist_show_with_account(account);
    }
    XSRETURN_EMPTY;
}


XS_EUPXS(XS_Purple__Roomlist_unref); /* prototype to pass -Wmissing-prototypes */
XS_EUPXS(XS_Purple__Roomlist_unref)
{
    dVAR; dXSARGS;
    if (items != 1)
       croak_xs_usage(cv,  "list");
    {
	Purple__Roomlist	list = purple_perl_ref_object(ST(0))
;

	purple_roomlist_unref(list);
    }
    XSRETURN_EMPTY;
}

#ifdef __cplusplus
extern "C"
#endif
XS_EXTERNAL(boot_Purple__Roomlist); /* prototype to pass -Wmissing-prototypes */
XS_EXTERNAL(boot_Purple__Roomlist)
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

        (void)newXSproto_portable("Purple::Roomlist::cancel_get_list", XS_Purple__Roomlist_cancel_get_list, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::expand_category", XS_Purple__Roomlist_expand_category, file, "$$");
        (void)newXSproto_portable("Purple::Roomlist::get_in_progress", XS_Purple__Roomlist_get_in_progress, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::get_list", XS_Purple__Roomlist_get_list, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::new", XS_Purple__Roomlist_new, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::ref", XS_Purple__Roomlist_ref, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::room_add", XS_Purple__Roomlist_room_add, file, "$$");
        (void)newXSproto_portable("Purple::Roomlist::room_add_field", XS_Purple__Roomlist_room_add_field, file, "$$$");
        (void)newXSproto_portable("Purple::Roomlist::room_join", XS_Purple__Roomlist_room_join, file, "$$");
        (void)newXSproto_portable("Purple::Roomlist::set_fields", XS_Purple__Roomlist_set_fields, file, "$$");
        (void)newXSproto_portable("Purple::Roomlist::set_in_progress", XS_Purple__Roomlist_set_in_progress, file, "$$");
        (void)newXSproto_portable("Purple::Roomlist::show_with_account", XS_Purple__Roomlist_show_with_account, file, "$");
        (void)newXSproto_portable("Purple::Roomlist::unref", XS_Purple__Roomlist_unref, file, "$");

    /* Initialisation Section */

#line 7 "Roomlist.xs"
{
	HV *room_stash = gv_stashpv("Purple::Roomlist::Room::Type", 1);
	HV *field_stash = gv_stashpv("Purple::Roomlist::Field::Type", 1);

	static const constiv *civ, room_const_iv[] = {
#define const_iv(name) {#name, (IV)PURPLE_ROOMLIST_ROOMTYPE_##name}
		const_iv(CATEGORY),
		const_iv(ROOM),
	};
	static const constiv field_const_iv[] = {
#undef const_iv
#define const_iv(name) {#name, (IV)PURPLE_ROOMLIST_FIELD_##name}
		const_iv(BOOL),
		const_iv(INT),
		const_iv(STRING),
	};

	for (civ = room_const_iv + sizeof(room_const_iv) / sizeof(room_const_iv[0]); civ-- > room_const_iv; )
		newCONSTSUB(room_stash, (char *)civ->name, newSViv(civ->iv));

	for (civ = field_const_iv + sizeof(field_const_iv) / sizeof(field_const_iv[0]); civ-- > field_const_iv; )
		newCONSTSUB(field_stash, (char *)civ->name, newSViv(civ->iv));
}

#line 479 "Roomlist.c"

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

