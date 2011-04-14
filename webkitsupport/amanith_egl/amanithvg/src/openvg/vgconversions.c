/****************************************************************************
** Copyright (C) 2004-2009 Mazatech S.r.l. All rights reserved.
**
** This file is part of AmanithVG software, an OpenVG implementation.
** This file is strictly confidential under the signed Mazatech Software
** Non-disclosure agreement and it's provided according to the signed
** Mazatech Software licensing agreement.
**
** Khronos and OpenVG are trademarks of The Khronos Group Inc.
** OpenGL is a registered trademark and OpenGL ES is a trademark of
** Silicon Graphics, Inc.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** For any information, please contact info@mazatech.com
**
****************************************************************************/

/*!
	\file vgconversions.c
	\brief Pixelmap conversion routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgconversions.h"
#include "vgimage.h"

// [destination bits][source bits]
// used to map a [0; 2^N - 1] -> [0; 2^M - 1]
// y = x * (amBitConversionTable[N - 1][M - 1]) / 256
// where y belongs to [0; 2^N -1], x belongs to [0; 2^M - 1]
const AMuint32 amBitConversionTable[8][8] = {
	{   256, 0, 0,   32,   16,    0, 0,   2 }, // to 1 bit
	{     0, 0, 0,    0,    0,    0, 0,   0 }, // to 2 bit
	{     0, 0, 0,    0,    0,    0, 0,   0 }, // to 3 bit
	{  3840, 0, 0,  256,  128,   64, 0,  16 }, // to 4 bit
	{     0, 0, 0,  530,  256,  128, 0,  32 }, // to 5 bit
	{     0, 0, 0, 1076,  521,  256, 0,  64 }, // to 6 bit
	{     0, 0, 0,    0,    0,    0, 0,   0 }, // to 7 bit
	{ 65280, 0, 0, 4352, 2106, 1037, 0, 256 }  // to 8 bit
};

// dither tables
const AMuint8 amDitherLumTable[16][16] = {
	{   1, 128,  32, 160,   8, 136,  40, 168,   2, 130,  34, 162,  10, 138,  42, 170 },
	{ 192,  64, 224,  96, 200,  72, 232, 104, 194,  66, 226,  98, 202,  74, 234, 106 },
	{  48, 176,  16, 144,  56, 184,  24, 152,  50, 178,  18, 146,  58, 186,  26, 154 },
	{ 240, 112, 208,  80, 248, 120, 216,  88, 242, 114, 210,  82, 250, 122, 218,  90 },
	{  12, 140,  44, 172,   4, 132,  36, 164,  14, 142,  46, 174,   6, 134,  38, 166 },
	{ 204,  76, 236, 108, 196,  68, 228, 100, 206,  78, 238, 110, 198,  70, 230, 102 },
	{  60, 188,  28, 156,  52, 180,  20, 148,  62, 190,  30, 158,  54, 182,  22, 150 },
	{ 252, 124, 220,  92, 244, 116, 212,  84, 254, 126, 222,  94, 246, 118, 214,  86 },
	{   3, 131,  35, 163,  11, 139,  43, 171,   1, 129,  33, 161,   9, 137,  41, 169 },
	{ 195,  67, 227,  99, 203,  75, 235, 107, 193,  65, 225,  97, 201,  73, 233, 105 },
	{  51, 179,  19, 147,  59, 187,  27, 155,  49, 177,  17, 145,  57, 185,  25, 153 },
	{ 243, 115, 211,  83, 251, 123, 219,  91, 241, 113, 209,  81, 249, 121, 217,  89 },
	{  15, 143,  47, 175,   7, 135,  39, 167,  13, 141,  45, 173,   5, 133,  37, 165 },
	{ 207,  79, 239, 111, 199,  71, 231, 103, 205,  77, 237, 109, 197,  69, 229, 101 },
	{  63, 191,  31, 159,  55, 183,  23, 151,  61, 189,  29, 157,  53, 181,  21, 149 },
	{ 254, 127, 223,  95, 247, 119, 215,  87, 253, 125, 221,  93, 245, 117, 213,  85 }
};

const AMuint8 amDitherRedTable[16][16] = {
	{ 192,  11, 183, 125,  26, 145,  44, 244,   8, 168, 139,  38, 174,  27, 141,  43 },
	{ 115, 211, 150,  68, 194,  88, 177, 131,  61, 222,  87, 238,  74, 224, 100, 235 },
	{  59,  33,  96, 239,  51, 232,  16, 210, 117,  32, 187,   1, 157, 121,  14, 165 },
	{ 248, 128, 217,   2, 163, 105, 154,  81, 247, 149,  97, 205,  52, 182, 209,  84 },
	{  20, 172,  80, 140, 202,  41, 185,  55,  24, 197,  65, 129, 252,  35,  70, 147 },
	{ 201,  63, 189,  28,  90, 254, 116, 219, 137, 107, 231,  17, 144, 119, 228, 109 },
	{  46, 245, 103, 229, 134,  13,  67, 162,   6, 170,  47, 178,  76, 193,   4, 167 },
	{ 133,   9, 159,  54, 175, 124, 225,  93, 242,  79, 214,  99, 241,  56, 221,  92 },
	{ 186, 218,  78, 208,  37, 196,  25, 188,  42, 142,  29, 158,  21, 130, 156,  40 },
	{ 102,  31, 148, 111, 234,  85, 151, 120, 207, 113, 255,  86, 184, 212,  69, 236 },
	{ 176,  73, 253,   0, 138,  58, 249,  71,  10, 173,  62, 200,  50, 114,  12, 123 },
	{  23, 204, 118, 191,  91, 181,  19, 164, 216, 101, 233,   3, 135, 169, 246, 152 },
	{ 223,  60, 143,  48, 240,  34, 220,  82, 132,  36, 146, 106, 227,  30,  95,  49 },
	{  83, 166,  18, 199,  98, 155, 122,  53, 237, 179,  57, 190,  77, 195, 127, 180 },
	{ 230, 108, 215,  64, 171,   5, 206, 161,  22,  94, 251,  15, 153,  45, 243,   7 },
	{  72, 136,  39, 250, 104, 226,  75, 112, 198, 126,  66, 213, 110, 203,  89, 160 }
};

const AMuint8 amDitherGreenTable[16][16] = {
	{ 184,  63, 169,  99, 204,  70, 111, 179,  62, 231, 101, 206, 132,  85, 237, 109 },
	{ 125, 226,  27, 246,  42, 163,  31, 251, 140,  21, 174,  35, 223,  43, 143,   8 },
	{ 200,  40, 113, 137, 191,  86, 198, 124,  79, 215, 105, 151,  93, 192, 168,  75 },
	{  90, 149, 182,  69,  11, 238,  58,   4, 167,  50, 242,  12, 253,  59,  24, 232 },
	{  54, 248,  16, 220, 157,  96, 130, 227,  88, 186, 126,  73, 162, 118, 216, 138 },
	{ 175, 102, 205, 117,  47, 209, 172,  37, 202,  18, 146, 211,  32, 194,  82,   7 },
	{ 131,  38,  78, 144, 254,  25,  71, 150, 114, 236,  64, 107, 228,  49, 156, 240 },
	{ 197, 110, 187,   1, 161, 106, 180, 247,  92,  44, 183,   2, 127, 177,  95,  28 },
	{  67, 234,  56, 218,  83, 229,  52,  14, 139, 222,  81, 160, 249,  61, 208, 123 },
	{ 152,  19, 165, 128,  30, 135, 195, 116, 171,  23, 201, 100,  34, 141,  15, 244 },
	{  46, 224,  72, 207,  98, 243,  66, 214,  57, 255,  68, 190, 119, 217, 166,  89 },
	{ 136, 115, 178,   9, 189,  36, 158,   6, 103, 148, 129,  10, 233,  41,  74, 185 },
	{ 230,  33,  94, 252, 142, 122,  76, 239, 181,  29, 219,  87, 153, 112, 203,   5 },
	{ 104, 196, 154,  60,  48, 212, 164,  45, 108, 199,  53, 170,  26, 250,  55, 145 },
	{ 241,  22,  84, 221, 173,  20,  91, 225, 134,  80, 245, 121, 188,  97, 176,  77 },
	{  51, 210, 133,   3, 120, 235, 147,  13, 193,  39, 155,   0,  65, 213,  17, 159 }
};

const AMuint8 amDitherBlueTable[16][16] = {
	{  23, 233, 121, 159,  89, 149,  46,  82, 157, 122,  24, 150,  52, 226,  61, 143 },
	{ 170,  49, 205,  32, 215,  20, 248, 169,  37, 232,  98, 245, 128,  33, 182, 102 },
	{ 200, 131,  93, 176, 109, 183, 132,  14, 197,  66, 190,   5, 209,  79, 238,  13 },
	{  69, 250,   7,  71, 229,  53,  96, 237, 116, 145,  87, 165,  44, 141, 120, 160 },
	{  38, 114, 189, 153,  29, 139, 191,  76,  31, 224,  55, 254, 104, 204,  28, 223 },
	{ 138, 231,  60, 208,  84, 253,   2, 162, 211, 123, 179,  17, 152,  67, 174,  97 },
	{  78,  19, 167, 126, 103,  47, 203, 108,  62,  10, 134, 218,  81, 241,   1, 213 },
	{ 196, 148, 246,  12, 220, 173,  73, 155, 243, 185, 100,  50, 194, 110, 156,  56 },
	{ 119,  88,  43, 112, 144,  34, 234,  22,  91,  36, 249, 140,  21, 228,  40, 181 },
	{  26, 217, 127, 236,  59, 201, 135, 117, 193, 124, 166,  63, 177,  74, 133, 251 },
	{ 161,  51, 186,   4, 178,  94,  15, 225,  48, 214,   6, 235,  95, 206,   8,  86 },
	{  99, 199,  68, 151,  80, 255, 164,  70, 147,  85, 158, 107,  30, 146, 221, 171 },
	{ 227,  16, 242, 115, 210,  39, 105, 188,  27, 202,  58, 180, 240, 118,  35,  64 },
	{ 106, 154,  41, 175,  25, 142, 239,  65, 230, 130, 252,  42, 136,  72, 192, 129 },
	{  45, 212, 137,  75, 195, 101,   0, 172, 113,  11,  92, 168,  18, 216,   3, 247 },
	{ 187,  83,   9, 244,  57, 222, 125, 207,  54, 219, 184,  77, 198, 111, 163,  90 }
};

// (gamma) table for conversion from linear (l) to non-linear (s) color space, one channel value
const AMuint32 amGammaTable[256] = {
	  0,  13,  22,  28,  33,  38,  42,  46,  49,  53,  56,  58,  61,  64,  66,  68, 
	 71,  73,  75,  77,  79,  81,  83,  85,  86,  88,  90,  91,  93,  95,  96,  98, 
	 99, 101, 102, 103, 105, 106, 108, 109, 110, 112, 113, 114, 115, 116, 118, 119, 
	120, 121, 122, 123, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 
	137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 147, 148, 149, 150, 151, 
	152, 153, 154, 154, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 163, 164, 
	165, 166, 167, 167, 168, 169, 170, 170, 171, 172, 173, 173, 174, 175, 175, 176, 
	177, 178, 178, 179, 180, 180, 181, 182, 182, 183, 184, 184, 185, 186, 186, 187, 
	188, 188, 189, 190, 190, 191, 192, 192, 193, 194, 194, 195, 195, 196, 197, 197, 
	198, 199, 199, 200, 200, 201, 202, 202, 203, 203, 204, 205, 205, 206, 206, 207, 
	207, 208, 209, 209, 210, 210, 211, 211, 212, 213, 213, 214, 214, 215, 215, 216, 
	216, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224, 224, 
	225, 226, 226, 227, 227, 228, 228, 229, 229, 230, 230, 231, 231, 232, 232, 233, 
	233, 234, 234, 235, 235, 236, 236, 237, 237, 237, 238, 238, 239, 239, 240, 240, 
	241, 241, 242, 242, 243, 243, 244, 244, 245, 245, 246, 246, 246, 247, 247, 248, 
	248, 249, 249, 250, 250, 251, 251, 251, 252, 252, 253, 253, 254, 254, 255, 255
};

// (inverse gamma) table for conversion from non-linear (s) to linear (l) color space, one channel value
const AMuint32 amGammaInvTable[256] = {
	  0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1, 
	  1,   1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4, 
	  4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7, 
	  8,   8,   8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13, 
	 13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  18,  19,  19,  20, 
	 21,  21,  22,  22,  23,  23,  24,  24,  25,  26,  26,  27,  27,  28,  29,  29, 
	 30,  31,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  38,  39,  40,  41, 
	 41,  42,  43,  44,  45,  45,  46,  47,  48,  49,  50,  51,  51,  52,  53,  54, 
	 55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70, 
	 71,  72,  73,  74,  76,  77,  78,  79,  80,  81,  82,  84,  85,  86,  87,  88, 
	 90,  91,  92,  93,  95,  96,  97,  99, 100, 101, 103, 104, 105, 107, 108, 109, 
	111, 112, 114, 115, 116, 118, 119, 121, 122, 124, 125, 127, 128, 130, 131, 133, 
	134, 136, 138, 139, 141, 142, 144, 146, 147, 149, 151, 152, 154, 156, 157, 159, 
	161, 163, 164, 166, 168, 170, 172, 173, 175, 177, 179, 181, 183, 184, 186, 188, 
	190, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 
	222, 224, 226, 229, 231, 233, 235, 237, 239, 242, 244, 246, 248, 250, 253, 255
};

const AMuint8 am8To4BitTable[256] = {
	 0,   0,   0,   0,   0,   0,   0,   0,
	 0,   1,   1,   1,   1,   1,   1,   1,
	 1,   1,   1,   1,   1,   1,   1,   1,
	 1,   1,   2,   2,   2,   2,   2,   2,
	 2,   2,   2,   2,   2,   2,   2,   2,
	 2,   2,   2,   3,   3,   3,   3,   3,
	 3,   3,   3,   3,   3,   3,   3,   3,
	 3,   3,   3,   3,   4,   4,   4,   4,
	 4,   4,   4,   4,   4,   4,   4,   4,
	 4,   4,   4,   4,   4,   5,   5,   5,
	 5,   5,   5,   5,   5,   5,   5,   5,
	 5,   5,   5,   5,   5,   5,   6,   6,
	 6,   6,   6,   6,   6,   6,   6,   6,
	 6,   6,   6,   6,   6,   6,   6,   7,
	 7,   7,   7,   7,   7,   7,   7,   7,
	 7,   7,   7,   7,   7,   7,   7,   7,
	 8,   8,   8,   8,   8,   8,   8,   8,
	 8,   8,   8,   8,   8,   8,   8,   8,
	 8,   9,   9,   9,   9,   9,   9,   9,
	 9,   9,   9,   9,   9,   9,   9,   9,
	 9,   9,  10,  10,  10,  10,  10,  10,
	10,  10,  10,  10,  10,  10,  10,  10,
	10,  10,  10,  11,  11,  11,  11,  11,
	11,  11,  11,  11,  11,  11,  11,  11,
	11,  11,  11,  11,  12,  12,  12,  12,
	12,  12,  12,  12,  12,  12,  12,  12,
	12,  12,  12,  12,  12,  13,  13,  13,
	13,  13,  13,  13,  13,  13,  13,  13,
	13,  13,  13,  13,  13,  13,  14,  14,
	14,  14,  14,  14,  14,  14,  14,  14,
	14,  14,  14,  14,  14,  14,  14,  15,
	15,  15,  15,  15,  15,  15,  15,  15
};

const AMuint32 amUnpremultiplyTable[256] = {
	         0, 2139095040, 1069547520,  713031680,  534773760,  427819008,  356515840,  305585005,  267386880,  237677226,  213909504,  194463185,  178257920,  164545772,  152792502,  142606336,
	 133693440,  125829120,  118838613,  112583949,  106954752,  101861668,   97231592,   93004132,   89128960,   85563801,   82272886,   79225742,   76396251,   73761897,   71303168,   69003065,
	  66846720,   64821061,   62914560,   61117001,   59419306,   57813379,   56291974,   54848590,   53477376,   52173049,   50930834,   49746396,   48615796,   47535445,   46502066,   45512660,
	  44564480,   43655000,   42781900,   41943040,   41136443,   40360283,   39612871,   38892637,   38198125,   37527983,   36880948,   36255848,   35651584,   35067131,   34501532,   33953889,
	  33423360,   32909154,   32410530,   31926791,   31457280,   31001377,   30558500,   30128099,   29709653,   29302671,   28906689,   28521267,   28145987,   27780455,   27424295,   27077152,
	  26738688,   26408580,   26086524,   25772229,   25465417,   25165824,   24873198,   24587299,   24307898,   24034775,   23767722,   23506538,   23251033,   23001021,   22756330,   22516789,
	  22282240,   22052526,   21827500,   21607020,   21390950,   21179158,   20971520,   20767913,   20568221,   20372333,   20180141,   19991542,   19806435,   19624725,   19446318,   19271126,
	  19099062,   18930044,   18763991,   18600826,   18440474,   18282863,   18127924,   17975588,   17825792,   17678471,   17533565,   17391016,   17250766,   17112760,   16976944,   16843268,
	  16711680,   16582132,   16454577,   16328969,   16205265,   16083421,   15963395,   15845148,   15728640,   15613832,   15500688,   15389172,   15279250,   15170886,   15064049,   14958706,
	  14854826,   14752379,   14651335,   14551666,   14453344,   14356342,   14260633,   14166192,   14072993,   13981013,   13890227,   13800613,   13712147,   13624809,   13538576,   13453427,
	  13369344,   13286304,   13204290,   13123282,   13043262,   12964212,   12886114,   12808952,   12732708,   12657367,   12582912,   12509327,   12436599,   12364711,   12293649,   12223400,
	  12153949,   12085282,   12017387,   11950251,   11883861,   11818204,   11753269,   11689043,   11625516,   11562675,   11500510,   11439010,   11378165,   11317963,   11258394,   11199450,
	  11141120,   11083393,   11026263,   10969718,   10913750,   10858350,   10803510,   10749221,   10695475,   10642263,   10589579,   10537413,   10485760,   10434609,   10383956,   10333792,
	  10284110,   10234904,   10186166,   10137891,   10090070,   10042699,    9995771,    9949279,    9903217,    9857580,    9812362,    9767557,    9723159,    9679163,    9635563,    9592354,
	   9549531,    9507089,    9465022,    9423326,    9381995,    9341026,    9300413,    9260151,    9220237,    9180665,    9141431,    9102532,    9063962,    9025717,    8987794,    8950188,
	   8912896,    8875913,    8839235,    8802860,    8766782,    8731000,    8695508,    8660303,    8625383,    8590743,    8556380,    8522290,    8488472,    8454921,    8421634,    8388608
};

//*************************************************************************************
//                Conversion between two pixelmaps (low level functions)
//*************************************************************************************

/*!
	\brief Copy a subregion from a source pixelmap to a destination pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels and srcPixels must point to valid 32bit aligned memory regions.
*/
void amPxlMapCopy32(void *dstPixels,
				   const AMint32 dstDataStride,
				   const AMint32 dstX,
				   const AMint32 dstY,
				   const void *srcPixels,
				   const AMint32 srcDataStride,
				   const AMint32 srcX,
				   const AMint32 srcY,
				   const AMint32 width,
				   const AMint32 height) {

	AMint32 y;
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 2);
	AMint32 dstJump = (dstDataStride >> 2);

	AM_ASSERT((srcDataStride & 0x03) == 0);
	AM_ASSERT((dstDataStride & 0x03) == 0);

	src32 = (const AMuint32 *)srcPixels;
	dst32 = (AMuint32 *)dstPixels;
	src32 += srcY * (srcDataStride >> 2) + srcX;
	dst32 += dstY * (dstDataStride >> 2) + dstX;

	for (y = height; y != 0; --y) {
		amMemcpy(dst32, src32, width * sizeof(AMuint32));
		src32 += srcJump;
		dst32 += dstJump;
	}
}

