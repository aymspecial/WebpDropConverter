// Copyright 2016 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Save image
#include "pch.h"
#include <setjmp.h>   // note: this must be included *after* png.h

#define WEBP_HAVE_PNG
#include <png.h>

#include "./image_enc.h"

#pragma comment(lib, "libpng16_static.lib")
#pragma comment(lib, "zlibstatic.lib")

#include "./imageio_util.h"
#include "../examples/unicode.h"

//------------------------------------------------------------------------------
// PNG

static void PNGAPI PNGErrorFunction( png_structp png, png_const_charp unused )
{
	(void)unused;  // remove variable-unused warning
	longjmp( png_jmpbuf( png ), 1 );
}

int WebPWritePNG( FILE* out_file, const WebPDecBuffer* const buffer )
{
	volatile png_structp png;
	volatile png_infop info;

	if( out_file == NULL || buffer == NULL ) return 0;

	png = png_create_write_struct( PNG_LIBPNG_VER_STRING,
								   NULL, PNGErrorFunction, NULL );
	if( png == NULL )
	{
		return 0;
	}
	info = png_create_info_struct( png );
	if( info == NULL )
	{
		png_destroy_write_struct( (png_structpp)&png, NULL );
		return 0;
	}
	if( setjmp( png_jmpbuf( png ) ) )
	{
		png_destroy_write_struct( (png_structpp)&png, (png_infopp)&info );
		return 0;
	}
	png_init_io( png, out_file );
	{
		const uint32_t width = buffer->width;
		const uint32_t height = buffer->height;
		png_bytep row = buffer->u.RGBA.rgba;
		const int stride = buffer->u.RGBA.stride;
		const int has_alpha = WebPIsAlphaMode( buffer->colorspace );
		uint32_t y;

		png_set_IHDR( png, info, width, height, 8,
					  has_alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
					  PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
					  PNG_FILTER_TYPE_DEFAULT );
		png_write_info( png, info );
		for( y = 0; y < height; ++y )
		{
			png_write_rows( png, &row, 1 );
			row += stride;
		}
	}
	png_write_end( png, info );
	png_destroy_write_struct( (png_structpp)&png, (png_infopp)&info );
	return 1;
}

//------------------------------------------------------------------------------
// PPM / PAM

static int WritePPMPAM( FILE* fout, const WebPDecBuffer* const buffer,
						int alpha )
{
	if( fout == NULL || buffer == NULL )
	{
		return 0;
	}
	else
	{
		const uint32_t width = buffer->width;
		const uint32_t height = buffer->height;
		const uint8_t* row = buffer->u.RGBA.rgba;
		const int stride = buffer->u.RGBA.stride;
		const size_t bytes_per_px = alpha ? 4 : 3;
		uint32_t y;

		if( row == NULL ) return 0;

		if( alpha )
		{
			fprintf( fout, "P7\nWIDTH %u\nHEIGHT %u\nDEPTH 4\nMAXVAL 255\n"
					 "TUPLTYPE RGB_ALPHA\nENDHDR\n", width, height );
		}
		else
		{
			fprintf( fout, "P6\n%u %u\n255\n", width, height );
		}
		for( y = 0; y < height; ++y )
		{
			if( fwrite( row, width, bytes_per_px, fout ) != bytes_per_px )
			{
				return 0;
			}
			row += stride;
		}
	}
	return 1;
}

int WebPWritePPM( FILE* fout, const WebPDecBuffer* const buffer )
{
	return WritePPMPAM( fout, buffer, 0 );
}

int WebPWritePAM( FILE* fout, const WebPDecBuffer* const buffer )
{
	return WritePPMPAM( fout, buffer, 1 );
}

//------------------------------------------------------------------------------
// Raw PGM

// Save 16b mode (RGBA4444, RGB565, ...) for debugging purpose.
int WebPWrite16bAsPGM( FILE* fout, const WebPDecBuffer* const buffer )
{
	const uint32_t width = buffer->width;
	const uint32_t height = buffer->height;
	const uint8_t* rgba = buffer->u.RGBA.rgba;
	const int stride = buffer->u.RGBA.stride;
	const uint32_t bytes_per_px = 2;
	uint32_t y;

	if( fout == NULL || buffer == NULL || rgba == NULL ) return 0;

	fprintf( fout, "P5\n%u %u\n255\n", width * bytes_per_px, height );
	for( y = 0; y < height; ++y )
	{
		if( fwrite( rgba, width, bytes_per_px, fout ) != bytes_per_px )
		{
			return 0;
		}
		rgba += stride;
	}
	return 1;
}

