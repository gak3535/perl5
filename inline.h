/*    inline.h
 *
 *    Copyright (C) 2012 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 * This file is a home for static inline functions that cannot go in other
 * headers files, because they depend on proto.h (included after most other
 * headers) or struct definitions.
 *
 * Each section names the header file that the functions "belong" to.
 */

/* ------------------------------- av.h ------------------------------- */

PERL_STATIC_INLINE SSize_t
S_av_top_index(pTHX_ AV *av)
{
    PERL_ARGS_ASSERT_AV_TOP_INDEX;
    assert(SvTYPE(av) == SVt_PVAV);

    return AvFILL(av);
}

/* ------------------------------- cv.h ------------------------------- */

PERL_STATIC_INLINE GV *
S_CvGV(pTHX_ CV *sv)
{
    return CvNAMED(sv)
	? Perl_cvgv_from_hek(aTHX_ sv)
	: ((XPVCV*)MUTABLE_PTR(SvANY(sv)))->xcv_gv_u.xcv_gv;
}

PERL_STATIC_INLINE I32 *
S_CvDEPTHp(const CV * const sv)
{
    assert(SvTYPE(sv) == SVt_PVCV || SvTYPE(sv) == SVt_PVFM);
    return &((XPVCV*)SvANY(sv))->xcv_depth;
}

/*
 CvPROTO returns the prototype as stored, which is not necessarily what
 the interpreter should be using. Specifically, the interpreter assumes
 that spaces have been stripped, which has been the case if the prototype
 was added by toke.c, but is generally not the case if it was added elsewhere.
 Since we can't enforce the spacelessness at assignment time, this routine
 provides a temporary copy at parse time with spaces removed.
 I<orig> is the start of the original buffer, I<len> is the length of the
 prototype and will be updated when this returns.
 */

#ifdef PERL_CORE
PERL_STATIC_INLINE char *
S_strip_spaces(pTHX_ const char * orig, STRLEN * const len)
{
    SV * tmpsv;
    char * tmps;
    tmpsv = newSVpvn_flags(orig, *len, SVs_TEMP);
    tmps = SvPVX(tmpsv);
    while ((*len)--) {
	if (!isSPACE(*orig))
	    *tmps++ = *orig;
	orig++;
    }
    *tmps = '\0';
    *len = tmps - SvPVX(tmpsv);
		return SvPVX(tmpsv);
}
#endif

/* ------------------------------- mg.h ------------------------------- */

#if defined(PERL_CORE) || defined(PERL_EXT)
/* assumes get-magic and stringification have already occurred */
PERL_STATIC_INLINE STRLEN
S_MgBYTEPOS(pTHX_ MAGIC *mg, SV *sv, const char *s, STRLEN len)
{
    assert(mg->mg_type == PERL_MAGIC_regex_global);
    assert(mg->mg_len != -1);
    if (mg->mg_flags & MGf_BYTES || !DO_UTF8(sv))
	return (STRLEN)mg->mg_len;
    else {
	const STRLEN pos = (STRLEN)mg->mg_len;
	/* Without this check, we may read past the end of the buffer: */
	if (pos > sv_or_pv_len_utf8(sv, s, len)) return len+1;
	return sv_or_pv_pos_u2b(sv, s, pos, NULL);
    }
}
#endif

/* ------------------------------- pad.h ------------------------------ */

#if defined(PERL_IN_PAD_C) || defined(PERL_IN_OP_C)
PERL_STATIC_INLINE bool
PadnameIN_SCOPE(const PADNAME * const pn, const U32 seq)
{
    /* is seq within the range _LOW to _HIGH ?
     * This is complicated by the fact that PL_cop_seqmax
     * may have wrapped around at some point */
    if (COP_SEQ_RANGE_LOW(pn) == PERL_PADSEQ_INTRO)
	return FALSE; /* not yet introduced */

    if (COP_SEQ_RANGE_HIGH(pn) == PERL_PADSEQ_INTRO) {
    /* in compiling scope */
	if (
	    (seq >  COP_SEQ_RANGE_LOW(pn))
	    ? (seq - COP_SEQ_RANGE_LOW(pn) < (U32_MAX >> 1))
	    : (COP_SEQ_RANGE_LOW(pn) - seq > (U32_MAX >> 1))
	)
	    return TRUE;
    }
    else if (
	(COP_SEQ_RANGE_LOW(pn) > COP_SEQ_RANGE_HIGH(pn))
	?
	    (  seq >  COP_SEQ_RANGE_LOW(pn)
	    || seq <= COP_SEQ_RANGE_HIGH(pn))

	:    (  seq >  COP_SEQ_RANGE_LOW(pn)
	     && seq <= COP_SEQ_RANGE_HIGH(pn))
    )
	return TRUE;
    return FALSE;
}
#endif

