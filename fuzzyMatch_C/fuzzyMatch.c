/**
 * Copyright (C) 2017 Yggdroot
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>


#if defined(_MSC_VER) && \
    (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64))

    #define FM_BITSCAN_WINDOWS

    #include <intrin.h>
    #pragma intrinsic(_BitScanReverse)

    #if defined(_M_AMD64) || defined(_M_X64)
        #define FM_BITSCAN_WINDOWS64
        #pragma intrinsic(_BitScanReverse64)
    #endif

#endif

#if defined(FM_BITSCAN_WINDOWS)

    uint32_t FM_BitLength(uint64_t x)
    {
        unsigned long index;
        #if defined(FM_BITSCAN_WINDOWS64)
        if ( !_BitScanReverse64(&index, x) )
            return 0;
        else
            return index + 1;
        #else
        if ( (x & 0xFFFFFFFF00000000) == 0 )
        {
            if ( !_BitScanReverse(&index, (unsigned long)x) )
                return 0;
            else
                return index + 1;
        }
        else
        {
            _BitScanReverse(&index, (unsigned long)(x >> 32));
            return index + 33;
        }
        #endif
    }

    #define FM_BIT_LENGTH(x) FM_BitLength(x)

#elif defined(__GNUC__)

    #define FM_BIT_LENGTH(x) ((uint32_t)(8 * sizeof(unsigned long long) - __builtin_clzll(x)))

#elif defined(__clang__)

    #if __has_builtin(__builtin_clzll)
        #define FM_BIT_LENGTH(x) ((uint32_t)(8 * sizeof(unsigned long long) - __builtin_clzll(x)))
    #endif

#endif

#if !defined(FM_BIT_LENGTH)

    static uint8_t clz_table_8bit[256] =
    {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    uint32_t FM_BIT_LENGTH(uint64_t x)
    {
        uint32_t n = 0;
        if ((x & 0xFFFFFFFF00000000) == 0) {n  = 32; x <<= 32;}
        if ((x & 0xFFFF000000000000) == 0) {n += 16; x <<= 16;}
        if ((x & 0xFF00000000000000) == 0) {n +=  8; x <<=  8;}
        n += (uint32_t)clz_table_8bit[x >> 56];

        return 64 - n;
    }

#endif

static uint64_t deBruijn = 0x022FDD63CC95386D;

static uint8_t MultiplyDeBruijnBitPosition[64] =
{
    0,  1,  2,  53, 3,  7,  54, 27,
    4,  38, 41, 8,  34, 55, 48, 28,
    62, 5,  39, 46, 44, 42, 22, 9,
    24, 35, 59, 56, 49, 18, 29, 11,
    63, 52, 6,  26, 37, 40, 33, 47,
    61, 45, 43, 21, 23, 58, 17, 10,
    51, 25, 36, 32, 60, 20, 57, 16,
    50, 31, 19, 15, 30, 14, 13, 12,
};

#define FM_CTZ(x) MultiplyDeBruijnBitPosition[((uint64_t)((x) & -(int64_t)(x)) * deBruijn) >> 58]


typedef struct TextContext
{
    char* text;
    uint16_t text_len;
    uint64_t* text_mask;
    uint16_t col_num;
    uint16_t offset;
}TextContext;

typedef struct PatternContext
{
    char* pattern;
    uint16_t pattern_len;
    int64_t pattern_mask[256];
    uint8_t is_lower;
}PatternContext;

typedef struct ValueElements
{
    float score;
    uint16_t beg;
    uint16_t end;
}ValueElements;


PatternContext* initPattern(char* pattern, uint16_t pattern_len)
{
    PatternContext* pPattern_ctxt = (PatternContext*)malloc(sizeof(PatternContext));
    if ( !pPattern_ctxt )
    {
        fprintf(stderr, "Out of memory in initPattern()!\n");
        return NULL;
    }
    pPattern_ctxt->pattern = pattern;
    pPattern_ctxt->pattern_len = pattern_len;
    memset(pPattern_ctxt->pattern_mask, -1, sizeof(pPattern_ctxt->pattern_mask));

    uint16_t i;
    for ( i = 0; i < pattern_len; ++i )
    {
        pPattern_ctxt->pattern_mask[(uint8_t)pattern[i]] ^= (1LL << i);
        if ( islower(pattern[i]) && pPattern_ctxt->pattern_mask[toupper(pattern[i])] != -1 )
        {
            pPattern_ctxt->pattern_mask[toupper(pattern[i])] ^= (1LL << i);
        }
    }
    pPattern_ctxt->is_lower = 1;

    for ( i = 0; i < pattern_len; ++i )
    {
        if ( isupper(pattern[i]) )
        {
            pPattern_ctxt->is_lower = 0;
            break;
        }
    }

    return pPattern_ctxt;
}

ValueElements* evaluate(TextContext* pText_ctxt,
                        PatternContext* pPattern_ctxt,
                        uint16_t k,
                        ValueElements* val)
{
    uint64_t* text_mask = pText_ctxt->text_mask;
    uint16_t col_num = pText_ctxt->col_num;
    uint16_t j = pText_ctxt->offset;

    char* pattern = pPattern_ctxt->pattern;
    uint16_t base_offset = pattern[k] * col_num;
    uint64_t x = text_mask[base_offset + (j >> 6)] >> (j & 63);
    uint16_t i = 0;

    if ( x == 0 )
    {
        uint64_t bits = 0;
        uint16_t col = 0;
        for ( col = (j >> 6) + 1; col < col_num; ++col )
        {
            if ( (bits = text_mask[base_offset + col]) != 0 )
                break;
        }
        if ( bits == 0 )
        {
            memset(val, 0, sizeof(ValueElements));
            return val;
        }
        else
        {
            i = (col << 6) + FM_CTZ(bits);
        }
    }
    else
    {
        i = j + FM_CTZ(x);
    }

    /**
     * e.g., text = '~ab~~_abd~b~d', pattern = 'abbd'
     * val[k].end > 0 means k in val
     */
    if ( val[k].end > j + 1 )
        return val + k;

    uint16_t beg = 0;
    uint16_t end = 0;

    float max_prefix_score = 0.0f;
    float max_score = 0.0f;

    char* text = pText_ctxt->text;
    uint16_t text_len = pText_ctxt->text_len;
    uint16_t pattern_len = pPattern_ctxt->pattern_len - k;
    int64_t* pattern_mask = pPattern_ctxt->pattern_mask;

    float special = 0.0f;
    if ( i == 0 || text[i-1] == '_' || text[i-1] == '-' || text[i-1] == ' ' )
        special = 2;
    else if ( isupper(text[i]) )
        special = !isupper(text[i-1]) || (i+1 < text_len && islower(text[i+1])) ? 2 : 0.0f;
    else if ( text[i-1] == '.' )
        special = 1.9f;
    else if ( !isalnum(text[i-1]) )
        special = 2;
    else
        special = 0;
    ++i;
    int64_t d = -2;     /* ~1 */
    int64_t last = d;
    while ( i < text_len )
    {
        last = d;
        char c = text[i];
        /* c in pattern */
        if ( pattern_mask[(uint8_t)c] != -1 )
            d = (d << 1) | (pattern_mask[(uint8_t)c] >> k);
        /**
         * text = 'xxABC', pattern = 'abc'; text[i] == 'B'
         * text = 'xxABC', pattern = 'abc'; text[i] == 'C'
         * NOT text = 'xxABCd', pattern = 'abc'; text[i] == 'C'
         * 'Cd' is considered as a word
         */
        else if ( isupper(text[i-1]) && pattern_mask[tolower(c)] != -1
                  && (i+1 == text_len || !islower(text[i+1])) )
            d = (d << 1) | (pattern_mask[tolower(c)] >> k);
        else
            d = ~0;

        if ( d >= last )
        {
            float score = 0.0f;
            uint16_t end_pos = 0;
            uint16_t n = FM_BIT_LENGTH(~last);
            /* e.g., text = '~~abcd~~~~', pattern = 'abcd' */
            if ( n == pattern_len )
            {
                score = n*n + special;
                if ( special == 2 )
                {
                    val[k].score = score;
                    val[k].beg = i - n;
                    val[k].end = i;
                    return val + k;
                }
                else
                    end_pos = i;
            }
            else
            {
                float prefix_score = n*n + special;
                /**
                 * e.g., text = 'AbcxxAbcyyde', pattern = 'abcde'
                 * prefer matching 'Abcyyde'
                 */
                if ( prefix_score > max_prefix_score
                     || (special && prefix_score == max_prefix_score) )
                {
                    max_prefix_score = prefix_score;
                    pText_ctxt->offset = i;
                    ValueElements *pVal = evaluate(pText_ctxt, pPattern_ctxt, k + n, val);
                    score = pVal->score ? prefix_score + pVal->score : 0.0f;
                    end_pos = pVal->end;
                }
            }
            if ( score > max_score || (special && score == max_score) )
            {
                max_score = score;
                beg = i - n;
                end = end_pos;
            }
        }

        /*
         * e.g., text = 'a~c~~~~ab~c', pattern = 'abc',
         * to find the index of the second 'a'
         * `d == last` is for the case when text = 'kpi_oos1', pattern = 'kos'
         */
        if ( d == ~0 || d == last )
        {
            x = text_mask[base_offset + (i >> 6)] >> (i & 63);

            if ( x == 0 )
            {
                uint64_t bits = 0;
                uint16_t col = 0;
                for ( col = (i >> 6) + 1; col < col_num; ++col )
                {
                    if ( (bits = text_mask[base_offset + col]) != 0 )
                        break;
                }
                if ( bits == 0 )
                    break;
                else
                    i = (col << 6) + FM_CTZ(bits);
            }
            else
            {
                i += FM_CTZ(x);
            }

            if ( isupper(text[i]) )
                special = !isupper(text[i-1]) || (i+1 < text_len && islower(text[i+1])) ? 2 : 0.0f;
            else if ( text[i-1] == '_' || text[i-1] == '-' || text[i-1] == ' ' )
                special = 2;
            else if ( text[i-1] == '.' )
                special = 1.9f;
            else if ( !isalnum(text[i-1]) )
                special = 2;
            else
                special = 0;
            d = -2;
            ++i;
        }
        else
            ++i;
    }

    /* e.g., text = '~~~~abcd', pattern = 'abcd' */
    if ( i == text_len )
    {
        if ( ~d >> (pattern_len - 1) )
        {
            float score = 0.0f;
            if ( (score = pattern_len*pattern_len + special) > max_score )
            {
                max_score = score;
                beg = i - pattern_len;
                end = i;
            }
        }
    }

    val[k].score = max_score;
    val[k].beg = beg;
    val[k].end = end;

    return val + k;

}

