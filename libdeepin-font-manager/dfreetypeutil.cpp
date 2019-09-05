#include "dfreetypeutil.h"

static void
FcStrCaseWalkerInit (const FcChar8 *src, FcCaseWalker *w)
{
    w->src = src;
    w->read = 0;
}

static FcChar8
FcStrCaseWalkerLong (FcCaseWalker *w, FcChar8 r)
{
    FcChar32 ucs4;
    int slen;
    int len = strlen((char *)w->src);

    slen = FcUtf8ToUcs4 (w->src - 1, &ucs4, len + 1);
    if (slen <= 0)
        return r;
    if (FC_MIN_FOLD_CHAR <= ucs4 && ucs4 <= FC_MAX_FOLD_CHAR) {
        int min = 0;
        int max = FC_NUM_CASE_FOLD;

        while (min <= max) {
            int mid = (min + max) >> 1;
            FcChar32    low = fcCaseFold[mid].upper;
            FcChar32    high = low + FcCaseFoldUpperCount (&fcCaseFold[mid]);

            if (high <= ucs4)
                min = mid + 1;
            else if (ucs4 < low)
                max = mid - 1;
            else {
                const FcCaseFold    *fold = &fcCaseFold[mid];
                int dlen;

                switch (fold->method) {
                case  FC_CASE_FOLD_EVEN_ODD:
                    if ((ucs4 & 1) != (fold->upper & 1))
                        return r;
                /* fall through ... */
                default:
                    dlen = FcUcs4ToUtf8 (ucs4 + fold->offset, w->utf8);
                    break;
                case FC_CASE_FOLD_FULL:
                    dlen = fold->count;
                    memcpy (w->utf8, fcCaseFoldChars + fold->offset, dlen);
                    break;
                }

                /* consume rest of src utf-8 bytes */
                w->src += slen - 1;

                /* read from temp buffer */
                w->utf8[dlen] = '\0';
                w->read = w->utf8;
                return *w->read++;
            }
        }
    }
    return r;
}

static FcChar8
FcStrCaseWalkerNext (FcCaseWalker *w, const char *delims)
{
    FcChar8 r;

    if (w->read) {
        if ((r = *w->read++))
            return r;
        w->read = 0;
    }
    do {
        r = *w->src++;
    } while (r != 0 && delims && strchr (delims, r));

    if ((r & 0xc0) == 0xc0)
        return FcStrCaseWalkerLong (w, r);
    if ('A' <= r && r <= 'Z')
        r = r - 'A' + 'a';
    return r;
}

FcChar8 *
FcStrDowncase (const FcChar8 *s)
{
    FcCaseWalker w;
    int len = 0;
    FcChar8 *dst, *d;

    FcStrCaseWalkerInit (s, &w);
    while (FcStrCaseWalkerNext (&w, NULL))
        len++;
    d = dst = (FcChar8 *)malloc (len + 1);
    if (!d)
        return 0;
    FcStrCaseWalkerInit (s, &w);
    while ((*d++ = FcStrCaseWalkerNext (&w, NULL)));
    return dst;
}

int
FcStrCmpIgnoreCase (const FcChar8 *s1, const FcChar8 *s2)
{
    FcCaseWalker w1, w2;
    FcChar8 c1, c2;

    if (s1 == s2) return 0;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;) {
        c1 = FcStrCaseWalkerNext (&w1, NULL);
        c2 = FcStrCaseWalkerNext (&w2, NULL);
        if (!c1 || (c1 != c2))
            break;
    }
    return (int) c1 - (int) c2;
}

int
FcStrCmpIgnoreCaseAndDelims (const FcChar8 *s1, const FcChar8 *s2, const FcChar8 *delims)
{
    FcCaseWalker w1, w2;
    FcChar8 c1, c2;

    if (s1 == s2) return 0;

    FcStrCaseWalkerInit (s1, &w1);
    FcStrCaseWalkerInit (s2, &w2);

    for (;;) {
        c1 = FcStrCaseWalkerNext (&w1, (const char *)delims);
        c2 = FcStrCaseWalkerNext (&w2, (const char *)delims);
        if (!c1 || (c1 != c2))
            break;
    }
    return (int) c1 - (int) c2;
}

int
FcStrCmpIgnoreBlanksAndCase (const FcChar8 *s1, const FcChar8 *s2)
{
    return FcStrCmpIgnoreCaseAndDelims (s1, s2, (const FcChar8 *)" ");
}

static FcBool
FcLooksLikeSJIS (FcChar8 *string, int len)
{
    int nhigh = 0, nlow = 0;

    while (len-- > 0) {
        if (*string++ & 0x80) nhigh++;
        else nlow++;
    }
    /*
     * Heuristic -- if more than 1/3 of the bytes have the high-bit set,
     * this is likely to be SJIS and not ROMAN
     */
    if (nhigh * 2 > nlow)
        return FcTrue;
    return FcFalse;
}