/*!
	\brief Copy a subregion of a source pixelmap inside a destination pixelmap. Source and destination pixelmaps
	must have the same 16bit pixel format.
	\param dstPixels pointer to the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels and srcPixels must point to valid 16bit aligned memory regions.
*/
void amPxlMapCopy16(void *dstPixels,
				   const AMint32 dstDataStride,
				   const AMint32 dstX,
				   const AMint32 dstY,
				   const void *srcPixels,
				   const AMint32 srcDataStride,
				   const AMint32 srcX,
				   const AMint32 srcY,
				   const AMint32 width,
				   const AMint32 height) {

	AMint32 y;
	const AMuint16 *src16 = (const AMuint16 *)srcPixels;
	AMuint16 *dst16 = (AMuint16 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 1);
	AMint32 dstJump = (dstDataStride >> 1);

	AM_ASSERT((srcDataStride & 0x01) == 0);
	AM_ASSERT((dstDataStride & 0x01) == 0);

	src16 += srcY * (srcDataStride >> 1) + srcX;
	dst16 += dstY * (dstDataStride >> 1) + dstX;
	for (y = height; y != 0; --y) {
		amMemcpy(dst16, src16, width * sizeof(AMuint16));
		src16 += srcJump;
		dst16 += dstJump;
	}
}