/* ------------------------------- pp.h ------------------------------- */

PERL_STATIC_INLINE I32
S_TOPMARK(pTHX)
{
    DEBUG_s(DEBUG_v(PerlIO_printf(Perl_debug_log,
				 "MARK top  %p %"IVdf"\n",
				  PL_markstack_ptr,
				  (IV)*PL_markstack_ptr)));
    return *PL_markstack_ptr;
}

PERL_STATIC_INLINE I32
S_POPMARK(pTHX)
{
    DEBUG_s(DEBUG_v(PerlIO_printf(Perl_debug_log,
				 "MARK pop  %p %"IVdf"\n",
				  (PL_markstack_ptr-1),
				  (IV)*(PL_markstack_ptr-1))));
    assert((PL_markstack_ptr > PL_markstack) || !"MARK underflow");
    return *PL_markstack_ptr--;
}

/* ----------------------------- regexp.h ----------------------------- */

PERL_STATIC_INLINE struct regexp *
S_ReANY(const REGEXP * const re)
{
    assert(isREGEXP(re));
    return re->sv_u.svu_rx;
}

/* ------------------------------- sv.h ------------------------------- */

PERL_STATIC_INLINE SV *
S_SvREFCNT_inc(SV *sv)
{
    if (LIKELY(sv != NULL))
	SvREFCNT(sv)++;
    return sv;
}
PERL_STATIC_INLINE SV *
S_SvREFCNT_inc_NN(SV *sv)
{
    SvREFCNT(sv)++;
    return sv;
}
PERL_STATIC_INLINE void
S_SvREFCNT_inc_void(SV *sv)
{
    if (LIKELY(sv != NULL))
	SvREFCNT(sv)++;
}
PERL_STATIC_INLINE void
S_SvREFCNT_dec(pTHX_ SV *sv)
{
    if (LIKELY(sv != NULL)) {
	U32 rc = SvREFCNT(sv);
	if (LIKELY(rc > 1))
	    SvREFCNT(sv) = rc - 1;
	else
	    Perl_sv_free2(aTHX_ sv, rc);
    }
}

PERL_STATIC_INLINE void
S_SvREFCNT_dec_NN(pTHX_ SV *sv)
{
    U32 rc = SvREFCNT(sv);
    if (LIKELY(rc > 1))
	SvREFCNT(sv) = rc - 1;
    else
	Perl_sv_free2(aTHX_ sv, rc);
}

PERL_STATIC_INLINE void
SvAMAGIC_on(SV *sv)
{
    assert(SvROK(sv));
    if (SvOBJECT(SvRV(sv))) HvAMAGIC_on(SvSTASH(SvRV(sv)));
}
PERL_STATIC_INLINE void
SvAMAGIC_off(SV *sv)
{
    if (SvROK(sv) && SvOBJECT(SvRV(sv)))
	HvAMAGIC_off(SvSTASH(SvRV(sv)));
}

PERL_STATIC_INLINE U32
S_SvPADSTALE_on(SV *sv)
{
    assert(!(SvFLAGS(sv) & SVs_PADTMP));
    return SvFLAGS(sv) |= SVs_PADSTALE;
}
PERL_STATIC_INLINE U32
S_SvPADSTALE_off(SV *sv)
{
    assert(!(SvFLAGS(sv) & SVs_PADTMP));
    return SvFLAGS(sv) &= ~SVs_PADSTALE;
}
#if defined(PERL_CORE) || defined (PERL_EXT)
PERL_STATIC_INLINE STRLEN
S_sv_or_pv_pos_u2b(pTHX_ SV *sv, const char *pv, STRLEN pos, STRLEN *lenp)
{
    PERL_ARGS_ASSERT_SV_OR_PV_POS_U2B;
    if (SvGAMAGIC(sv)) {
	U8 *hopped = utf8_hop((U8 *)pv, pos);
	if (lenp) *lenp = (STRLEN)(utf8_hop(hopped, *lenp) - hopped);
	return (STRLEN)(hopped - (U8 *)pv);
    }
    return sv_pos_u2b_flags(sv,pos,lenp,SV_CONST_RETURN);
}
#endif

