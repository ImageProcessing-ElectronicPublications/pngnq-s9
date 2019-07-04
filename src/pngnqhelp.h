/*  
 * pngnq-s9: pngnqhelp.h
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
 * pngnqhelp.h -- Help strings for pngnq-s9.
 *
 * Contributors: Coyle, Lesiński, Pellas-Rice.
 *
 * ------------------------------------------------------------------------
 * 
 */


#ifndef PNGNQHELP_H
#define PNGNQHELP_H

#define PNGNQ_USAGE "\
Usage:  pngnq [-hHvV][-d dir][-e ext][-f][-n colours][-s sample rate]\n\
              [-pP palette file][-Q dither mode][-A]\n\
              [-C colour space][-gG gamma][-tT transparency extenuation]\n\
              [-u unisolate factor][-x exclusion threshold]\n\
              [-a012 sensitivity] [-R] [-a012 new sensitivity]\n\
              [input files...]\n\
Options:\n\
   -h Print this help.\n\
   -H Print extended help.\n\
   -v Verbose mode. Prints status messages.\n\
   -V Print version number and library versions.\n\
\n\
   -n Number of colours, 1 to 256 (default).\n\
   -A Turn off alpha colour importance heuristic (rarely necessary).\n\
   -C Colour space: r = RGB (default) y = YUV.\n\
   -d Directory to put quantized images into.\n\
   -e Specify the new extension for quantized files. Default -nq8.png\n\
   -f Force ovewriting of files.\n\
   -g Override input file gamma and don't write output file gamma.\n\
   -G Override input file gamma and DO write output file gamma.\n\
   -L Low colour settings for when -n is under 40 and image has strong colour.\n\
   -p Palette file, allow gamma to vary RGBA values in output.\n\
   -P Palette file, strictly keep palette RGBA values regardless of gamma.\n\
   -Q Dither mode: n = no dithering, f = Floyd-Steinberg, or supply a number\n\
      to turn on Floyd-Steinberg with that persistence level.\n\
      0 = minimal persistence, 10 = full, default is 5.\n\
   -s Sample rate: 1 = slow, high quality; 10 = fast, lower quality.\n\
   -t Try harder to keep alpha values of 255 and 0 exactly accurate in output.\n\
      0 = none, 8 = some, 15 = a lot.\n\
   -T Same as -t, but force alpha 255 and 0 to be accurate, or else warn.\n\
   -u Unisolate distinct but rarely used colours. Try -u15 first.\n\
   -x Choose colours that differ by at least this amount in one component.\n\
   -0 Sensitivity reduction factor for colour component zero (R in RGB, Y in\n\
      YUV). 0.125 = least sensitive, 1.0 = fully sensitive (default).\n\
      Applies to both colour selection and pixel remapping when before -R,\n\
      but only to remapping when after -R.\n\
   -1 Sensitivity for component one (G in RGB, U in YUV). Works like -0.\n\
   -2 Sensitivity for colour component two (B in RGB, V in YUV). Works like -0.\n\
   -a Sensitivity reduction factor for alpha.  Works like -0.\n\
   -R All following -0, -1, -2 and -a flags apply only to remapping.\n\
\n\
   input files: The png files to be processed. Defaults to standard input if\n\
   not specified.\n\
\n\
  Quantizes a 32-bit RGBA PNG image to an 8 bit RGBA palette PNG\n\
  using the neuquant algorithm. The output file name is the input file name\n\
  extended with \"-nq8.png\"\n\n\
\
"

#define PNGNQ_LONG_USAGE_ZERO "\
pngnq - Quantize a PNG image to 256 or fewer colours using the Neuquant\n\
        algorithm.\n\
\n\
Usage:  pngnq OPTIONS INPUT-FILES\n\
\n\
        pngnq use a neural network to choose the best combination of 256\n\
        colours for each input file, and then redraws the input file using\n\
        only those colours and writes the result as its output file.\n\
