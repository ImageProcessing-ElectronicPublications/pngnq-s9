/*  
 * pngnq-s9: pngnq.c
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
 * pngnq.c -- Quantize a png image using a Neuquant procedure.
 *
 * This is the top level module for pngnq.  It is directly responsible for:
 *  -- using getopt to parse the command line arguments
 *  -- using pngrw to read and write input and output png files
 *  -- using neuquant32.c to learn a good colour palette for each input image
 *  -- redrawing (remapping) each input image to its chosen palette 
 *
 * Contributors: Roelf, Coyle, Lesiński, Pellas-Rice.
 *
 * ------------------------------------------------------------------------
 * 
 */


#define PNGNQ_VERSION "2.0.2"

#define FNMAX 1024

#include "pngnqhelp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h> /* isprint() and features.h */
#include <getopt.h>

#if HAVE_VALGRIND_H
# include <valgrind.h>
#endif

#if defined(WIN32) || defined(MSDOS)        
#  include <fcntl.h>        /* O_BINARY */
#  include <io.h>        /* setmode() */
#  define DIR_SEPARATOR_CHAR                '\\'
#  define DIR_SEPARATOR_STR                "\\"
#endif

#ifndef DIR_SEPARATOR_CHAR
#  define DIR_SEPARATOR_CHAR                '/'
#  define DIR_SEPARATOR_STR                "/"
#endif  

#include "png.h"
#include "neuquant32.h"
#include "rwpng.h"
#include "errors.h"

#define true 1
#define false 0

/* Error codes for pngnq(): */
#define INPUT_FILE_OPEN_ERROR 30
#define OUTPUT_FILE_EXISTS_ERROR 33
#define OUTPUT_FILE_OPEN_ERROR 35
#define PALETTE_FILE_OPEN_ERROR 37
#define PALETTE_TRNS_SORT_ERROR 39
#define REMAP_OUT_OF_MEMORY_ERROR 40

/* Image information struct */
static mainprog_info rwpng_info;
static mainprog_info rwpngpal_info;


/* Function prototypes, (documented at point of definition). */
static int pngnq(
        char* filename, 
        char* input_palette_name,
        char* newext,
        char* newdir,
        int using_stdin,
        int force,

        int n_colours,
        int sample_factor,
                int quantization_method,
        int colour_space,

        double force_gamma,
        int write_gamma,
        int strict_pal_rgba,

        double unisolate,
        double exclusion_threshold,

        double r_sens,
        double g_sens,
        double b_sens,
        double a_sens,
    
        double remap_r_sens,
        double remap_g_sens,
        double remap_b_sens,
        double remap_a_sens,

        double alpha_class_correction,
        int force_alpha_class_correctness,
        int use_alpha_importance_heuristic,

                int verbose
        ); 



/*
 * Function Definitions
 */


/* main() drives the pngnq program:  It parses the command line arguments and then repeatedly invokes pngnq(), quantizing each
 * input file one at a time.  main() will generally try to recover from errors caused by behavioural options that are out of
 * range or inaccessible files, but in certain cases it will fail and exit().
 *
 * Returns the following error codes:
 *      - EXIT_FAILURE if there was an error involving a global parameter that affects every input file, or
 *      - The number of input files affected by errors, if there were only errors that affected individual input files, or
 *      - Zero (EXIT_SUCCESS) otherwise.
 */  