/*!
	\brief Copy a subregion of a source pixelmap inside a destination pixelmap. Source and destination pixelmaps
	must have the same 8bit pixel format.
	\param dstPixels pointer to the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels and srcPixels must point to valid 8bit aligned memory regions.
*/
void amPxlMapCopy8(void *dstPixels,
				  const AMint32 dstDataStride,
				  const AMint32 dstX,
				  const AMint32 dstY,
				  const void *srcPixels,
				  const AMint32 srcDataStride,
				  const AMint32 srcX,
				  const AMint32 srcY,
				  const AMint32 width,
				  const AMint32 height) {

	AMint32 y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = srcDataStride;
	AMint32 dstJump = dstDataStride;

	src8 += srcY * srcDataStride + srcX;
	dst8 += dstY * dstDataStride + dstX;

	for (y = height; y != 0; --y) {
		amMemcpy(dst8, src8, width * sizeof(AMuint8));
		src8 += srcJump;
		dst8 += dstJump;
	}
}

/*!
	\brief Copy a subregion of a source pixelmap inside a destination pixelmap. Source and destination pixelmaps
	must have black and white format.
	\param dstPixels pointer to the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels and srcPixels must point to valid 8bit aligned memory regions.
*/
void amPxlMapCopy1(void *dstPixels,
				   const AMint32 dstDataStride,
				   const AMint32 dstX,
				   const AMint32 dstY,
				   const void *srcPixels,
				   const AMint32 srcDataStride,
				   const AMint32 srcX,
				   const AMint32 srcY,
				   const AMint32 width,
				   const AMint32 height) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {
			// read BW bit
			AMuint32 i = x + srcX;
			AMuint8 j = (AMuint8)(i & 0x07);
			AMuint8 bw = (src8[i >> 3] >> j) & 0x01;
			AMuint8 mask;

			// write it to destination
			i = x + dstX;
			j = (AMuint8)(i & 0x07);
			mask = 1 << j;
			if (!bw)
				dst8[i >> 3] &= mask ^ 0xFF;
			else
				dst8[i >> 3] |= mask;
		}
		src8 += srcDataStride;
		dst8 += dstDataStride;
	}
}

//*************************************************************************************
//                  32 bit to all other formats conversion
//*************************************************************************************

/*!
	\brief Convert a subregion from a source pixelmap to a destination pixelmap. Source and destination pixelmaps
	must have a 32bit pixel format.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param consistentSrc AM_TRUE if source pixels are considered consistent (i.e. valid premultiplied rgba values for format with alpha channel, X channel = 255
	for format without alpha), else AM_FALSE.
	\pre dstPixels and srcPixels must point to valid 32bit aligned memory regions.
*/
void amPxlMapConvert32To32(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool consistentSrc) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 32);
	AM_ASSERT((srcDataStride & 0x03) == 0);
	AM_ASSERT((dstDataStride & 0x03) == 0);

	// in the case of same 32bit format with the same byte order and the same color space l/s:
	// 1) same premultiplication / unpremultiplication format
	// 2) source without alpha channel
	//
	// we can use a raw 32bit copy.
	if (consistentSrc &&
		pxlFormatTable[srcIdx][FMT_ORDER] == pxlFormatTable[dstIdx][FMT_ORDER] &&
		(src_flags & FMT_L) == (dst_flags & FMT_L) &&
		 (((src_flags & FMT_PRE) == (dst_flags & FMT_PRE)) || (!(src_flags & FMT_ALPHA))))
		amPxlMapCopy32(dstPixels, dstDataStride, dstX, dstY, srcPixels, srcDataStride, srcX, srcY, width, height);
	else {
		AMint32 x, y;
		const AMuint32 *src32 = (const AMuint32 *)srcPixels;
		AMuint32 *dst32 = (AMuint32 *)dstPixels;
		AMint32 srcJump = (srcDataStride >> 2) - width;
		AMint32 dstJump = (dstDataStride >> 2) - width;
		AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
		AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
		AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
		AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
		AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
		AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
		AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
		AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
		AMuint32 alphaFix;

		// we have to force a 255 alpha when:
		// 1) source format is without alpha (e.g. VG_sRGBX_8888 -> VG_lRGBA_8888_PRE)
		// 2) destination format is withour alpha (e.g  VG_lRGBA_8888_PRE -> VG_sRGBX_8888)
		alphaFix = ((src_flags & FMT_ALPHA) && (dst_flags & FMT_ALPHA)) ? 0x00 : 0xFF;

		src32 += srcY * (srcDataStride >> 2) + srcX;
		dst32 += dstY * (dstDataStride >> 2) + dstX;

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint32 r = (*src32 >> src_rSh) & 0xFF;
				AMuint32 g = (*src32 >> src_gSh) & 0xFF;
				AMuint32 b = (*src32 >> src_bSh) & 0xFF;
				AMuint32 a = ((*src32 >> src_aSh) & 0xFF) | alphaFix;

				if ((src_flags & (FMT_PRE | FMT_L)) != (dst_flags & (FMT_PRE | FMT_L))) {
					// remove premultiplication, if source format was premultiplied
					if (src_flags & FMT_PRE) {
						if (a == 0) {
							// in this case the result will be always 0
							src32++;
							*dst32++ = 0;
							continue;
						}
						else {
							// ensure valid premultiplied values
							r = AM_MIN(r, a);
							g = AM_MIN(g, a);
							b = AM_MIN(b, a);
							// remove premultiplication
							AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
						}
					}
					// l --> s conversion
					if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
						r = AM_GAMMA_TABLE(r);
						g = AM_GAMMA_TABLE(g);
						b = AM_GAMMA_TABLE(b);
					}
					else
					// s --> l conversion
					if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
						r = AM_GAMMA_INV_TABLE(r);
						g = AM_GAMMA_INV_TABLE(g);
						b = AM_GAMMA_INV_TABLE(b);
					}
					// premultiply if requested by the destination format
					if (dst_flags & FMT_PRE) {
						MULT_DIV_255(r, r, a);
						MULT_DIV_255(g, g, a);
						MULT_DIV_255(b, b, a);
					}
				}
				// write the pixel
				src32++;
				*dst32++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src32 += srcJump;
			dst32 += dstJump;
		}
	}
}

/*!
	\brief Convert a subregion from a source 32bit pixelmap to a destination 16bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 16bit aligned memory region, srcPixels must point to valid 32bit aligned memory region.
*/
void amPxlMapConvert32To16(void *dstPixels,
						   const VGImageFormat dstFormat,
						   const AMint32 dstDataStride,
						   const AMint32 dstX,
						   const AMint32 dstY,
						   const void *srcPixels,
						   const VGImageFormat srcFormat,
						   const AMint32 srcDataStride,
						   const AMint32 srcX,
						   const AMint32 srcY,
						   const AMint32 width,
						   const AMint32 height,
						   const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint16 *dst16 = (AMuint16 *)dstPixels;
#if defined(__x86_64__) || defined(__amd64__)
	AMint32 srcJump = (srcDataStride >> 2) - width;
	AMint32 dstJump = (dstDataStride >> 1) - width;
#else
	AMint32 srcJump = (srcDataStride >> 2) - width;
	AMint32 dstJump = (dstDataStride >> 1) - width;
#endif
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 dst_rBits = pxlFormatTable[dstIdx][FMT_R_BITS];
	AMuint32 dst_gBits = pxlFormatTable[dstIdx][FMT_G_BITS];
	AMuint32 dst_bBits = pxlFormatTable[dstIdx][FMT_B_BITS];
	AMuint32 dst_aBits = pxlFormatTable[dstIdx][FMT_A_BITS];
	AMuint32 alphaFix, rMap, gMap, bMap, aMap;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 16);
	AM_ASSERT((srcDataStride & 0x03) == 0);
	AM_ASSERT((dstDataStride & 0x01) == 0);

	// going from a format without alpha, we have to force a 255 alpha
	alphaFix = (!(src_flags & FMT_ALPHA)) ? 0xFF : 0x00;
	rMap = amBitConversionTable[dst_rBits - 1][8 - 1];
	gMap = amBitConversionTable[dst_gBits - 1][8 - 1];
	bMap = amBitConversionTable[dst_bBits - 1][8 - 1];
	aMap = (dst_aBits > 0) ? amBitConversionTable[dst_aBits - 1][8 - 1] :	0;

	src32 += srcY * (srcDataStride >> 2) + srcX;
	dst16 += dstY * (dstDataStride >> 1) + dstX;

	for (y = height; y != 0; --y) {
		for (x = width; x != 0; --x) {

			AMuint16 r = (AMuint16)((*src32 >> src_rSh) & 0xFF);
			AMuint16 g = (AMuint16)((*src32 >> src_gSh) & 0xFF);
			AMuint16 b = (AMuint16)((*src32 >> src_bSh) & 0xFF);
			AMuint16 a = (AMuint16)(((*src32 >> src_aSh) & 0xFF) | alphaFix);

			// remove premultiplication, if source format was premultiplied
			if (src_flags & FMT_PRE) {
				if (a == 0) {
					// in this case the result will be always 0
					src32++;
					*dst16++ = 0;
					continue;
				}
				else {
					// ensure valid premultiplied values
					r = AM_MIN(r, a);
					g = AM_MIN(g, a);
					b = AM_MIN(b, a);
					// remove premultiplication
					AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
				}
			}
			// l --> s conversion
			if (src_flags & FMT_L) {
				r = AM_GAMMA_TABLE(r);
				g = AM_GAMMA_TABLE(g);
				b = AM_GAMMA_TABLE(b);
			}

			if (dither) {
				r += (amDitherRedTable[y & 0x0F][x & 0x0F] >> dst_rBits);
				g += (amDitherGreenTable[y & 0x0F][x & 0x0F] >> dst_gBits);
				b += (amDitherBlueTable[y & 0x0F][x & 0x0F] >> dst_bBits);
				if (dst_aBits == 1) {
					a = (a > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 0xFF : 0x00;
				}
				else {
					a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> dst_aBits);
					a = AM_MIN(a, 0xFF);
				}
				r = AM_MIN(r, 0xFF);
				g = AM_MIN(g, 0xFF);
				b = AM_MIN(b, 0xFF);
			}

			r = (rMap * r) >> 8;
			g = (gMap * g) >> 8;
			b = (bMap * b) >> 8;
			a = (aMap * a) >> 8;

			// write the pixel
			src32++;
			*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
		}
		src32 += srcJump;
		dst16 += dstJump;
	}
}