float getWeight(char* text, uint16_t text_len, PatternContext* pPattern_ctxt)
{
    if ( !text || !pPattern_ctxt )
        return 0;

    uint16_t j = 0;
    uint16_t col_num = 0;
    uint64_t* text_mask = NULL;
    char* pattern = pPattern_ctxt->pattern;
    uint16_t pattern_len = pPattern_ctxt->pattern_len;
    int64_t* pattern_mask = pPattern_ctxt->pattern_mask;
    char first_char = pattern[0];
    char last_char = pattern[pattern_len - 1];

    if ( pattern_len == 1 )
    {
        if ( isupper(first_char) )
        {
            int16_t first_char_pos = -1;
            int16_t i;
            for ( i = 0; i < text_len; ++i )
            {
                if ( text[i] == first_char )
                {
                    first_char_pos = i;
                    break;
                }
            }
            if ( first_char_pos == -1 )
                return 0;
            else
                return 1.0f/(first_char_pos + 1) + 1.0f/text_len;
        }
        else
        {
            int16_t first_char_pos = -1;
            int16_t i;
            for ( i = 0; i < text_len; ++i )
            {
                if ( tolower(text[i]) == first_char )
                {
                    if ( first_char_pos == -1 )
                        first_char_pos = i;

                    if ( isupper(text[i]) || i == 0 || !isalnum(text[i-1]) )
                        return 2 + 1.0f/(i + 1) + 1.0f/text_len;
                }
            }
            if ( first_char_pos == -1 )
                return 0;
            else
                return 1.0f/(first_char_pos + 1) + 1.0f/text_len;
        }
    }

    if ( pPattern_ctxt->is_lower )
    {
        int16_t first_char_pos = -1;
        int16_t i;
        for ( i = 0; i < text_len; ++i )
        {
            if ( tolower(text[i]) == first_char )
            {
                first_char_pos = i;
                break;
            }
        }
        if ( first_char_pos == -1 )
            return 0;

        int16_t last_char_pos = -1;
        for ( i = text_len - 1; i >= first_char_pos; --i )
        {
            if ( tolower(text[i]) == last_char )
            {
                last_char_pos = i;
                break;
            }
        }
        if ( last_char_pos == -1 )
            return 0;

        col_num = (text_len + 63) >> 6;     /* (text_len + 63)/64 */
        size_t text_mask_size = (col_num << 8) * sizeof(uint64_t);
        /* uint64_t text_mask[256][col_num] */
        text_mask = (uint64_t*)malloc(text_mask_size);
        if ( !text_mask )
        {
            fprintf(stderr, "Out of memory in getWeight()!\n");
            return 0;
        }
        memset(text_mask, 0, text_mask_size);
        char c;
        for ( i = first_char_pos; i <= last_char_pos; ++i )
        {
            c = tolower(text[i]);
            /* c in pattern */
            if ( pattern_mask[(uint8_t)c] != -1 )
            {
                text_mask[c * col_num + (i >> 6)] |= 1ULL << (i & 63);
                if ( j < pattern_len && c == pattern[j] )
                    ++j;
            }
        }
    }
    else
    {
        int16_t first_char_pos = -1;
        if ( isupper(first_char) )
        {
            int16_t i;
            for ( i = 0; i < text_len; ++i )
            {
                if ( text[i] == first_char )
                {
                    first_char_pos = i;
                    break;
                }
            }
        }
        else
        {
            int16_t i;
            for ( i = 0; i < text_len; ++i )
            {
                if ( tolower(text[i]) == first_char )
                {
                    first_char_pos = i;
                    break;
                }
            }
        }
        if ( first_char_pos == -1 )
            return 0;

        int16_t last_char_pos = -1;
        if ( isupper(last_char) )
        {
            int16_t i;
            for ( i = text_len - 1; i >= first_char_pos; --i )
            {
                if ( text[i] == last_char )
                {
                    last_char_pos = i;
                    break;
                }
            }
        }
        else
        {
            int16_t i;
            for ( i = text_len - 1; i >= first_char_pos; --i )
            {
                if ( tolower(text[i]) == last_char )
                {
                    last_char_pos = i;
                    break;
                }
            }
        }
        if ( last_char_pos == -1 )
            return 0;

        col_num = (text_len + 63) >> 6;
        size_t text_mask_size = (col_num << 8) * sizeof(uint64_t);
        /* uint64_t text_mask[256][col_num] */
        text_mask = (uint64_t*)malloc(text_mask_size);
        if ( !text_mask )
        {
            fprintf(stderr, "Out of memory in getWeight()!\n");
            return 0;
        }
        memset(text_mask, 0, text_mask_size);
        char c;
        int16_t i;
        for ( i = first_char_pos; i <= last_char_pos; ++i )
        {
            c = text[i];
            if ( isupper(c) )
            {
                /* c in pattern */
                if ( pattern_mask[(uint8_t)c] != -1 )
                    text_mask[c * col_num + (i >> 6)] |= 1ULL << (i & 63);
                if ( pattern_mask[tolower(c)] != -1 )
                    text_mask[tolower(c) * col_num + (i >> 6)] |= 1ULL << (i & 63);
                if ( j < pattern_len && c == toupper(pattern[j]) )
                    ++j;
            }
            else
            {
                /* c in pattern */
                if ( pattern_mask[(uint8_t)c] != -1 )
                {
                    text_mask[c * col_num + (i >> 6)] |= 1ULL << (i & 63);
                    if ( j < pattern_len && c == pattern[j] )
                        ++j;
                }
            }
        }
    }

    if ( j < pattern_len )
    {
        free(text_mask);
        return 0;
    }

    TextContext text_ctxt;
    text_ctxt.text = text;
    text_ctxt.text_len = text_len;
    text_ctxt.text_mask = text_mask;
    text_ctxt.col_num = col_num;
    text_ctxt.offset = 0;

    ValueElements val[64];
    memset(val, 0, sizeof(val));
    ValueElements *pVal = evaluate(&text_ctxt, pPattern_ctxt, 0, val);
    float score = pVal->score;
    uint16_t beg = pVal->beg;
    uint16_t end = pVal->end;

    free(text_mask);

    return score + 0.4f/(end - beg) + 1.0f/text_len;
}


