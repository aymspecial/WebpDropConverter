#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "webp/config.h"
#endif

int cwebp_main( int argc, char* argv[] );
int dwebp_main( int argc, char* argv[] );

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
#pragma comment( lib, "exampleutil.lib" )
//#pragma comment( lib, "imagedec.lib" )
#pragma comment( lib, "imageenc.lib" )
#pragma comment( lib, "webp.lib" )
#pragma comment( lib, "imageioutil.lib" )
#pragma comment( lib, "webpdemux.lib" )
