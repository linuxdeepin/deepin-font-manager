#ifndef DFREETYPEUTIL_H
#define DFREETYPEUTIL_H

#include <fontconfig/fontconfig.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H


#define NUM_PLATFORM_ORDER (sizeof (platform_order) / sizeof (platform_order[0]))
#define NUM_NAMEID_ORDER  (sizeof (nameid_order) / sizeof (nameid_order[0]))
#define TT_ENCODING_DONT_CARE   0xffff
#define FC_ENCODING_MAC_ROMAN   "MACINTOSH"

#define FC_NUM_CASE_FOLD    295
#define FC_NUM_CASE_FOLD_CHARS  471
#define FC_MAX_CASE_FOLD_CHARS  6
#define FC_MAX_CASE_FOLD_EXPAND 4
#define FC_MIN_FOLD_CHAR    0x00000041
#define FC_MAX_FOLD_CHAR    0x0001e921


#define FC_CASE_FOLD_RANGE 0
#define FC_CASE_FOLD_EVEN_ODD 1
#define FC_CASE_FOLD_FULL 2

#define NUM_FC_FT_ENCODING  (int) (sizeof (fcFtEncoding) / sizeof (fcFtEncoding[0]))
#define NUM_FC_MAC_ROMAN_FAKE   (int) (sizeof (fcMacRomanFake) / sizeof (fcMacRomanFake[0]))

#define FcCaseFoldUpperCount(cf) \
    ((cf)->method == FC_CASE_FOLD_FULL ? 1 : (cf)->count)

static const FT_UShort platform_order[] = {
    TT_PLATFORM_MICROSOFT,
    TT_PLATFORM_APPLE_UNICODE,
    TT_PLATFORM_MACINTOSH,
};

static const FT_UShort nameid_order[] = {
    TT_NAME_ID_PREFERRED_FAMILY,
    TT_NAME_ID_FONT_FAMILY,
    TT_NAME_ID_MAC_FULL_NAME,
    TT_NAME_ID_FULL_NAME,
    TT_NAME_ID_PREFERRED_SUBFAMILY,
    TT_NAME_ID_FONT_SUBFAMILY,
    TT_NAME_ID_TRADEMARK,
    TT_NAME_ID_MANUFACTURER,
};

typedef struct {
    const FT_UShort platform_id;
    const FT_UShort encoding_id;
    const char  fromcode[12];
} FcFtEncoding;

typedef struct _FcCaseFold {
    FcChar32    upper;
    FcChar16    method : 2;
    FcChar16    count : 14;
    short       offset; /* lower - upper for RANGE, table id for FULL */
} FcCaseFold;

typedef struct {
    FT_UShort   language_id;
    char    fromcode[12];
} FcMacRomanFake;

typedef struct _FcCaseWalker {
    const FcChar8   *read;
    const FcChar8   *src;
    FcChar8 utf8[FC_MAX_CASE_FOLD_CHARS + 1];
} FcCaseWalker;

static const FcMacRomanFake fcMacRomanFake[] = {
    {  TT_MS_LANGID_JAPANESE_JAPAN,    "SJIS-WIN" },
    {  TT_MS_LANGID_ENGLISH_UNITED_STATES, "ASCII" },
};

static const FcFtEncoding   fcFtEncoding[] = {
    { TT_PLATFORM_APPLE_UNICODE,    TT_ENCODING_DONT_CARE,  "UCS-2BE" },
    { TT_PLATFORM_MACINTOSH,    TT_MAC_ID_ROMAN,    "MACINTOSH" },
    { TT_PLATFORM_MACINTOSH,    TT_MAC_ID_JAPANESE, "SJIS" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_UNICODE_CS,    "UTF-16BE" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_SJIS,      "SJIS-WIN" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_GB2312,    "GB3212" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_BIG_5,     "BIG-5" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_WANSUNG,   "Wansung" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_JOHAB,     "Johab" },
    { TT_PLATFORM_MICROSOFT,    TT_MS_ID_UCS_4,     "UCS4" },
    { TT_PLATFORM_ISO,      TT_ISO_ID_7BIT_ASCII,   "ASCII" },
    { TT_PLATFORM_ISO,      TT_ISO_ID_10646,    "UCS-2BE" },
    { TT_PLATFORM_ISO,      TT_ISO_ID_8859_1,   "ISO-8859-1" },
};

