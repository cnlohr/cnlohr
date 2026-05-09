#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


#define STB_IMAGE_WRITE_STATIC
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

stbtt_fontinfo font;


const int marginBig = 64;
const float pxd = 0.01;
const int downscale = 32;

// Output channels.
#define OCH 2

float ComputePixelError( int boole, int x, int y, float * sdfF, float * dsrgb, int w, int h, int ow, int oh )
{
	int usx, usy;
	float err = 0;
	for( usy = y * downscale - downscale + 1; usy < y * downscale + downscale; usy++ )
	for( usx = x * downscale - downscale + 1; usx < x * downscale + downscale; usx++ )
	{
		float alphX = (((float)(usx))/downscale - usx/downscale);
		float alphY = (((float)(usy))/downscale - usy/downscale);

		float realSdf = sdfF[usx+(downscale/2)+(usy+(downscale/2))*w];
		float calcSdf = 0;
		int ch = 0;
		for( ch = 0; ch < OCH; ch++ )
		{
			// Linear scale-up, matching what the GPU would do.
			float nn = dsrgb[(usx/downscale+0+(usy/downscale+0)*ow)*OCH+ch];
			float pn = dsrgb[(usx/downscale+1+(usy/downscale+0)*ow)*OCH+ch];
			float np = dsrgb[(usx/downscale+0+(usy/downscale+1)*ow)*OCH+ch];
			float pp = dsrgb[(usx/downscale+1+(usy/downscale+1)*ow)*OCH+ch];
			float vX0 = pn * alphX + nn * (1.0 - alphX);
			float vX1 = pp * alphX + np * (1.0 - alphX);
			float o = vX1 * alphY + vX0 * (1.0 - alphY);
			if( o > calcSdf ) calcSdf = o;
		}


		float delta = (!boole) ? ((calcSdf - realSdf)) : ((calcSdf>0.5) - (realSdf>0.5));
		err += delta * delta;
	}
	return err;
}

void WriteSDFTest( const char * filename, int ow, int oh, int w, int h, int downscale, float * sdfF, float * dsrgb )
{
	int usx, usy, x, y, r;
	float err = 0;
	uint8_t * sdfupscaletest = calloc( w * h, 1 );
	for( y = 0; y < oh-1; y++ )
	for( x = 0; x < ow-1; x++ )
	for( usy = y * downscale - downscale + 1; usy < y * downscale + downscale; usy++ )
	for( usx = x * downscale - downscale + 1; usx < x * downscale + downscale; usx++ )
	{
		float alphX = (((float)(usx))/downscale - usx/downscale);
		float alphY = (((float)(usy))/downscale - usy/downscale);

		float realSdf = sdfF[usx+(downscale/2)+(usy+(downscale/2))*w];
		float calcSdf = 0;
		int ch = 0;
		for( ch = 0; ch < OCH; ch++ )
		{
			// Linear scale-up, matching what the GPU would do.
			float nn = dsrgb[(usx/downscale+0+(usy/downscale+0)*ow)*OCH+ch];
			float pn = dsrgb[(usx/downscale+1+(usy/downscale+0)*ow)*OCH+ch];
			float np = dsrgb[(usx/downscale+0+(usy/downscale+1)*ow)*OCH+ch];
			float pp = dsrgb[(usx/downscale+1+(usy/downscale+1)*ow)*OCH+ch];
			float vX0 = pn * alphX + nn * (1.0 - alphX);
			float vX1 = pp * alphX + np * (1.0 - alphX);
			float o = vX1 * alphY + vX0 * (1.0 - alphY);
			if( o > calcSdf ) calcSdf = o;
		}
		int uso = calcSdf * 255.5;
		if( uso < 0 ) uso = 0;
		if( uso > 255 ) uso = 255;
		int px = usx + downscale/2;
		int py = usy + downscale/2;
		if( px < 0 ) continue;
		if( py < 0 ) continue;
		if( px >= w ) continue;
		if( py >= h ) continue;
		sdfupscaletest[usx+(downscale/2)+(usy+(downscale/2))*w] = uso;
	}
	r = stbi_write_png( filename, w, h, 1, sdfupscaletest, w);
	if( !r )
	{
		fprintf( stderr, "Error: image failed to write.\n" );
		exit( -5 );
	}
	free( sdfupscaletest );
}

