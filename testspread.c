#include <spreadgine.h>
#include <CNFG3D.h>
#include <unistd.h>
#include <os_generic.h>

void HandleKey( int keycode, int bDown )
{
}

void HandleButton( int x, int y, int button, int bDown )
{
}

void HandleMotion( int x, int y, int mask )
{
}

int main()
{
#if defined( MALI ) || defined( RASPI_GPU )
	Spreadgine * e = SpreadInit( 2160, 1200, "Spread Test", 8888, 2, stderr );
#else
	Spreadgine * e = SpreadInit( 800, 600, "Spread Test", 8888, 2, stderr );
#endif

	//First: Add a defualt shader
	const char * attribos[3] = { "vpos", "vcolor", "vtex" };
	SpreadShader * shd1 = SpreadLoadShader( e, "shd1", "assets/textured.frag", "assets/textured.vert", 2, attribos );
	if( !shd1 )
	{
		fprintf( stderr, "Error making shader.\n" );
	}

	SpreadGeometry * platform = LoadOBJ( e, "assets/platform.obj", 0, 0 );
	SpreadGeometry * plat2 = MakeSquareMesh( e, 2, 2 );
	SpreadGeometry * batchedTri = CreateMeshGen( e, "batchedTri", GL_TRIANGLES, 65535 );


	float eye[3] = { .014, 5, 5 };
	float at[3] =  { 0, 0, 0 };
	float up[3] =  { 0, 0, 1 };
	tdLookAt( e->vpviews[0], eye, at,up );
	tdTranslate( e->vpviews[0], -.4, 0, 0 ); //Shift vanishing point
	eye[0] = -.014;
	tdLookAt( e->vpviews[1], eye, at,up );
	tdTranslate( e->vpviews[1], .4, 0, 0 ); //Shift vanishing point


	SpreadChangeCameaView(e, 0, e->vpviews[0] );
	SpreadChangeCameaView(e, 1, e->vpviews[1] );

	//e->geos[0].render_type = GL_LINES;
	//UpdateSpreadGeometry( &e->geos[0], -1, 0 );

	tdMode( tdMODELVIEW );
	tdIdentity( gSMatrix );
	tdTranslate( gSMatrix, 0., 0., 0. );
	tdScale( gSMatrix, .1, .1, .1 );		//Operates ON f
	tdTranslate( gSMatrix, 00., 0., 0. );

	SpreadTexture * tex = SpreadCreateTexture( e, "tex0", 2048, 2048, 4, GL_UNSIGNED_BYTE );

	{
		static int lin = 0;
		uint32_t * rad = malloc(2048*2048*4);
		int i;
		for( i = 0; i < 2048*2048; i++ )
		{
			rad[i] = i | ((i*10)<<16);//rand();
		}
		SpreadUpdateSubTexture( tex, rad, 0, 0, 2048, 2048 );
		free( rad );
	}



	int x, y;

	int frames = 0, tframes = 0;
	double lastframetime = OGGetAbsoluteTime();
	while(1)
	{
		double Now = OGGetAbsoluteTime();
		spglClearColor( e, .0, 0.0, 0.0, 1.0 );
		spglClear( e, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		spglEnable( e, GL_DEPTH_TEST );

		spglLineWidth( e, 4 );

		{
			static int lin = 0;
			lin+=4;
			if( lin >= tex->h ) lin = 0;
			SpreadUpdateSubTexture( tex, tex->pixeldata + tex->pixwid * tex->w * lin, 0, lin, tex->w, 4 );
		}

		int slot = SpreadGetUniformSlot( shd1, "texSize0");
		if( slot >= 0 )
		{
			float ssf[4] = { 2048, 2048, 0, 0 };
			SpreadUniform4f( shd1, slot, ssf );
		}
		else
		{
			fprintf( stderr, "Error: Can't find parameter in shader\n" );
		}


		SpreadApplyTexture( tex, 0 );
		SpreadApplyShader( shd1 );

/*
		tdPush();
		//tdScale( gSMatrix, .1, .1, .1 );
		SpreadRenderGeometry( plat2, gSMatrix, 0, -1 ); 
		tdPop();
*/

		tdRotateEA( gSMatrix, 0,.2125,1 );		//Operates ON f
		//tdTranslate( modelmatrix, 0, 0, .1 );

		tdPush();
		tdScale( gSMatrix, 1., 1., 1. );
		SpreadRenderGeometry( plat2, gSMatrix, 0, -1 ); 
		//SpreadRenderGeometry( &e->geos[0], gSMatrix, 0, -1 ); 
		tdPop();

		StartImmediateMode( batchedTri );

		tdPush();
		tdIdentity( gSMatrix );
		tdTranslate( gSMatrix, -5., -5., 0. );
		for( y = 0; y < 40; y++ )
		{
			tdTranslate( gSMatrix, 0.0, 2, 0 );
			tdPush();
			for( x = 0; x < 40; x++ )
			{
				tdTranslate( gSMatrix, 2, 0, 0 );
				tdPush();
				tdTranslate( gSMatrix, 0, 0, sin(x*.3+y*.2+tframes*.1)*5 );
				//int rstart = ((tframes)*6)%36;
				//				SpreadRenderGeometry( e->geos[0], gSMatrix, 0, -1 ); 
				float tcoff[4] = { x/40., y/40., 0, 0 };
				float tcscale[4] = { .025, .025, 0, 0 };
				ImmediateModeMesh( plat2, gSMatrix, 0, 0, tcoff, tcscale );
				tdPop();
			}
			tdPop();
		}
		tdPop();

		UpdateMeshToGen( batchedTri );
		SpreadRenderGeometry( batchedTri, gSMatrix, 0, -1 ); 
		//SpreadRenderGeometry( e->geos[0], gSMatrix, 0, -1 ); 


		usleep(20000);
		spglSwap( e );
		SpreadCheckShaders( e );
		frames++;
		tframes++;
		if( Now - lastframetime > 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			lastframetime++;
		}
	}
}