/* ------------------------------- handy.h ------------------------------- */

/* saves machine code for a common noreturn idiom typically used in Newx*() */
#ifdef GCC_DIAG_PRAGMA
GCC_DIAG_IGNORE(-Wunused-function) /* Intentionally left semicolonless. */
#endif
static void
S_croak_memory_wrap(void)
{
    Perl_croak_nocontext("%s",PL_memory_wrap);
}
#ifdef GCC_DIAG_PRAGMA
GCC_DIAG_RESTORE /* Intentionally left semicolonless. */
#endif

/* ------------------------------- utf8.h ------------------------------- */

/*
=head1 Unicode Support
*/

PERL_STATIC_INLINE void
S_append_utf8_from_native_byte(const U8 byte, U8** dest)
{
    /* Takes an input 'byte' (Latin1 or EBCDIC) and appends it to the UTF-8
     * encoded string at '*dest', updating '*dest' to include it */

    PERL_ARGS_ASSERT_APPEND_UTF8_FROM_NATIVE_BYTE;

    if (NATIVE_BYTE_IS_INVARIANT(byte))
        *(*dest)++ = byte;
    else {
        *(*dest)++ = UTF8_EIGHT_BIT_HI(byte);
        *(*dest)++ = UTF8_EIGHT_BIT_LO(byte);
    }
}

/*
=for apidoc valid_utf8_to_uvchr
Like L</utf8_to_uvchr_buf>(), but should only be called when it is known that
the next character in the input UTF-8 string C<s> is well-formed (I<e.g.>,
it passes C<L</isUTF8_CHAR>>.  Surrogates, non-character code points, and
non-Unicode code points are allowed.

=cut

 */

PERL_STATIC_INLINE UV
Perl_valid_utf8_to_uvchr(const U8 *s, STRLEN *retlen)
{
    UV expectlen = UTF8SKIP(s);
    const U8* send = s + expectlen;
    UV uv = *s;

    PERL_ARGS_ASSERT_VALID_UTF8_TO_UVCHR;

    if (retlen) {
        *retlen = expectlen;
    }

    /* An invariant is trivially returned */
    if (expectlen == 1) {
	return uv;
    }

    /* Remove the leading bits that indicate the number of bytes, leaving just
     * the bits that are part of the value */
    uv = NATIVE_UTF8_TO_I8(uv) & UTF_START_MASK(expectlen);

    /* Now, loop through the remaining bytes, accumulating each into the
     * working total as we go.  (I khw tried unrolling the loop for up to 4
     * bytes, but there was no performance improvement) */
    for (++s; s < send; s++) {
        uv = UTF8_ACCUMULATE(uv, *s);
    }

    return UNI_TO_NATIVE(uv);

}

/*
=for apidoc is_utf8_invariant_string

Returns true iff the first C<len> bytes of the string C<s> are the same
regardless of the UTF-8 encoding of the string (or UTF-EBCDIC encoding on
EBCDIC machines).  That is, if they are UTF-8 invariant.  On ASCII-ish
machines, all the ASCII characters and only the ASCII characters fit this
definition.  On EBCDIC machines, the ASCII-range characters are invariant, but
so also are the C1 controls and C<\c?> (which isn't in the ASCII range on
EBCDIC).

If C<len> is 0, it will be calculated using C<strlen(s)>, (which means if you
use this option, that C<s> can't have embedded C<NUL> characters and has to
have a terminating C<NUL> byte).

See also L</is_utf8_string>(), L</is_utf8_string_loclen>(), and
L</is_utf8_string_loc>().

=cut
*/

PERL_STATIC_INLINE bool
S_is_utf8_invariant_string(const U8* const s, const STRLEN len)
{
    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;

    PERL_ARGS_ASSERT_IS_UTF8_INVARIANT_STRING;

    for (; x < send; ++x) {
	if (!UTF8_IS_INVARIANT(*x))
	    return FALSE;
    }

    return TRUE;
}

