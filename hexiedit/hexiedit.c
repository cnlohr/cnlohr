#include <stdio.h>

#define CNFG_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "rawdraw_sf.h"
#include "os_generic.h"
#include "stb_image.h"
#include "stb_image_write.h"

int lmx, lmy;
int down = 0;
int slot = 0;
int colorQueueLength = 50;
uint32_t colorQueue[50];

void LoadIndex();
void SaveImage();

int HandleDestroy()
{
	SaveImage();
	return 0;
}

void HandleKey( int keycode, int bDown )
{
//sprintf( cts, "slot: %02d n: next slot / p: previous slot / s: save", slot );
	if( bDown )
	switch( keycode )
	{
		case 'p': case 'P': SaveImage(); slot--; if( slot < 0 ) slot = 99; LoadIndex(); break;
		case 'n': case 'N': SaveImage(); slot++; if( slot>=99 ) slot = 0 ; LoadIndex(); break;
		case 's': case 'S': SaveImage(); break;
	}
}

void HandleButton( int x, int y, int button, int bDown )
{
	lmx = x;
	lmy = y;
	if( bDown )
	{
		if( button = 0 )
		{
			down = 1;
		}
		else
		{
			down = 2;
		}
	}
	if( !bDown )
	{
		if( down == 1 )
		{
			int c;
			for( c = 0; c < colorQueueLength-1; c++ )
				colorQueue[c] = colorQueue[c+1];
			colorQueue[c] = 0xff | (rand()<<8);
		}
		down = 0;
	}
}

void HandleMotion( int x, int y, int mask )
{
	down = (mask & 1) | ((mask & 4)>>1);
	lmx = x;
	lmy = y;
}

int imgw;
int imgh;
uint32_t * colors;

float hexsize = 34;
float hexmargin = 5;


typedef struct { float x, y; } hpoint;
typedef struct { float q, r; } hhex;
typedef struct { float q, r, s; } hcube;
typedef struct { int row, col; } hoffsetcoord;
//https://www.redblobgames.com/grids/hexagons/#hex-to-pixel

hpoint flat_hex_to_pixel( hhex h )
{
	// hex to cartesian
	float x = (     3./2 * h.q                    );
	float y = (sqrt(3)/2 * h.q  +  sqrt(3) * h.r);
	// scale cartesian coordinates
	hpoint ret;
	ret.x = x * hexsize;
	ret.y = y * hexsize;
	return ret;
}

hcube axial_to_cube(hhex hex)
{
	float q = hex.q;
	float r = hex.r;
	float s = -q-r;
	return (hcube){q, r, s};
}

hhex cube_to_axial(hcube cube)
{
    float q = cube.q;
    float r = cube.r;
    return (hhex){q, r};
}

hcube cube_round(hcube frac)
{
    float q = round(frac.q);
    float r = round(frac.r);
    float s = round(frac.s);

    float q_diff = abs(q - frac.q);
    float r_diff = abs(r - frac.r);
    float s_diff = abs(s - frac.s);

    if( q_diff > r_diff && q_diff > s_diff )
        q = -r-s;
    else if( r_diff > s_diff )
        r = -q-s;
    else
        s = -q-r;

    return (hcube){q, r, s};
}

hhex axial_round(hhex hex)
{
	return cube_to_axial(cube_round(axial_to_cube(hex)));
}

hhex pixel_to_flat_hex( float px, float py )
{
    // invert the scaling
    float x = px / hexsize;
    float y = py / hexsize;
    // cartesian to hex
    float q = ( 2./3 * x                  );
    float r = (-1./3 * x  +  sqrt(3)/3 * y);
    return axial_round((hhex){q, r});
}

// Odd-q axial.
hoffsetcoord axial_to_oddq(hhex hex)
{
    float parity = ((int)(hex.q+0.5))&1;
    float col = hex.q;
    float row = hex.r + (hex.q - parity) / 2;
    return (hoffsetcoord){col, row};
}

hhex oddq_to_axial(hoffsetcoord hex)
{
    float parity = hex.col&1;
    float q = hex.col;
    float r = hex.row - (hex.col - parity) / 2;
    return (hhex){q, r};
}

void LoadIndex()
{
	char fname[64];
	snprintf( fname, sizeof(fname), "%02d.png", slot );
	if( colors ) free( colors );

	int n;
	uint8_t * loadimg = (uint8_t*)stbi_load(fname, &imgw, &imgh, &n, 4);
	if( n != 4 || loadimg == 0 )
	{
		fprintf( stderr, "Error: Need a 4-component valid png.\n" );
		imgw = 25;
		imgh = 20;
		colors = calloc( 4 * imgw * imgh, 1 );
	}
	else
	{
		colors = calloc( 4 * imgw * imgh, 1 );
		int x, y;
		for( y = 0; y < imgh; y++ )
		for( x = 0; x < imgw; x++ )
		{
			colors[x+y*imgw] = 0xff | (loadimg[(x+y*imgw)*4+0]<<24) | ( loadimg[(x+y*imgw)*4+1]<<16) | ( loadimg[(x+y*imgw)*4+2]<<8);
		}
		free( loadimg );
	}
}

