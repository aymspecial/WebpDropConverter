#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "webp/config.h"


int cwebp_main( int argc, char* argv[] );
int dwebp_main( int argc, char* argv[] );
int cmodmod_main( int argc, char* argv[] );

int xImgIoUtilReadFile( const char* const file_name,
						const uint8_t** data, size_t* data_size );

#define WEBP_HAVE_JPEG

#include "example_util.h"
#include "image_dec.h"
#include "image_enc.h"
#include "imageio_util.h"
#include "webpdec.h"
#include "stopwatch.h"
#include "unicode.h"
#include "webp/encode.h"
#include "webp/decode.h"

#pragma comment( lib, "webputils.lib" )
#pragma comment( lib, "webpdecode.lib" )
#pragma comment( lib, "libexampleutil.lib" )
//#pragma comment( lib, "libimagedec.lib" )
#//pragma comment( lib, "libimageenc.lib" )
#pragma comment( lib, "libwebp.lib" )
//#pragma comment( lib, "libimageioutil.lib" )
#pragma comment( lib, "libwebpdemux.lib" )
#pragma comment( lib, "libsharpyuv.lib" )
