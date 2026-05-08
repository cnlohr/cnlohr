#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

stbtt_fontinfo font;

int main()
{
	int r;

	FILE * f = fopen( "wlmaru2004emojip.ttf", "rb" );
	//FILE * f = fopen( "AudioLinkConsole-Bold.ttf", "rb" );
	fseek( f, 0, SEEK_END );
	int len = ftell( f );
	fseek( f, 0, SEEK_SET );
	printf( "LEN: %d\n", len );
	uint8_t * ttf_buffer = malloc( len + 1024 );
	r = fread( ttf_buffer, 1, len, f );
	if( r < 1 )
	{
		fprintf( stderr, "Error: Could not read file\n" );
		return -5;
	}
	fclose( f );

	int c = L'は';//'c';

	int offset = stbtt_GetFontOffsetForIndex(ttf_buffer, 0);
	r = stbtt_InitFont(&font, ttf_buffer, offset);

	int ascent;
	float scale = stbtt_ScaleForPixelHeight(&font, 96);
	stbtt_GetFontVMetrics(&font, &ascent,0,0);
	int baseline = (int) (ascent*scale);

	printf( "Baseline %d\n", baseline );
	unsigned char *bitmap;

	printf( "%f\n", scale );

	int w,h,i,j;

	bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, 0,0);
	for (j=0; j < h; ++j) {
		for (i=0; i < w; ++i)
			putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
		putchar('\n');
	}

	free( bitmap );

	free( ttf_buffer );

	return 0;
}