//------------------------------------------------------------------------------
// BMP

static void PutLE16( uint8_t* const dst, uint32_t value )
{
	dst[ 0 ] = ( value >> 0 ) & 0xff;
	dst[ 1 ] = ( value >> 8 ) & 0xff;
}

static void PutLE32( uint8_t* const dst, uint32_t value )
{
	PutLE16( dst + 0, ( value >> 0 ) & 0xffff );
	PutLE16( dst + 2, ( value >> 16 ) & 0xffff );
}

#define BMP_HEADER_SIZE 54
int WebPWriteBMP( FILE* fout, const WebPDecBuffer* const buffer )
{
	const int has_alpha = WebPIsAlphaMode( buffer->colorspace );
	const uint32_t width = buffer->width;
	const uint32_t height = buffer->height;
	const uint8_t* rgba = buffer->u.RGBA.rgba;
	const int stride = buffer->u.RGBA.stride;
	const uint32_t bytes_per_px = has_alpha ? 4 : 3;
	uint32_t y;
	const uint32_t line_size = bytes_per_px * width;
	const uint32_t bmp_stride = ( line_size + 3 ) & ~3;   // pad to 4
	const uint32_t total_size = bmp_stride * height + BMP_HEADER_SIZE;
	uint8_t bmp_header[ BMP_HEADER_SIZE ] = { 0 };

	if( fout == NULL || buffer == NULL || rgba == NULL ) return 0;

	// bitmap file header
	PutLE16( bmp_header + 0, 0x4d42 );                // signature 'BM'
	PutLE32( bmp_header + 2, total_size );            // size including header
	PutLE32( bmp_header + 6, 0 );                     // reserved
	PutLE32( bmp_header + 10, BMP_HEADER_SIZE );      // offset to pixel array
	// bitmap info header
	PutLE32( bmp_header + 14, 40 );                   // DIB header size
	PutLE32( bmp_header + 18, width );                // dimensions
	PutLE32( bmp_header + 22, -(int)height );         // vertical flip!
	PutLE16( bmp_header + 26, 1 );                    // number of planes
	PutLE16( bmp_header + 28, bytes_per_px * 8 );     // bits per pixel
	PutLE32( bmp_header + 30, 0 );                    // no compression (BI_RGB)
	PutLE32( bmp_header + 34, 0 );                    // image size (placeholder)
	PutLE32( bmp_header + 38, 2400 );                 // x pixels/meter
	PutLE32( bmp_header + 42, 2400 );                 // y pixels/meter
	PutLE32( bmp_header + 46, 0 );                    // number of palette colors
	PutLE32( bmp_header + 50, 0 );                    // important color count

	// TODO(skal): color profile

	// write header
	if( fwrite( bmp_header, sizeof( bmp_header ), 1, fout ) != 1 )
	{
		return 0;
	}

	// write pixel array
	for( y = 0; y < height; ++y )
	{
		if( fwrite( rgba, line_size, 1, fout ) != 1 )
		{
			return 0;
		}
		// write padding zeroes
		if( bmp_stride != line_size )
		{
			const uint8_t zeroes[ 3 ] = { 0 };
			if( fwrite( zeroes, bmp_stride - line_size, 1, fout ) != 1 )
			{
				return 0;
			}
		}
		rgba += stride;
	}
	return 1;
}
#undef BMP_HEADER_SIZE

//------------------------------------------------------------------------------
// TIFF

#define NUM_IFD_ENTRIES 15
#define EXTRA_DATA_SIZE 16
// 10b for signature/header + n * 12b entries + 4b for IFD terminator:
#define EXTRA_DATA_OFFSET (10 + 12 * NUM_IFD_ENTRIES + 4)
#define TIFF_HEADER_SIZE (EXTRA_DATA_OFFSET + EXTRA_DATA_SIZE)

