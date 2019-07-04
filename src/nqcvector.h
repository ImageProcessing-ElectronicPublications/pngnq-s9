/*  
 * pngnq-s9: nqcvector.h 
 * 
 * ------------------------------------------------------------------------
 *  pngnq-s9 Authorship and Copyright
 * --------------------------
 * 
 *  pngnq-s9 is a modification of pngnq.  pngnq is based on pngquant and
 *  the NeuQuant procedure.  pngquant, in turn, was based on ppmquant. 
 * 
 * 
 * ------------------------------------------------------------------------
 *  NeuQuant Notice
 * -----------------
 * 
 *   NeuQuant Neural-Net Quantization Algorithm
 *  
 *   Copyright (c) 1994 Anthony Dekker
 *  
 *   NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 *   See "Kohonen neural networks for optimal colour quantization" in
 *   "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 *   for a discussion of the algorithm.
 *   See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *  
 *   Any party obtaining a copy of these files from the author, directly or
 *   indirectly, is granted, free of charge, a full and unrestricted
 *   irrevocable, world-wide, paid up, royalty-free, nonexclusive right and
 *   license to deal in this software and documentation files (the
 *   "Software"), including without limitation the rights to use, copy,
 *   modify, merge, publish, distribute, sublicense, and/or sell copies of
 *   the Software, and to permit persons who receive copies from any such
 *   party to do so, with the only requirement being that this copyright
 *   notice remain intact.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngquant Notice
 * -----------------
 * 
 *   Copyright (c) 1998-2000 Greg Roelofs.  All rights reserved.
 * 
 *   This software is provided "as is," without warranty of any kind,
 *   express or implied.  In no event shall the author or contributors
 *   be held liable for any damages arising in any way from the use of
 *   this software.
 * 
 *   Permission is granted to anyone to use this software for any purpose,
 *   including commercial applications, and to alter it and redistribute
 *   it freely, subject to the following restrictions:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, disclaimer, and this list of conditions.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, disclaimer, and this list of conditions in the documenta-
 *      tion and/or other materials provided with the distribution.
 *   3. All advertising materials mentioning features or use of this
 *      software must display the following acknowledgment:
 * 
 *         This product includes software developed by Greg Roelofs
 *         and contributors for the book, "PNG: The Definitive Guide,"
 *         published by O'Reilly and Associates.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngnq Notice
 * --------------
 *
 *  Based on Greg Roelf's pngquant which was itself based on Jef
 *  Poskanzer's ppmquant.  Uses Anthony Dekker's Neuquant algorithm
 *  extended to handle the alpha channel.
 *
 *  Modified to quantize 32bit RGBA images for the pngnq program.  Also
 *  modified to accept a numebr of colors argument. 
 *  Copyright (c) Stuart Coyle 2004-2006
 * 
 *  Rewritten by Kornel Lesiński (2009):
 *  Euclidean distance, color matching dependent on alpha channel and with
 *  gamma correction. code refreshed for modern compilers/architectures:
 *  ANSI C, floats, removed pointer tricks and used arrays and structs.
 *
 *  Copyright (C) 1989, 1991 by Jef Poskanzer.
 *  Copyright (C) 1997, 2000, 2002 by Greg Roelofs; based on an idea by
 *                                Stefan Schneider.
 *  Copyright (C) 2004-2009 by Stuart Coyle
 *  Copyright (C) Kornel Lesiński (2009)
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and
 *  that both that copyright notice and this permission notice appear in
 *  supporting documentation.  This software is provided "as is" without
 *  express or implied warranty.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngnq-s9 Notice
 * -----------------
 * 
 *  Further alterations copyright (c) Adrian Pellas-Rice 2011-2012: YUV
 *  colour space, unisolate, transparency extenuation, user-supplied
 *  palette, per-component sensitivity, exclusion threshold, repel learning
 *  phase, remap result caching and SIMD oriented code, etc.  Alterations
 *  available under the same terms as for pngnq above.
 * 
 * 
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 *
 * nqcvector.h -- Colour vectors with 4 floating point components and SIMD
 *                friendly operations.
 *
 * Contributors: Pellas-Rice.
 *
 * ------------------------------------------------------------------------
 * 
 */
   
#ifndef __NQCVECTOR_H 
#define __NQCVECTOR_H

/*
 *  nq_colour_vector
 *
 *  nq_colour_vector is a vector with four float components, suitable for
 *  specifying an RGBA or YUVA colour in pngnq.
 *
 *  nq_colour_vector has two different definitions depending on the compiler.
 *  If we are using GCC, then we take advantage of GCC's built in platform
 *  independent SIMD vector type.  If not, we use a fixed-width array of floats
 *  - which a smart compiler would be able to generate SIMD instructions for
 *  anyway.
 *   
 *  Unfortunately, with the version of GCC used during coding, SIMD vector
 *  types such as v4sf had to be synthetically wrapped in a union to be
 *  accessed by non-SIMD code.  (This has changed in newer releases of GCC, so
 *  expect some refactoring in the future.)
 * 
 *  So we don't have to have lots and lots of code specific to compiler
 *  variants, for the non-GCC case, we define nq_colour_vector as a struct
 *  containing a fixed size array of floats.  This means that,
 *  syntactically, the two forms will be equivalent, ie. 
 *  "float current_red = curr_colour.elems[red];" will work in both
 *  versions.  
 *
 *  First we specify the size of the vector and various indices to access its
 *  components: */