int main(int argc, char** argv)
{
    int verbose = 0;
    int force = 0;
    int sample_factor = 0; /* will be set depending on image size */

    char *input_file_name = NULL;
    char *output_file_extension = "-nq8.png";
    char *output_directory = NULL;

    char *input_palette_name = NULL;

    int using_stdin = FALSE;
    int c; /* argument character */

    int errors = 0, file_count =0;
    int retval;
    int n_colours = 256; /* number of colours to quantize to. Default 256 */
    int use_floyd = 0;
    int dither_persistence = 10; /* Valid range 6 - 15, but user inputs 1 - 10*/
    int strict_pal_rgba = false;
    int force_alpha_class_correctness = false; /* Says whether -t or -T was used. */
    int colour_space = RGB;
    int use_alpha_importance_heuristic = true;
    int set_both_sens = true; /* set both sets of sens, or only remap ones. */
    int write_gamma = false;

    double force_gamma = 0;
    double unisolate = 0.0;
    double alpha_class_correction = 0.0; /* argument to -t or -T */
    double exclusion_threshold = 0.5;
    double r_sens = 1.0;
    double g_sens = 1.0;
    double b_sens = 1.0;
    double a_sens = 1.0;
    double remap_r_sens = 1.0;
    double remap_g_sens = 1.0;
    double remap_b_sens = 1.0;
    double remap_a_sens = 1.0;

    const float min_sens = 0.0625;
    const float max_sens = 1.0;

    int tempi;
    double deci;

    /* Parse arguments */
    while((c = getopt(argc,argv,"hHVvfLRAn:s:C:d:e:g:p:P:Q:t:T:u:x:0:1:2:a:")) != -1) {

        switch(c) {

            case 'A':
                use_alpha_importance_heuristic = false;
                break;

            case 'C':
                if(optarg[0] == 'r') {
                  colour_space = RGB;
                } else if(optarg[0] == 'y') {
                  colour_space = YUV;
                } else {
                  PNGNQ_WARNING("  -C%s does not specify a valid colour space.\n"
                                "  The valid options are -C r for RGB, or -C y for YUV.\n"
                                "Defaulting to RGB.\n", optarg);
                }
                break;

            case 'd':
                output_directory = optarg;
                break;

            case 'e':
                output_file_extension = optarg;
                break;


            case 'f':
                force = 1;
                break;

            case 'g':
                if(1 != sscanf(optarg, "%lf", &deci) || deci <= 0.001 || deci > 10.0) {
                    PNGNQ_WARNING(  "  -g %s does not specify a valid gamma value.\n"
                                    "  The valid options are rational numbers in the range "
                                       "(0.001, 10.0], -g2.1 for example.\n"
                                    "  Forcing to 1.0 for now.\nGamma writing is off.\n",
                                    optarg);
                } else {
                    force_gamma = deci;
                }
                write_gamma = false;
                break; 

            case 'G':
                if(1 != sscanf(optarg, "%lf", &deci) || deci <= 0.001 || deci > 10.0) {
                    PNGNQ_WARNING(  "  -G %s does not specify a valid gamma value.\n"
                                    "  The valid options are rational numbers in the range "
                                       "(0.001, 10.0], -g2.1 for example.\n"
                                    "  Forcing to 1.0 for now.  Gamma writing is ON.\n",
                                    optarg);
                } else {
                    force_gamma = deci;
                }
                write_gamma = true;
                break; 

            case 'h':
                fprintf(stderr,PNGNQ_USAGE);
                exit(EXIT_SUCCESS);
                break;

            case 'H':
                fprintf(stderr,"%s",PNGNQ_LONG_USAGE_ZERO);
                fprintf(stderr,"%s",PNGNQ_LONG_USAGE_ONE);
                fprintf(stderr,"%s",PNGNQ_LONG_USAGE_TWO);
                fprintf(stderr,"%s",PNGNQ_LONG_USAGE_THREE);
                exit(EXIT_SUCCESS);
                break;

            case 'L':
                /* Inflate relative sensitivity to chroma, not intensity or alpha. */
                colour_space = YUV;
                r_sens = 0.5;
                a_sens = 0.5;
                remap_r_sens = 0.75;
                remap_a_sens = 0.75;
                exclusion_threshold = 3.125;
                sample_factor = 1;
                use_floyd = 1;
                unisolate = 15.0;
                force_gamma = 1.0;
                break;

            case 'n':
                if(1 != sscanf(optarg, "%d", &tempi)) {
                    PNGNQ_WARNING(  "  -n %s does not specify a valid number of colours.\n"
                                    "  The valid options are integers from 1 to 256,"
                                    " -n64 for example.\n", optarg);
                    n_colours = 256;
                } else {
                    n_colours = tempi;
                }
                break;

            case 'P':
                strict_pal_rgba = true;
            case 'p':
                if(input_palette_name) {
                      PNGNQ_ERROR("At most one palette can be specified using -p or -P.\n"
                          " %s was specified as a palette before %s\n", input_palette_name, optarg);
                      exit(EXIT_FAILURE);
                }
                input_palette_name = optarg;
                break;

            case 'Q':
                if(optarg[0] == 'f') {
                    use_floyd = 1;
                } else if (optarg[0] == 'n') {
                    use_floyd = 0;
                } else {
                    if(1 != sscanf(optarg, "%d", &tempi) || tempi < 1 || tempi > 10) {
                        PNGNQ_WARNING(  "  -f %s does not specify a valid dithering"
                                           " method or persistence value.\n"
                                        "  Use an integer from 1 to 10 to turn on dithering and"
                                           "specify the dithering persistence.\n"
                                        "  -Qf is equivalent to -Q5, -Qn turns dithering off.\n"
                                        "  Assuming -Q5 this time.\n", optarg);
                        dither_persistence = 5 + 5;
                        use_floyd = 1;
                    } else {
                        dither_persistence = tempi + 5;
                        use_floyd = 1;
                    }
                }
                break;

            case 'R':
                set_both_sens = false;
                break;

            case 's':
                if(1 != sscanf(optarg, "%d", &tempi) || tempi < 1 || tempi > 1000) {
                    PNGNQ_WARNING(  "  -s %s does not specify a valid sample factor.\n"
                                    "  The valid options are integers from 1 to 1000,"
                                    " -s4 for example.\n", optarg);
                } else {
                    sample_factor = tempi;
                }
                break;

            case 'T':
                force_alpha_class_correctness = true;
            case 't':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < 0.0 || deci > 32.0) {
                    PNGNQ_WARNING(  "  %s is not a valid argument for -t or -T.\n"
                                    "  The valid options are rational numbers in the range "
                                       "[0.0, 32.0], -t8.0 for example.\n"
                                    "  -t and -T are OFF for now.\n", optarg);
                    alpha_class_correction = 0.0;
                    force_alpha_class_correctness = false;
                } else {
                    alpha_class_correction = deci;
                }
                break;

            case 'u':
                if(1 != sscanf(optarg, "%lf", &deci) || deci <= 0.001 || deci > 100.0) {
                    PNGNQ_WARNING(  "  -u %s does not specify a valid un-isolate value.\n"
                                    "  The valid options are rational numbers in the range "
                                       "(0.001, 100.0], -g12.5 for example.\n"
                                    "  Turning un-isolate off for now.\n", optarg);
                } else {
                    unisolate = deci;
                }
                break;
            
            case 'v':
                verbose = 1;
                break;

            case 'V':
                verbose = 1;
                PNGNQ_MESSAGE("pngnq %s\n",PNGNQ_VERSION);
                rwpng_version_info();
                exit(EXIT_SUCCESS);
                break;

            case 'x':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < 0.0 || deci > 32.0) {
                    PNGNQ_WARNING(  "  %s is not a valid argument for -x.\n"
                                    "  The valid options are rational numbers in the range "
                                       "[0.0, 32.0], -x4.25 for example.\n"
                                    "  Reverting to 0.5.\n", optarg);
                    exclusion_threshold = 0.5;
                } else {
                    exclusion_threshold = deci;
                }
                break;

            case '0':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < min_sens || deci > max_sens) {
                      PNGNQ_WARNING(  "  %s is not a valid argument for -0.\n"
                                      "  The valid options are rational numbers in the range "
                                         "[%f, %f], for example -0 0.75.\n"
                                      "  Reverting to 1.0.\n", optarg, min_sens, max_sens);
                      remap_r_sens = 1.0;
                } else {
                      remap_r_sens = deci;
                }
                
                if(set_both_sens) {
                  r_sens = remap_r_sens;
                }
                break;

            case '1':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < min_sens || deci > max_sens) {
                      PNGNQ_WARNING(  "  %s is not a valid argument for -1.\n"
                                      "  The valid options are rational numbers in the range "
                                         "[%f, %f], for example -1 0.75.\n"
                                      "  Reverting to 1.0.\n", optarg, min_sens, max_sens);
                      remap_g_sens = 1.0;
                } else {
                      remap_g_sens = deci;
                }
                
                if(set_both_sens) {
                  g_sens = remap_g_sens;
                }
                break;

            case '2':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < min_sens || deci > max_sens) {
                      PNGNQ_WARNING(  "  %s is not a valid argument for -2.\n"
                                      "  The valid options are rational numbers in the range "
                                         "[%f, %f], for example -2 0.75.\n"
                                      "  Reverting to 1.0.\n", optarg, min_sens, max_sens);
                      remap_b_sens = 1.0;
                } else {
                      remap_b_sens = deci;
                }
                
                if(set_both_sens) {
                  b_sens = remap_b_sens;
                }
                break;


            case 'a':
                if(1 != sscanf(optarg, "%lf", &deci) || deci < min_sens || deci > max_sens) {
                      PNGNQ_WARNING(  "  %s is not a valid argument for -a.\n"
                                      "  The valid options are rational numbers in the range "
                                         "[%f, %f], for example -a 0.75.\n"
                                      "  Reverting to 1.0.\n", optarg, min_sens, max_sens);
                      remap_a_sens = 1.0;
                } else {
                      remap_a_sens = deci;
                }
                
                if(set_both_sens) {
                  a_sens = remap_a_sens;
                }
                break;

            default:
                fprintf(stderr,PNGNQ_USAGE);
                exit(EXIT_FAILURE);
        }
    }

    /* In verbose mode, explictly print all the non-file settings. */

    PNGNQ_MESSAGE( "  Quantization settings:\n");
    PNGNQ_MESSAGE( "    Number of colours -n:               %d\n", n_colours);
    PNGNQ_MESSAGE( "    Sample factor -s:                   %d\n", sample_factor);
    PNGNQ_MESSAGE( "    Colour space -C:                    %s\n", 
                                        (colour_space == RGB) ? "RGB" : "YUV");
    PNGNQ_MESSAGE( "    Colorimportance suppression -A:     %s\n", 
                                        (use_alpha_importance_heuristic) ? "off" : "on");
    PNGNQ_MESSAGE( "    Override gamma value -g -G:         %f\n", force_gamma);
    PNGNQ_MESSAGE( "    Write png gAMA chunk -g -G:         %s\n",
                                        (write_gamma) ? "on" : "off");
    PNGNQ_MESSAGE( "    User palette -p -P:                 %s\n", input_palette_name);
    PNGNQ_MESSAGE( "    Palette strict RGBA -p -P:          %s\n", 
                                        (strict_pal_rgba) ? "on" : "off");
    PNGNQ_MESSAGE( "    Dither mode -Q:                     %s\n",
                                        (use_floyd) ? "floyd-steinberg" : "none");
    PNGNQ_MESSAGE( "    Dither persistence -Q:              %d\n", dither_persistence - 5);
    PNGNQ_MESSAGE( "    Transparency extenuation -t -T:     %f\n", alpha_class_correction);
    PNGNQ_MESSAGE( "    Force transparency accuracy -t -T:  %s\n", 
                                        (force_alpha_class_correctness) ? "on" : "off");
    PNGNQ_MESSAGE( "    Un-isolate factor -u:               %f\n", unisolate);
    PNGNQ_MESSAGE( "    Exclusion threshold -x:             %f\n", exclusion_threshold);
    PNGNQ_MESSAGE( "    \n");
    PNGNQ_MESSAGE( "    Sensitivities during colour selection:\n");
    PNGNQ_MESSAGE( "        Component zero -0:              %f\n", r_sens);
    PNGNQ_MESSAGE( "        Component one -1:               %f\n", g_sens);
    PNGNQ_MESSAGE( "        Component two -2:               %f\n", b_sens);
    PNGNQ_MESSAGE( "        Alpha component -a:             %f\n", a_sens);
    PNGNQ_MESSAGE( "    \n");
    PNGNQ_MESSAGE( "    Sensitivities during remapping -R:\n");
    PNGNQ_MESSAGE( "        Component zero -0:              %f\n", remap_r_sens);
    PNGNQ_MESSAGE( "        Component one -1:               %f\n", remap_g_sens);
    PNGNQ_MESSAGE( "        Component two -2:               %f\n", remap_b_sens);
    PNGNQ_MESSAGE( "        Alpha component -a:             %f\n", remap_a_sens);
    PNGNQ_MESSAGE( "    \n");


    /* determine input files */
    if(optind == argc) {
        using_stdin = TRUE;
        input_file_name = "stdin";
    } else {
        input_file_name=argv[optind];
        optind++;
    }
        
    /* Process each input file */
    while(optind<=argc) {

        PNGNQ_MESSAGE("  quantizing: %s \n",input_file_name);
        PNGNQ_MESSAGE("  output directory: %s \n",output_directory);
        
        retval = pngnq(
           input_file_name, input_palette_name, output_file_extension, output_directory,
           using_stdin, force,
           n_colours, sample_factor, (use_floyd) ? dither_persistence : 0, colour_space, 
           force_gamma, write_gamma, strict_pal_rgba, 
           unisolate, exclusion_threshold,
           r_sens, g_sens, b_sens, a_sens,
           remap_r_sens, remap_g_sens, remap_b_sens, remap_a_sens,
           alpha_class_correction, force_alpha_class_correctness, 
           use_alpha_importance_heuristic, verbose);

        if(retval){
          errors++;
        }
               
        input_file_name=argv[optind];
        file_count++;
        optind++;
    }

    /* Return, and perhaps say, how many errors there were. */
    if (errors) { 
        PNGNQ_MESSAGE("There were errors quantizing %d file%s out of a total of %d file%s.\n",
             errors, (errors == 1)? "" : "s",file_count, (file_count == 1)? "" : "s");
    } else {
        PNGNQ_MESSAGE("No errors detected while quantizing %d image%s.\n",
             file_count, (file_count == 1)? "" : "s");
    }
    exit(errors);
}


