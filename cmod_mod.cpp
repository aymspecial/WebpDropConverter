// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
//  simple command line calling the WebPEncode function.
//  Encodes a raw .YUV into WebP bitstream
//
// Author: Skal (pascal.massimino@gmail.com)
// modifier: Aymspecial
#include "pch.h"
#include "webp_mod.h"

#define __STDC_LIB_EXT1__
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#ifndef WEBP_DLL
#ifdef __cplusplus
extern "C" {
#endif

	extern void* VP8GetCPUInfo;   // opaque forward declaration.

#ifdef __cplusplus
}    // extern "C"
#endif
#endif  // WEBP_DLL

//------------------------------------------------------------------------------

static int verbose = 0;











//----------------------------------------------------------------------------
static void HelpShort() {}
static void HelpLong() {}

//------------------------------------------------------------------------------
#ifdef FREE_WARGV_AND_RETURN
#undef FREE_WARGV_AND_RETURN
#define FREE_WARGV_AND_RETURN( a ) return( a )
#endif
#ifdef GET_WARGV
#undef GET_WARGV
#define GET_WARGV( a, b ) a[ b ]
#endif

enum class OutFormat
{
	PNG = 0,
	BMP,
	JPEG,
};

int cmodmod_main( int argc, char* argv[] )
{
	int return_value = false;
	const char* in_file = NULL, * out_file = NULL;
	int short_output = 0;
	int quiet = 0;
	int keep_alpha = 1;
	int blend_alpha = 0;
	uint32_t background_color = 0xffffffu;
	int crop = 0, crop_x = 0, crop_y = 0, crop_w = 0, crop_h = 0;
	int resize_w = 0, resize_h = 0;
	int lossless_preset = 6;
	int use_lossless_preset = -1;  // -1=unset, 0=don't use, 1=use it
	int show_progress = 0;
	int keep_metadata = 0;
	int metadata_written = 0;
	int print_distortion = -1;        // -1=off, 0=PSNR, 1=SSIM, 2=LSIM
	OutFormat format = OutFormat::BMP;
	int jpgQuality = -1;

	if( argc == 1 )
	{
		HelpShort();
		FREE_WARGV_AND_RETURN( 0 );
	}

	for( int c = 1; c < argc; ++c )
	{
		int parse_error = 0;
		if( !strcmp( argv[ c ], "-bmp" ) )
		{
			format = OutFormat::BMP;
		}
		else if( !strcmp( argv[ c ], "-jpeg" ) )
		{
			format = OutFormat::JPEG;
		}
		else if( !strcmp( argv[ c ], "-png" ) )
		{
			format = OutFormat::PNG;
		}
		else if( !strcmp( argv[ c ], "-o" ) && c < argc - 1 )
		{
			out_file = (const char*)GET_WARGV( argv, ++c );
		}
		else if( !strcmp( argv[ c ], "-q" ) && c < argc - 1 )
		{
			jpgQuality = atoi( argv[ ++c ] );
		}
		else
		{
			in_file = (const char*)GET_WARGV( argv, c );
		}
	}

	if( in_file == NULL )
	{
		fprintf( stderr, "No input file specified!\n" );
		HelpShort();
		goto Error;
	}

	int width, height, channels;

	// ‰æ‘œƒf[ƒ^‚ð“Ç‚Ýž‚Þ
	unsigned char* image_data = stbi_load( in_file, &width, &height, &channels, 0 );
	if( image_data == nullptr )
	{
		WFPRINTF( stderr, "Error! Cannot read input picture file '%s'\n",
				  (const W_CHAR*)in_file );
		goto Error;
	}

	switch( format )
	{
		case OutFormat::JPEG:
			// Jpeg ‚Æ‚µ‚Ä•Û‘¶
			stbi_write_jpg( out_file, width, height, channels, image_data, jpgQuality );
			break;
		case OutFormat::PNG:
			// PNG ‚Æ‚µ‚Ä•Û‘¶
			stbi_write_png( out_file, width, height, channels, image_data, width * 3 );
			break;
		case OutFormat::BMP:
			// Bmp ‚Æ‚µ‚Ä•Û‘¶
			stbi_write_bmp( out_file, width, height, channels, image_data );
			break;
		default:
			;
	}
	// ƒƒ‚ƒŠ‚ð‰ð•ú
	stbi_image_free( image_data );

Error:
	return return_value;
}

//------------------------------------------------------------------------------