/*!
	\brief Convert a subregion from a source 32bit pixelmap to a destination 8bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 32bit aligned memory region.
*/
void amPxlMapConvert32To8(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 2) - width;
	AMint32 dstJump = dstDataStride - width;
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];
	AMuint32 alphaFix;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 8);
	AM_ASSERT((srcDataStride & 0x03) == 0);

	// going from a format without alpha, we have to force a 255 alpha (e.g. VG_sRGBX_8888 -> VG_lRGBA_8888_PRE)
	alphaFix = (!(src_flags & FMT_ALPHA)) ? 0xFF : 0x00;

	src32 += srcY * (srcDataStride >> 2) + srcX;
	dst8 += dstY * dstDataStride + dstX;

	if (dstFormat == VG_A_8) {

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x)
				// write the pixel
				*dst8++ = (AMuint8)(((*src32++ >> src_aSh) & 0xFF) | alphaFix);
			src32 += srcJump;
			dst8 += dstJump;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint8 lum;
				AMuint32 r = (*src32 >> src_rSh) & 0xFF;
				AMuint32 g = (*src32 >> src_gSh) & 0xFF;
				AMuint32 b = (*src32 >> src_bSh) & 0xFF;
				AMuint32 a = ((*src32 >> src_aSh) & 0xFF) | alphaFix;

				// remove premultiplication, if source format was premultiplied
				if (src_flags & FMT_PRE) {
					if (a == 0) {
						// in this case the result will be always 0
						src32++;
						*dst8++ = 0;
						continue;
					}
					else {
						// ensure valid premultiplied values
						r = AM_MIN(r, a);
						g = AM_MIN(g, a);
						b = AM_MIN(b, a);
						// remove premultiplication
						AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
					}
				}
				// s --> l conversion (luminance calculation must be done in linear space)
				if (!(src_flags & FMT_L)) {
					r = AM_GAMMA_INV_TABLE(r);
					g = AM_GAMMA_INV_TABLE(g);
					b = AM_GAMMA_INV_TABLE(b);
				}
				r *= 13933;	// 13933 = 0.2126 * 65536
				g *= 46871;	// 46871 = 0.7152 * 65536
				b *= 4732;	// 4732 = 0.0722 * 65536
				lum = (AMuint8)((r + g + b) >> 16);
				if (!(dst_flags & FMT_L))
					lum = AM_GAMMA_TABLE(lum);
				// write luminance
				src32++;
				*dst8++ = lum;
			}
			src32 += srcJump;
			dst8 += dstJump;
		}
	}
}

#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Convert a subregion from a source 32bit pixelmap to a destination 4bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 32bit aligned memory region.
*/
void amPxlMapConvert32To4(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 2) - width;
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 alphaFix;

	AM_ASSERT(dstFormat == VG_A_4);
	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 32);
	AM_ASSERT((srcDataStride & 0x03) == 0);

	(void)dstFormat;

	// going from a format without alpha, we have to force a 255 alpha
	alphaFix = (!(src_flags & FMT_ALPHA)) ? 0xFF : 0x00;

	src32 += srcY * (srcDataStride >> 2) + srcX;
	dst8 += dstY * dstDataStride;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {

			AMuint32 a = ((*src32++ >> src_aSh) & 0xFF) | alphaFix;
			AMuint32 i = x + dstX;
			AMuint8 dst = dst8[i >> 1];
			AMuint8 a8;

			if (dither) {
				a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> 4);
				a = AM_MIN(a, 0xFF);
			}
			a8 = am8To4BitTable[a];
			// write the pixel
			dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | (a8 << 4) : (dst & 0xF0) | (a8);
		}
		src32 += srcJump;
		dst8 += dstDataStride;
	}
}
#endif

/*!
	\brief Convert a subregion from a source 32bit pixelmap to a destination 1bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 32bit aligned memory region.
*/
void amPxlMapConvert32To1(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	const AMuint32 *src32 = (const AMuint32 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 2) - width;
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];

	AM_ASSERT((srcDataStride & 0x03) == 0);

	src32 += srcY * (srcDataStride >> 2) + srcX;
	dst8 += dstY * dstDataStride;

#if (AM_OPENVG_VERSION >= 110)
	if (dstFormat == VG_BW_1) {
#else
	(void)dstFormat;
#endif
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint8 lum, bw;
				AMuint32 r = (*src32 >> src_rSh) & 0xFF;
				AMuint32 g = (*src32 >> src_gSh) & 0xFF;
				AMuint32 b = (*src32 >> src_bSh) & 0xFF;
				AMuint32 a = ((*src32 >> src_aSh) & 0xFF);
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				// remove premultiplication, if source format was premultiplied
				if (src_flags & FMT_PRE) {
					if (a == 0) {
						// in this case the result will be always 0
						src32++;
						// write a 0 bit
						dst8[i >> 3] &= mask ^ 0xFF;
						continue;
					}
					else {
						// ensure valid premultiplied values
						r = AM_MIN(r, a);
						g = AM_MIN(g, a);
						b = AM_MIN(b, a);
						// remove premultiplication
						AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
					}
				}
				// s --> l conversion (luminance calculation must be done in linear space)
				if (!(src_flags & FMT_L)) {
					r = AM_GAMMA_INV_TABLE(r);
					g = AM_GAMMA_INV_TABLE(g);
					b = AM_GAMMA_INV_TABLE(b);
				}
				r *= 13933;	// 13933 = 0.2126 * 65536
				g *= 46871;	// 46871 = 0.7152 * 65536
				b *= 4732;	// 4732 = 0.0722 * 65536
				lum = (AMuint8)((r + g + b) >> 16);
					
				if (dither)
					bw = (lum > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = lum >> 7;

				src32++;
				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;
			}
			src32 += srcJump;
			dst8 += dstDataStride;
		}
#if (AM_OPENVG_VERSION >= 110)
	}
	else {
		// going from a format without alpha, we have to force a 255 alpha
		AMuint32 alphaFix = (!(src_flags & FMT_ALPHA)) ? 0xFF : 0x00;

		AM_ASSERT(dstFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint8 a = (AMuint8)(((*src32++ >> src_aSh) & 0xFF) | alphaFix);
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));
				AMuint8 bw;

				if (dither)
					bw = (a > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = a >> 7;

				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;

			}
			src32 += srcJump;
			dst8 += dstDataStride;
		}
	}
#endif
}

//*************************************************************************************
//                  16 bit to all other formats conversion
//*************************************************************************************

/*!
	\brief Convert a subregion from a source 16bit pixelmap to a destination 32bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 32bit aligned memory region, srcPixels must point to valid 16bit aligned memory region.
*/
void amPxlMapConvert16To32(void *dstPixels,
						   const VGImageFormat dstFormat,
						   const AMint32 dstDataStride,
						   const AMint32 dstX,
						   const AMint32 dstY,
						   const void *srcPixels,
						   const VGImageFormat srcFormat,
						   const AMint32 srcDataStride,
						   const AMint32 srcX,
						   const AMint32 srcY,
						   const AMint32 width,
						   const AMint32 height) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint16 *src16 = (const AMuint16 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 1) - width;
	AMint32 dstJump = (dstDataStride >> 2) - width;
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint16 src_rMask = (1 << pxlFormatTable[srcIdx][FMT_R_BITS]) - 1;
	AMuint16 src_gMask = (1 << pxlFormatTable[srcIdx][FMT_G_BITS]) - 1;
	AMuint16 src_bMask = (1 << pxlFormatTable[srcIdx][FMT_B_BITS]) - 1;
	AMuint16 src_aMask = (1 << pxlFormatTable[srcIdx][FMT_A_BITS]) - 1;
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];
	AMuint32 rMap = amBitConversionTable[pxlFormatTable[dstIdx][FMT_R_BITS] - 1][pxlFormatTable[srcIdx][FMT_R_BITS] - 1];
	AMuint32 gMap = amBitConversionTable[pxlFormatTable[dstIdx][FMT_G_BITS] - 1][pxlFormatTable[srcIdx][FMT_G_BITS] - 1];
	AMuint32 bMap = amBitConversionTable[pxlFormatTable[dstIdx][FMT_B_BITS] - 1][pxlFormatTable[srcIdx][FMT_B_BITS] - 1];
	AMuint32 aMap = amBitConversionTable[pxlFormatTable[dstIdx][FMT_A_BITS] - 1][pxlFormatTable[srcIdx][FMT_A_BITS] - 1];
	AMuint32 maxAlpha = (1 << pxlFormatTable[dstIdx][FMT_A_BITS]) - 1;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 16);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 32);
	AM_ASSERT((srcDataStride & 1) == 0);
	AM_ASSERT((dstDataStride & 3) == 0);

	src16 += srcY * (srcDataStride >> 1) + srcX;
	dst32 += dstY * (dstDataStride >> 2) + dstX;
	
	for (y = height; y != 0; --y) {
		for (x = width; x != 0; --x) {

			AMuint32 r = (AMuint32)((*src16 >> src_rSh) & src_rMask);
			AMuint32 g = (AMuint32)((*src16 >> src_gSh) & src_gMask);
			AMuint32 b = (AMuint32)((*src16 >> src_bSh) & src_bMask);
			AMuint32 a;

			r = (rMap * r) >> 8;
			g = (gMap * g) >> 8;
			b = (bMap * b) >> 8;

			if (!(src_flags & FMT_ALPHA))
				// going from a format without alpha to a format with alpha we have to force the
				// maximum value for the destination alpha
				a = maxAlpha;
			else {
				a = (AMuint32)((*src16) >> src_aSh) & src_aMask;
				a = (aMap * a) >> 8;
			}
			// s --> l conversion
			if (dst_flags & FMT_L) {
				r = AM_GAMMA_INV_TABLE(r);
				g = AM_GAMMA_INV_TABLE(g);
				b = AM_GAMMA_INV_TABLE(b);
			}
			// premultiply if requested by the destination format
			if (dst_flags & FMT_PRE) {
				MULT_DIV_255(r, r, a)
				MULT_DIV_255(g, g, a)
				MULT_DIV_255(b, b, a)
			}
			// write the pixel
			src16++;
			*dst32++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
		}
		src16 += srcJump;
		dst32 += dstJump;
	}
}