/* Creates an output file name based on the input file, extension and directory requested */
char *createoutname(char *infilename, char* newext, char *newdir){

    char *outname = NULL;
    int fn_len, ext_len, dir_len = 0;
    char *loc;

    outname = malloc(FNMAX);
    if (!outname) {
        PNGNQ_ERROR("  out of memory, cannot allocate output file name\n");
        exit(EXIT_FAILURE);
    }

    fn_len = strlen(infilename); 
    ext_len = strlen(newext)+1; /* include the terminating NULL*/
 
    if(newdir){
        dir_len = strlen(newdir);
        if(dir_len+fn_len+ext_len > FNMAX){
            PNGNQ_WARNING("  directory name too long, ignoring -d option\n");
            dir_len = 0;
            newdir=NULL;
        }
    }

    if(newdir){
        /* find the last directory separator */
        loc = strrchr(infilename, DIR_SEPARATOR_CHAR);
        if(loc) {
            /* strip original directory from filename */
            infilename = loc+1;           
            fn_len = strlen(infilename);
        }
         
        /* copy new directory name to output */
        strncpy(outname,newdir,dir_len);

        /* add a separator to newdir if needed */
        if(newdir[dir_len-1] != DIR_SEPARATOR_CHAR){
            *(outname + dir_len) = DIR_SEPARATOR_CHAR;
            dir_len++;
        }
    }

    if (fn_len > FNMAX-ext_len-dir_len) {
        PNGNQ_WARNING("  base filename [%s] will be truncated\n", infilename);
        fn_len = FNMAX-ext_len;
    }

    /* copy filename to output */
    strncpy(outname+dir_len,infilename,fn_len);

    /* Add extension */
    if (strncmp(outname+dir_len+fn_len-4, ".png", 4) == 0) {
        strncpy(outname+dir_len+fn_len-4, newext, ext_len);
    } else {
        strncpy(outname+dir_len+fn_len, newext, ext_len);
    }
  
    return(outname);
}