\n\
        The output file name is the input file base name with a new extension\n\
        appended, \"-nq8.png\" by default.\n\
\n\
        If no input files are provided, input is read from stdin, and output\n\
        is written to stdout.\n\
\n\
        pngnq does not copy the structure of the original png file.  The output\n\
        file will always be indexed, and will not necessarily contain the same\n\
        gamma, background colour or 'comment' information as the original.\n\
\n\
"

#define PNGNQ_LONG_USAGE_ONE "\
Options:\n\
\n\
    Help and Information\n\
\n\
    [-H] prints this help message.\n\
    [-h] prints a shorter help message.\n\
    [-V] prints version information, including library versions.\n\
    [-v] prints verbose status messages.\n\
\n\
\n\
    Basic Settings\n\
\n\
    [-n NUMBER] Number of Colours\n\
      The number of colours the quantized image is to\n\
      contain, from 1 to 256 (default).  Example: -n64 for 64 colours.\n\
\n\
    [-s NUMBER] Sample Rate\n\
      Tell pngnq to sample one in how many pixels from the input image.\n\
      Example: -s3 samples one third of the image.\n\
      1 = slow, high quality; 10 = fast, lower quality.\n\
\n\
\n\
    File Settings\n\
\n\
    [-d DIRECTORY] Output Directory\n\
      Stipulates which directory the output files should be written to.\n\
      Normally each output file stays in the same directory as the file it\n\
      was derived from.  Example: -d ./results\n\
\n\
    [-e EXTENSION] Output File Extension\n\
      Specifies the new extension for quantized files.\n\
      Example: If your input file is myfile.png, with -e_LOW.png the output\n\
      file will be myfile_LOW.png. Defaults to -nq8.png\n\
\n\
    [-f] Force Overwrite\n\
      Forces pngnq to overwrite existing files.\n\
      It is not recommended to overwrite an input file while it is in use.\n\
\n\
\n\
    Palettes\n\
\n\
    The user can optionally supply a palette of fixed colours to be used in the\n\
    output image.  The palette should be provided in the form of a png image\n\
    that has exactly one pixel the colour of each palette entry.  If -n\n\
    requests more colours than there are colours in the palette, then pngnq\n\
    will freely select remaining colours as usual.  Only the colours that are\n\
    actually needed will be present in the output image, so using -n 240 with\n\
    a 240 colour palette may result in an output file that uses only 150\n\
    colours.\n\
    \n\
    [-P PALETTE-FILENAME] User-Supplied Palette, Strict RGBA\n\
      Uses the named palette, and keeps the exact RGBA values and ordering of\n\
      the palette colours in the output image. (Palette colours are still gamma\n\
      corrected internally, but the procedure is perfectly reversed prior to\n\
      writing output.)\n\
\n\
    [-p PALETTE-FILENAME] User-Supplied Palette, Nudge-able\n\
      Uses the named palette, as described above, except that this time pngnq's\n\
      internal processing (mainly gamma correction) is allowed to nudge the\n\
      palette colours and reorder them.\n\
\n\
\n\
    Gamma\n\
\n\
      pngnq uses gamma correction to help it choose and remap colours more\n\
      intelligently.  The gamma value can be specified in three ways:  \n\
      1) Using -g or -G, in which case the same gamma value is used for all\n\
         files.\n\
      2) In the absence of -g or -G, if a supplied file contain an explicit\n\
         gamma values (png gAMA), that value will be used for that file only.\n\
      3) In the absence of the above, we assume a gamma value of 1.8.\n\
      \n\
      To force gamma correction like a typical monitor, for example, you would\n\
      use -g2.2.\n\
\n\
      pngnq will not record the gamma value in the output file unless you use\n\
      -G and provide your own explicit gamma setting.\n\
\n\
    [-g NUMBER] Gamma Correction Value, Unrecorded\n\
      Force the use of the supplied gamma correction value, (but don't record\n\
      it in the output file).  Example: -g2.2 (monitor gamma).\n\
      Values in the range [0.1, 10] are accepted.\n\
      \n\
    [-G NUMBER] Gamma Correction Value, Recorded\n\
      Force the use of the supplied gamma value, and record it in the output\n\
      file in a png gAMA chunk.  Example: -G1.0 (no gamma, recorded).\n\
      Values in the range [0.1, 10] are accepted.\n\