int WebPWriteTIFF( FILE* fout, const WebPDecBuffer* const buffer )
{
	const int has_alpha = WebPIsAlphaMode( buffer->colorspace );
	const uint32_t width = buffer->width;
	const uint32_t height = buffer->height;
	const uint8_t* rgba = buffer->u.RGBA.rgba;
	const int stride = buffer->u.RGBA.stride;
	const uint8_t bytes_per_px = has_alpha ? 4 : 3;
	const uint8_t assoc_alpha =
		WebPIsPremultipliedMode( buffer->colorspace ) ? 1 : 2;
	// For non-alpha case, we omit tag 0x152 (ExtraSamples).
	const uint8_t num_ifd_entries = has_alpha ? NUM_IFD_ENTRIES
		: NUM_IFD_ENTRIES - 1;
	uint8_t tiff_header[ TIFF_HEADER_SIZE ] = {
	  0x49, 0x49, 0x2a, 0x00,   // little endian signature
	  8, 0, 0, 0,               // offset to the unique IFD that follows
	  // IFD (offset = 8). Entries must be written in increasing tag order.
	  num_ifd_entries, 0,       // Number of entries in the IFD (12 bytes each).
	  0x00, 0x01, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0,    //  10: Width  (TBD)
	  0x01, 0x01, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0,    //  22: Height (TBD)
	  0x02, 0x01, 3, 0, bytes_per_px, 0, 0, 0,     //  34: BitsPerSample: 8888
		  EXTRA_DATA_OFFSET + 0, 0, 0, 0,
	  0x03, 0x01, 3, 0, 1, 0, 0, 0, 1, 0, 0, 0,    //  46: Compression: none
	  0x06, 0x01, 3, 0, 1, 0, 0, 0, 2, 0, 0, 0,    //  58: Photometric: RGB
	  0x11, 0x01, 4, 0, 1, 0, 0, 0,                //  70: Strips offset:
		  TIFF_HEADER_SIZE, 0, 0, 0,               //      data follows header
	  0x12, 0x01, 3, 0, 1, 0, 0, 0, 1, 0, 0, 0,    //  82: Orientation: topleft
	  0x15, 0x01, 3, 0, 1, 0, 0, 0,                //  94: SamplesPerPixels
		  bytes_per_px, 0, 0, 0,
	  0x16, 0x01, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0,    // 106: Rows per strip (TBD)
	  0x17, 0x01, 4, 0, 1, 0, 0, 0, 0, 0, 0, 0,    // 118: StripByteCount (TBD)
	  0x1a, 0x01, 5, 0, 1, 0, 0, 0,                // 130: X-resolution
		  EXTRA_DATA_OFFSET + 8, 0, 0, 0,
	  0x1b, 0x01, 5, 0, 1, 0, 0, 0,                // 142: Y-resolution
		  EXTRA_DATA_OFFSET + 8, 0, 0, 0,
	  0x1c, 0x01, 3, 0, 1, 0, 0, 0, 1, 0, 0, 0,    // 154: PlanarConfiguration
	  0x28, 0x01, 3, 0, 1, 0, 0, 0, 2, 0, 0, 0,    // 166: ResolutionUnit (inch)
	  0x52, 0x01, 3, 0, 1, 0, 0, 0,
		  assoc_alpha, 0, 0, 0,                    // 178: ExtraSamples: rgbA/RGBA
	  0, 0, 0, 0,                                  // 190: IFD terminator
	  // EXTRA_DATA_OFFSET:
	  8, 0, 8, 0, 8, 0, 8, 0,      // BitsPerSample
	  72, 0, 0, 0, 1, 0, 0, 0      // 72 pixels/inch, for X/Y-resolution
	};
	uint32_t y;

	if( fout == NULL || buffer == NULL || rgba == NULL ) return 0;

	// Fill placeholders in IFD:
	PutLE32( tiff_header + 10 + 8, width );
	PutLE32( tiff_header + 22 + 8, height );
	PutLE32( tiff_header + 106 + 8, height );
	PutLE32( tiff_header + 118 + 8, width * bytes_per_px * height );
	if( !has_alpha ) PutLE32( tiff_header + 178, 0 );  // IFD terminator

	// write header
	if( fwrite( tiff_header, sizeof( tiff_header ), 1, fout ) != 1 )
	{
		return 0;
	}
	// write pixel values
	for( y = 0; y < height; ++y )
	{
		if( fwrite( rgba, bytes_per_px, width, fout ) != width )
		{
			return 0;
		}
		rgba += stride;
	}

	return 1;
}

#undef TIFF_HEADER_SIZE
#undef EXTRA_DATA_OFFSET
#undef EXTRA_DATA_SIZE
#undef NUM_IFD_ENTRIES

//------------------------------------------------------------------------------
// JPEG
#include "jpeglib.h"
#pragma warning( disable : 26451 6386 6011 )

