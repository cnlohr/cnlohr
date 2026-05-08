#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

stbtt_fontinfo font;

int main()
{
	FILE * f = fopen( "wlmaru2004emojip.ttf", "rb" );
	fseek( f, SEEK_END, 0 );
	int len = ftell( f );
	fseek( f, SEEK_SET	, 0 );
	uint8_t * ttf_buffer = malloc( len );
	fread( ttf_buffer, 1, len, f );
	fclose( f );

	int c = 'A';

	stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));

	free( ttf_buffer );

	int ascent;
	float scale = stbtt_ScaleForPixelHeight(&font, 15);
	stbtt_GetFontVMetrics(&font, &ascent,0,0);
	int baseline = (int) (ascent*scale);

	printf( "Baseline %d\n", baseline );
	unsigned char *bitmap;

	printf( "%f\n", scale );

	int w,h,i,j;

	bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &w, &h, 0,0);
	printf( "Codepoint %d\n", bitmap );
	for (j=0; j < h; ++j) {
		for (i=0; i < w; ++i)
			putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
		putchar('\n');
	}
	return 0;
}