/*!
	\brief Convert a subregion from a source 16bit pixelmap to a destination 16bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 16bit aligned memory region, srcPixels must point to valid 16bit aligned memory region.
*/
void amPxlMapConvert16To16(void *dstPixels,
						   const VGImageFormat dstFormat,
						   const AMint32 dstDataStride,
						   const AMint32 dstX,
						   const AMint32 dstY,
						   const void *srcPixels,
						   const VGImageFormat srcFormat,
						   const AMint32 srcDataStride,
						   const AMint32 srcX,
						   const AMint32 srcY,
						   const AMint32 width,
						   const AMint32 height,
						   const AMbool dither) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 16);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 16);
	AM_ASSERT((srcDataStride & 0x01) == 0);
	AM_ASSERT((dstDataStride & 0x01) == 0);

	// in the case of same 16bit format, use raw 16bit copy
	if (srcFormat == dstFormat)
		amPxlMapCopy16(dstPixels, dstDataStride, dstX, dstY, srcPixels, srcDataStride, srcX, srcY, width, height);
	else {
		AMint32 x, y;
		const AMuint16 *src16 = (const AMuint16 *)srcPixels;
		AMuint16 *dst16 = (AMuint16 *)dstPixels;
		AMint32 srcJump = (srcDataStride >> 1) - width;
		AMint32 dstJump = (dstDataStride >> 1) - width;
		AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
		AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
		AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
		AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
		AMuint16 src_rBits = pxlFormatTable[srcIdx][FMT_R_BITS];
		AMuint16 src_gBits = pxlFormatTable[srcIdx][FMT_G_BITS];
		AMuint16 src_bBits = pxlFormatTable[srcIdx][FMT_B_BITS];
		AMuint16 src_aBits = pxlFormatTable[srcIdx][FMT_A_BITS];
		AMuint16 src_rMask = (1 << src_rBits) - 1;
		AMuint16 src_gMask = (1 << src_gBits) - 1;
		AMuint16 src_bMask = (1 << src_bBits) - 1;
		AMuint16 src_aMask = (1 << src_aBits) - 1;
		AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
		AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
		AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
		AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
		AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
		AMuint16 dst_rBits = pxlFormatTable[dstIdx][FMT_R_BITS];
		AMuint16 dst_gBits = pxlFormatTable[dstIdx][FMT_G_BITS];
		AMuint16 dst_bBits = pxlFormatTable[dstIdx][FMT_B_BITS];
		AMuint16 dst_aBits = pxlFormatTable[dstIdx][FMT_A_BITS];
		AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];
		AMuint32 rMap8 = 0, gMap8 = 0, bMap8 = 0, aMap8 = 0;
		AMuint32 rMap, gMap, bMap, aMap;
		AMuint16 maxAlpha = (1 << dst_aBits) - 1;

		if (dither) {
			rMap8 = amBitConversionTable[8 - 1][src_rBits - 1];
			gMap8 = amBitConversionTable[8 - 1][src_gBits - 1];
			bMap8 = amBitConversionTable[8 - 1][src_bBits - 1];
			aMap8 = (src_flags & FMT_ALPHA) ? amBitConversionTable[8 - 1][src_aBits - 1] : 0;

			rMap = amBitConversionTable[dst_rBits - 1][8 - 1];
			gMap = amBitConversionTable[dst_gBits - 1][8 - 1];
			bMap = amBitConversionTable[dst_bBits - 1][8 - 1];
			aMap = (dst_flags & FMT_ALPHA) ? amBitConversionTable[dst_aBits - 1][8 - 1] : 0;
		}
		else {
			rMap = amBitConversionTable[dst_rBits - 1][src_rBits - 1];
			gMap = amBitConversionTable[dst_gBits - 1][src_gBits - 1];
			bMap = amBitConversionTable[dst_bBits - 1][src_bBits - 1];
			aMap = (src_flags & FMT_ALPHA && dst_flags & FMT_ALPHA) ? amBitConversionTable[dst_aBits - 1][src_aBits - 1] : 0;
		}

		src16 += srcY * (srcDataStride >> 1) + srcX;
		dst16 += dstY * (dstDataStride >> 1) + dstX;

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint16 r = (*src16 >> src_rSh) & src_rMask;
				AMuint16 g = (*src16 >> src_gSh) & src_gMask;
				AMuint16 b = (*src16 >> src_bSh) & src_bMask;
				AMuint16 a;

				if (dither) {
					// r, g, b, a must be mapped towards [0;255], in order to do dithering
					r = (rMap8 * r) >> 8;
					if (src_rBits > dst_rBits) {
						r += (amDitherRedTable[y & 0x0F][x & 0x0F] >> dst_rBits);
						r = AM_MIN(r, 0xFF);
					}
					g = (gMap8 * g) >> 8;
					if (src_gBits > dst_gBits) {
						g += (amDitherGreenTable[y & 0x0F][x & 0x0F] >> dst_gBits);
						g = AM_MIN(g, 0xFF);
					}
					b = (bMap8 * b) >> 8;
					if (src_bBits > dst_bBits) {
						b += (amDitherBlueTable[y & 0x0F][x & 0x0F] >> dst_bBits);
						b = AM_MIN(b, 0xFF);
					}
					if (!(src_flags & FMT_ALPHA))
						// going from a format without alpha to a format with alpha we have to force the
						// maximum value for the destination alpha
						a = 0xFF;
					else {
						a = (*src16 >> src_aSh) & src_aMask;
						a = (aMap8 * a) >> 8;
						if (src_aBits > dst_aBits) {
							if (dst_aBits == 1)
								a = (a > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 0xFF : 0x00;
							else {
								a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> dst_aBits);
								a = AM_MIN(a, 0xFF);
							}
						}
					}
					r = (rMap * r) >> 8;
					g = (gMap * g) >> 8;
					b = (bMap * b) >> 8;
					a = (aMap * a) >> 8;
				}
				else {
					r = (rMap * r) >> 8;
					g = (gMap * g) >> 8;
					b = (bMap * b) >> 8;
					if (!(src_flags & FMT_ALPHA))
						// going from a format without alpha to a format with alpha we have to force the
						// maximum value for the destination alpha
						a = maxAlpha;
					else {
						a = (*src16 >> src_aSh) & src_aMask;
						a = (aMap * a) >> 8;
					}
				}

				// write the pixel
				src16++;
				*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src16 += srcJump;
			dst16 += dstJump;
		}
	}
}

/*!
	\brief Convert a subregion from a source 16bit pixelmap to a destination 8bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 16bit aligned memory region.
*/
void amPxlMapConvert16To8(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint16 *src16 = (const AMuint16 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 1) - width;
	AMint32 dstJump = dstDataStride - width;
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint16 src_rBits = pxlFormatTable[srcIdx][FMT_R_BITS];
	AMuint16 src_gBits = pxlFormatTable[srcIdx][FMT_G_BITS];
	AMuint16 src_bBits = pxlFormatTable[srcIdx][FMT_B_BITS];
	AMuint16 src_aBits = pxlFormatTable[srcIdx][FMT_A_BITS];
	AMuint16 src_rMask = (1 << src_rBits) - 1;
	AMuint16 src_gMask = (1 << src_gBits) - 1;
	AMuint16 src_bMask = (1 << src_bBits) - 1;
	AMuint16 src_aMask = (1 << src_aBits) - 1;
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];
	AMuint32 rMap, gMap, bMap, aMap;
	AMuint16 alphaFix;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 16);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 8);
	AM_ASSERT((srcDataStride & 0x01) == 0);

	rMap = amBitConversionTable[8 - 1][src_rBits - 1];
	gMap = amBitConversionTable[8 - 1][src_gBits - 1];
	bMap = amBitConversionTable[8 - 1][src_bBits - 1];
	if (src_flags & FMT_ALPHA) {
		aMap = amBitConversionTable[8 - 1][src_aBits - 1];
		alphaFix = 0x00;
	}
	else {
		aMap = 0;
		alphaFix = 0xFF;
	}

	src16 += srcY * (srcDataStride >> 1) + srcX;
	dst8 += dstY * dstDataStride + dstX;

	if (dstFormat == VG_A_8) {

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint16 a = (*src16 >> src_aSh) & src_aMask;

				// write the pixel
				src16++;
				*dst8++ = (AMuint8)(((aMap * a) >> 8) | alphaFix);
			}
			src16 += srcJump;
			dst8 += dstJump;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint8 lum;
				AMuint32 r = (AMuint32)((*src16 >> src_rSh) & src_rMask);
				AMuint32 g = (AMuint32)((*src16 >> src_gSh) & src_gMask);
				AMuint32 b = (AMuint32)((*src16 >> src_bSh) & src_bMask);

				// map r, g, b to full 8bit
				r = (rMap * r) >> 8;
				g = (gMap * g) >> 8;
				b = (bMap * b) >> 8;
				// s --> l conversion (luminance calculation must be done in linear space)
				r = AM_GAMMA_INV_TABLE(r);
				g = AM_GAMMA_INV_TABLE(g);
				b = AM_GAMMA_INV_TABLE(b);
				// luminance calculation
				r *= 13933;	// 13933 = 0.2126 * 65536
				g *= 46871;	// 46871 = 0.7152 * 65536
				b *= 4732;	// 4732 = 0.0722 * 65536
				lum = (AMuint8)((r + g + b) >> 16);
				if (!(dst_flags & FMT_L))
					lum = AM_GAMMA_TABLE(lum);
				// write luminance
				src16++;
				*dst8++ = lum;
			}
			src16 += srcJump;
			dst8 += dstJump;
		}
	}
}