"

#define PNGNQ_LONG_USAGE_TWO "\
\n\
\n\
    Transparency\n\
\n\
    [-t NUMBER] Transparency Extenuation\n\
      -t tries harder to keep alpha values of 255 and 0 exactly  accu‐\n\
      rate  in  the  output by using transparency extenuation when the\n\
      quantized colour palette is first selected.  Example:  -t8.   In\n\
      general, 0 = none, 8 = some, 15 = a lot.  Defaults to zero.\n\
\n\
    [-T NUMBER] Transparency Extenuation with Strict Remapping\n\
      -T works the same way as -t when the colour palette is initially\n\
      selected.  But it then also tries to force alpha values  of  255\n\
      and  0 to be strictly retained in the output, even if that means\n\
      making an otherwise poor substitution - opaque  red  for  opaque\n\
      blue, for example.  -T will warn when forcing fails.\n\
\n\
    [-A] Alpha Heuristic Off\n\
       Turn off the alpha colour importance heuristic. This heuristic improves\n\
       images with semi-transparent areas, but can harm mostly grey images\n\
       with a lot of transparency.\n\
\n\
\n\
    Colour Selection\n\
\n\
    [-C LETTER] Colour Space\n\
      Selects the colour space used for internal processing (both colour\n\
      selection and remapping).  Use -Cr for RGBA (default), or -Cy for YUVA.\n\
      Note that pngnq's default YUVA settings effectively allocate 8 bits of\n\
      precision to each component - to alter the relative importance of Y, U,\n\
      V and A, use the sensitivity commands.\n\
\n\
    [-u NUMBER] Un-isolate\n\
      Un-isolates distinct but rarely used colours by the given factor.\n\
      Use -u when you notice small important patches of colour going missing in\n\
      the output image. Values in the range [0.0, 100.0] are accepted.\n\
      7.0 = a little, 15.0 = some, 31.0 = a lot.  High values can result in\n\
      degenerate output. When -u is needed, try -u15.0 first, and work from\n\
      there.  Defaults to zero (no effect).\n\
\n\
    [-x NUMBER] Exclusion Threshold\n\
      Try to choose colours that differ by at least this amount in at least one\n\
      component. -x4.25 will choose colours about 4 steps apart, so RGBA\n\
      (10,10,10,10) and (15,10,10,10) could both be chosen, but not\n\
      (14,10,10,10) as well.  Values in the range [0.0, 32.0] are accpeted.\n\
      Defaults to 0.5.  Use -x to push colours apart when you notice pngnq\n\
      is choosing too many similar colours.\n\
\n\
    [-Q LETTER] Dither Mode\n\
      Selects either Floyd-Steinberg dithering (-Qf) or no dithering (-Qn),\n\
      the default.  -Qf results in a default dithering extent equivalent to\n\
      -Q5, as described below.\n\
      \n\
      pngnq tends to choose colours that result in less dithering than\n\
      traditional quantizers.  When quantizing to a large number of\n\
      colours this is usually a good thing, resulting in subtle dithering and\n\
      smoother output.  However, when quantizing to very few colours intense\n\
      dithering may be the best option, in which case pngnq's performance may\n\
      be poor.\n\
\n\
    [-Q NUMBER] Dither Mode and Persistence\n\
      Turns on Floyd-Steinberg dithering *and* specifies its persistence.\n\
      Persistence values are integers in the range [1,10], -Q1 dithers with\n\
      minimal peristence, -Q10 with the maximum.  See above for more notes\n\
      about dithering.\n\
\n\
    [-L] Low Colour Mode\n\
      Shorthand used to apply various settings  suited  to  quantizing\n\
      richly coloured images to under 40 colours. -L overrides and can\n\
      be overridden by other options, so the position  of  -L  on  the\n\
      command  line  is significant.  Equivalent to -s1 -Cy -g1.0 -u15\n\
      -x3.125 -Q5 -0 0.5 -a 0.5 -R -0 0.75 -a 0.75.  Not  advised  for\n\
      images with soft chromatic variation.\n\