static const FcChar16 fcMacRomanNonASCIIToUnicode[128] = {
    /*0x80*/ 0x00C4, /* LATIN CAPITAL LETTER A WITH DIAERESIS */
    /*0x81*/ 0x00C5, /* LATIN CAPITAL LETTER A WITH RING ABOVE */
    /*0x82*/ 0x00C7, /* LATIN CAPITAL LETTER C WITH CEDILLA */
    /*0x83*/ 0x00C9, /* LATIN CAPITAL LETTER E WITH ACUTE */
    /*0x84*/ 0x00D1, /* LATIN CAPITAL LETTER N WITH TILDE */
    /*0x85*/ 0x00D6, /* LATIN CAPITAL LETTER O WITH DIAERESIS */
    /*0x86*/ 0x00DC, /* LATIN CAPITAL LETTER U WITH DIAERESIS */
    /*0x87*/ 0x00E1, /* LATIN SMALL LETTER A WITH ACUTE */
    /*0x88*/ 0x00E0, /* LATIN SMALL LETTER A WITH GRAVE */
    /*0x89*/ 0x00E2, /* LATIN SMALL LETTER A WITH CIRCUMFLEX */
    /*0x8A*/ 0x00E4, /* LATIN SMALL LETTER A WITH DIAERESIS */
    /*0x8B*/ 0x00E3, /* LATIN SMALL LETTER A WITH TILDE */
    /*0x8C*/ 0x00E5, /* LATIN SMALL LETTER A WITH RING ABOVE */
    /*0x8D*/ 0x00E7, /* LATIN SMALL LETTER C WITH CEDILLA */
    /*0x8E*/ 0x00E9, /* LATIN SMALL LETTER E WITH ACUTE */
    /*0x8F*/ 0x00E8, /* LATIN SMALL LETTER E WITH GRAVE */
    /*0x90*/ 0x00EA, /* LATIN SMALL LETTER E WITH CIRCUMFLEX */
    /*0x91*/ 0x00EB, /* LATIN SMALL LETTER E WITH DIAERESIS */
    /*0x92*/ 0x00ED, /* LATIN SMALL LETTER I WITH ACUTE */
    /*0x93*/ 0x00EC, /* LATIN SMALL LETTER I WITH GRAVE */
    /*0x94*/ 0x00EE, /* LATIN SMALL LETTER I WITH CIRCUMFLEX */
    /*0x95*/ 0x00EF, /* LATIN SMALL LETTER I WITH DIAERESIS */
    /*0x96*/ 0x00F1, /* LATIN SMALL LETTER N WITH TILDE */
    /*0x97*/ 0x00F3, /* LATIN SMALL LETTER O WITH ACUTE */
    /*0x98*/ 0x00F2, /* LATIN SMALL LETTER O WITH GRAVE */
    /*0x99*/ 0x00F4, /* LATIN SMALL LETTER O WITH CIRCUMFLEX */
    /*0x9A*/ 0x00F6, /* LATIN SMALL LETTER O WITH DIAERESIS */
    /*0x9B*/ 0x00F5, /* LATIN SMALL LETTER O WITH TILDE */
    /*0x9C*/ 0x00FA, /* LATIN SMALL LETTER U WITH ACUTE */
    /*0x9D*/ 0x00F9, /* LATIN SMALL LETTER U WITH GRAVE */
    /*0x9E*/ 0x00FB, /* LATIN SMALL LETTER U WITH CIRCUMFLEX */
    /*0x9F*/ 0x00FC, /* LATIN SMALL LETTER U WITH DIAERESIS */
    /*0xA0*/ 0x2020, /* DAGGER */
    /*0xA1*/ 0x00B0, /* DEGREE SIGN */
    /*0xA2*/ 0x00A2, /* CENT SIGN */
    /*0xA3*/ 0x00A3, /* POUND SIGN */
    /*0xA4*/ 0x00A7, /* SECTION SIGN */
    /*0xA5*/ 0x2022, /* BULLET */
    /*0xA6*/ 0x00B6, /* PILCROW SIGN */
    /*0xA7*/ 0x00DF, /* LATIN SMALL LETTER SHARP S */
    /*0xA8*/ 0x00AE, /* REGISTERED SIGN */
    /*0xA9*/ 0x00A9, /* COPYRIGHT SIGN */
    /*0xAA*/ 0x2122, /* TRADE MARK SIGN */
    /*0xAB*/ 0x00B4, /* ACUTE ACCENT */
    /*0xAC*/ 0x00A8, /* DIAERESIS */
    /*0xAD*/ 0x2260, /* NOT EQUAL TO */
    /*0xAE*/ 0x00C6, /* LATIN CAPITAL LETTER AE */
    /*0xAF*/ 0x00D8, /* LATIN CAPITAL LETTER O WITH STROKE */
    /*0xB0*/ 0x221E, /* INFINITY */
    /*0xB1*/ 0x00B1, /* PLUS-MINUS SIGN */
    /*0xB2*/ 0x2264, /* LESS-THAN OR EQUAL TO */
    /*0xB3*/ 0x2265, /* GREATER-THAN OR EQUAL TO */
    /*0xB4*/ 0x00A5, /* YEN SIGN */
    /*0xB5*/ 0x00B5, /* MICRO SIGN */
    /*0xB6*/ 0x2202, /* PARTIAL DIFFERENTIAL */
    /*0xB7*/ 0x2211, /* N-ARY SUMMATION */
    /*0xB8*/ 0x220F, /* N-ARY PRODUCT */
    /*0xB9*/ 0x03C0, /* GREEK SMALL LETTER PI */
    /*0xBA*/ 0x222B, /* INTEGRAL */
    /*0xBB*/ 0x00AA, /* FEMININE ORDINAL INDICATOR */
    /*0xBC*/ 0x00BA, /* MASCULINE ORDINAL INDICATOR */
    /*0xBD*/ 0x03A9, /* GREEK CAPITAL LETTER OMEGA */
    /*0xBE*/ 0x00E6, /* LATIN SMALL LETTER AE */
    /*0xBF*/ 0x00F8, /* LATIN SMALL LETTER O WITH STROKE */
    /*0xC0*/ 0x00BF, /* INVERTED QUESTION MARK */
    /*0xC1*/ 0x00A1, /* INVERTED EXCLAMATION MARK */
    /*0xC2*/ 0x00AC, /* NOT SIGN */
    /*0xC3*/ 0x221A, /* SQUARE ROOT */
    /*0xC4*/ 0x0192, /* LATIN SMALL LETTER F WITH HOOK */
    /*0xC5*/ 0x2248, /* ALMOST EQUAL TO */
    /*0xC6*/ 0x2206, /* INCREMENT */
    /*0xC7*/ 0x00AB, /* LEFT-POINTING DOUBLE ANGLE QUOTATION MARK */
    /*0xC8*/ 0x00BB, /* RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK */
    /*0xC9*/ 0x2026, /* HORIZONTAL ELLIPSIS */
    /*0xCA*/ 0x00A0, /* NO-BREAK SPACE */
    /*0xCB*/ 0x00C0, /* LATIN CAPITAL LETTER A WITH GRAVE */
    /*0xCC*/ 0x00C3, /* LATIN CAPITAL LETTER A WITH TILDE */
    /*0xCD*/ 0x00D5, /* LATIN CAPITAL LETTER O WITH TILDE */
    /*0xCE*/ 0x0152, /* LATIN CAPITAL LIGATURE OE */
    /*0xCF*/ 0x0153, /* LATIN SMALL LIGATURE OE */
    /*0xD0*/ 0x2013, /* EN DASH */
    /*0xD1*/ 0x2014, /* EM DASH */
    /*0xD2*/ 0x201C, /* LEFT DOUBLE QUOTATION MARK */
    /*0xD3*/ 0x201D, /* RIGHT DOUBLE QUOTATION MARK */
    /*0xD4*/ 0x2018, /* LEFT SINGLE QUOTATION MARK */
    /*0xD5*/ 0x2019, /* RIGHT SINGLE QUOTATION MARK */
    /*0xD6*/ 0x00F7, /* DIVISION SIGN */
    /*0xD7*/ 0x25CA, /* LOZENGE */
    /*0xD8*/ 0x00FF, /* LATIN SMALL LETTER Y WITH DIAERESIS */
    /*0xD9*/ 0x0178, /* LATIN CAPITAL LETTER Y WITH DIAERESIS */
    /*0xDA*/ 0x2044, /* FRACTION SLASH */
    /*0xDB*/ 0x20AC, /* EURO SIGN */
    /*0xDC*/ 0x2039, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
    /*0xDD*/ 0x203A, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
    /*0xDE*/ 0xFB01, /* LATIN SMALL LIGATURE FI */
    /*0xDF*/ 0xFB02, /* LATIN SMALL LIGATURE FL */
    /*0xE0*/ 0x2021, /* DOUBLE DAGGER */
    /*0xE1*/ 0x00B7, /* MIDDLE DOT */
    /*0xE2*/ 0x201A, /* SINGLE LOW-9 QUOTATION MARK */
    /*0xE3*/ 0x201E, /* DOUBLE LOW-9 QUOTATION MARK */
    /*0xE4*/ 0x2030, /* PER MILLE SIGN */
    /*0xE5*/ 0x00C2, /* LATIN CAPITAL LETTER A WITH CIRCUMFLEX */
    /*0xE6*/ 0x00CA, /* LATIN CAPITAL LETTER E WITH CIRCUMFLEX */
    /*0xE7*/ 0x00C1, /* LATIN CAPITAL LETTER A WITH ACUTE */
    /*0xE8*/ 0x00CB, /* LATIN CAPITAL LETTER E WITH DIAERESIS */
    /*0xE9*/ 0x00C8, /* LATIN CAPITAL LETTER E WITH GRAVE */
    /*0xEA*/ 0x00CD, /* LATIN CAPITAL LETTER I WITH ACUTE */
    /*0xEB*/ 0x00CE, /* LATIN CAPITAL LETTER I WITH CIRCUMFLEX */
    /*0xEC*/ 0x00CF, /* LATIN CAPITAL LETTER I WITH DIAERESIS */
    /*0xED*/ 0x00CC, /* LATIN CAPITAL LETTER I WITH GRAVE */
    /*0xEE*/ 0x00D3, /* LATIN CAPITAL LETTER O WITH ACUTE */
    /*0xEF*/ 0x00D4, /* LATIN CAPITAL LETTER O WITH CIRCUMFLEX */
    /*0xF0*/ 0xF8FF, /* Apple logo */
    /*0xF1*/ 0x00D2, /* LATIN CAPITAL LETTER O WITH GRAVE */
    /*0xF2*/ 0x00DA, /* LATIN CAPITAL LETTER U WITH ACUTE */
    /*0xF3*/ 0x00DB, /* LATIN CAPITAL LETTER U WITH CIRCUMFLEX */
    /*0xF4*/ 0x00D9, /* LATIN CAPITAL LETTER U WITH GRAVE */
    /*0xF5*/ 0x0131, /* LATIN SMALL LETTER DOTLESS I */
    /*0xF6*/ 0x02C6, /* MODIFIER LETTER CIRCUMFLEX ACCENT */
    /*0xF7*/ 0x02DC, /* SMALL TILDE */
    /*0xF8*/ 0x00AF, /* MACRON */
    /*0xF9*/ 0x02D8, /* BREVE */
    /*0xFA*/ 0x02D9, /* DOT ABOVE */
    /*0xFB*/ 0x02DA, /* RING ABOVE */
    /*0xFC*/ 0x00B8, /* CEDILLA */
    /*0xFD*/ 0x02DD, /* DOUBLE ACUTE ACCENT */
    /*0xFE*/ 0x02DB, /* OGONEK */
    /*0xFF*/ 0x02C7, /* CARON */
};