#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Convert a subregion from a source 16bit pixelmap to a destination 4bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 16bit aligned memory region.
*/
void amPxlMapConvert16To4(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	const AMuint16 *src16 = (const AMuint16 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 1) - width;
	AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
	AMuint16 src_aBits = pxlFormatTable[srcIdx][FMT_A_BITS];
	AMuint16 src_aMask = (1 << src_aBits) - 1;
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 aMap;
	AMuint32 alphaFix;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 16);
	AM_ASSERT(dstFormat == VG_A_4);
	AM_ASSERT((srcDataStride & 0x01) == 0);

	(void)dstFormat;

	if (src_flags & FMT_ALPHA) {
		aMap = amBitConversionTable[8 - 1][src_aBits - 1];
		alphaFix = 0x00;
	}
	else {
		aMap = 0;
		alphaFix = 0xFF;
	}

	src16 += srcY * (srcDataStride >> 1) + srcX;
	dst8 += dstY * dstDataStride;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {

			AMuint8 a = ((AMuint8)(((aMap * ((*src16++ >> src_aSh) & src_aMask)) >> 8) | alphaFix));
			AMuint32 i = x + dstX;
			AMuint8 dst = dst8[i >> 1];

			// write the pixel
			dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | (a & 0xF0) : (dst & 0xF0) | (a >> 4);
		}
		src16 += srcJump;
		dst8 += dstDataStride;
	}
}
#endif

/*!
	\brief Convert a subregion from a source 16bit pixelmap to a destination 1bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 16bit aligned memory region.
*/
void amPxlMapConvert16To1(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	const AMuint16 *src16 = (const AMuint16 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = (srcDataStride >> 1) - width;
#if defined RIM_VG_SRC || defined TORCH_VG_SRC
#if (AM_OPENVG_VERSION >= 110)
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
#endif
#endif
	AMuint32 src_rSh = pxlFormatTable[srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[srcIdx][FMT_B_SH];
	AMuint16 src_rBits = pxlFormatTable[srcIdx][FMT_R_BITS];
	AMuint16 src_gBits = pxlFormatTable[srcIdx][FMT_G_BITS];
	AMuint16 src_bBits = pxlFormatTable[srcIdx][FMT_B_BITS];
	AMuint16 src_rMask = (1 << src_rBits) - 1;
	AMuint16 src_gMask = (1 << src_gBits) - 1;
	AMuint16 src_bMask = (1 << src_bBits) - 1;
	AMuint32 rMap, gMap, bMap;

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 16);

	rMap = amBitConversionTable[8 - 1][src_rBits - 1];
	gMap = amBitConversionTable[8 - 1][src_gBits - 1];
	bMap = amBitConversionTable[8 - 1][src_bBits - 1];

	src16 += srcY * (srcDataStride >> 1) + srcX;
	dst8 += dstY * dstDataStride;

#if (AM_OPENVG_VERSION >= 110)
	if (dstFormat == VG_BW_1) {
#else
	(void)dstFormat;
#endif
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint8 lum, bw;
				AMuint32 r = (AMuint32)((*src16 >> src_rSh) & src_rMask);
				AMuint32 g = (AMuint32)((*src16 >> src_gSh) & src_gMask);
				AMuint32 b = (AMuint32)((*src16 >> src_bSh) & src_bMask);
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 0x07));

				// map r, g, b to full 8bit
				r = (rMap * r) >> 8;
				g = (gMap * g) >> 8;
				b = (bMap * b) >> 8;
				// s --> l conversion (luminance calculation must be done in linear space)
				r = AM_GAMMA_INV_TABLE(r);
				g = AM_GAMMA_INV_TABLE(g);
				b = AM_GAMMA_INV_TABLE(b);
				// luminance calculation
				r *= 13933;	// 13933 = 0.2126 * 65536
				g *= 46871;	// 46871 = 0.7152 * 65536
				b *= 4732;	// 4732 = 0.0722 * 65536
				lum = (AMuint8)((r + g + b) >> 16);

				if (dither)
					bw = (lum > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = lum >> 7;

				src16++;
				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;
			}
			src16 += srcJump;
			dst8 += dstDataStride;
		}
#if (AM_OPENVG_VERSION >= 110)
	}
	else {
		AMuint32 src_aSh = pxlFormatTable[srcIdx][FMT_A_SH];
		AMuint16 src_aBits = pxlFormatTable[srcIdx][FMT_A_BITS];
		AMuint16 src_aMask = (1 << src_aBits) - 1;
		AMuint32 aMap;
		AMuint16 alphaFix;

		AM_ASSERT(dstFormat == VG_A_1);

		if (src_flags & FMT_ALPHA) {
			aMap = amBitConversionTable[8 - 1][src_aBits - 1];
			alphaFix = 0x00;
		}
		else {
			aMap = 0;
			alphaFix = 0xFF;
		}

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint16 a = (*src16++ >> src_aSh) & src_aMask;
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));
				AMuint8 bw;

				bw = (AMuint8)(((aMap * a) >> 8) | alphaFix);

				if (dither && src_aBits != 1)
					bw = (bw > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = bw >> 7;

				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;

			}
			src16 += srcJump;
			dst8 += dstDataStride;
		}
	}
#endif
}

//*************************************************************************************
//                    8 bit to all other formats conversion
//*************************************************************************************