/*
=for apidoc is_utf8_string

Returns true if the first C<len> bytes of string C<s> form a valid
UTF-8 string, false otherwise.  If C<len> is 0, it will be calculated
using C<strlen(s)> (which means if you use this option, that C<s> can't have
embedded C<NUL> characters and has to have a terminating C<NUL> byte).  Note
that all characters being ASCII constitute 'a valid UTF-8 string'.

See also L</is_utf8_invariant_string>(), L</is_utf8_string_loclen>(), and
L</is_utf8_string_loc>().

=cut
*/

PERL_STATIC_INLINE bool
Perl_is_utf8_string(const U8 *s, STRLEN len)
{
    /* This is now marked pure in embed.fnc, because isUTF8_CHAR now is pure.
     * Be aware of possible changes to that */

    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;

    PERL_ARGS_ASSERT_IS_UTF8_STRING;

    while (x < send) {
        STRLEN len = isUTF8_CHAR(x, send);
        if (UNLIKELY(! len)) {
            return FALSE;
        }
        x += len;
    }

    return TRUE;
}

/*
Implemented as a macro in utf8.h

=for apidoc is_utf8_string_loc

Like L</is_utf8_string> but stores the location of the failure (in the
case of "utf8ness failure") or the location C<s>+C<len> (in the case of
"utf8ness success") in the C<ep>.

See also L</is_utf8_string_loclen>() and L</is_utf8_string>().

=for apidoc is_utf8_string_loclen

Like L</is_utf8_string>() but stores the location of the failure (in the
case of "utf8ness failure") or the location C<s>+C<len> (in the case of
"utf8ness success") in the C<ep>, and the number of UTF-8
encoded characters in the C<el>.

See also L</is_utf8_string_loc>() and L</is_utf8_string>().

=cut
*/

PERL_STATIC_INLINE bool
Perl_is_utf8_string_loclen(const U8 *s, STRLEN len, const U8 **ep, STRLEN *el)
{
    const U8* const send = s + (len ? len : strlen((const char *)s));
    const U8* x = s;
    STRLEN outlen = 0;

    PERL_ARGS_ASSERT_IS_UTF8_STRING_LOCLEN;

    while (x < send) {
        STRLEN len = isUTF8_CHAR(x, send);
        if (UNLIKELY(! len)) {
            break;
        }
        x += len;
        outlen++;
    }

    if (el)
        *el = outlen;

    if (ep) {
        *ep = x;
    }

    return (x == send);
}

/*
=for apidoc utf8_distance

Returns the number of UTF-8 characters between the UTF-8 pointers C<a>
and C<b>.

WARNING: use only if you *know* that the pointers point inside the
same UTF-8 buffer.

=cut
*/

PERL_STATIC_INLINE IV
Perl_utf8_distance(pTHX_ const U8 *a, const U8 *b)
{
    PERL_ARGS_ASSERT_UTF8_DISTANCE;

    return (a < b) ? -1 * (IV) utf8_length(a, b) : (IV) utf8_length(b, a);
}

/*
=for apidoc utf8_hop

Return the UTF-8 pointer C<s> displaced by C<off> characters, either
forward or backward.

WARNING: do not use the following unless you *know* C<off> is within
the UTF-8 data pointed to by C<s> *and* that on entry C<s> is aligned
on the first byte of character or just after the last byte of a character.

=cut
*/

PERL_STATIC_INLINE U8 *
Perl_utf8_hop(const U8 *s, SSize_t off)
{
    PERL_ARGS_ASSERT_UTF8_HOP;

    /* Note: cannot use UTF8_IS_...() too eagerly here since e.g
     * the bitops (especially ~) can create illegal UTF-8.
     * In other words: in Perl UTF-8 is not just for Unicode. */

    if (off >= 0) {
	while (off--)
	    s += UTF8SKIP(s);
    }
    else {
	while (off++) {
	    s--;
	    while (UTF8_IS_CONTINUATION(*s))
		s--;
	}
    }
    return (U8 *)s;
}