static const FcCaseFold    fcCaseFold[FC_NUM_CASE_FOLD] = {
    { 0x00000041, FC_CASE_FOLD_RANGE,    0x001a,     32 },
    { 0x000000b5, FC_CASE_FOLD_RANGE,    0x0001,    775 },
    { 0x000000c0, FC_CASE_FOLD_RANGE,    0x0017,     32 },
    { 0x000000d8, FC_CASE_FOLD_RANGE,    0x0007,     32 },
    { 0x000000df, FC_CASE_FOLD_FULL,     0x0002,      0 },
    { 0x00000100, FC_CASE_FOLD_EVEN_ODD, 0x002f,      1 },
    { 0x00000130, FC_CASE_FOLD_FULL,     0x0003,      2 },
    { 0x00000132, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x00000139, FC_CASE_FOLD_EVEN_ODD, 0x000f,      1 },
    { 0x00000149, FC_CASE_FOLD_FULL,     0x0003,      5 },
    { 0x0000014a, FC_CASE_FOLD_EVEN_ODD, 0x002d,      1 },
    { 0x00000178, FC_CASE_FOLD_RANGE,    0x0001,   -121 },
    { 0x00000179, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x0000017f, FC_CASE_FOLD_RANGE,    0x0001,   -268 },
    { 0x00000181, FC_CASE_FOLD_RANGE,    0x0001,    210 },
    { 0x00000182, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00000186, FC_CASE_FOLD_RANGE,    0x0001,    206 },
    { 0x00000187, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000189, FC_CASE_FOLD_RANGE,    0x0002,    205 },
    { 0x0000018b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000018e, FC_CASE_FOLD_RANGE,    0x0001,     79 },
    { 0x0000018f, FC_CASE_FOLD_RANGE,    0x0001,    202 },
    { 0x00000190, FC_CASE_FOLD_RANGE,    0x0001,    203 },
    { 0x00000191, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000193, FC_CASE_FOLD_RANGE,    0x0001,    205 },
    { 0x00000194, FC_CASE_FOLD_RANGE,    0x0001,    207 },
    { 0x00000196, FC_CASE_FOLD_RANGE,    0x0001,    211 },
    { 0x00000197, FC_CASE_FOLD_RANGE,    0x0001,    209 },
    { 0x00000198, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000019c, FC_CASE_FOLD_RANGE,    0x0001,    211 },
    { 0x0000019d, FC_CASE_FOLD_RANGE,    0x0001,    213 },
    { 0x0000019f, FC_CASE_FOLD_RANGE,    0x0001,    214 },
    { 0x000001a0, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x000001a6, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001a7, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001a9, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001ac, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001ae, FC_CASE_FOLD_RANGE,    0x0001,    218 },
    { 0x000001af, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001b1, FC_CASE_FOLD_RANGE,    0x0002,    217 },
    { 0x000001b3, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x000001b7, FC_CASE_FOLD_RANGE,    0x0001,    219 },
    { 0x000001b8, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001bc, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001c4, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001c5, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001c7, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001c8, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000001ca, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001cb, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x000001de, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x000001f0, FC_CASE_FOLD_FULL,     0x0003,      8 },
    { 0x000001f1, FC_CASE_FOLD_RANGE,    0x0001,      2 },
    { 0x000001f2, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x000001f6, FC_CASE_FOLD_RANGE,    0x0001,    -97 },
    { 0x000001f7, FC_CASE_FOLD_RANGE,    0x0001,    -56 },
    { 0x000001f8, FC_CASE_FOLD_EVEN_ODD, 0x0027,      1 },
    { 0x00000220, FC_CASE_FOLD_RANGE,    0x0001,   -130 },
    { 0x00000222, FC_CASE_FOLD_EVEN_ODD, 0x0011,      1 },
    { 0x0000023a, FC_CASE_FOLD_RANGE,    0x0001,  10795 },
    { 0x0000023b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000023d, FC_CASE_FOLD_RANGE,    0x0001,   -163 },
    { 0x0000023e, FC_CASE_FOLD_RANGE,    0x0001,  10792 },
    { 0x00000241, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00000243, FC_CASE_FOLD_RANGE,    0x0001,   -195 },
    { 0x00000244, FC_CASE_FOLD_RANGE,    0x0001,     69 },
    { 0x00000245, FC_CASE_FOLD_RANGE,    0x0001,     71 },
    { 0x00000246, FC_CASE_FOLD_EVEN_ODD, 0x0009,      1 },
    { 0x00000345, FC_CASE_FOLD_RANGE,    0x0001,    116 },
    { 0x00000370, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00000376, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000037f, FC_CASE_FOLD_RANGE,    0x0001,    116 },
    { 0x00000386, FC_CASE_FOLD_RANGE,    0x0001,     38 },
    { 0x00000388, FC_CASE_FOLD_RANGE,    0x0003,     37 },
    { 0x0000038c, FC_CASE_FOLD_RANGE,    0x0001,     64 },
    { 0x0000038e, FC_CASE_FOLD_RANGE,    0x0002,     63 },
    { 0x00000390, FC_CASE_FOLD_FULL,     0x0006,     11 },
    { 0x00000391, FC_CASE_FOLD_RANGE,    0x0011,     32 },
    { 0x000003a3, FC_CASE_FOLD_RANGE,    0x0009,     32 },
    { 0x000003b0, FC_CASE_FOLD_FULL,     0x0006,     17 },
    { 0x000003c2, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003cf, FC_CASE_FOLD_RANGE,    0x0001,      8 },
    { 0x000003d0, FC_CASE_FOLD_RANGE,    0x0001,    -30 },
    { 0x000003d1, FC_CASE_FOLD_RANGE,    0x0001,    -25 },
    { 0x000003d5, FC_CASE_FOLD_RANGE,    0x0001,    -15 },
    { 0x000003d6, FC_CASE_FOLD_RANGE,    0x0001,    -22 },
    { 0x000003d8, FC_CASE_FOLD_EVEN_ODD, 0x0017,      1 },
    { 0x000003f0, FC_CASE_FOLD_RANGE,    0x0001,    -54 },
    { 0x000003f1, FC_CASE_FOLD_RANGE,    0x0001,    -48 },
    { 0x000003f4, FC_CASE_FOLD_RANGE,    0x0001,    -60 },
    { 0x000003f5, FC_CASE_FOLD_RANGE,    0x0001,    -64 },
    { 0x000003f7, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003f9, FC_CASE_FOLD_RANGE,    0x0001,     -7 },
    { 0x000003fa, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000003fd, FC_CASE_FOLD_RANGE,    0x0003,   -130 },
    { 0x00000400, FC_CASE_FOLD_RANGE,    0x0010,     80 },
    { 0x00000410, FC_CASE_FOLD_RANGE,    0x0020,     32 },
    { 0x00000460, FC_CASE_FOLD_EVEN_ODD, 0x0021,      1 },
    { 0x0000048a, FC_CASE_FOLD_EVEN_ODD, 0x0035,      1 },
    { 0x000004c0, FC_CASE_FOLD_RANGE,    0x0001,     15 },
    { 0x000004c1, FC_CASE_FOLD_EVEN_ODD, 0x000d,      1 },
    { 0x000004d0, FC_CASE_FOLD_EVEN_ODD, 0x005f,      1 },
    { 0x00000531, FC_CASE_FOLD_RANGE,    0x0026,     48 },
    { 0x00000587, FC_CASE_FOLD_FULL,     0x0004,     23 },
    { 0x000010a0, FC_CASE_FOLD_RANGE,    0x0026,   7264 },
    { 0x000010c7, FC_CASE_FOLD_RANGE,    0x0001,   7264 },
    { 0x000010cd, FC_CASE_FOLD_RANGE,    0x0001,   7264 },
    { 0x000013f8, FC_CASE_FOLD_RANGE,    0x0006,     -8 },
    { 0x00001c80, FC_CASE_FOLD_RANGE,    0x0001,  -6222 },
    { 0x00001c81, FC_CASE_FOLD_RANGE,    0x0001,  -6221 },
    { 0x00001c82, FC_CASE_FOLD_RANGE,    0x0001,  -6212 },
    { 0x00001c83, FC_CASE_FOLD_RANGE,    0x0002,  -6210 },
    { 0x00001c85, FC_CASE_FOLD_RANGE,    0x0001,  -6211 },
    { 0x00001c86, FC_CASE_FOLD_RANGE,    0x0001,  -6204 },
    { 0x00001c87, FC_CASE_FOLD_RANGE,    0x0001,  -6180 },
    { 0x00001c88, FC_CASE_FOLD_RANGE,    0x0001, -30269 },
    { 0x00001c90, FC_CASE_FOLD_RANGE,    0x002b,  -3008 },
    { 0x00001cbd, FC_CASE_FOLD_RANGE,    0x0003,  -3008 },
    { 0x00001e00, FC_CASE_FOLD_EVEN_ODD, 0x0095,      1 },
    { 0x00001e96, FC_CASE_FOLD_FULL,     0x0003,     27 },
    { 0x00001e97, FC_CASE_FOLD_FULL,     0x0003,     30 },
    { 0x00001e98, FC_CASE_FOLD_FULL,     0x0003,     33 },
    { 0x00001e99, FC_CASE_FOLD_FULL,     0x0003,     36 },
    { 0x00001e9a, FC_CASE_FOLD_FULL,     0x0003,     39 },
    { 0x00001e9b, FC_CASE_FOLD_RANGE,    0x0001,    -58 },
    { 0x00001e9e, FC_CASE_FOLD_FULL,     0x0002,     42 },
    { 0x00001ea0, FC_CASE_FOLD_EVEN_ODD, 0x005f,      1 },
    { 0x00001f08, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f18, FC_CASE_FOLD_RANGE,    0x0006,     -8 },
    { 0x00001f28, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f38, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f48, FC_CASE_FOLD_RANGE,    0x0006,     -8 },
    { 0x00001f50, FC_CASE_FOLD_FULL,     0x0004,     44 },
    { 0x00001f52, FC_CASE_FOLD_FULL,     0x0006,     48 },
    { 0x00001f54, FC_CASE_FOLD_FULL,     0x0006,     54 },
    { 0x00001f56, FC_CASE_FOLD_FULL,     0x0006,     60 },
    { 0x00001f59, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5b, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5d, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f5f, FC_CASE_FOLD_RANGE,    0x0001,     -8 },
    { 0x00001f68, FC_CASE_FOLD_RANGE,    0x0008,     -8 },
    { 0x00001f80, FC_CASE_FOLD_FULL,     0x0005,     66 },
    { 0x00001f81, FC_CASE_FOLD_FULL,     0x0005,     71 },
    { 0x00001f82, FC_CASE_FOLD_FULL,     0x0005,     76 },
    { 0x00001f83, FC_CASE_FOLD_FULL,     0x0005,     81 },
    { 0x00001f84, FC_CASE_FOLD_FULL,     0x0005,     86 },
    { 0x00001f85, FC_CASE_FOLD_FULL,     0x0005,     91 },
    { 0x00001f86, FC_CASE_FOLD_FULL,     0x0005,     96 },
    { 0x00001f87, FC_CASE_FOLD_FULL,     0x0005,    101 },
    { 0x00001f88, FC_CASE_FOLD_FULL,     0x0005,    106 },
    { 0x00001f89, FC_CASE_FOLD_FULL,     0x0005,    111 },
    { 0x00001f8a, FC_CASE_FOLD_FULL,     0x0005,    116 },
    { 0x00001f8b, FC_CASE_FOLD_FULL,     0x0005,    121 },
    { 0x00001f8c, FC_CASE_FOLD_FULL,     0x0005,    126 },
    { 0x00001f8d, FC_CASE_FOLD_FULL,     0x0005,    131 },
    { 0x00001f8e, FC_CASE_FOLD_FULL,     0x0005,    136 },
    { 0x00001f8f, FC_CASE_FOLD_FULL,     0x0005,    141 },
    { 0x00001f90, FC_CASE_FOLD_FULL,     0x0005,    146 },
    { 0x00001f91, FC_CASE_FOLD_FULL,     0x0005,    151 },
    { 0x00001f92, FC_CASE_FOLD_FULL,     0x0005,    156 },
    { 0x00001f93, FC_CASE_FOLD_FULL,     0x0005,    161 },
    { 0x00001f94, FC_CASE_FOLD_FULL,     0x0005,    166 },
    { 0x00001f95, FC_CASE_FOLD_FULL,     0x0005,    171 },
    { 0x00001f96, FC_CASE_FOLD_FULL,     0x0005,    176 },
    { 0x00001f97, FC_CASE_FOLD_FULL,     0x0005,    181 },
    { 0x00001f98, FC_CASE_FOLD_FULL,     0x0005,    186 },
    { 0x00001f99, FC_CASE_FOLD_FULL,     0x0005,    191 },
    { 0x00001f9a, FC_CASE_FOLD_FULL,     0x0005,    196 },
    { 0x00001f9b, FC_CASE_FOLD_FULL,     0x0005,    201 },
    { 0x00001f9c, FC_CASE_FOLD_FULL,     0x0005,    206 },
    { 0x00001f9d, FC_CASE_FOLD_FULL,     0x0005,    211 },
    { 0x00001f9e, FC_CASE_FOLD_FULL,     0x0005,    216 },
    { 0x00001f9f, FC_CASE_FOLD_FULL,     0x0005,    221 },
    { 0x00001fa0, FC_CASE_FOLD_FULL,     0x0005,    226 },
    { 0x00001fa1, FC_CASE_FOLD_FULL,     0x0005,    231 },
    { 0x00001fa2, FC_CASE_FOLD_FULL,     0x0005,    236 },
    { 0x00001fa3, FC_CASE_FOLD_FULL,     0x0005,    241 },
    { 0x00001fa4, FC_CASE_FOLD_FULL,     0x0005,    246 },
    { 0x00001fa5, FC_CASE_FOLD_FULL,     0x0005,    251 },
    { 0x00001fa6, FC_CASE_FOLD_FULL,     0x0005,    256 },
    { 0x00001fa7, FC_CASE_FOLD_FULL,     0x0005,    261 },
    { 0x00001fa8, FC_CASE_FOLD_FULL,     0x0005,    266 },
    { 0x00001fa9, FC_CASE_FOLD_FULL,     0x0005,    271 },
    { 0x00001faa, FC_CASE_FOLD_FULL,     0x0005,    276 },
    { 0x00001fab, FC_CASE_FOLD_FULL,     0x0005,    281 },
    { 0x00001fac, FC_CASE_FOLD_FULL,     0x0005,    286 },
    { 0x00001fad, FC_CASE_FOLD_FULL,     0x0005,    291 },
    { 0x00001fae, FC_CASE_FOLD_FULL,     0x0005,    296 },
    { 0x00001faf, FC_CASE_FOLD_FULL,     0x0005,    301 },
    { 0x00001fb2, FC_CASE_FOLD_FULL,     0x0005,    306 },
    { 0x00001fb3, FC_CASE_FOLD_FULL,     0x0004,    311 },
    { 0x00001fb4, FC_CASE_FOLD_FULL,     0x0004,    315 },
    { 0x00001fb6, FC_CASE_FOLD_FULL,     0x0004,    319 },
    { 0x00001fb7, FC_CASE_FOLD_FULL,     0x0006,    323 },
    { 0x00001fb8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fba, FC_CASE_FOLD_RANGE,    0x0002,    -74 },
    { 0x00001fbc, FC_CASE_FOLD_FULL,     0x0004,    329 },
    { 0x00001fbe, FC_CASE_FOLD_RANGE,    0x0001,  -7173 },
    { 0x00001fc2, FC_CASE_FOLD_FULL,     0x0005,    333 },
    { 0x00001fc3, FC_CASE_FOLD_FULL,     0x0004,    338 },
    { 0x00001fc4, FC_CASE_FOLD_FULL,     0x0004,    342 },
    { 0x00001fc6, FC_CASE_FOLD_FULL,     0x0004,    346 },
    { 0x00001fc7, FC_CASE_FOLD_FULL,     0x0006,    350 },
    { 0x00001fc8, FC_CASE_FOLD_RANGE,    0x0004,    -86 },
    { 0x00001fcc, FC_CASE_FOLD_FULL,     0x0004,    356 },
    { 0x00001fd2, FC_CASE_FOLD_FULL,     0x0006,    360 },
    { 0x00001fd3, FC_CASE_FOLD_FULL,     0x0006,    366 },
    { 0x00001fd6, FC_CASE_FOLD_FULL,     0x0004,    372 },
    { 0x00001fd7, FC_CASE_FOLD_FULL,     0x0006,    376 },
    { 0x00001fd8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fda, FC_CASE_FOLD_RANGE,    0x0002,   -100 },
    { 0x00001fe2, FC_CASE_FOLD_FULL,     0x0006,    382 },
    { 0x00001fe3, FC_CASE_FOLD_FULL,     0x0006,    388 },
    { 0x00001fe4, FC_CASE_FOLD_FULL,     0x0004,    394 },
    { 0x00001fe6, FC_CASE_FOLD_FULL,     0x0004,    398 },
    { 0x00001fe7, FC_CASE_FOLD_FULL,     0x0006,    402 },
    { 0x00001fe8, FC_CASE_FOLD_RANGE,    0x0002,     -8 },
    { 0x00001fea, FC_CASE_FOLD_RANGE,    0x0002,   -112 },
    { 0x00001fec, FC_CASE_FOLD_RANGE,    0x0001,     -7 },
    { 0x00001ff2, FC_CASE_FOLD_FULL,     0x0005,    408 },
    { 0x00001ff3, FC_CASE_FOLD_FULL,     0x0004,    413 },
    { 0x00001ff4, FC_CASE_FOLD_FULL,     0x0004,    417 },
    { 0x00001ff6, FC_CASE_FOLD_FULL,     0x0004,    421 },
    { 0x00001ff7, FC_CASE_FOLD_FULL,     0x0006,    425 },
    { 0x00001ff8, FC_CASE_FOLD_RANGE,    0x0002,   -128 },
    { 0x00001ffa, FC_CASE_FOLD_RANGE,    0x0002,   -126 },
    { 0x00001ffc, FC_CASE_FOLD_FULL,     0x0004,    431 },
    { 0x00002126, FC_CASE_FOLD_RANGE,    0x0001,  -7517 },
    { 0x0000212a, FC_CASE_FOLD_RANGE,    0x0001,  -8383 },
    { 0x0000212b, FC_CASE_FOLD_RANGE,    0x0001,  -8262 },
    { 0x00002132, FC_CASE_FOLD_RANGE,    0x0001,     28 },
    { 0x00002160, FC_CASE_FOLD_RANGE,    0x0010,     16 },
    { 0x00002183, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x000024b6, FC_CASE_FOLD_RANGE,    0x001a,     26 },
    { 0x00002c00, FC_CASE_FOLD_RANGE,    0x002f,     48 },
    { 0x00002c60, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c62, FC_CASE_FOLD_RANGE,    0x0001, -10743 },
    { 0x00002c63, FC_CASE_FOLD_RANGE,    0x0001,  -3814 },
    { 0x00002c64, FC_CASE_FOLD_RANGE,    0x0001, -10727 },
    { 0x00002c67, FC_CASE_FOLD_EVEN_ODD, 0x0005,      1 },
    { 0x00002c6d, FC_CASE_FOLD_RANGE,    0x0001, -10780 },
    { 0x00002c6e, FC_CASE_FOLD_RANGE,    0x0001, -10749 },
    { 0x00002c6f, FC_CASE_FOLD_RANGE,    0x0001, -10783 },
    { 0x00002c70, FC_CASE_FOLD_RANGE,    0x0001, -10782 },
    { 0x00002c72, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c75, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x00002c7e, FC_CASE_FOLD_RANGE,    0x0002, -10815 },
    { 0x00002c80, FC_CASE_FOLD_EVEN_ODD, 0x0063,      1 },
    { 0x00002ceb, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x00002cf2, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000a640, FC_CASE_FOLD_EVEN_ODD, 0x002d,      1 },
    { 0x0000a680, FC_CASE_FOLD_EVEN_ODD, 0x001b,      1 },
    { 0x0000a722, FC_CASE_FOLD_EVEN_ODD, 0x000d,      1 },
    { 0x0000a732, FC_CASE_FOLD_EVEN_ODD, 0x003d,      1 },
    { 0x0000a779, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x0000a77d, FC_CASE_FOLD_RANGE,    0x0001,  30204 },
    { 0x0000a77e, FC_CASE_FOLD_EVEN_ODD, 0x0009,      1 },
    { 0x0000a78b, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000a78d, FC_CASE_FOLD_RANGE,    0x0001,  23256 },
    { 0x0000a790, FC_CASE_FOLD_EVEN_ODD, 0x0003,      1 },
    { 0x0000a796, FC_CASE_FOLD_EVEN_ODD, 0x0013,      1 },
    { 0x0000a7aa, FC_CASE_FOLD_RANGE,    0x0001,  23228 },
    { 0x0000a7ab, FC_CASE_FOLD_RANGE,    0x0001,  23217 },
    { 0x0000a7ac, FC_CASE_FOLD_RANGE,    0x0001,  23221 },
    { 0x0000a7ad, FC_CASE_FOLD_RANGE,    0x0001,  23231 },
    { 0x0000a7ae, FC_CASE_FOLD_RANGE,    0x0001,  23228 },
    { 0x0000a7b0, FC_CASE_FOLD_RANGE,    0x0001,  23278 },
    { 0x0000a7b1, FC_CASE_FOLD_RANGE,    0x0001,  23254 },
    { 0x0000a7b2, FC_CASE_FOLD_RANGE,    0x0001,  23275 },
    { 0x0000a7b3, FC_CASE_FOLD_RANGE,    0x0001,    928 },
    { 0x0000a7b4, FC_CASE_FOLD_EVEN_ODD, 0x000b,      1 },
    { 0x0000a7c2, FC_CASE_FOLD_EVEN_ODD, 0x0001,      1 },
    { 0x0000a7c4, FC_CASE_FOLD_RANGE,    0x0001,    -48 },
    { 0x0000a7c5, FC_CASE_FOLD_RANGE,    0x0001,  23229 },
    { 0x0000a7c6, FC_CASE_FOLD_RANGE,    0x0001,  30152 },
    { 0x0000ab70, FC_CASE_FOLD_RANGE,    0x0050,  26672 },
    { 0x0000fb00, FC_CASE_FOLD_FULL,     0x0002,    435 },
    { 0x0000fb01, FC_CASE_FOLD_FULL,     0x0002,    437 },
    { 0x0000fb02, FC_CASE_FOLD_FULL,     0x0002,    439 },
    { 0x0000fb03, FC_CASE_FOLD_FULL,     0x0003,    441 },
    { 0x0000fb04, FC_CASE_FOLD_FULL,     0x0003,    444 },
    { 0x0000fb05, FC_CASE_FOLD_FULL,     0x0002,    447 },
    { 0x0000fb06, FC_CASE_FOLD_FULL,     0x0002,    449 },
    { 0x0000fb13, FC_CASE_FOLD_FULL,     0x0004,    451 },
    { 0x0000fb14, FC_CASE_FOLD_FULL,     0x0004,    455 },
    { 0x0000fb15, FC_CASE_FOLD_FULL,     0x0004,    459 },
    { 0x0000fb16, FC_CASE_FOLD_FULL,     0x0004,    463 },
    { 0x0000fb17, FC_CASE_FOLD_FULL,     0x0004,    467 },
    { 0x0000ff21, FC_CASE_FOLD_RANGE,    0x001a,     32 },
    { 0x00010400, FC_CASE_FOLD_RANGE,    0x0028,     40 },
    { 0x000104b0, FC_CASE_FOLD_RANGE,    0x0024,     40 },
    { 0x00010c80, FC_CASE_FOLD_RANGE,    0x0033,     64 },
    { 0x000118a0, FC_CASE_FOLD_RANGE,    0x0020,     32 },
    { 0x00016e40, FC_CASE_FOLD_RANGE,    0x0020,     32 },
    { 0x0001e900, FC_CASE_FOLD_RANGE,    0x0022,     34 },
};