/*!
	\brief Convert a subregion from a source 8bit pixelmap to a destination 32bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 32bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert8To32(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 srcJump = srcDataStride - width;
	AMint32 dstJump = (dstDataStride >> 2) - width;
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 8);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 32);
	AM_ASSERT((dstDataStride & 0x03) == 0);

	src8 += srcY * srcDataStride + srcX;
	dst32 += dstY * (dstDataStride >> 2) + dstX;

	if (srcFormat == VG_A_8) {

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint32 r, g, b;
				AMuint32 a = *src8;

				r = g = b = (dst_flags & FMT_PRE) ? a : 0xFF;
				// write the pixel
				src8++;
				*dst32++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src8 += srcJump;
			dst32 += dstJump;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint32 lum = *src8;

				// l --> s
				if ((src_flags & FMT_L) && !(dst_flags & FMT_L))
					lum = AM_GAMMA_TABLE(lum);
				else
				// s --> l
				if (!(src_flags & FMT_L) && (dst_flags & FMT_L))
					lum = AM_GAMMA_INV_TABLE(lum);
				// write the pixel
				src8++;
				*dst32++ = (lum << dst_rSh) | (lum << dst_gSh) | (lum << dst_bSh) | (0xFF << dst_aSh);
			}
			src8 += srcJump;
			dst32 += dstJump;
		}
	}
}

/*!
	\brief Convert a subregion from a source 8bit pixelmap to a destination 16bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 16bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert8To16(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint16 *dst16 = (AMuint16 *)dstPixels;
	AMint32 srcJump = srcDataStride - width;
	AMint32 dstJump = (dstDataStride >> 1) - width;
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 dst_rBits = pxlFormatTable[dstIdx][FMT_R_BITS];
	AMuint32 dst_gBits = pxlFormatTable[dstIdx][FMT_G_BITS];
	AMuint32 dst_bBits = pxlFormatTable[dstIdx][FMT_B_BITS];
	AMuint32 dst_aBits = pxlFormatTable[dstIdx][FMT_A_BITS];
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 8);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 16);
	AM_ASSERT((dstDataStride & 0x01) == 0);

	src8 += srcY * srcDataStride + srcX;
	dst16 += dstY * (dstDataStride >> 1) + dstX;

	if (srcFormat == VG_A_8) {

		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint16 r, g, b;
				AMuint16 a = *src8;

				// force r, g, b, to maximum value
				r = (1 << dst_rBits) - 1;
				g = (1 << dst_gBits) - 1;
				b = (1 << dst_bBits) - 1;
				// dither alpha, if required
				if (dither) {
					a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> dst_aBits);
					a = AM_MIN(a, 0xFF);
				}
				a >>= (8 - dst_aBits);
				// write the pixel
				src8++;
				*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src8 += srcJump;
			dst16 += dstJump;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = width; x != 0; --x) {

				AMuint16 r, g, b, a;
				AMuint32 lum = *src8;

				// l --> s
				if ((src_flags & FMT_L) && !(dst_flags & FMT_L))
					lum = AM_GAMMA_TABLE(lum);

				r = g = b = lum;
				if (dither) {

					AMuint32 k = amDitherLumTable[y & 0x0F][x & 0x0F];
					
					r += (k >> dst_rBits);
					g += (k >> dst_gBits);
					b += (k >> dst_bBits);
					r = AM_MIN(r, 0xFF);
					g = AM_MIN(g, 0xFF);
					b = AM_MIN(b, 0xFF);
				}

				r >>= (8 - dst_rBits);
				g >>= (8 - dst_gBits);
				b >>= (8 - dst_bBits);
				// force alpha to maximum value
				a = (1 << dst_aBits) - 1;

				// write the pixel
				src8++;
				*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src8 += srcJump;
			dst16 += dstJump;
		}
	}
}

/*!
	\brief Convert a subregion from a source 8bit pixelmap to a destination 8bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert8To8(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 8);

	// in the case of same 8bit format, use raw 8bit copy
	if (srcFormat == dstFormat)
		amPxlMapCopy8(dstPixels, dstDataStride, dstX, dstY, srcPixels, srcDataStride, srcX, srcY, width, height);
	else {
		AMint32 x, y;
		const AMuint8 *src8 = (const AMuint8 *)srcPixels;
		AMuint8 *dst8 = (AMuint8 *)dstPixels;
		AMint32 srcJump = srcDataStride - width;
		AMint32 dstJump = dstDataStride - width;
		AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];

		src8 += srcY * srcDataStride + srcX;
		dst8 += dstY * dstDataStride + dstX;

		if (srcFormat == VG_A_8 || dstFormat == VG_A_8) {
			for (y = height; y != 0; --y) {
				amMemset(dst8, 0xFF, width);
				dst8 += (dstJump + width);
			}
		}
		else {
			for (y = height; y != 0; --y) {
				for (x = width; x != 0; --x) {

					AMuint8 lum = *src8;

					lum = (src_flags & FMT_L) ? AM_GAMMA_TABLE(lum) : AM_GAMMA_INV_TABLE(lum);
					src8++;
					*dst8++ = lum;
				}
				src8 += srcJump;
				dst8 += dstJump;
			}
		}
	}
}

#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Convert a subregion from a source 8bit pixelmap to a destination 4bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert8To4(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height,
						 const AMbool dither) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = srcDataStride - width;

	AM_ASSERT(dstFormat == VG_A_4);
	(void)dstFormat;

	src8 += srcY * srcDataStride + srcX;
	dst8 += dstY * dstDataStride;

	if (srcFormat == VG_A_8) {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint32 a = (AMuint32)(*src8++);
				AMuint32 i = x + dstX;
				AMuint8 dst = dst8[i >> 1];
				AMuint8 a8;

				if (dither) {
					a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> 4);
					a = AM_MIN(a, 0xFF);
				}
				a8 = am8To4BitTable[a];
				// write the pixel
				dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | (a8 << 4) : (dst & 0xF0) | (a8);
			}
			src8 += srcJump;
			dst8 += dstDataStride;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint32 i = x + dstX;
				AMuint8 dst = dst8[i >> 1];

				// write the pixel
				dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | 0xF0 : (dst & 0xF0) | 0x0F;
			}
			dst8 += dstDataStride;
		}
	}
}
#endif

/*!
	\brief Convert a subregion from a source 8bit pixelmap to a destination 1bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert8To1(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height,
						 const AMbool dither) {

	AMint32 x, y;
	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 srcJump = srcDataStride - width;
	AMuint32 src_flags = pxlFormatTable[srcIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == 8);

	src8 += srcY * srcDataStride + srcX;
	dst8 += dstY * dstDataStride;

	if ((srcFormat == VG_A_8 && dstFormat == VG_BW_1)
	#if (AM_OPENVG_VERSION >= 110)
		|| (srcFormat != VG_A_8 && dstFormat == VG_A_1)
	#endif
		) {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				dst8[i >> 3] |= mask;
			}
			src8 += srcJump;
			dst8 += dstDataStride;
		}
	}
	else
	if (srcFormat != VG_A_8) {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint8 bw;
				AMuint8 lum = *src8;
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				// s --> l conversion
				if (!(src_flags & FMT_L))
					lum = AM_GAMMA_INV_TABLE(lum);

				if (dither)
					bw = (lum > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = lum >> 7;

				src8++;
				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;
			}
			src8 += srcJump;
			dst8 += dstDataStride;
		}
	}
#if (AM_OPENVG_VERSION >= 110)
	else {
		AM_ASSERT(srcFormat == VG_A_8);
		AM_ASSERT(dstFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint8 bw;
				AMuint8 a = *src8;
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				if (dither)
					bw = (a > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = a >> 7;

				src8++;
				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;
			}
			src8 += srcJump;
			dst8 += dstDataStride;
		}
	}
#endif
}

//*************************************************************************************
//                    4 bit to all other formats conversion
//*************************************************************************************
#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Convert a subregion from a source 4bit pixelmap to a destination 32bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 32bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert4To32(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 dstJump = (dstDataStride >> 2) - width;
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];

	AM_ASSERT(srcFormat == VG_A_4);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 32);
	AM_ASSERT((dstDataStride & 0x03) == 0);
	(void)srcFormat;

	src8 += srcY * srcDataStride;
	dst32 += dstY * (dstDataStride >> 2) + dstX;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {

			AMuint32 r, g, b;
			AMuint32 i = x + srcX;
			AMuint32 a = (i & 1) ? (src8[i >> 1] >> 4) * 17 : (src8[i >> 1] & 0x0F) * 17;

			r = g = b = (dst_flags & FMT_PRE) ? a : 0xFF;
			// write the pixel
			*dst32++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
		}
		src8 += srcDataStride;
		dst32 += dstJump;
	}
}

/*!
	\brief Convert a subregion from a source 4bit pixelmap to a destination 16bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 16bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert4To16(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height,
						  const AMbool dither) {

	AMint32 x, y;
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint16 *dst16 = (AMuint16 *)dstPixels;
	AMint32 dstJump = (dstDataStride >> 1) - width;
	AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 dst_rBits = pxlFormatTable[dstIdx][FMT_R_BITS];
	AMuint32 dst_gBits = pxlFormatTable[dstIdx][FMT_G_BITS];
	AMuint32 dst_bBits = pxlFormatTable[dstIdx][FMT_B_BITS];
	AMuint32 dst_aBits = pxlFormatTable[dstIdx][FMT_A_BITS];

	AM_ASSERT(srcFormat == VG_A_4);
	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 16);
	AM_ASSERT((dstDataStride & 0x01) == 0);
	(void)srcFormat;

	src8 += srcY * srcDataStride;
	dst16 += dstY * (dstDataStride >> 1) + dstX;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {

			AMuint16 r, g, b;
			AMuint32 i = x + srcX;
			AMuint16 a = (i & 1) ? (src8[i >> 1] >> 4) * 17 : (src8[i >> 1] & 0x0F) * 17;

			// force r, g, b, to maximum value
			r = (1 << dst_rBits) - 1;
			g = (1 << dst_gBits) - 1;
			b = (1 << dst_bBits) - 1;
			// dither alpha, if required
			if (dither) {
				a += (amDitherLumTable[y & 0x0F][x & 0x0F] >> dst_aBits);
				a = AM_MIN(a, 0xFF);
			}
			a >>= (8 - dst_aBits);
			// write the pixel
			*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
		}
		src8 += srcDataStride;
		dst16 += dstJump;
	}
}

/*!
	\brief Convert a subregion from a source 4bit pixelmap to a destination 8bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert4To8(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 dstJump = dstDataStride - width;

	AM_ASSERT(srcFormat == VG_A_4);
	(void)srcFormat;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride + dstX;

	if (dstFormat == VG_A_8) {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint32 i = x + srcX;
				AMuint8 a = (i & 1) ? (src8[i >> 1] >> 4) * 17 : (src8[i >> 1] & 0x0F) * 17;

				*dst8++ = a;
			}
			src8 += srcDataStride;
			dst8 += dstJump;
		}
	}
	else {
		AM_ASSERT(dstFormat == VG_sL_8 || dstFormat == VG_lL_8);
		
		for (y = height; y != 0; --y) {
			amMemset(dst8, 0xFF, width);
			dst8 += (dstJump + width);
		}
	}
}

/*!
	\brief Convert a subregion from a source 4bit pixelmap to a destination 4bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert4To4(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;

	AM_ASSERT(srcFormat == VG_A_4);
	AM_ASSERT(dstFormat == VG_A_4);
	(void)srcFormat;
	(void)dstFormat;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride;

	for (y = height; y != 0; --y) {
		for (x = 0; x < width; ++x) {

			AMuint32 i = x + srcX;
			AMuint8 a8 = (i & 1) ? (src8[i >> 1] >> 4) : (src8[i >> 1] & 0x0F);
			AMuint8 dst;

			i = x + dstX;
			dst = dst8[i >> 1];
			// write the pixel
			dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | (a8 << 4) : (dst & 0xF0) | a8;
		}
		src8 += srcDataStride;
		dst8 += dstDataStride;
	}
}

/*!
	\brief Convert a subregion from a source 4bit pixelmap to a destination 1bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert4To1(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height,
						 const AMbool dither) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;

	AM_ASSERT(srcFormat == VG_A_4);
	AM_ASSERT(dstFormat == VG_A_1 || dstFormat == VG_BW_1);
	(void)srcFormat;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride;

	if (dstFormat == VG_BW_1) {

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				dst8[i >> 3] |= mask;
			}
			dst8 += dstDataStride;
		}
	}
	else {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint32 i = x + srcX;
				AMuint8 a = (i & 1) ? (src8[i >> 1] >> 4) * 17 : (src8[i >> 1] & 0x0F) * 17;
				AMuint8 bw, mask;

				if (dither)
					bw = (a > amDitherLumTable[y & 0x0F][x & 0x0F]) ? 1 : 0;
				else
					bw = a >> 7;
				i = x + dstX;
				mask = 1 << (i & 7);
				// write the pixel
				if (bw == 0)
					dst8[i >> 3] &= mask ^ 0xFF;
				else
					dst8[i >> 3] |= mask;
			}
			src8 += srcDataStride;
			dst8 += dstDataStride;
		}
	}
}
#endif

//*************************************************************************************
//                    1 bit to all other formats conversion
//*************************************************************************************

/*!
	\brief Convert a subregion from a source 1bit pixelmap to a destination 32bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 32bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert1To32(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint32 *dst32 = (AMuint32 *)dstPixels;
	AMint32 dstJump = (dstDataStride >> 2) - width;
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint32 black = 0xFF << dst_aSh;

	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 32);
	AM_ASSERT((dstDataStride & 0x03) == 0);
	
	src8 += srcY * srcDataStride;
	dst32 += dstY * (dstDataStride >> 2) + dstX;

#if (AM_OPENVG_VERSION >= 110)
	if (srcFormat == VG_BW_1) {
#endif
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 0x07);
				AMuint8 bw = (src8[i >> 3] >> j) & 0x01;
				// write it to destination
				*dst32++ = (bw) ? 0xFFFFFFFF : black;
			}
			src8 += srcDataStride;
			dst32 += dstJump;
		}
#if (AM_OPENVG_VERSION >= 110)
	}
	else {
		AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
		AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
		AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
		AMuint32 dst_flags = pxlFormatTable[dstIdx][FMT_FLAGS];

		AM_ASSERT(srcFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 0x07);
				AMuint32 a = ((src8[i >> 3] >> j) & 0x01) * 255;
				AMuint32 r, g, b;

				r = g = b = (dst_flags & FMT_PRE) ? a : 0xFF;
				// write the pixel
				*dst32++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src8 += srcDataStride;
			dst32 += dstJump;
		}
	}
#endif
}

/*!
	\brief Convert a subregion from a source 1bit pixelmap to a destination 16bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 16bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert1To16(void *dstPixels,
						  const VGImageFormat dstFormat,
						  const AMint32 dstDataStride,
						  const AMint32 dstX,
						  const AMint32 dstY,
						  const void *srcPixels,
						  const VGImageFormat srcFormat,
						  const AMint32 srcDataStride,
						  const AMint32 srcX,
						  const AMint32 srcY,
						  const AMint32 width,
						  const AMint32 height) {

	AMint32 x, y;
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint16 *dst16 = (AMuint16 *)dstPixels;
	AMint32 dstJump = (dstDataStride >> 1) - width;
	AMuint32 dst_aSh = pxlFormatTable[dstIdx][FMT_A_SH];
	AMuint16 black = ((1 << pxlFormatTable[dstIdx][FMT_A_BITS]) - 1) << dst_aSh;

	AM_ASSERT(pxlFormatTable[dstIdx][FMT_BITS] == 16);
	AM_ASSERT((dstDataStride & 0x01) == 0);

	src8 += srcY * srcDataStride;
	dst16 += dstY * (dstDataStride >> 1) + dstX;

#if (AM_OPENVG_VERSION >= 110)
	if (srcFormat == VG_BW_1) {
#endif
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 7);
				AMuint8 bw = (src8[i >> 3] >> j) & 1;
				// write it to destination
				*dst16++ = (bw) ? 0xFFFF : black;
			}
			src8 += srcDataStride;
			dst16 += dstJump;
		}
#if (AM_OPENVG_VERSION >= 110)
	}
	else {
		AMuint32 dst_rSh = pxlFormatTable[dstIdx][FMT_R_SH];
		AMuint32 dst_gSh = pxlFormatTable[dstIdx][FMT_G_SH];
		AMuint32 dst_bSh = pxlFormatTable[dstIdx][FMT_B_SH];
		AMuint32 dst_rBits = pxlFormatTable[dstIdx][FMT_R_BITS];
		AMuint32 dst_gBits = pxlFormatTable[dstIdx][FMT_G_BITS];
		AMuint32 dst_bBits = pxlFormatTable[dstIdx][FMT_B_BITS];
		AMuint32 dst_aBits = pxlFormatTable[dstIdx][FMT_A_BITS];
		AMuint16 aMap = (1 << dst_aBits) - 1;

		AM_ASSERT(srcFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 7);
				AMuint16 a = ((src8[i >> 3] >> j) & 1) * aMap;
				// force r, g, b, to maximum value
				AMuint16 r = (1 << dst_rBits) - 1;
				AMuint16 g = (1 << dst_gBits) - 1;
				AMuint16 b = (1 << dst_bBits) - 1;

				// write the pixel
				*dst16++ = (r << dst_rSh) | (g << dst_gSh) | (b << dst_bSh) | (a << dst_aSh);
			}
			src8 += srcDataStride;
			dst16 += dstJump;
		}
	}
#endif
}

/*!
	\brief Convert a subregion from a source 1bit pixelmap to a destination 8bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert1To8(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	AMint32 x, y;
	AMuint8 black;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;
	AMint32 dstJump = dstDataStride  - width;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride + dstX;

	// in the case of VG_A_8, we have to write always 255, for VG_sL_8 and VG_lL_8 formats black is 0
	black = (dstFormat == VG_A_8) ? 0xFF : 0x00;

#if (AM_OPENVG_VERSION >= 110)
	if (srcFormat == VG_BW_1) {
#endif
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 0x07);
				AMuint8 bw = (src8[i >> 0x03] >> j) & 0x01;

				// write it to destination
				*dst8++ = (bw) ? 0xFF : black;
			}
			src8 += srcDataStride;
			dst8 += dstJump;
		}
#if (AM_OPENVG_VERSION >= 110)
	}
	else {
		AMuint8 alphaFix = (dstFormat == VG_A_8) ? 0x00 : 0xFF;

		AM_ASSERT(srcFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read BW bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 0x07);

				*dst8++ = (((src8[i >> 0x03] >> j) & 0x01) * 255) | alphaFix;
			}
			src8 += srcDataStride;
			dst8 += dstJump;
		}
	}
#endif
}

#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Convert a subregion from a source 1bit pixelmap to a destination 4bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert1To4(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	AMint32 x, y;
	const AMuint8 *src8 = (const AMuint8 *)srcPixels;
	AMuint8 *dst8 = (AMuint8 *)dstPixels;

	AM_ASSERT(dstFormat == VG_A_4);
	(void)dstFormat;

	src8 += srcY * srcDataStride;
	dst8 += dstY * dstDataStride;

	if (srcFormat == VG_BW_1) {
		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {

				AMuint32 i = x + dstX;
				AMuint8 dst = dst8[i >> 1];

				// write the pixel
				dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | 0xF0 : (dst & 0xF0) | 0x0F;
			}
			dst8 += dstDataStride;
		}
	}
	else {
		AM_ASSERT(srcFormat == VG_A_1);

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// read 1 bit
				AMuint32 i = x + srcX;
				AMuint8 j = (AMuint8)(i & 7);
				AMuint8 bw = (src8[i >> 3] >> j) & 1;
				AMuint8 dst;

				i = x + dstX;
				dst = dst8[i >> 1];
				// write the pixel
				if (bw)
					dst8[i >> 1] = (i & 1) ? (dst & 0x0F) | 0xF0 : (dst & 0xF0) | 0x0F;
				else
					dst8[i >> 1] = (i & 1) ? (dst & 0x0F) : (dst & 0xF0);
			}
			src8 += srcDataStride;
			dst8 += dstDataStride;
		}
	}
}
#endif

/*!
	\brief Convert a subregion from a source 1bit pixelmap to a destination 1bit pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\pre dstPixels must point to valid 8bit aligned memory region, srcPixels must point to valid 8bit aligned memory region.
*/
void amPxlMapConvert1To1(void *dstPixels,
						 const VGImageFormat dstFormat,
						 const AMint32 dstDataStride,
						 const AMint32 dstX,
						 const AMint32 dstY,
						 const void *srcPixels,
						 const VGImageFormat srcFormat,
						 const AMint32 srcDataStride,
						 const AMint32 srcX,
						 const AMint32 srcY,
						 const AMint32 width,
						 const AMint32 height) {

	if (srcFormat == dstFormat)
		amPxlMapCopy1(dstPixels, dstDataStride, dstX, dstY, srcPixels, srcDataStride, srcX, srcY, width, height);
	else {
		AMint32 x, y;
		const AMuint8 *src8 = (const AMuint8 *)srcPixels;
		AMuint8 *dst8 = (AMuint8 *)dstPixels;

		src8 += srcY * srcDataStride;
		dst8 += dstY * dstDataStride;

		for (y = height; y != 0; --y) {
			for (x = 0; x < width; ++x) {
				// always set the destination bit to 1
				AMuint32 i = x + dstX;
				AMuint8 mask = 1 << ((AMuint8)(i & 7));

				dst8[i >> 3] |= mask;
			}
			src8 += srcDataStride;
			dst8 += dstDataStride;
		}
	}
}