/* remap_floyd creates the 8 bit indexed pixel data for the output image by examining the 32 bit RGBA pixel data from the input
 * image and selecting the best matching colour for each pixel from the remap[] palette.  remap_floyd keeps track of the
 * accumulated colour error, and performs Floyd-Steinberg style dithering.
 */ 
static void remap_floyd(int cols, int rows, unsigned char map[MAXNETSIZE][4], unsigned int* remap,  
                        uch **row_pointers, int quantization_method, int use_alpha_importance_heuristic) {    
    uch *outrow = NULL; /* Output image pixels */

    int i,row;
    #define CLAMP(a) ((a)>=0 ? ((a)<=255 ? (a) : 255)  : 0)      

    /* Do each image row */
    for ( row = 0; (ulg)row < rows; ++row ) {
        int offset, nextoffset;
        outrow = rwpng_info.interlaced? row_pointers[row] :
        rwpng_info.indexed_data;
    
        int rederr=0;
        int blueerr=0;
        int greenerr=0;
        int alphaerr=0;
        
        offset = row*cols*4;
        nextoffset = offset; if (row+1<rows) nextoffset += cols*4;        
        int increment = 4; 
        
        if (0)//row&1)
        {
            offset += cols*4 - 4;
            nextoffset += cols*4 - 4;
            increment = -4;
        }
        
        for( i=0;i<cols;i++, offset+=increment, nextoffset+=increment)
        {
            int idx;
            unsigned int floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr;
            
            idx = inxsearch(CLAMP(rwpng_info.rgba_data[offset+3] - alphaerr),
                            CLAMP(rwpng_info.rgba_data[offset+2] - blueerr),
                            CLAMP(rwpng_info.rgba_data[offset+1] - greenerr),
                            CLAMP(rwpng_info.rgba_data[offset]   - rederr  ));                
                                    
            outrow[increment > 0 ? i : cols-i-1] = remap[idx];            
           
            int alpha = MAX(map[idx][3],rwpng_info.rgba_data[offset+3]);
            int colorimp = 255;
            if(use_alpha_importance_heuristic) {
                colorimp = 255 - ((255-alpha) * (255-alpha) / 255);
            } 

            int thisrederr=(map[idx][0] -   rwpng_info.rgba_data[offset]) * colorimp   / 255; 
            int thisblueerr=(map[idx][1] - rwpng_info.rgba_data[offset+1]) * colorimp  / 255; 
            int thisgreenerr=(map[idx][2] -  rwpng_info.rgba_data[offset+2]) * colorimp  / 255;
            int thisalphaerr=map[idx][3] - rwpng_info.rgba_data[offset+3];         

            rederr += thisrederr;
            greenerr += thisblueerr;
            blueerr +=  thisgreenerr;
            alphaerr += thisalphaerr;
            
            unsigned int thiserr = (thisrederr*thisrederr + thisblueerr*thisblueerr + thisgreenerr*thisgreenerr + thisalphaerr*thisalphaerr)*2;
             floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr;
            
            int L = quantization_method;
            while (rederr*rederr > L*L || greenerr*greenerr > L*L || blueerr*blueerr > L*L || alphaerr*alphaerr > L*L ||
                   floyderr > thiserr || floyderr > L*L*2)
            {            
                rederr /=2;greenerr /=2;blueerr /=2;alphaerr /=2;
                floyderr = rederr*rederr + greenerr*greenerr + blueerr*blueerr + alphaerr*alphaerr; 
            }
            
            if (i>0)
            {
                rwpng_info.rgba_data[nextoffset-increment+3]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+3] - alphaerr*3/16);
                rwpng_info.rgba_data[nextoffset-increment+2]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+2] - blueerr*3/16 );
                rwpng_info.rgba_data[nextoffset-increment+1]=CLAMP(rwpng_info.rgba_data[nextoffset-increment+1] - greenerr*3/16);
                rwpng_info.rgba_data[nextoffset-increment]  =CLAMP(rwpng_info.rgba_data[nextoffset-increment]   - rederr*3/16  );           
            }
            if (i+1<cols)
            {
                rwpng_info.rgba_data[nextoffset+increment+3]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+3] - alphaerr/16); 
                rwpng_info.rgba_data[nextoffset+increment+2]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+2] - blueerr/16 ); 
                rwpng_info.rgba_data[nextoffset+increment+1]=CLAMP(rwpng_info.rgba_data[nextoffset+increment+1] - greenerr/16);
                rwpng_info.rgba_data[nextoffset+increment]  =CLAMP(rwpng_info.rgba_data[nextoffset+increment]   - rederr/16  );           
            }
            rwpng_info.rgba_data[nextoffset+3]=CLAMP(rwpng_info.rgba_data[nextoffset+3] - alphaerr*5/16); 
            rwpng_info.rgba_data[nextoffset+2]=CLAMP(rwpng_info.rgba_data[nextoffset+2] - blueerr*5/16 ); 
            rwpng_info.rgba_data[nextoffset+1]=CLAMP(rwpng_info.rgba_data[nextoffset+1] - greenerr*5/16);
            rwpng_info.rgba_data[nextoffset]  =CLAMP(rwpng_info.rgba_data[nextoffset]   - rederr*5/16  );                   
            
            rederr = rederr*7/16; greenerr =greenerr*7/16; blueerr =blueerr*7/16; alphaerr =alphaerr*7/16; 
        }
        
      
        /* if non-interlaced PNG, write row now */
        if (!rwpng_info.interlaced)
            rwpng_write_image_row(&rwpng_info);
    }
    
}