/*

=for apidoc is_utf8_valid_partial_char

Returns 1 if there exists some sequence of bytes, call it C<s'>, that when
appended to the sequence from C<s> through S<C<e - 1>> causes the entire
sequence starting at C<s> (including C<s'>) to be the well-formed UTF-8 of
some code point; otherwise returns 0.

In other words this returns TRUE if C<s> points to the beginning, but partial,
sequence of the UTF-8 for some code point.

This is useful when some fixed-length buffer is being tested for being
well-formed UTF-8, but the final few bytes in it don't comprise a full
character: it is split somewhere in the middle of its UTF-8 representation.
(Presumably when the buffer is refreshed with the next chunk of data, the new
first bytes will complete the partial code point.)   This function is used to
verify that the final bytes in the current buffer are in fact the legal
beginning of some code point, so that if they aren't, the failure can be
signalled without having to wait for the next read.

If the bytes terminated at S<C<e - 1>> are a full character (or more), 0 is
returned.

=cut
*/
PERL_STATIC_INLINE bool
S_is_utf8_valid_partial_char(const U8 * const s, const U8 * const e)
{

    PERL_ARGS_ASSERT_IS_UTF8_VALID_PARTIAL_CHAR;

    if (s >= e || s + UTF8SKIP(s) < e) {
        return FALSE;
    }

    return cBOOL(_is_utf8_char_slow(s, e - s));
}

/* ------------------------------- perl.h ----------------------------- */

/*
=head1 Miscellaneous Functions

=for apidoc AiR|bool|is_safe_syscall|const char *pv|STRLEN len|const char *what|const char *op_name

Test that the given C<pv> doesn't contain any internal C<NUL> characters.
If it does, set C<errno> to C<ENOENT>, optionally warn, and return FALSE.

Return TRUE if the name is safe.

Used by the C<IS_SAFE_SYSCALL()> macro.

=cut
*/

PERL_STATIC_INLINE bool
S_is_safe_syscall(pTHX_ const char *pv, STRLEN len, const char *what, const char *op_name) {
    /* While the Windows CE API provides only UCS-16 (or UTF-16) APIs
     * perl itself uses xce*() functions which accept 8-bit strings.
     */

    PERL_ARGS_ASSERT_IS_SAFE_SYSCALL;

    if (len > 1) {
        char *null_at;
        if (UNLIKELY((null_at = (char *)memchr(pv, 0, len-1)) != NULL)) {
                SETERRNO(ENOENT, LIB_INVARG);
                Perl_ck_warner(aTHX_ packWARN(WARN_SYSCALLS),
                                   "Invalid \\0 character in %s for %s: %s\\0%s",
                                   what, op_name, pv, null_at+1);
                return FALSE;
        }
    }

    return TRUE;
}

/*

Return true if the supplied filename has a newline character
immediately before the first (hopefully only) NUL.

My original look at this incorrectly used the len from SvPV(), but
that's incorrect, since we allow for a NUL in pv[len-1].

So instead, strlen() and work from there.

This allow for the user reading a filename, forgetting to chomp it,
then calling:

  open my $foo, "$file\0";

*/

#ifdef PERL_CORE

PERL_STATIC_INLINE bool
S_should_warn_nl(const char *pv) {
    STRLEN len;

    PERL_ARGS_ASSERT_SHOULD_WARN_NL;

    len = strlen(pv);

    return len > 0 && pv[len-1] == '\n';
}

#endif

/* ------------------ pp.c, regcomp.c, toke.c, universal.c ------------ */

#define MAX_CHARSET_NAME_LENGTH 2

PERL_STATIC_INLINE const char *
get_regex_charset_name(const U32 flags, STRLEN* const lenp)
{
    /* Returns a string that corresponds to the name of the regex character set
     * given by 'flags', and *lenp is set the length of that string, which
     * cannot exceed MAX_CHARSET_NAME_LENGTH characters */

    *lenp = 1;
    switch (get_regex_charset(flags)) {
        case REGEX_DEPENDS_CHARSET: return DEPENDS_PAT_MODS;
        case REGEX_LOCALE_CHARSET:  return LOCALE_PAT_MODS;
        case REGEX_UNICODE_CHARSET: return UNICODE_PAT_MODS;
	case REGEX_ASCII_RESTRICTED_CHARSET: return ASCII_RESTRICT_PAT_MODS;
	case REGEX_ASCII_MORE_RESTRICTED_CHARSET:
	    *lenp = 2;
	    return ASCII_MORE_RESTRICT_PAT_MODS;
    }
    /* The NOT_REACHED; hides an assert() which has a rather complex
     * definition in perl.h. */
    NOT_REACHED; /* NOTREACHED */
    return "?";	    /* Unknown */
}

/*

Return false if any get magic is on the SV other than taint magic.

*/