int WebPWriteJPEG( FILE* fout, const WebPDecBuffer* const buffer, int jpgQuality )
{
	const uint32_t image_width = buffer->width;
	const uint32_t image_height = buffer->height;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	uint8_t* image_buffer = (uint8_t*)malloc( image_width * image_height * 3 );
	
	for( uint32_t iy = 0; iy < image_height; iy++ )
	{
		for( uint32_t ix = 0; ix < image_width; ix++ )
		{
			int ip = ( image_width * iy + ix ) * 4;
			int op = ( image_width * iy + ix ) * 3;

			image_buffer[ op + 0 ] = buffer->u.RGBA.rgba[ ip + 0 ];
			image_buffer[ op + 1 ] = buffer->u.RGBA.rgba[ ip + 1 ];
			image_buffer[ op + 2 ] = buffer->u.RGBA.rgba[ ip + 2 ];
		}
	}

	JSAMPROW row_pointer[ 1 ];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */
	cinfo.err = jpeg_std_error( &jerr );
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress( &cinfo );


	/* Here we use the library-supplied code to send compressed data to a
	 * stdio stream.  You can also write your own code to do something else.
	 * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	 * requires it in order to write binary files.
	 */
	jpeg_stdio_dest( &cinfo, fout );

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	 * Four fields of the cinfo struct must be filled in:
	 */
	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	 * (You must set at least cinfo.in_color_space before calling this,
	 * since the defaults depend on the source color space.)
	 */
	jpeg_set_defaults( &cinfo );
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	jpeg_set_quality( &cinfo, jpgQuality, TRUE /* limit to baseline-JPEG values */ );

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress( &cinfo, TRUE );

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though.
	 */
	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	while( cinfo.next_scanline < cinfo.image_height )
	{
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		row_pointer[ 0 ] = &image_buffer[ cinfo.next_scanline * row_stride ];
		(void)jpeg_write_scanlines( &cinfo, row_pointer, 1 );
	}

	jpeg_finish_compress( &cinfo );

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress( &cinfo );

	free( image_buffer );

	return 1;
}

//------------------------------------------------------------------------------
// Raw Alpha

int WebPWriteAlphaPlane( FILE* fout, const WebPDecBuffer* const buffer )
{
	if( fout == NULL || buffer == NULL )
	{
		return 0;
	}
	else
	{
		const uint32_t width = buffer->width;
		const uint32_t height = buffer->height;
		const uint8_t* a = buffer->u.YUVA.a;
		const int a_stride = buffer->u.YUVA.a_stride;
		uint32_t y;

		if( a == NULL ) return 0;

		fprintf( fout, "P5\n%u %u\n255\n", width, height );
		for( y = 0; y < height; ++y )
		{
			if( fwrite( a, width, 1, fout ) != 1 ) return 0;
			a += a_stride;
		}
		return 1;
	}
}

//------------------------------------------------------------------------------
// PGM with IMC4 layout

int WebPWritePGM( FILE* fout, const WebPDecBuffer* const buffer )
{
	if( fout == NULL || buffer == NULL )
	{
		return 0;
	}
	else
	{
		const int width = buffer->width;
		const int height = buffer->height;
		const WebPYUVABuffer* const yuv = &buffer->u.YUVA;
		const uint8_t* src_y = yuv->y;
		const uint8_t* src_u = yuv->u;
		const uint8_t* src_v = yuv->v;
		const uint8_t* src_a = yuv->a;
		const int uv_width = ( width + 1 ) / 2;
		const int uv_height = ( height + 1 ) / 2;
		const int a_height = ( src_a != NULL ) ? height : 0;
		int ok = 1;
		int y;

		if( src_y == NULL || src_u == NULL || src_v == NULL ) return 0;

		fprintf( fout, "P5\n%d %d\n255\n",
				 ( width + 1 ) & ~1, height + uv_height + a_height );
		for( y = 0; ok && y < height; ++y )
		{
			ok &= ( fwrite( src_y, width, 1, fout ) == 1 );
			if( width & 1 ) fputc( 0, fout );    // padding byte
			src_y += yuv->y_stride;
		}
		for( y = 0; ok && y < uv_height; ++y )
		{
			ok &= ( fwrite( src_u, uv_width, 1, fout ) == 1 );
			ok &= ( fwrite( src_v, uv_width, 1, fout ) == 1 );
			src_u += yuv->u_stride;
			src_v += yuv->v_stride;
		}
		for( y = 0; ok && y < a_height; ++y )
		{
			ok &= ( fwrite( src_a, width, 1, fout ) == 1 );
			if( width & 1 ) fputc( 0, fout );    // padding byte
			src_a += yuv->a_stride;
		}
		return ok;
	}
}