/* remap_simple creates the 8 bit indexed pixel data for the output image by examining the 32 bit RGBA pixel data from the input
 * image and selecting the best matching colour for each pixel from the remap[] palette.  remap_simple does not attempt to
 * dither.
 */ 
static void remap_simple(unsigned int cols, unsigned int rows, unsigned char map[MAXNETSIZE][4], unsigned int* remap,  uch **row_pointers)
{
    uch *outrow = NULL; /* Output image pixels */
    
    unsigned int i,row;
    /* Do each image row */
    for ( row = 0; (ulg)row < rows; ++row ) 
    {
        unsigned int offset;
        outrow = rwpng_info.interlaced? row_pointers[row] : rwpng_info.indexed_data;
        /* Assign the new colors */
        offset = row*cols*4;
        for( i=0;i<cols;i++){
            outrow[i] = remap[inxsearch(rwpng_info.rgba_data[i*4+offset+3],
                                        rwpng_info.rgba_data[i*4+offset+2],
                                        rwpng_info.rgba_data[i*4+offset+1],
                                        rwpng_info.rgba_data[i*4+offset])];
        }
        
        /* if non-interlaced PNG, write row now */
        if (!rwpng_info.interlaced)
            rwpng_write_image_row(&rwpng_info);
    }
    
    
}

/* set_binary_mode(file) puts file in binary mode appropriately for the underlying OS. */
static void set_binary_mode(FILE *fp)
{
#if defined(MSDOS) || defined(FLEXOS) || defined(OS2) || defined(WIN32)
#if (defined(__HIGHC__) && !defined(FLEXOS))
    setmode(fp, _BINARY);
#else
    setmode(fp == stdout ? 1 : 0, O_BINARY);
#endif
#endif
}


/*
 *  pngnq() inspects a png image file, selects a limited number colours to represent the total colour gamut in that image, and
 *  redraws the image using only the selected colours into an new file.  The colours are selected using a modified Neuquant
 *  procedure.  pngnq() aims to produce output that is a close visual match to its input.
 *
 *  File naming, reading or writing errors will generally result in a non-zero error code being returned, however in certain
 *  cases pngnq() will print a warning message and attempt an arbitrary recovery.
 *
 *  Returns:  zero if execution was successful, or a non-zero error code otherwise, either a pngrw error code (as documented in
 *  that file) or: 30 if the input file could not be opened, 
 *
 *  Parameters:
 *
 *  filename is a standard file name string specifying the input image.  filename is only used if using_stdin is false.
 *  If filename is not a readable png file, an error code will be returned.
 *
 *  input_palette_name is a standard file name string specifying a user-supplied colour palette file.  If the user has not
 *  supplied one, input_palette_name should be NULL.  If input_palette_name is not a readable file, an error code will be
 *  returned.
 *
 *  newext is the new extension string for the output file name (if created), which will be constructed from the input image
 *  file name.  An input file name "BASE.png" and a new extension "EXT" will result in an output image file name of "BASEEXT",
 *  so EXT should normally include a ".png".  newext must not be NULL; for no extension use an empty string.
 *
 *  newdir is the standard file name string specifying the directory to which the output image file should be written.  newdir
 *  may be NULL if the user has specified no such directory.
 * 
 *  using_stdin should be true only if the input image(s) are to be piped from stdin, and false otherwise.  In this case, the
 *  output will be sent straight to stdout, making newdir and newext irrelevant.
 *
 *  force should be true iff the user intends to force overwriting, see -f. 
 *
 *  n_colours is the number of colours to quantize to, see -n.
 *
 *  sample_factor determines the ratio of input pixels to sample, see -s.
 *
 *  quantization_method should be 0 for no quantization, or else a dither persistence value above zero for Floyd-Steinberg
 *  dithering.  See -Q.
 *
 *  colour_space should be RGB or YUV.  See -c.
 *
 *  force_gamma is a (non-zero) gamma function value the user is insisting on, and which will override any file-embedded gamma
 *  values.  If the user has not supplied such a value, force_gamma should be zero.  See -g, -G.
 *
 *  write_gamma must be true if a gamma value is to be written into the output file, or false otherwise.  See -g, -G.
 *
 *  strict_pal_rgba must be true if user-supplied palette RGBA values are to be strictly retained, or false otherwise.
 *  See -p, -P.
 *
 *  unisolate is the un-isolate parameter.  Should be at least zero.  See -u.
 *
 *  exclusion_threshold is the exclusion theshold.  Should be at least zero.  See -x.
 *
 *  r_sens etc are the colour component sensitivities during colour selection.  Should be in the range (0.0, 1.0].
 *  r, g, b and a represent components 0, 1, 2, and 3 respectively in other colour models.  See -R and -0.
 *
 *  remap_r_sens etc are the colour component sensitivities during colour remapping.  Should be in the range (0.0, 1.0].
 *  r, g, b and a represent components 0, 1, 2, and 3 respectively in other colour models.  See -R and -0.
 *
 *  alpha_class_correction is the transparency extenuation value.  Should be at least zero.  See -t and -T.
 *
 *  force_alpha_class_correctness is true when alpha values of zero and 255 should be strictly
 *  retained in the output, and false otherwise.  See -t and -T.
 *
 *  use_alpha_importance_heuristic is true when the colorimportance heuristic should be used, and false otherwise.  See -A.
 */