PERL_STATIC_INLINE bool
S_sv_only_taint_gmagic(SV *sv) {
    MAGIC *mg = SvMAGIC(sv);

    PERL_ARGS_ASSERT_SV_ONLY_TAINT_GMAGIC;

    while (mg) {
        if (mg->mg_type != PERL_MAGIC_taint
            && !(mg->mg_flags & MGf_GSKIP)
            && mg->mg_virtual->svt_get) {
            return FALSE;
        }
        mg = mg->mg_moremagic;
    }

    return TRUE;
}

/* ------------------ cop.h ------------------------------------------- */


/* Enter a block. Push a new base context and return its address. */

PERL_STATIC_INLINE PERL_CONTEXT *
S_cx_pushblock(pTHX_ U8 type, U8 gimme, SV** sp, I32 saveix)
{
    PERL_CONTEXT * cx;

    PERL_ARGS_ASSERT_CX_PUSHBLOCK;

    CXINC;
    cx = CX_CUR();
    cx->cx_type        = type;
    cx->blk_gimme      = gimme;
    cx->blk_oldsaveix  = saveix;
    cx->blk_oldsp      = (I32)(sp - PL_stack_base);
    cx->blk_oldcop     = PL_curcop;
    cx->blk_oldmarksp  = (I32)(PL_markstack_ptr - PL_markstack);
    cx->blk_oldscopesp = PL_scopestack_ix;
    cx->blk_oldpm      = PL_curpm;
    cx->blk_old_tmpsfloor = PL_tmps_floor;

    PL_tmps_floor        = PL_tmps_ix;
    CX_DEBUG(cx, "PUSH");
    return cx;
}


/* Exit a block (RETURN and LAST). */

PERL_STATIC_INLINE void
S_cx_popblock(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_POPBLOCK;

    CX_DEBUG(cx, "POP");
    /* these 3 are common to cx_popblock and cx_topblock */
    PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp;
    PL_scopestack_ix = cx->blk_oldscopesp;
    PL_curpm         = cx->blk_oldpm;

    /* LEAVE_SCOPE() should have made this true. /(?{})/ cheats
     * and leaves a CX entry lying around for repeated use, so
     * skip for multicall */                  \
    assert(   (CxTYPE(cx) == CXt_SUB && CxMULTICALL(cx))
            || PL_savestack_ix == cx->blk_oldsaveix);
    PL_curcop     = cx->blk_oldcop;
    PL_tmps_floor = cx->blk_old_tmpsfloor;
}

/* Continue a block elsewhere (e.g. NEXT, REDO, GOTO).
 * Whereas cx_popblock() restores the state to the point just before
 * cx_pushblock() was called,  cx_topblock() restores it to the point just
 * *after* cx_pushblock() was called. */

PERL_STATIC_INLINE void
S_cx_topblock(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_TOPBLOCK;

    CX_DEBUG(cx, "TOP");
    /* these 3 are common to cx_popblock and cx_topblock */
    PL_markstack_ptr = PL_markstack + cx->blk_oldmarksp;
    PL_scopestack_ix = cx->blk_oldscopesp;
    PL_curpm         = cx->blk_oldpm;

    PL_stack_sp      = PL_stack_base + cx->blk_oldsp;
}


PERL_STATIC_INLINE void
S_cx_pushsub(pTHX_ PERL_CONTEXT *cx, CV *cv, OP *retop, bool hasargs)
{
    U8 phlags = CX_PUSHSUB_GET_LVALUE_MASK(Perl_was_lvalue_sub);

    PERL_ARGS_ASSERT_CX_PUSHSUB;

    PERL_DTRACE_PROBE_ENTRY(cv);
    cx->blk_sub.cv = cv;
    cx->blk_sub.olddepth = CvDEPTH(cv);
    cx->blk_sub.prevcomppad = PL_comppad;
    cx->cx_type |= (hasargs) ? CXp_HASARGS : 0;
    cx->blk_sub.retop = retop;
    SvREFCNT_inc_simple_void_NN(cv);
    cx->blk_u16 = PL_op->op_private & (phlags|OPpDEREF);
}


/* subsets of cx_popsub() */