//------------------------------------------------------------------------------
// Raw YUV(A) planes

int WebPWriteYUV( FILE* fout, const WebPDecBuffer* const buffer )
{
	if( fout == NULL || buffer == NULL )
	{
		return 0;
	}
	else
	{
		const int width = buffer->width;
		const int height = buffer->height;
		const WebPYUVABuffer* const yuv = &buffer->u.YUVA;
		const uint8_t* src_y = yuv->y;
		const uint8_t* src_u = yuv->u;
		const uint8_t* src_v = yuv->v;
		const uint8_t* src_a = yuv->a;
		const int uv_width = ( width + 1 ) / 2;
		const int uv_height = ( height + 1 ) / 2;
		const int a_height = ( src_a != NULL ) ? height : 0;
		int ok = 1;
		int y;

		if( src_y == NULL || src_u == NULL || src_v == NULL ) return 0;

		for( y = 0; ok && y < height; ++y )
		{
			ok &= ( fwrite( src_y, width, 1, fout ) == 1 );
			src_y += yuv->y_stride;
		}
		for( y = 0; ok && y < uv_height; ++y )
		{
			ok &= ( fwrite( src_u, uv_width, 1, fout ) == 1 );
			src_u += yuv->u_stride;
		}
		for( y = 0; ok && y < uv_height; ++y )
		{
			ok &= ( fwrite( src_v, uv_width, 1, fout ) == 1 );
			src_v += yuv->v_stride;
		}
		for( y = 0; ok && y < a_height; ++y )
		{
			ok &= ( fwrite( src_a, width, 1, fout ) == 1 );
			src_a += yuv->a_stride;
		}
		return ok;
	}
}

//------------------------------------------------------------------------------
// Generic top-level call

int WebPSaveImage( const WebPDecBuffer* const buffer,
				   WebPOutputFileFormat format,
				   const char* const out_file_name,
				   int jpgQuality )
{
	FILE* fout = NULL;
	int needs_open_file = 1;
	const int use_stdout =
		( out_file_name != NULL ) && !WSTRCMP( out_file_name, "-" );
	int ok = 1;

	if( buffer == NULL || out_file_name == NULL ) return 0;

#ifdef HAVE_WINCODEC_H
	needs_open_file = ( format != PNG );
#endif

	if( needs_open_file )
	{
		if( use_stdout )
			fout = ImgIoUtilSetBinaryMode( stdout );
		else
			fopen_s( &fout, out_file_name, "wb" );
		//fout = use_stdout ? ImgIoUtilSetBinaryMode( stdout )
		//	: fopen_s( out_file_name, "wb" );
		if( fout == NULL )
		{
			WFPRINTF( stderr, "Error opening output file %s\n",
					  (const W_CHAR*)out_file_name );
			return 0;
		}
	}

	if( format == PNG ||
		format == RGBA || format == BGRA || format == ARGB ||
		format == rgbA || format == bgrA || format == Argb )
	{
#ifdef HAVE_WINCODEC_H
		ok &= WebPWritePNG( out_file_name, use_stdout, buffer );
#else
		ok &= WebPWritePNG( fout, buffer );
#endif
	}
	else if( format == PAM )
	{
		ok &= WebPWritePAM( fout, buffer );
	}
	else if( format == PPM || format == RGB || format == BGR )
	{
		ok &= WebPWritePPM( fout, buffer );
	}
	else if( format == RGBA_4444 || format == RGB_565 || format == rgbA_4444 )
	{
		ok &= WebPWrite16bAsPGM( fout, buffer );
	}
	else if( format == BMP )
	{
		ok &= WebPWriteBMP( fout, buffer );
	}
	else if( format == TIFF )
	{
		ok &= WebPWriteTIFF( fout, buffer );
	}
	else if( format == JPEG )
	{
		ok &= WebPWriteJPEG( fout, buffer, jpgQuality );
	}
	else if( format == RAW_YUV )
	{
		ok &= WebPWriteYUV( fout, buffer );
	}
	else if( format == PGM || format == YUV || format == YUVA )
	{
		ok &= WebPWritePGM( fout, buffer );
	}
	else if( format == ALPHA_PLANE_ONLY )
	{
		ok &= WebPWriteAlphaPlane( fout, buffer );
	}
	if( fout != NULL && fout != stdout )
	{
		fclose( fout );
	}
	return ok;
}