static int pngnq(
        char* filename, 
        char* input_palette_name,
        char* newext,
        char* newdir,
        int using_stdin,
        int force,

        int n_colours,
        int sample_factor,
        int quantization_method,
        int colour_space,

        double force_gamma,
        int write_gamma,
        int strict_pal_rgba,

        double unisolate,
        double exclusion_threshold,

        double r_sens,
        double g_sens,
        double b_sens,
        double a_sens,
        
        double remap_r_sens,
        double remap_g_sens,
        double remap_b_sens,
        double remap_a_sens,

        double alpha_class_correction,
        int force_alpha_class_correctness,
        int use_alpha_importance_heuristic,

        int verbose
    ) {

    char *outname = NULL;
    FILE *infile = NULL;
    FILE *outfile = NULL;
    FILE *inpalette = NULL;

    int bot_idx, top_idx; /* for remapping of indices */
    unsigned int remap[MAXNETSIZE];

    ulg cols, rows;
    ulg row;
    unsigned char map[MAXNETSIZE][4];
    int x;
    uch **row_pointers=NULL; /* Pointers to rows of pixels */
    int newcolors = n_colours;

    double file_gamma;
    double quantization_gamma;

    int inpal_colours = 0;
    double pal_gamma = 1.8; 

    /* Open palette input file */
    if(input_palette_name) {
        if((inpalette = fopen(input_palette_name, "rb"))==NULL){
            PNGNQ_ERROR("  Cannot open %s for reading.\n",input_palette_name);
            return PALETTE_FILE_OPEN_ERROR;
        }

        /* Read palette input file */
        rwpng_read_image(inpalette, &rwpngpal_info);
        fclose(inpalette);

        if (rwpngpal_info.retval) {
            PNGNQ_ERROR("  rwpng_read_image() error: %d\n", rwpngpal_info.retval);
            return(rwpngpal_info.retval); 
        }
    }

    /* Open input file, or use stdin. */
    if(using_stdin) {        
        set_binary_mode(stdin);
        infile=stdin;
    } else {
        if((infile = fopen(filename, "rb"))==NULL){
            PNGNQ_ERROR("  Cannot open %s for reading.\n",filename);
            return INPUT_FILE_OPEN_ERROR;
        }
    }

    /* Read input file */
    rwpng_read_image(infile, &rwpng_info);
    if (!using_stdin) {
        fclose(infile);
    }

    if (rwpng_info.retval) {
        PNGNQ_ERROR("  rwpng_read_image() error: %d\n", rwpng_info.retval);
        if (!using_stdin) {
            fclose(outfile);
        }
        return(rwpng_info.retval); 
    }

    /* Open output file */  
    if(using_stdin) { 
        set_binary_mode(stdout);
        outfile = stdout;
    } else { 
        outname = createoutname(filename,newext,newdir);

        if (!force) {
            if ((outfile = fopen(outname, "rb")) != NULL) {
                PNGNQ_ERROR("  %s exists, not overwriting. Use -f to force.\n",outname);
                fclose(outfile);
                return OUTPUT_FILE_EXISTS_ERROR;
            }
        }

        if ((outfile = fopen(outname, "wb")) == NULL) {
            PNGNQ_ERROR("  Cannot open %s for writing\n", outname);
            return OUTPUT_FILE_OPEN_ERROR;
        }
    }

    /* Gather information about input palette file */

    if(input_palette_name) {

        inpal_colours = rwpngpal_info.width * rwpngpal_info.height;
        
        if(inpal_colours < 1) {
            PNGNQ_WARNING("  The palette file %s does not contain any pixels.\n"
                          "  An entirely new palette will be generated.\n", input_palette_name);
        
        } else if(inpal_colours > 256) {
            PNGNQ_ERROR("  The palette file %s contains more than 256 pixels.\n"
                        "  At most 256 pixels are allowed in the palette.\n", input_palette_name);
            return EXIT_FAILURE;
        }
      
      
        if(inpal_colours > n_colours ) {
            PNGNQ_WARNING("  Cannot quantize to only %d colours since the palette file %s\n"
                        "  already has more colours than that (%d).\n"
                        "  The image will be quantized using any of the %d colours in the palette,\n"
                        "  and only those colours.\n",
                        n_colours, input_palette_name, inpal_colours, inpal_colours);
            newcolors = inpal_colours;
        
        } else if(inpal_colours == n_colours) {
            PNGNQ_MESSAGE("  The palette file %s contains %d colours, which is exactly the number of\n"
                          "  colours to quantize to.  No new colours will used in the finished palette.\n",
                          input_palette_name, inpal_colours);
        } else {
            PNGNQ_MESSAGE("  The palette file %s contains %d colours, so up to %d additional colour(s)\n"
                          "  will be added to the finished palette during quantization.\n",
                          input_palette_name, inpal_colours, n_colours - inpal_colours);
        }


        if(!rwpngpal_info.rgba_data) {
           PNGNQ_WARNING("  no pixel data found for input palette.");
        }
    }


    /* Gather information about image and select which gamma to use */

    cols = rwpng_info.width;
    rows = rwpng_info.height;

    if(!rwpng_info.rgba_data) {
         PNGNQ_WARNING("  no pixel data found.");
    }

    file_gamma = rwpng_info.gamma;

    if (force_gamma > 0) {
         quantization_gamma = force_gamma;
         file_gamma=0;
      
         PNGNQ_MESSAGE("The input image will be gamma corrected"
                    " internally using the gamma value provided by the -g or "
                    "-G flag, "
                    "%1.4f (1/%1.1f)\n",
                    quantization_gamma,1.0/quantization_gamma);   
    
    } else if (file_gamma > 0) {          
        quantization_gamma = file_gamma;

        PNGNQ_MESSAGE("In the absence of -g or -G, the input image will be gamma"
                    " corrected internally using"
                    " the image file's own gamma value: %e (1/%e)\n",
                     file_gamma, 1.0/file_gamma);

    } else {
        quantization_gamma = 1.8;
        file_gamma = 0;
      
        PNGNQ_MESSAGE("In the absence of -g, -G or an explicit gamma value in "
                    "the input image, the input image will be gamma corrected"
                    " internally using the assumed gamma value of "
                    "%1.4f (1/%1.1f)\n",
                    quantization_gamma,1.0/quantization_gamma);   
    }

    /* Change rwpng_info for writing the output file:
     * - We need to kill any background colour, since it will be invalid after we quantize the image.
     * - If -G is on, we need to write the correct gamma value.  Otherwise, we need to kill the gamma value. 
     */ 
    rwpng_info.have_bg = false;
    if(write_gamma) {
       rwpng_info.gamma = quantization_gamma;
    } else {
       rwpng_info.gamma = 0;
    }


    /* If the user has not supplied a sample factor, choose one. We sample two in every sample factor pixels, (which actually
     * makes sample factor a 'sample divisor'), so a big sample factor means we do less work.
     */
    if (sample_factor<1) {
        sample_factor = 1 + rows*cols / (512*512);
        if (sample_factor > 10) {
            sample_factor = 10;
        }
        
        PNGNQ_MESSAGE("Sampling 1//%d of image\n", sample_factor);
    }

     /* Sort out palette gamma.  What a pain.  We need to decide *how* to get the palettes gamma value, and then *if* it will be
      * the same as the gamma value for the input image.  If it won't be, we then need to warn the user that they're probably
      * doing something stupid.  The 'how' part is explained by the four PNGNQ_MESSAGE calls below.  
      */

    if(input_palette_name)
    {
        if(strict_pal_rgba) {
            pal_gamma = quantization_gamma;
            PNGNQ_MESSAGE("Because you have used the -P strict RGBA flag, "
                        "the palette colours will be internally gamma "
                        "corrected using the same gamma value as the image, "
                        "%e (1/%e)\n", pal_gamma, 1.0/pal_gamma);

        } else if (force_gamma > 0) {
            pal_gamma = force_gamma;
            PNGNQ_MESSAGE("The palette will be gamma corrected"
                    " internally using the gamma value provided by the -g or "
                    "-G flag, "
                    "%1.4f (1/%1.1f)\n",
                    quantization_gamma,1.0/quantization_gamma);   

        } else if (rwpngpal_info.gamma > 0) {          
            pal_gamma = rwpngpal_info.gamma;
            PNGNQ_MESSAGE("In the absence of -g or -G, the palette will be"
                    " gamma corrected internally using"
                    " the palette file's own gamma value: %e (1/%e)\n",
                     pal_gamma, 1.0/pal_gamma);

        } else {
            pal_gamma = 1.8;
            PNGNQ_MESSAGE("In the absence of -g, -G or an explicit gamma value "
                    "in the palette file, the palette will be gamma corrected"
                    " internally using the assumed gamma value of "
                    "%1.4f (1/%1.1f)\n",
                    quantization_gamma,1.0/quantization_gamma);   
        }


        if(pal_gamma != quantization_gamma)
        {
            PNGNQ_WARNING(
            "Note that the palette gamma and image gamma are different.\n"
            "Colours read in from the palette file will be adjusted according"
            " to the palette gamma.\n"
            "But they will be written out using the main image gamma.\n"
            "To retain exact palette RGBA values, use -P when supplying"
            " the palette file name.\n"
            "Use -V verbose mode to find out how the gamma values were"
            " obtained.\n"
            );
        }
    }


    /* Start neuquant */
    if(!input_palette_name) {
        palinitnet(NULL, 0, 1.0, (unsigned char*)rwpng_info.rgba_data,rows*cols*4,newcolors,
            colour_space, quantization_gamma, alpha_class_correction,
            force_alpha_class_correctness, r_sens, g_sens, b_sens, a_sens,
            remap_r_sens, remap_g_sens, remap_b_sens, remap_a_sens,
            exclusion_threshold, use_alpha_importance_heuristic);
    } else {
        palinitnet((unsigned char*)rwpngpal_info.rgba_data,inpal_colours,pal_gamma,
               (unsigned char*)rwpng_info.rgba_data,rows*cols*4,newcolors,
            colour_space, quantization_gamma, alpha_class_correction, 
            force_alpha_class_correctness, r_sens, g_sens, b_sens, a_sens,
            remap_r_sens, remap_g_sens, remap_b_sens, remap_a_sens,
            exclusion_threshold, use_alpha_importance_heuristic);
    }
    learn(sample_factor, unisolate, verbose);
    if(strict_pal_rgba) {
        getcolormap_strict((unsigned char*) map, (unsigned char*) rwpngpal_info.rgba_data, inpal_colours);
    } else {
        getcolormap((unsigned char*)map);
    }

    /* Remap indexes so all tRNS chunks are together, unless strict_pal_rgba is on. */
    if(!strict_pal_rgba) {
        PNGNQ_MESSAGE("  Remapping colormap to eliminate opaque tRNS-chunk entries...\n");

        for (top_idx = newcolors-1, bot_idx = x = 0;  x < newcolors;  ++x) {
            if (map[x][3] == 255) { /* maxval */
              remap[x] = top_idx--;
            } else {
              remap[x] = bot_idx++;
            }
        }

        rwpng_info.num_trans = bot_idx;
        PNGNQ_MESSAGE( "%d entr%s left\n", bot_idx,(bot_idx == 1)? "y" : "ies");


        /* sanity check:  top and bottom indices should have just crossed paths */
        if (bot_idx != top_idx + 1) {
            PNGNQ_WARNING("  Internal logic error: remapped bot_idx = %d, top_idx = %d\n",bot_idx, top_idx);
            if (rwpng_info.row_pointers) {
                free(rwpng_info.row_pointers);
            }
            if (rwpng_info.rgba_data) {
                free(rwpng_info.rgba_data);
            }
            if (!using_stdin) {
                fclose(outfile);
            }
            return PALETTE_TRNS_SORT_ERROR;
        }
    } else {
        /* Make the remap do nothing when strict_pal_rgba is on */
        for(x = 0; x < newcolors; ++x) {
            remap[x] = x;
        }
        rwpng_info.num_trans = newcolors;
    }

    /* Fill in the palette info in the pngrw structure. */
    rwpng_info.sample_depth = 8;
    rwpng_info.num_palette = newcolors;

    /* GRR TO DO:  if bot_idx == 0, check whether all RGB samples are gray
     and if so, whether grayscale sample_depth would be same
     => skip following palette section and go grayscale */
     
    /* Remap and make palette entries */
    for (x = 0; x < newcolors; ++x) {
        rwpng_info.palette[remap[x]].red  = map[x][0];
        rwpng_info.palette[remap[x]].green = map[x][1];
        rwpng_info.palette[remap[x]].blue = map[x][2];
        rwpng_info.trans[remap[x]] = map[x][3];
    }

    /* Allocate memory*/
    if (rwpng_info.interlaced) {
        if ((rwpng_info.indexed_data = (uch *)malloc(rows * cols)) != NULL) {
            if ((row_pointers = (uch **)malloc(rows * sizeof(uch *))) != NULL) {
                for (row = 0;  (ulg)row < rows;  ++row) {
                    row_pointers[row] = rwpng_info.indexed_data + row*cols;
                }
            }
        }
    } else {
        rwpng_info.indexed_data = (uch *)malloc(cols);
    }
        
    if (rwpng_info.indexed_data == NULL || (rwpng_info.interlaced && row_pointers == NULL)) {
        PNGNQ_ERROR(" Insufficient memory for indexed data and/or row pointers\n");
        if (rwpng_info.row_pointers) {
            free(rwpng_info.row_pointers);
        }
        if (rwpng_info.rgba_data) {
            free(rwpng_info.rgba_data);
        }
        if (rwpng_info.indexed_data) {
            free(rwpng_info.indexed_data);
        }
        if (!using_stdin) {
            fclose(outfile);
        }
        return REMAP_OUT_OF_MEMORY_ERROR;
    }        

    /* Write headers and such. */
    if (rwpng_write_image_init(outfile, &rwpng_info) != 0) {
        PNGNQ_ERROR("  rwpng_write_image_init() error\n" );
        if (rwpng_info.rgba_data) {
            free(rwpng_info.rgba_data);
        }
        if (rwpng_info.row_pointers) {
            free(rwpng_info.row_pointers);
        }
        if (rwpng_info.indexed_data) {
            free(rwpng_info.indexed_data);
        }
        if (row_pointers) {
            free(row_pointers);
        }
        if (!using_stdin) {
            fclose(outfile);
        }
        return rwpng_info.retval;
    }

    /* Actually build the quantized output image using the colour palette Neuquant has already selected for us. */
    if (quantization_method > 0) {
        remap_floyd(cols,rows,map,remap,row_pointers, quantization_method,
                use_alpha_importance_heuristic);        
    } else {
        remap_simple(cols,rows,map,remap,row_pointers);
    }

    /* now we're done with the INPUT data and row_pointers, so free 'em */
    if (rwpng_info.rgba_data) {
        free(rwpng_info.rgba_data);
        rwpng_info.rgba_data = NULL;
    }
    if (rwpng_info.row_pointers) {
        free(rwpng_info.row_pointers);
        rwpng_info.row_pointers = NULL;
    }

    /* write entire interlaced palette PNG, or finish/flush noninterlaced one */
    if (rwpng_info.interlaced) {
        rwpng_info.row_pointers = row_pointers;   /* now for OUTPUT data */
        rwpng_write_image_whole(&rwpng_info);
    } else {
        rwpng_write_image_finish(&rwpng_info);
    }

    /* Have finished writing file */
    if (!using_stdin) {
        fclose(outfile);
    }

    /* now we're done with the OUTPUT data and row_pointers, too */
    if (rwpng_info.indexed_data) {
        free(rwpng_info.indexed_data);
        rwpng_info.indexed_data = NULL;
    }
    if (row_pointers) {
        free(row_pointers);
        row_pointers = rwpng_info.row_pointers = NULL;
    }

    /* Clean up file name */
    if(outname) {
        free(outname);
        outname = NULL;
    }

    return 0;
}