PERL_STATIC_INLINE void
S_cx_popsub_common(pTHX_ PERL_CONTEXT *cx)
{
    CV *cv;

    PERL_ARGS_ASSERT_CX_POPSUB_COMMON;
    assert(CxTYPE(cx) == CXt_SUB);

    PL_comppad = cx->blk_sub.prevcomppad;
    PL_curpad = LIKELY(PL_comppad) ? AvARRAY(PL_comppad) : NULL;
    cv = cx->blk_sub.cv;
    CvDEPTH(cv) = cx->blk_sub.olddepth;
    cx->blk_sub.cv = NULL;
    SvREFCNT_dec(cv);
}


/* handle the @_ part of leaving a sub */

PERL_STATIC_INLINE void
S_cx_popsub_args(pTHX_ PERL_CONTEXT *cx)
{
    AV *av;

    PERL_ARGS_ASSERT_CX_POPSUB_ARGS;
    assert(CxTYPE(cx) == CXt_SUB);
    assert(AvARRAY(MUTABLE_AV(
        PadlistARRAY(CvPADLIST(cx->blk_sub.cv))[
                CvDEPTH(cx->blk_sub.cv)])) == PL_curpad);

    CX_POP_SAVEARRAY(cx);
    av = MUTABLE_AV(PAD_SVl(0));
    if (UNLIKELY(AvREAL(av)))
        /* abandon @_ if it got reified */
        clear_defarray(av, 0);
    else {
        CLEAR_ARGARRAY(av);
    }
}


PERL_STATIC_INLINE void
S_cx_popsub(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_POPSUB;
    assert(CxTYPE(cx) == CXt_SUB);

    PERL_DTRACE_PROBE_RETURN(cx->blk_sub.cv);

    if (CxHASARGS(cx))
        cx_popsub_args(cx);
    cx_popsub_common(cx);
}


PERL_STATIC_INLINE void
S_cx_pushformat(pTHX_ PERL_CONTEXT *cx, CV *cv, OP *retop, GV *gv)
{
    PERL_ARGS_ASSERT_CX_PUSHFORMAT;

    cx->blk_format.cv          = cv;
    cx->blk_format.retop       = retop;
    cx->blk_format.gv          = gv;
    cx->blk_format.dfoutgv     = PL_defoutgv;
    cx->blk_format.prevcomppad = PL_comppad;
    cx->blk_u16                = 0;

    SvREFCNT_inc_simple_void_NN(cv);
    CvDEPTH(cv)++;
    SvREFCNT_inc_void(cx->blk_format.dfoutgv);
}


PERL_STATIC_INLINE void
S_cx_popformat(pTHX_ PERL_CONTEXT *cx)
{
    CV *cv;
    GV *dfout;

    PERL_ARGS_ASSERT_CX_POPFORMAT;
    assert(CxTYPE(cx) == CXt_FORMAT);

    dfout = cx->blk_format.dfoutgv;
    setdefout(dfout);
    cx->blk_format.dfoutgv = NULL;
    SvREFCNT_dec_NN(dfout);

    PL_comppad = cx->blk_format.prevcomppad;
    PL_curpad = LIKELY(PL_comppad) ? AvARRAY(PL_comppad) : NULL;
    cv = cx->blk_format.cv;
    cx->blk_format.cv = NULL;
    --CvDEPTH(cv);
    SvREFCNT_dec_NN(cv);
}


PERL_STATIC_INLINE void
S_cx_pusheval(pTHX_ PERL_CONTEXT *cx, OP *retop, SV *namesv)
{
    PERL_ARGS_ASSERT_CX_PUSHEVAL;

    cx->blk_eval.retop         = retop;
    cx->blk_eval.old_namesv    = namesv;
    cx->blk_eval.old_eval_root = PL_eval_root;
    cx->blk_eval.cur_text      = PL_parser ? PL_parser->linestr : NULL;
    cx->blk_eval.cv            = NULL; /* later set by doeval_compile() */
    cx->blk_eval.cur_top_env   = PL_top_env;

    assert(!(PL_in_eval     & ~ 0x7F));
    assert(!(PL_op->op_type & ~0x1FF));
    cx->blk_u16 = (PL_in_eval & 0x7F) | ((U16)PL_op->op_type << 7);
}