//*************************************************************************************
//                  Conversion between two images (high level function)
//*************************************************************************************
/*!
	\brief Convert a subregion from a source pixelmap to a destination pixelmap.
	\param dstPixels pointer to the destination pixels.
	\param dstFormat pixel format of the destination pixels.
	\param dstDataStride destination data stride.
	\param dstX x-coordinate of the first pixel to write.
	\param dstY y-coordinate of the first pixel to write.
	\param srcPixels pointer to the source pixels.
	\param srcFormat pixel format of the source pixels.
	\param srcDataStride source data stride.
	\param srcX x-coordinate of the first pixel to read.
	\param srcY y-coordinate of the first pixel to read.
	\param width width of the subregion to copy, in pixels.
	\param height height of the subregion to copy, in pixels.
	\param dither AM_TRUE to convert using dithering, else AM_FALSE.
	\param consistentSrc AM_TRUE if source pixels are considered consistent (i.e. valid premultiplied rgba values for format with alpha channel, X channel = 255
	for format without alpha), else AM_FALSE.
*/
void amPxlMapConvert(void *dstPixels,
					 const VGImageFormat dstFormat,
					 const AMint32 dstDataStride,
					 const AMint32 dstX,
					 const AMint32 dstY,
					 const void *srcPixels,
					 const VGImageFormat srcFormat,
					 const AMint32 srcDataStride,
					 const AMint32 srcX,
					 const AMint32 srcY,
					 const AMint32 width,
					 const AMint32 height,
					 const AMbool dither,
					 const AMbool consistentSrc) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(srcFormat);
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dstFormat);

    AM_ASSERT(srcPixels);
	AM_ASSERT(dstPixels);
	AM_ASSERT(dstX >= 0 && dstY >= 0 && srcX >= 0 && srcY >= 0 && width > 0 && height > 0);
	AM_ASSERT(amImageFormatValid(dstFormat));
	AM_ASSERT(amImageFormatValid(srcFormat));
#if defined(_DEBUG)
	if (srcPixels == dstPixels) {
		// if source and destination pixels are the same, format bits must correspond!
		AM_ASSERT(pxlFormatTable[srcIdx][FMT_BITS] == pxlFormatTable[dstIdx][FMT_BITS]);
	}
#endif

	switch (pxlFormatTable[srcIdx][FMT_BITS]) {
		case 32:
			switch (pxlFormatTable[dstIdx][FMT_BITS]) {
				case 32:
					amPxlMapConvert32To32(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, consistentSrc);
					break;
				case 16:
					amPxlMapConvert32To16(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				case 8:
					amPxlMapConvert32To8(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					amPxlMapConvert32To4(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
			#endif
				case 1:
					amPxlMapConvert32To1(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
		case 16:
			switch (pxlFormatTable[dstIdx][FMT_BITS]) {
				case 32:
					amPxlMapConvert16To32(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 16:
					amPxlMapConvert16To16(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				case 8:
					amPxlMapConvert16To8(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					amPxlMapConvert16To4(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#endif
				case 1:
					amPxlMapConvert16To1(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
		case 8:
			switch (pxlFormatTable[dstIdx][FMT_BITS]) {
				case 32:
					amPxlMapConvert8To32(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 16:
					amPxlMapConvert8To16(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				case 8:
					amPxlMapConvert8To8(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					amPxlMapConvert8To4(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
			#endif
				case 1:
					amPxlMapConvert8To1(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case 4:
			switch (pxlFormatTable[dstIdx][FMT_BITS]) {
				case 32:
					amPxlMapConvert4To32(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 16:
					amPxlMapConvert4To16(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				case 8:
					amPxlMapConvert4To8(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 4:
					amPxlMapConvert4To4(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 1:
					amPxlMapConvert4To1(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height, dither);
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
	#endif
		case 1:
			switch (pxlFormatTable[dstIdx][FMT_BITS]) {
				case 32:
					amPxlMapConvert1To32(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 16:
					amPxlMapConvert1To16(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				case 8:
					amPxlMapConvert1To8(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#if (AM_OPENVG_VERSION >= 110)
				case 4:
					amPxlMapConvert1To4(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
			#endif
				case 1:
					amPxlMapConvert1To1(dstPixels, dstFormat, dstDataStride, dstX, dstY, srcPixels, srcFormat, srcDataStride, srcX, srcY, width, height);
					break;
				default:
					AM_ASSERT(0 == 1);
					break;
			}
			break;
		default:
			AM_ASSERT(0 == 1);
			break;
	}
}

#if defined (RIM_VG_SRC)
#pragma pop 
#endif