#define NQ_COLOUR_NUM_COMPONENTS 4

/* Indices when the colour space doesn't matter. */
static const int zero = 0;
static const int one = 1;
static const int two = 2;
static const int three = 3;
static const int alpha = 3;

/* Indices for RGB: */    
static const int red = 0;
static const int green = 1;
static const int blue = 2;

/* Indices for YUV: */
static const int yy = 0;
static const int uu = 1;
static const int vv = 2;


/* If this compiler conforms with gcc 4.0 or more recent, we can use gcc's
 * built in vector SIMD ops. */     
#if __GNUC__ > 3
    #define __NQ_VECTOR_MODE
#endif

#ifdef __NQ_VECTOR_MODE
    
    /* GCC's weird 4x32bit float SIMD vector type. */
    typedef float v4sf __attribute__((vector_size(4*sizeof(float)))); 
  
    /* We wrap v4sf in a union.  This allows us to
     * inspect the individual floats inside. */
    typedef union _nq_colour_vector {
        v4sf vec;
        float elems[NQ_COLOUR_NUM_COMPONENTS];
    } nq_colour_vector;

#else
    
    typedef struct _nq_colour_vector {
        float elems[NQ_COLOUR_NUM_COMPONENTS];
    } nq_colour_vector;

#endif


/* 
 * init_vec -- Implementation independent initialiser.
 *
 * init_vec(a,b,c,d) initialises an nq_colour_vector declaration in an
 * implementation independent way.  Components zero to four are initialised by
 * a to d respectively.
 */
#ifdef __NQ_VECTOR_MODE
    #define init_vec(a,b,c,d) (nq_colour_vector){(v4sf){ (a), (b), (c), (d) }}
#else
    #define init_vec(a,b,c,d) (nq_colour_vector){ (a), (b), (c), (d) }
#endif


/* 
 * Vector operation macros, implmentation independent.
 *
 * These macros either provide a GCC SIMD v4sf vector operation or call an inline
 * function to sequentially simulate one.
 *
 * vec_add(a,b) takes two nq_colour_vectors, and returns the nq_colour_vector
 * that is the component-wise sum of a and b.
 *
 * vec_mul, vec_sub, and vec_div work similarly to vec_add().
 *
 * sca_mul(a,b) takes an individual scalar float, a, and an nq_colour_vector,
 * b, and returns the nq_colour_vector you get by multiplying each component of
 * b by a.  GCC versions circa 4.4 don't have a simple operation to do this
 * with type V4sf, so we provide a function for that case.
 *
 * sum_com(a) takes an nq_vector and returns the scalar float that is the
 * sum of all the vector's components.  Useful for matrix multiplication.
 *
 * Also note that the = assignment operator will perform memberwise assignment
 * for both implementations.
 */

#ifdef __NQ_VECTOR_MODE
    #define vec_add(A,B)  ((nq_colour_vector)((A).vec + (B).vec))
    #define vec_mul(A,B)  ((nq_colour_vector)((A).vec * (B).vec))
    #define vec_sub(A,B)  ((nq_colour_vector)((A).vec - (B).vec))
    #define vec_div(A,B)  ((nq_colour_vector)((A).vec / (B).vec))
    #define sca_mul(A,B)  (seq_sca_mul((A),(B)))
    #define sum_com(A)    (seq_sum_com((A)))
#else
    #define vec_add(A,B)  (seq_vec_add((A),(B)))
    #define vec_mul(A,B)  (seq_vec_mul((A),(B)))
    #define vec_sub(A,B)  (seq_vec_sub((A),(B)))
    #define vec_div(A,B)  (seq_vec_div((A),(B)))
    #define sca_mul(A,B)  (seq_sca_mul((A),(B)))
    #define sum_com(A)    (seq_sum_com((A)))
#endif



/* 
 * Inline functions for vector macros, as described above.
 *
 * These functions implement vec_add() etc using normal sequential C code.  An
 * intelligent compiler will be able to fully unroll these loops and SIMD-ify
 * the code if given the appopriate flags.
 */

static inline nq_colour_vector seq_vec_add(nq_colour_vector a, nq_colour_vector b) {
    nq_colour_vector ret;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret.elems[idx] = a.elems[idx] + b.elems[idx];
    }
    return ret;
}

static inline nq_colour_vector seq_vec_sub(nq_colour_vector a, nq_colour_vector b) {
    nq_colour_vector ret;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret.elems[idx] = a.elems[idx] - b.elems[idx];
    }
    return ret;
}

static inline nq_colour_vector seq_vec_mul(nq_colour_vector a, nq_colour_vector b) {
    nq_colour_vector ret;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret.elems[idx] = a.elems[idx] * b.elems[idx];
    }
    return ret;
}

static inline nq_colour_vector seq_vec_div(nq_colour_vector a, nq_colour_vector b) {
    nq_colour_vector ret;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret.elems[idx] = a.elems[idx] / b.elems[idx];
    }
    return ret;
}

static inline nq_colour_vector seq_sca_mul(float a, nq_colour_vector b) {
    nq_colour_vector ret;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret.elems[idx] = a * b.elems[idx];
    }
    return ret;
}

static inline float seq_sum_com(nq_colour_vector a) {
    float ret = 0.0;
    int idx;
    for(idx = 0; idx < NQ_COLOUR_NUM_COMPONENTS; idx++) {
        ret += a.elems[idx];
    }
    return ret;
}

#endif