static const FcChar8    fcCaseFoldChars[FC_NUM_CASE_FOLD_CHARS] = {
    0x73, 0x73, 0x69, 0xcc, 0x87, 0xca, 0xbc, 0x6e, 0x6a, 0xcc, 0x8c, 0xce, 0xb9, 0xcc, 0x88, 0xcc,
    0x81, 0xcf, 0x85, 0xcc, 0x88, 0xcc, 0x81, 0xd5, 0xa5, 0xd6, 0x82, 0x68, 0xcc, 0xb1, 0x74, 0xcc,
    0x88, 0x77, 0xcc, 0x8a, 0x79, 0xcc, 0x8a, 0x61, 0xca, 0xbe, 0x73, 0x73, 0xcf, 0x85, 0xcc, 0x93,
    0xcf, 0x85, 0xcc, 0x93, 0xcc, 0x80, 0xcf, 0x85, 0xcc, 0x93, 0xcc, 0x81, 0xcf, 0x85, 0xcc, 0x93,
    0xcd, 0x82, 0xe1, 0xbc, 0x80, 0xce, 0xb9, 0xe1, 0xbc, 0x81, 0xce, 0xb9, 0xe1, 0xbc, 0x82, 0xce,
    0xb9, 0xe1, 0xbc, 0x83, 0xce, 0xb9, 0xe1, 0xbc, 0x84, 0xce, 0xb9, 0xe1, 0xbc, 0x85, 0xce, 0xb9,
    0xe1, 0xbc, 0x86, 0xce, 0xb9, 0xe1, 0xbc, 0x87, 0xce, 0xb9, 0xe1, 0xbc, 0x80, 0xce, 0xb9, 0xe1,
    0xbc, 0x81, 0xce, 0xb9, 0xe1, 0xbc, 0x82, 0xce, 0xb9, 0xe1, 0xbc, 0x83, 0xce, 0xb9, 0xe1, 0xbc,
    0x84, 0xce, 0xb9, 0xe1, 0xbc, 0x85, 0xce, 0xb9, 0xe1, 0xbc, 0x86, 0xce, 0xb9, 0xe1, 0xbc, 0x87,
    0xce, 0xb9, 0xe1, 0xbc, 0xa0, 0xce, 0xb9, 0xe1, 0xbc, 0xa1, 0xce, 0xb9, 0xe1, 0xbc, 0xa2, 0xce,
    0xb9, 0xe1, 0xbc, 0xa3, 0xce, 0xb9, 0xe1, 0xbc, 0xa4, 0xce, 0xb9, 0xe1, 0xbc, 0xa5, 0xce, 0xb9,
    0xe1, 0xbc, 0xa6, 0xce, 0xb9, 0xe1, 0xbc, 0xa7, 0xce, 0xb9, 0xe1, 0xbc, 0xa0, 0xce, 0xb9, 0xe1,
    0xbc, 0xa1, 0xce, 0xb9, 0xe1, 0xbc, 0xa2, 0xce, 0xb9, 0xe1, 0xbc, 0xa3, 0xce, 0xb9, 0xe1, 0xbc,
    0xa4, 0xce, 0xb9, 0xe1, 0xbc, 0xa5, 0xce, 0xb9, 0xe1, 0xbc, 0xa6, 0xce, 0xb9, 0xe1, 0xbc, 0xa7,
    0xce, 0xb9, 0xe1, 0xbd, 0xa0, 0xce, 0xb9, 0xe1, 0xbd, 0xa1, 0xce, 0xb9, 0xe1, 0xbd, 0xa2, 0xce,
    0xb9, 0xe1, 0xbd, 0xa3, 0xce, 0xb9, 0xe1, 0xbd, 0xa4, 0xce, 0xb9, 0xe1, 0xbd, 0xa5, 0xce, 0xb9,
    0xe1, 0xbd, 0xa6, 0xce, 0xb9, 0xe1, 0xbd, 0xa7, 0xce, 0xb9, 0xe1, 0xbd, 0xa0, 0xce, 0xb9, 0xe1,
    0xbd, 0xa1, 0xce, 0xb9, 0xe1, 0xbd, 0xa2, 0xce, 0xb9, 0xe1, 0xbd, 0xa3, 0xce, 0xb9, 0xe1, 0xbd,
    0xa4, 0xce, 0xb9, 0xe1, 0xbd, 0xa5, 0xce, 0xb9, 0xe1, 0xbd, 0xa6, 0xce, 0xb9, 0xe1, 0xbd, 0xa7,
    0xce, 0xb9, 0xe1, 0xbd, 0xb0, 0xce, 0xb9, 0xce, 0xb1, 0xce, 0xb9, 0xce, 0xac, 0xce, 0xb9, 0xce,
    0xb1, 0xcd, 0x82, 0xce, 0xb1, 0xcd, 0x82, 0xce, 0xb9, 0xce, 0xb1, 0xce, 0xb9, 0xe1, 0xbd, 0xb4,
    0xce, 0xb9, 0xce, 0xb7, 0xce, 0xb9, 0xce, 0xae, 0xce, 0xb9, 0xce, 0xb7, 0xcd, 0x82, 0xce, 0xb7,
    0xcd, 0x82, 0xce, 0xb9, 0xce, 0xb7, 0xce, 0xb9, 0xce, 0xb9, 0xcc, 0x88, 0xcc, 0x80, 0xce, 0xb9,
    0xcc, 0x88, 0xcc, 0x81, 0xce, 0xb9, 0xcd, 0x82, 0xce, 0xb9, 0xcc, 0x88, 0xcd, 0x82, 0xcf, 0x85,
    0xcc, 0x88, 0xcc, 0x80, 0xcf, 0x85, 0xcc, 0x88, 0xcc, 0x81, 0xcf, 0x81, 0xcc, 0x93, 0xcf, 0x85,
    0xcd, 0x82, 0xcf, 0x85, 0xcc, 0x88, 0xcd, 0x82, 0xe1, 0xbd, 0xbc, 0xce, 0xb9, 0xcf, 0x89, 0xce,
    0xb9, 0xcf, 0x8e, 0xce, 0xb9, 0xcf, 0x89, 0xcd, 0x82, 0xcf, 0x89, 0xcd, 0x82, 0xce, 0xb9, 0xcf,
    0x89, 0xce, 0xb9, 0x66, 0x66, 0x66, 0x69, 0x66, 0x6c, 0x66, 0x66, 0x69, 0x66, 0x66, 0x6c, 0x73,
    0x74, 0x73, 0x74, 0xd5, 0xb4, 0xd5, 0xb6, 0xd5, 0xb4, 0xd5, 0xa5, 0xd5, 0xb4, 0xd5, 0xab, 0xd5,
    0xbe, 0xd5, 0xb6, 0xd5, 0xb4, 0xd5, 0xad
};

class DFreeTypeUtil
{
public:
    static char *getFontFamilyName(FT_Face face);
};

#endif // DFREETYPEUTIL_H