int main()
{
	int r;

	FILE * f = fopen( "wlmaru2004emojip.ttf", "rb" );
	//FILE * f = fopen( "AudioLinkConsole-Bold.ttf", "rb" );
	fseek( f, 0, SEEK_END );
	int len = ftell( f );
	fseek( f, 0, SEEK_SET );
	uint8_t * ttf_buffer = malloc( len + 1024 );
	r = fread( ttf_buffer, 1, len, f );
	if( r < 1 )
	{
		fprintf( stderr, "Error: Could not read file\n" );
		return -5;
	}
	fclose( f );

	int c = 'A';//L'は';//'c';

	int size = 1024;

	int offset = stbtt_GetFontOffsetForIndex(ttf_buffer, 0);
	r = stbtt_InitFont(&font, ttf_buffer, offset);

	int ascent;
	float scale = stbtt_ScaleForPixelHeight(&font, size);
	stbtt_GetFontVMetrics(&font, &ascent,0,0);
	int baseline = (int) (ascent*scale);

	printf( "Baseline %d\n", baseline );
	unsigned char *bitmap;

	int iw,ih,i,j;
	int x,y;

	bitmap = stbtt_GetCodepointBitmap(&font, 0, scale, c, &iw, &ih, 0,0);

/*
	// stb_truetype console output demo.
	for (j=0; j < h; ++j) {
		for (i=0; i < w; ++i)
			putchar(" .:ioVM@"[bitmap[j*w+i]>>5]);
		putchar('\n');
	}
*/
	int w = iw + marginBig * 2;
	int h = ih + marginBig * 2;

	// STAGE 1: Create SDF

	/* NOTE: This is different from the solution arrived at by Chlumsky2015.
		They are focused on corner preserverence (see page 41 for pseudocode).
		I want to make edges look smooth, so I can decompose an analytical SDF */

	float * sdfF = calloc( w*h*4, 1 );
	float * origf = calloc( w*h*4, 1 );


	for( y = 0; y < ih; y++ ) for( x = 0; x < iw; x++ )
	{
		int ox = x + marginBig;
		int oy = y + marginBig;
		origf[ox+oy*w] = sdfF[ox+oy*w]  = (bitmap[x+y*iw] / 256.0);
	}

	free( bitmap );
	for( i = 0; i < 8; i++ ) // TODO: Optimize by reducing.
	{
		// Switch around directions of search.
		int dx = (!(i&1))?1:-1;
		int dy = (!(i&2))?1:-1;
		int xst = (!(i&1))?0:(w-1);
		int yst = (!(i&2))?0:(h-1);
		int xed = (!(i&1))?(w):-1;
		int yed = (!(i&2))?(h):-1;

		for( y = yst; y != yed; y+=dy ) for( x = xst; x != xed; x+=dx )
		{
			int lx, ly;
			float mv = sdfF[x+y*w];
			float o = origf[x+y*w];
			for( ly = -5; ly < 6; ly++ ) // TODO: Optimize by reducing.
			for( lx = -5; lx < 6; lx++ )
			{
				int tx = lx + x;
				int ty = ly + y;
				if( ty < 0 || tx < 0 || ty >= h || tx >= w ) continue;

				float p = sdfF[ty*w+tx];
				float po = origf[ty*w+tx];
				float dist = sqrtf((lx*lx) + (ly*ly));

				if( o < 0.5 )
				{
					if( p < po ) p = po;
					if( p-dist*pxd > mv )
					{
						mv = p-dist*pxd;
					}
				}
				else
				{
					if( p > po ) p = po;
					if( p+dist*pxd < mv )
					{
						mv = p+dist*pxd;
					}
				}
			}
			sdfF[y*w+x] = mv;
		}
	}

	// Convert 0..1, 0..1 to 0..0.5..1
	uint8_t * bmo = malloc( w * h );
	for( y = 0; y < h; y++ ) for( x = 0; x < w; x++ )
	{
		float f = sdfF[x+y*w] / 2.0;
		if( origf[x+y*w] >= 0.5 )
			f += 0.5 -pxd;
		sdfF[x+y*w] = f;

		int v = f * 255.5;
		if( v < 0 ) v = 0;
		if( v > 255 ) v = 255;
		bmo[x+y*w] = v;
	}

	free( origf );

	r = stbi_write_png( "stage1.png", w, h, 1, bmo, w);
	if( !r )
	{
		fprintf( stderr, "Error: image failed to write.\n" );
		exit( -5 );
	}

	// STAGE 2: Decompose SDF into RGB channels.
	//
	// Our goal here is to create an RGB downsampled image
	// that when upscaled and composed will minimize the error 
	// in the output image.

	int ow = ( w + downscale - 1 ) / downscale;
	int oh = ( h + downscale - 1 ) / downscale;


	float * dsrgb = malloc( ow * oh * 4 * OCH );

	// First, downsample into RGB.
	for( y = 0; y < oh; y++ )
	for( x = 0; x < ow; x++ )
	{
		float sum = 0;
		int div = 0;
		int ix, iy;
		for( iy = 0; iy < downscale; iy++ )
		for( ix = 0; ix < downscale; ix++ )
		{
			int tx = x*downscale+ix;
			int ty = y*downscale+iy;
			if( tx >= w || ty >= h ) continue;
			sum += sdfF[tx+(ty)*w];
			div++;
		}
		int n;
		//dsrgb[(x+y*ow)*OCH+0] = ;
		for( n = 0; n < OCH; n++ )
			dsrgb[(x+y*ow)*OCH+n] = sum/div;
	}

	WriteSDFTest( "stage2-in.png", ow, oh, w, h, downscale, sdfF, dsrgb );

	// Optimize dsrgb over the operational space.
	// Assume outermost edge of pixels should not be changed.
	// this helps us frame the solution to prefer red channel.  << Probably untrue, but let's see how it goes.
	float origTot = 0;
	float newTot = 0;
	for( y = 1; y < oh-1; y++ )
	for( x = 1; x < ow-1; x++ )
	{
		// Somehow tune pixel?

		// this pixel can affect output pixels (x * downscale - downscale + 1) to (x * downscale + downscale - 1)
		float bests[OCH];
		int n;
		for( n = 0; n < OCH; n++ )
			bests[n] = dsrgb[(x+y*ow)*OCH+n];

		float bestPE = ComputePixelError( 0, x, y, sdfF, dsrgb, w, h, ow, oh );
		float origPE = bestPE;
		int i;
		for( i = 0; i < 1000; i++ )
		{
			float celeste = (1000-i)/30000.0;
			for( n = 0; n < OCH; n++ )
			{
				float new = bests[n] + ((rand()%10000)-5000)/5000.0f * celeste;
				if( new < 0 ) new = 0; if( new > 1 ) new = 1;
				dsrgb[(x+y*ow)*OCH+n] = new;
			}

			float PE = ComputePixelError( 0, x, y, sdfF, dsrgb, w, h, ow, oh );
//			printf( "%f %f %f -> %f %f\n", dsrgb[(x+y*ow)*OCH+0] - bestR, dsrgb[(x+y*ow)*OCH+1] - bestG, dsrgb[(x+y*ow)*OCH+2] - bestB, PE, bestPE );
			if( PE < bestPE )
			{
				for( n = 0; n < OCH; n++ )
					bests[n] = dsrgb[(x+y*ow)*OCH+n];
				bestPE = PE;
			}
		}
		origTot += origPE;
		newTot += bestPE;
		for( n = 0; n < OCH; n++ )
			dsrgb[(x+y*ow)*OCH+n] = bests[n];

	}

	printf( "%f %f\n", origTot, newTot );
// Make sure upscaling algo is right.
	WriteSDFTest( "stage2-out.png", ow, oh, w, h, downscale, sdfF, dsrgb );

	uint8_t * dsrgbo = calloc( ow * oh, 3 );
	for( y = 0; y < oh; y++ )
	for( x = 0; x < ow; x++ )
	{
		int n;
		for( n = 0; n < OCH; n++ )
		{
			float r = dsrgb[(x+y*ow)*OCH+n];
			int o = r * 255.5;
			if( o < 0 ) o = 0;
			if( o > 255 ) o = 255;
			if( n < 3 )
				dsrgbo[(x+y*ow)*3+n] = o;
		}
	}

	r = stbi_write_png( "stage2.png", ow, oh, 3, dsrgbo, ow*3);
	if( !r )
	{
		fprintf( stderr, "Error: image failed to write.\n" );
		exit( -5 );
	}

	free( sdfF );
	free( bmo );

	free( ttf_buffer );

	return 0;
}