PERL_STATIC_INLINE void
S_cx_popeval(pTHX_ PERL_CONTEXT *cx)
{
    SV *sv;

    PERL_ARGS_ASSERT_CX_POPEVAL;
    assert(CxTYPE(cx) == CXt_EVAL);

    PL_in_eval = CxOLD_IN_EVAL(cx);
    PL_eval_root = cx->blk_eval.old_eval_root;
    sv = cx->blk_eval.cur_text;
    if (sv && SvSCREAM(sv)) {
        cx->blk_eval.cur_text = NULL;
        SvREFCNT_dec_NN(sv);
    }

    sv = cx->blk_eval.old_namesv;
    if (sv) {
        cx->blk_eval.old_namesv = NULL;
        SvREFCNT_dec_NN(sv);
    }
}


/* push a plain loop, i.e.
 *     { block }
 *     while (cond) { block }
 *     for (init;cond;continue) { block }
 * This loop can be last/redo'ed etc.
 */

PERL_STATIC_INLINE void
S_cx_pushloop_plain(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_PUSHLOOP_PLAIN;
    cx->blk_loop.my_op = cLOOP;
}


/* push a true for loop, i.e.
 *     for var (list) { block }
 */

PERL_STATIC_INLINE void
S_cx_pushloop_for(pTHX_ PERL_CONTEXT *cx, void *itervarp, SV* itersave)
{
    PERL_ARGS_ASSERT_CX_PUSHLOOP_FOR;

    /* this one line is common with cx_pushloop_plain */
    cx->blk_loop.my_op = cLOOP;

    cx->blk_loop.itervar_u.svp = (SV**)itervarp;
    cx->blk_loop.itersave      = itersave;
#ifdef USE_ITHREADS
    cx->blk_loop.oldcomppad = PL_comppad;
#endif
}


/* pop all loop types, including plain */

PERL_STATIC_INLINE void
S_cx_poploop(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_POPLOOP;

    assert(CxTYPE_is_LOOP(cx));
    if (  CxTYPE(cx) == CXt_LOOP_ARY
       || CxTYPE(cx) == CXt_LOOP_LAZYSV)
    {
        /* Free ary or cur. This assumes that state_u.ary.ary
         * aligns with state_u.lazysv.cur. See cx_dup() */
        SV *sv = cx->blk_loop.state_u.lazysv.cur;
        cx->blk_loop.state_u.lazysv.cur = NULL;
        SvREFCNT_dec_NN(sv);
        if (CxTYPE(cx) == CXt_LOOP_LAZYSV) {
            sv = cx->blk_loop.state_u.lazysv.end;
            cx->blk_loop.state_u.lazysv.end = NULL;
            SvREFCNT_dec_NN(sv);
        }
    }
    if (cx->cx_type & (CXp_FOR_PAD|CXp_FOR_GV)) {
        SV *cursv;
        SV **svp = (cx)->blk_loop.itervar_u.svp;
        if ((cx->cx_type & CXp_FOR_GV))
            svp = &GvSV((GV*)svp);
        cursv = *svp;
        *svp = cx->blk_loop.itersave;
        cx->blk_loop.itersave = NULL;
        SvREFCNT_dec(cursv);
    }
}


PERL_STATIC_INLINE void
S_cx_pushwhen(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_PUSHWHEN;

    cx->blk_givwhen.leave_op = cLOGOP->op_other;
}


PERL_STATIC_INLINE void
S_cx_popwhen(pTHX_ PERL_CONTEXT *cx)
{
    PERL_ARGS_ASSERT_CX_POPWHEN;
    assert(CxTYPE(cx) == CXt_WHEN);

    PERL_UNUSED_ARG(cx);
    PERL_UNUSED_CONTEXT;
    /* currently NOOP */
}


PERL_STATIC_INLINE void
S_cx_pushgiven(pTHX_ PERL_CONTEXT *cx, SV *orig_defsv)
{
    PERL_ARGS_ASSERT_CX_PUSHGIVEN;

    cx->blk_givwhen.leave_op = cLOGOP->op_other;
    cx->blk_givwhen.defsv_save = orig_defsv;
}


PERL_STATIC_INLINE void
S_cx_popgiven(pTHX_ PERL_CONTEXT *cx)
{
    SV *sv;

    PERL_ARGS_ASSERT_CX_POPGIVEN;
    assert(CxTYPE(cx) == CXt_GIVEN);

    sv = GvSV(PL_defgv);
    GvSV(PL_defgv) = cx->blk_givwhen.defsv_save;
    cx->blk_givwhen.defsv_save = NULL;
    SvREFCNT_dec(sv);
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