ValueElements* evaluateHighlights(TextContext* pText_ctxt,
                                  PatternContext* pPattern_ctxt,
                                  uint16_t k,
                                  ValueElements* val)
{
    return NULL;
}

static void delPatternContext(PyObject *obj)
{
    free(PyCapsule_GetPointer(obj, NULL));
}

static PyObject* fuzzyMatchC_initPattern(PyObject* self, PyObject* args)
{
    char* pattern;
    Py_ssize_t pattern_len;

    if ( !PyArg_ParseTuple(args, "s#:initPattern", &pattern, &pattern_len) )
        return NULL;

    PatternContext* pCtxt = initPattern(pattern, pattern_len);

    return PyCapsule_New(pCtxt, NULL, delPatternContext);
}

static PyObject* fuzzyMatchC_getWeight(PyObject* self, PyObject* args)
{
    char* text;
    Py_ssize_t text_len;
    PyObject* py_patternCtxt;

    if ( !PyArg_ParseTuple(args, "s#O:getWeight", &text, &text_len, &py_patternCtxt) )
        return NULL;

    PatternContext* pCtxt = (PatternContext*)PyCapsule_GetPointer(py_patternCtxt, NULL);
    if ( !pCtxt )
        return NULL;

    return Py_BuildValue("f", getWeight(text, text_len, pCtxt));
}

static PyMethodDef fuzzyMatchC_Methods[] =
{
    { "initPattern", fuzzyMatchC_initPattern, METH_VARARGS, "initialize the pattern." },
    { "getWeight", fuzzyMatchC_getWeight, METH_VARARGS, "" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef fuzzyMatchC_module = {
    PyModuleDef_HEAD_INIT,
    "fuzzyMatchC",   /* name of module */
    "fuzzy match algorithm written in C.",  /* module documentation, may be NULL */
    -1,
    fuzzyMatchC_Methods
};

PyMODINIT_FUNC PyInit_fuzzyMatchC(void)
{
    return PyModule_Create(&fuzzyMatchC_module);
}

#else

PyMODINIT_FUNC initfuzzyMatchC()
{
    Py_InitModule("fuzzyMatchC", fuzzyMatchC_Methods);
}

#endif

int main(int argc, const char *argv[])
{

    printf("%d\n", FM_BIT_LENGTH(0x2f00));

    return 0;
}