void SaveImage()
{
	char fname[64];
	snprintf( fname, sizeof(fname), "%02d.png", slot );

	uint32_t saveimg[imgw*imgh];
	int x, y;
	for( y = 0; y < imgh; y++ )
	for( x = 0; x < imgw; x++ )
	{
		uint32_t c = colors[x+y*imgw];
		saveimg[x+y*imgw] = 0xff000000 | (c>>24) | ((c>>8)&0xff00) | ((c<<8)&0xff0000);
	}


	int ret = stbi_write_png( fname, imgw, imgh, 4, saveimg, imgw*4);
	printf( "Save: %d\n", ret );
}

int extradebug;

float crossSub( RDPoint a0, RDPoint a1, RDPoint b0, RDPoint b1 )
{
	float dax = (float)a0.x - (float)a1.x;
	float day = (float)a0.y - (float)a1.y;
	float dbx = (float)b0.x - (float)b1.x;
	float dby = (float)b0.y - (float)b1.y;
//	if( extradebug )
//		printf( "+ %f %f %f %f\n", dax, day, dbx, dby );
	float cret = dax * dby - dbx * day;
	return cret;
} 

void DrawHex( hpoint hp, hoffsetcoord hc )
{
	const float sqrt3 = sqrt(3);

	float hexsizemm = (hexsize - hexmargin)/2.0;
	float sx = 1;
	float sy = sqrt3;

	RDPoint pts[6] = {
		{ hp.x - hexsizemm * 2, hp.y },
		{ hp.x - hexsizemm * sx, hp.y + hexsizemm * sy  },
		{ hp.x + hexsizemm * sx, hp.y + hexsizemm * sy },
		{ hp.x + hexsizemm * 2, hp.y },
		{ hp.x + hexsizemm * sx, hp.y - hexsizemm * sy },
		{ hp.x - hexsizemm * sx, hp.y - hexsizemm * sy },
	};


	RDPoint C = (RDPoint){ hp.x, hp.y };
	RDPoint m = (RDPoint){ lmx, lmy };

	int hit = 0;

	// Check if mouse is inside poly.
	for( int p = 0; p < 6; p++ )
	{
		RDPoint A = pts[p];
		RDPoint B = pts[(p+1)%6];

		// Check using barycentric coordiantes
		extradebug = hc.col == 0 && hc.row == 0;

		float bX = crossSub( m, C, A, C )/crossSub(B,C,A,C);
		float bY = crossSub( m, C, B, C )/crossSub(A,C,B,C);
		float bZ = 1 - bX - bY;

		if( bX >= 0 && bY >= 0 && bZ >= 0 && bX <= 1 && bY <= 1 && bZ <= 1 )
		{
			hit = 1;
		}		
	}

	if( hit )
	{
		if( down == 1 )
			colors[hc.col + hc.row * imgw] = colorQueue[0];
		else if( down == 2 )
			colors[hc.col + hc.row * imgw] = 0;
	}


	CNFGColor( colors[hc.col + hc.row * imgw] );
	CNFGTackPoly( pts, 6 );

	uint32_t outline = hit ? 0xffffffff : 0x00000000;

	CNFGColor( outline );
	CNFGTackSegment( pts[0].x, pts[0].y, pts[1].x, pts[1].y );
	CNFGTackSegment( pts[1].x, pts[1].y, pts[2].x, pts[2].y );
	CNFGTackSegment( pts[2].x, pts[2].y, pts[3].x, pts[3].y );
	CNFGTackSegment( pts[3].x, pts[3].y, pts[4].x, pts[4].y );
	CNFGTackSegment( pts[4].x, pts[4].y, pts[5].x, pts[5].y );
	CNFGTackSegment( pts[5].x, pts[5].y, pts[0].x, pts[0].y );
}

int main()
{
	CNFGSetup( "Hexiedit", 1296, 1280 );

	srand( OGGetAbsoluteTime() );

	int c;
	for( c = 0; c < colorQueueLength; c++ )
		colorQueue[c] = 0xff | (rand()<<8);

	LoadIndex();

	while(CNFGHandleInput())
	{
		short wx, wy;
		CNFGGetDimensions( &wx, &wy );
		CNFGBGColor = 0x101010FF;
		CNFGClearFrame();

		int x, y;
		for( y = 0; y < imgh; y++ )
		for( x = 0; x < imgw; x++ )
		{
			hoffsetcoord hoc = (hoffsetcoord){ y, x };
			hpoint hp = flat_hex_to_pixel( oddq_to_axial( hoc ) );
			hp.x += 35;
			hp.y += 85;
			DrawHex( hp, hoc );
		}

		int c = 0;
		CNFGColor( 0 );
		for( c = 0; c < colorQueueLength; c++ )
		{
			CNFGDialogColor = colorQueue[c];
			CNFGDrawBox( c*40+1, 1, c*40+40, 40 );
		}

//		CNFGTackRectangle( xco+margin/2, yco+margin/2, xco+gsx, yco+gsy );

		CNFGColor( 0xffffffff );
		CNFGPenX = 1;
		CNFGPenY = wy-15;
		char cts[1024];
		sprintf( cts, "slot: %02d n: next slot / p: previous slot / s: save / d: high res draw", slot );
		CNFGDrawText( cts, 3 );
		CNFGSwapBuffers();
	}
	return 0;
}