\n\
\n\
\n\
"

#define PNGNQ_LONG_USAGE_THREE "\
    Sensitivity\n\
\n\
    pngnq allows the individual components of the internal colour space to be\n\
    given less weight, or less sensitivity, in calculations.  If you need to\n\
    show fine-grained variations in blue, for example, you could desensitise\n\
    red, green and alpha to achieve this.\n\
    \n\
    Valid sensitivity values range from 0.0625 (one-sixteenth sensitivity, much\n\
    less accurate) to 1.0 (full sensitivity).\n\
\n\
    Normally the same sensitivity settings are used during colour selection and\n\
    input image remapping.  However it is possible to change the settings for\n\
    remapping only using -R.\n\
\n\
    [-0 NUMBER] Sensitivity Reduction Factor for Red or Y\n\
      Sets the sensitivity for component zero, (R in RGB, Y in YUV).\n\
      Example: -0 0.25 for one quarter the usual sensitivity.\n\
\n\
    [-1 NUMBER] Sensitivity Reduction Factor for Green or U\n\
      Sets the sensitivity for component one, (G in RGB, U in YUV).\n\
      Example: -1 0.5 for half the usual sensitivity.\n\
\n\
    [-2 NUMBER] Sensitivity Reduction Factor for Blue or V\n\
      Sets the sensitivity for component two, (B in RGB, V in YUV).\n\
      Example: -2 1.0 for full sensitivity.\n\
\n\
    [-a NUMBER] Sensitivity Reduction Factor for Alpha\n\
      Sets the sensitivity for alpha. \n\
      Example: -a 0.0625 for minimal sensitivity.\n\
     \n\
    [-R] Restrict Remaining Sensitivity Flags to Remapping\n\
      Causes all following sensitivity flags (-0 -1 -2 -a) to only apply to the\n\
      remapping phase of processing, not the colour selection phase.  Before\n\
      -R, or when -R is not present, the sensitivity flags apply to both colour\n\
      selection and remapping.  To choose colours with little regard to Y\n\
      'luminance', but then pay full attention to Y when remapping, you would\n\
      use: -0 0.0625 -R -0 1.0\n\
\n\
\n\
    Whole Command Examples:\n\
\n\
    Quantize mypicture.png down to 256 colours and save result as\n\
    mypicture-nq8.png:\n\
        pngnq mypicture.png\n\
\n\
    Quantize mypicture.png using 100 colours and processing internally using\n\
    the YUV colour space:\n\
        pngnq -Cy -n100 mypicture.png\n\
\n\
    Quantize mypicture.png with reduced sensitivity to alpha, but paying more\n\
    attention to distinct yet infrequent colours.  Write the result to\n\
    mypicture_new.png:\n\
        pngnq -e\"_new.png\" -a0.5 -u8.0 mypicture.png\n\
\n\
    Select quantization colours for mypicture.png with blue (-2) and alpha\n\
    (-a) at 30% (0.3) sensitivity, but then remap (recolour) the input image\n\
    with blue and alpha at full sensitivity.\n\
        pngnq -2 0.3 -a 0.3 -R -2 1.0 -a 1.0 mypicture.png\n\
\n\
    Quantize mypicture.png using only the 48 colours in mypalette.png.  Retain\n\
    the exact RGBA values from the palette.\n\
        pngnq -n48 -P mypalette.png mypicture.png\n\
\n\
    Quantize mypicture.png using the 30 colours in mypalette.png plus 20 more\n\
    chosen by the the program. Sample every input pixel for extra accuracy.\n\
    Don't necessarily retain the exact palette RGBA values if gamma or\n\
    sensitivity reduction alters them.\n\
        pngnq -s1 -n50 -p mypalette.png mypicture.png\n\
\n\
\n\
    Notes:\n\
\n\
    pngnq-s9 is used at your own risk, and carries  no  warranties  whatso‐\n\
    ever.  pngnq-s9 may make arbitrary assumptions in order to recover from\n\
    errors such as quantization parameters being out of range or file names\n\
    being too long.\n\
\n\
"

#endif