static FcChar8 *FcSfntNameTranscode(FT_SfntName *sname)
{
    int    i;
    const char *fromcode;

    FcChar8 *utf8;

    for (i = 0; i < NUM_FC_FT_ENCODING; i++)
        if (fcFtEncoding[i].platform_id == sname->platform_id &&
                (fcFtEncoding[i].encoding_id == TT_ENCODING_DONT_CARE ||
                 fcFtEncoding[i].encoding_id == sname->encoding_id))
            break;
    if (i == NUM_FC_FT_ENCODING)
        return 0;
    fromcode = fcFtEncoding[i].fromcode;

    /*
     * Many names encoded for TT_PLATFORM_MACINTOSH are broken
     * in various ways. Kludge around them.
     */
    if (!strcmp (fromcode, FC_ENCODING_MAC_ROMAN)) {
        if (sname->language_id == TT_MAC_LANGID_ENGLISH &&
                FcLooksLikeSJIS (sname->string, sname->string_len)) {
            fromcode = "SJIS";
        } else if (sname->language_id >= 0x100) {
            /*
             * "real" Mac language IDs are all less than 150.
             * Names using one of the MS language IDs are assumed
             * to use an associated encoding (Yes, this is a kludge)
             */
            int f;

            fromcode = NULL;
            for (f = 0; f < NUM_FC_MAC_ROMAN_FAKE; f++)
                if (fcMacRomanFake[f].language_id == sname->language_id) {
                    fromcode = fcMacRomanFake[f].fromcode;
                    break;
                }
            if (!fromcode)
                return 0;
        }
    }
    if (!strcmp (fromcode, "UCS-2BE") || !strcmp (fromcode, "UTF-16BE")) {
        FcChar8 *src = sname->string;
        int src_len = sname->string_len;
        int len;
        int wchar;
        int ilen, olen;
        FcChar8 *u8;
        FcChar32 ucs4;

        /*
         * Convert Utf16 to Utf8
         */

        if (!FcUtf16Len (src, FcEndianBig, src_len, &len, &wchar))
            return 0;

        /*
         * Allocate plenty of space.  Freed below
         */
        utf8 = (FcChar8 *)malloc (len * FC_UTF8_MAX_LEN + 1);
        if (!utf8)
            return 0;

        u8 = utf8;

        while ((ilen = FcUtf16ToUcs4 (src, FcEndianBig, &ucs4, src_len)) > 0) {
            src_len -= ilen;
            src += ilen;
            olen = FcUcs4ToUtf8 (ucs4, u8);
            u8 += olen;
        }
        *u8 = '\0';
        goto done;
    }
    if (!strcmp (fromcode, "ASCII") || !strcmp (fromcode, "ISO-8859-1")) {
        FcChar8 *src = sname->string;
        int src_len = sname->string_len;
        int olen;
        FcChar8 *u8;
        FcChar32 ucs4;

        /*
         * Convert Latin1 to Utf8. Freed below
         */
        utf8 = (FcChar8 *)malloc (src_len * 2 + 1);
        if (!utf8)
            return 0;

        u8 = utf8;
        while (src_len > 0) {
            ucs4 = *src++;
            src_len--;
            olen = FcUcs4ToUtf8 (ucs4, u8);
            u8 += olen;
        }
        *u8 = '\0';
        goto done;
    }
    if (!strcmp (fromcode, FC_ENCODING_MAC_ROMAN)) {
        FcChar8 *src = sname->string;
        int src_len = sname->string_len;
        int olen;
        FcChar8 *u8;
        FcChar32 ucs4;

        /*
         * Convert Latin1 to Utf8. Freed below
         */
        utf8 = (FcChar8 *)malloc (src_len * 3 + 1);
        if (!utf8)
            return 0;

        u8 = utf8;
        while (src_len > 0) {
            ucs4 = *src++;
            if (ucs4 >= 128)
                ucs4 = fcMacRomanNonASCIIToUnicode[ucs4 - 128];
            src_len--;
            olen = FcUcs4ToUtf8 (ucs4, u8);
            u8 += olen;
        }
        *u8 = '\0';
        goto done;
    }

    return 0;
done:
    if (FcStrCmpIgnoreBlanksAndCase (utf8, (FcChar8 *) "") == 0) {
        free (utf8);
        return 0;
    }
    return utf8;
}

char *DFreeTypeUtil::getFontFamilyName(FT_Face face)
{
    int p, platform;
    int n, nameid;
    FT_SfntName sname;
    FT_UInt snamei, snamec;

    nameid = TT_NAME_ID_FULL_NAME;

    FcChar8 *utf8 = NULL;
    snamec = FT_Get_Sfnt_Name_Count(face);
    for (p = 0; p <= NUM_PLATFORM_ORDER; p++) {
        if (p < NUM_PLATFORM_ORDER)
            platform = platform_order[p];
        else
            platform = 0xffff;

        for (n = 0; n < NUM_NAMEID_ORDER; n++) {
            for (snamei = 0; snamei < snamec; snamei++) {
                const FcChar8 *lang;
                const char *elt = 0, *eltlang = 0;
                int *np = 0, *nlangp = 0;

                if (FT_Get_Sfnt_Name(face, snamei, &sname) != 0)
                    continue;
                if (sname.name_id != nameid)
                    continue;

                if (p < NUM_PLATFORM_ORDER) {
                    if (sname.platform_id != platform)
                        continue;
                } else {
                    int sp;
                    for (sp = 0; sp < NUM_PLATFORM_ORDER; sp++)
                        if (sname.platform_id == platform_order[sp])
                            break;
                    if (sp != NUM_PLATFORM_ORDER)
                        continue;
                }

                utf8 = FcSfntNameTranscode(&sname);
            }
        }
    }

    return (char *)utf8;
}
