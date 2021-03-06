//Copyright <>< 2018 Charles Lohr, under the MIT-x11 or NewBSD licenses, you choose.

#include <string.h>
#include <spreadgine.h>
#include <spreadgine_remote.h>
#include <os_generic.h>
#include <arpa/inet.h> //For htonl
#include <stdlib.h>

//For rawdraw
#include <CNFGFunctions.h>
#include <CNFG3D.h>

//The raspi is a little slow on the uptake.
#ifndef GL_RED
#define GL_GEOMETRY_SHADER                0x8DD9
#define GL_RED 0x1903
#define GL_RG 0x8227
#endif


uint8_t SpreadTypeSizes[] = { 4, 1 };


Spreadgine * SpreadInit( int w, int h, const char * title, int httpport, int vps, FILE * fReport )
{
	int i;
	Spreadgine * ret;

#if 0
	if( CNFGSetup( title, w, h ) )
	{
		fprintf( fReport, "Error: Could not setup graphics frontend.\n" );
		return 0;
	}
#endif
	CNFGSetupFullscreen( title, 1 );

#ifdef RASPI_GPU
	if( w > 2048 )
	{
		w = 2048;
	}
#endif

	ret = calloc( 1, sizeof( Spreadgine ) );
	ret->fReport = fReport;
	ret->cbbuff = malloc( SPREADGINE_CIRCBUF );
	ret->vpperspectives = malloc( sizeof(float) * 16 * SPREADGINE_CAMERAS );
	ret->vpviews = malloc( sizeof(float) * 16 * SPREADGINE_CAMERAS );
	ret->vpnames = malloc( sizeof( char* ) * SPREADGINE_CAMERAS );


	for( i = 0; i < SPREADGINE_CAMERAS; i++ )
	{
		tdIdentity( ret->vpperspectives[i] );
		tdIdentity( ret->vpviews[i] );
		ret->vpnames[i] = 0;
	}

	if( vps > SPREADGINE_VIEWPORTS )
	{
		fprintf( fReport, "Error: SPREADGINE_VIEWPORTS insufficient for your system.\n" );
		return 0;
	}

	SpreadRemoteInit( ret, httpport );
	SpreadMessage( ret, "-setup", "biis", 64, w, h, title );


	ret->setvps = vps;

	for( i = 0; i < vps; i++ )
	{
		char EyeName[5] = { 'E', 'y', 'e', '0'+i };
		SpreadSetupCamera( ret, i, 75, (float)w/vps/h, .01, 1000, EyeName );
		tdIdentity(ret->vpviews[i]);
		SpreadChangeCameaView( ret, i, ret->vpviews[i] );


		ret->vpnames[i] = strdup( "EyeX" );
		ret->vpnames[i][3] = '0' + i;
		ret->vpedges[i][0] = i*w/vps;
		ret->vpedges[i][1] = 0;
		ret->vpedges[i][2] = w/vps;
		ret->vpedges[i][3] = h;
	}

	{
		//First: Add a defualt shader
		SpreadShader * shd0 = SpreadLoadShader( ret, "shd0", "assets/default.frag", "assets/default.vert", 0 );
		if( !shd0 )
		{
			fprintf( fReport, "Error making shader.\n" );
		}
	}

	{
		#define SIMPLECUBE
		#ifdef SIMPLECUBE
		/* init_resources */
		uint16_t CubeDataIndices[] = {
			0, 1, 2,	2, 3, 0,	// front
			1, 5, 6,	6, 2, 1,	// right
			7, 6, 5,	5, 4, 7,	// back
			4, 0, 3,	3, 7, 4,	// left
			4, 5, 1,	1, 0, 4,	// bottom
			3, 2, 6,	6, 7, 3,	// top
		}; 		int IndexQty = 36;

		static const float CubeDataVerts[] = {
			-1.0, -1.0,  1.0,	 1.0, -1.0,  1.0,	 1.0,  1.0,  1.0,	-1.0,  1.0,  1.0,			// front
			-1.0, -1.0, -1.0,	 1.0, -1.0, -1.0,	 1.0,  1.0, -1.0,	-1.0,  1.0, -1.0,			// back
		};		int VertQty = 8;

		static const float CubeDataColors[] = {
			1.0, 0.0, 0.0, 1.0,		0.0, 1.0, 0.0, 1.0,		0.0, 0.0, 1.0, 1.0,		1.0, 1.0, 1.0, 1.0,			// front colors
			1.0, 0.0, 0.0, 1.0,		0.0, 1.0, 0.0, 1.0,		0.0, 0.0, 1.0, 1.0,		1.0, 1.0, 1.0, 1.0,			// back colors
		};

		static const float CubeDataTCs[] = {
			1.0, 0.0, 0.0, 1.0,		0.0, 1.0, 0.0, 1.0,		0.0, 0.0, 1.0, 1.0,		1.0, 1.0, 1.0, 1.0,			// front vecs
			1.0, 0.0, 0.0, 1.0,		0.0, 1.0, 0.0, 1.0,		0.0, 0.0, 1.0, 1.0,		1.0, 1.0, 1.0, 1.0,			// back vecs
		};


		#else
		static const float CubeDataVerts[36*3] = {
			-1.0f,-1.0f,-1.0f,	-1.0f,-1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,
			-1.0f,-1.0f,-1.0f,	-1.0f, 1.0f, 1.0f,	-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,	1.0f,-1.0f,-1.0f,	1.0f, 1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,	1.0f, 1.0f, 1.0f,	1.0f,-1.0f, 1.0f,

			1.0f, 1.0f,-1.0f,	-1.0f,-1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f,-1.0f,	1.0f,-1.0f,-1.0f,	-1.0f,-1.0f,-1.0f,
			-1.0f, 1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	1.0f,-1.0f, 1.0f,
			1.0f, 1.0f, 1.0f,	-1.0f, 1.0f, 1.0f,	1.0f,-1.0f, 1.0f,

			1.0f,-1.0f, 1.0f,	-1.0f,-1.0f,-1.0f,	1.0f,-1.0f,-1.0f,
			1.0f,-1.0f, 1.0f,	-1.0f,-1.0f, 1.0f,	-1.0f,-1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,	1.0f, 1.0f,-1.0f,	-1.0f, 1.0f,-1.0f,
			1.0f, 1.0f, 1.0f,	-1.0f, 1.0f,-1.0f,	-1.0f, 1.0f, 1.0f,
		};	int VertQty = 36;

		static const float CubeDataColors[36*4] = {
			1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,
			0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,
			0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,
			0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,
			1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,
			1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,
		};

		static const float CubeDataTCs[36*4] = {
			1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,	1.,0.,0.,1.,
			0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,	0.,1.,0.,1.,
			0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,	0.,0.,1.,1.,
			0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,	0.,1.,1.,1.,
			1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,	1.,0.,1.,1.,
			1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,	1.,1.,0.,1.,
		};

		static uint16_t CubeDataIndices[36];
		for( i = 0; i < 36; i++ ) CubeDataIndices[i] = i;
		int IndexQty = 36;

		#endif

		static int strides[3] = { 3, 4, 4 };
		static int types[3] = { GL_FLOAT, GL_FLOAT, GL_FLOAT };
		const float * arrays[] = { CubeDataVerts, CubeDataColors, CubeDataTCs };

		SpreadGeometry * geo0 = SpreadCreateGeometry( ret, "geo1", GL_TRIANGLES, IndexQty, CubeDataIndices, VertQty, 3, (const void **)arrays, strides, types  );
		if( !geo0 )
		{
			fprintf( fReport, "Error making geometry.\n" );
		}
	}

	return ret;
}


void SpreadDestroy( Spreadgine * spr )
{
	if( !spr ) return;

	spr->doexit = 1;
	OGJoinThread( spr->spreadthread );

	int i;
	for( i = 0; i < spr->setshaders; i++ )
		free( spr->shaders[i] );
	if( spr->shaders )
		free( spr->shaders );
	for( i = 0; i < spr->setgeos; i++ )
		free( spr->geos[i] );
	if( spr->geos ) 
		free( spr->geos );
	for( i = 0; i < spr->settexs; i++ )
		free( spr->textures[i] );
	if( spr->textures )
		free( spr->textures );
	if( spr->vpnames[i] )
		free( spr->vpnames[i] );
	free( spr->vpnames );
	free( spr->vpperspectives );
	free( spr->vpviews );
	free( spr->cbbuff );
	free( spr );
}

void SpreadSetupCamera( Spreadgine * spr, uint8_t camid, float fov, float aspect, float near, float far, const char * camname )
{
	if( camid >= SPREADGINE_CAMERAS )
	{
		fprintf( spr->fReport, "Error: Camid %d too large for SpreadSetupCamera\n", camid );
		return;
	}
	if( spr->vpnames[camid] ) free( spr->vpnames[camid] );
	spr->vpnames[camid] = strdup( camname );
	tdPerspective( fov, aspect, near, far, spr->vpperspectives[camid] );
	tdIdentity(spr->vpviews[camid]);

	SpreadMessage( spr, "camera#", "bbffffs", camid, 68, camid, fov, aspect, near, far, camname );
	SpreadChangeCameaPerspective( spr, camid, spr->vpperspectives[camid]  );
}

void SpreadChangeCameaPerspective( Spreadgine * spr, uint8_t camid, float * newpersp )
{
	if( spr->vpperspectives[camid] != newpersp )
		memcpy( spr->vpperspectives[camid], newpersp, sizeof(float)*16 );
	SpreadMessage( spr, "campersp#", "bbX", camid, 66, camid, 16*sizeof(float), newpersp );
}

void SpreadChangeCameaView( Spreadgine * spr, uint8_t camid, float * newview )
{
	if( spr->vpviews[camid] != newview )
		memcpy( spr->vpviews[camid], newview, sizeof(float)*16 );
	SpreadMessage( spr, "camview#", "bbX", camid, 67, camid, 16*sizeof(float), newview );
}


void spglEnable( Spreadgine * e, uint32_t en )
{
	uint32_t endianout = htonl( en );
	SpreadPushMessage( e, 74, 4, &endianout );
	glEnable( en );
}

void spglDisable( Spreadgine * e,uint32_t de )
{
	uint32_t endianout = htonl( de );
	SpreadPushMessage( e, 75, 4, &endianout );
	glDisable( de );
}

void spglLineWidth( Spreadgine * e,float wid )
{
	SpreadPushMessage( e, 76, 4, &wid );
	glLineWidth( wid );
}

void spglSwap(Spreadgine * e)
{
	CNFGSwapBuffers();
	SpreadPushMessage(e, 77, 0, 0 );
}

void spglClearColor( Spreadgine * e, float r, float g, float b, float a )
{
	e->lastclearcolor[0] = r;
	e->lastclearcolor[1] = g;
	e->lastclearcolor[2] = b;
	e->lastclearcolor[3] = a;
	SpreadPushMessage(e, 78, 16, e->lastclearcolor );
	glClearColor( r, g, b, a );
}

void spglClear( Spreadgine * e, uint32_t clearmask )
{
	uint32_t lcmask = htonl(clearmask);
	SpreadPushMessage(e, 79, 4, &lcmask );
	glClear( clearmask );
}

static SpreadShader * LoadShaderAtPlace( SpreadShader * ret, Spreadgine * spr )
{
	int i;
	int retval;

	const char * shadername = ret->shadername;

	const char * fragmentShader = ret->fragment_shader_source;
	const char * vertexShader = ret->vertex_shader_source;
	const char * geometryShader = ret->geometry_shader_source;

	char * fragmentShader_text = 0;
	char * vertexShader_text = 0;
	char * geometryShader_text = 0;

	ret->fragment_shader_time = OGGetFileTime( fragmentShader );
	ret->vertex_shader_time = OGGetFileTime( vertexShader );
	ret->geometry_shader_time = geometryShader?OGGetFileTime( geometryShader ):0;

	FILE * f = fopen( fragmentShader, "rb" );
	if( !f )
	{
		fprintf( spr->fReport, "Error: Could not load fragment shader \"%s\" [%p]\n", fragmentShader, ret );
		goto qexit;
	}
	fseek( f, 0, SEEK_END );
	int rl = ftell(f);
	fseek( f, 0, SEEK_SET );
	fragmentShader_text = malloc( rl+1 );
	int _ignored_ = fread( fragmentShader_text, 1, rl, f );
	fragmentShader_text[rl] = 0;
	fclose( f );

	f = fopen( vertexShader, "rb" );
	if( !f )
	{
		fprintf( spr->fReport, "Error: Could not load vertex shader \"%s\"\n", vertexShader );
		goto qexit;
	}
	fseek( f, 0, SEEK_END );
	rl = ftell( f );
	fseek( f, 0, SEEK_SET );
	vertexShader_text = malloc( rl+1 );
	_ignored_ = fread( vertexShader_text, 1, rl, f );
	vertexShader_text[rl] = 0;
	fclose( f );

	if( geometryShader )
	{
		f = fopen( geometryShader, "rb" );
		if( !f )
		{
			fprintf( spr->fReport, "Error: Could not load vertex shader \"%s\"\n", vertexShader );
			goto qexit;
		}
		fseek( f, 0, SEEK_END );
		rl = ftell( f );
		fseek( f, 0, SEEK_SET );
		geometryShader_text = malloc( rl+1 );
		_ignored_ = fread( geometryShader_text, 1, rl, f );
		geometryShader_text[rl] = 0;
		fclose( f );
	}

	glUseProgram( 0 );

	if( ret->program_shader && ret->vertex_shader && ret->fragment_shader )
	{
		glDetachShader(ret->program_shader, ret->vertex_shader);
		glDetachShader(ret->program_shader, ret->fragment_shader);
		if( ret->geometry_shader )
			glDetachShader( ret->program_shader, ret->geometry_shader );
	}


	if( ret->fragment_shader ) { glDeleteShader( ret->fragment_shader ); ret->fragment_shader = 0; }
	if( ret->vertex_shader ) { glDeleteShader( ret->vertex_shader ); ret->vertex_shader = 0; }
	if( ret->geometry_shader ) { glDeleteShader( ret->geometry_shader ); ret->geometry_shader = 0; }
	if( ret->program_shader ) { glDeleteShader( ret->program_shader ); ret->program_shader = 0; }


	if( !ret->vertex_shader ) ret->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	if (!ret->vertex_shader)
	{
		fprintf(spr->fReport, "Error: glCreateShader(GL_VERTEX_SHADER) failed: 0x%08X\n", glGetError());
		goto qexit;
	}

	glShaderSource(ret->vertex_shader, 1, (const GLchar**)&vertexShader_text, NULL);
	glCompileShader(ret->vertex_shader);
	glGetShaderiv(ret->vertex_shader, GL_COMPILE_STATUS, &retval);

	if (!retval) {
		char *log;
		fprintf(spr->fReport, "Error: vertex shader compilation failed!\n");
		glGetShaderiv(ret->vertex_shader, GL_INFO_LOG_LENGTH, &retval);

		if (retval > 1) {
			log = malloc(retval);
			glGetShaderInfoLog(ret->vertex_shader, retval, NULL, log);
			fprintf(spr->fReport, "%s", log);
			free( log );
		}
		goto qexit;
	}

	if( !ret->fragment_shader ) ret->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	if (!ret->fragment_shader) {
		fprintf(spr->fReport, "Error: glCreateShader(GL_FRAGMENT_SHADER) failed: 0x%08X\n", glGetError());
		goto qexit;
	}
	glShaderSource(ret->fragment_shader, 1, (const GLchar**)&fragmentShader_text, NULL);
	glCompileShader(ret->fragment_shader);
	glGetShaderiv(ret->fragment_shader, GL_COMPILE_STATUS, &retval);
	if( !retval )
	{
		char *log;
		fprintf(spr->fReport, "Error: fragment shader compilation failed!\n");
		glGetShaderiv(ret->fragment_shader, GL_INFO_LOG_LENGTH, &retval);
		if (retval > 1) {
			log = malloc(retval);
			glGetShaderInfoLog(ret->fragment_shader, retval, NULL, log);
			fprintf(spr->fReport, "%s", log);
			free( log );
		}
		goto qexit;
	}

	if( geometryShader_text )
	{
		if( !ret->geometry_shader ) ret->geometry_shader = glCreateShader(GL_GEOMETRY_SHADER);
		if (!ret->geometry_shader) {
			fprintf(spr->fReport, "Error: glCreateShader(GL_GEOMETRY_SHADER) failed: 0x%08X\n", glGetError());
			goto qexit;
		}
		glShaderSource(ret->geometry_shader, 1, (const GLchar**)&geometryShader_text, NULL);
		glCompileShader(ret->geometry_shader);
		glGetShaderiv(ret->geometry_shader, GL_COMPILE_STATUS, &retval);
		if( !retval )
		{
			char *log;
			fprintf(spr->fReport, "Error: geometry shader compilation failed!\n");
			glGetShaderiv(ret->geometry_shader, GL_INFO_LOG_LENGTH, &retval);
			if (retval > 1) {
				log = malloc(retval);
				glGetShaderInfoLog(ret->geometry_shader, retval, NULL, log);
				fprintf(spr->fReport, "%s", log);
				free( log );
			}
			goto qexit;
		}
	}


	if( !ret->program_shader )
	{
		ret->program_shader = glCreateProgram();
	}
	if( !ret->program_shader ) {
		fprintf(spr->fReport, "Error: failed to create program!\n");
		free( ret );
		goto qexit;
	}

	glAttachShader(ret->program_shader, ret->vertex_shader);
	glAttachShader(ret->program_shader, ret->fragment_shader);
	if( ret->geometry_shader )
	{
		glAttachShader(ret->program_shader, ret->geometry_shader);
	}

	for( i = 0; i < MAX_ATTRIBUTES; i++ )
	{
		char ats[40];
		sprintf( ats, "attrib%d", i );
		glBindAttribLocation( ret->program_shader, i, ats );
	}

	glLinkProgram(ret->program_shader);
	glGetProgramiv(ret->program_shader, GL_LINK_STATUS, &retval);
	if (!retval) {
		char *log;
		fprintf(spr->fReport, "Error: program linking failed!\n");
		glGetProgramiv(ret->program_shader, GL_INFO_LOG_LENGTH, &retval);

		if (retval > 1) {
			log = malloc(retval);
			glGetProgramInfoLog(ret->program_shader, retval, NULL, log);
			fprintf(spr->fReport, "%s", log);
			free( log );
		}
		goto qexit;
	}


	for( i = 0; i < MAX_ATTRIBUTES; i++ )
	{
		char ats[40];
		sprintf( ats, "attrib%d", i );
		ret->attribute_slots[i] = glGetAttribLocation( ret->program_shader, ats );
	}


	ret->known_uniform_slots = 0;

	ret->model_index = glGetUniformLocation( ret->program_shader, "mmatrix" );
	ret->view_index = glGetUniformLocation( ret->program_shader, "vmatrix" );
	ret->perspective_index = glGetUniformLocation( ret->program_shader, "pmatrix" );

	glUseProgram(ret->program_shader);
	spr->current_shader = ret->shader_in_parent;

	int err = glGetError();
	if( err )
	{
		fprintf( spr->fReport, "Hanging error on shader compile %d (0x%02x)\n", err, err );
	}

	//Do 8 textures.
	for( i = 0; i < 8; i++ )
	{
		char ct[9] = { 't', 'e', 'x', 't', 'u', 'r', 'e', '0', 0 };
		ct[7] = '0' + i; 
		int slot = glGetUniformLocation(ret->program_shader, ct);
		glUniform1i(slot, i);
	}

	SpreadMessage( spr, "-shader#", "bbssss", ret->shader_in_parent, 69, ret->shader_in_parent, shadername, fragmentShader_text, vertexShader_text, geometryShader_text?geometryShader_text:"" );
	goto finalexit;
qexit:
	if( ret->fragment_shader ) { glDeleteShader( ret->fragment_shader ); ret->fragment_shader = 0; }
	if( ret->vertex_shader ) { glDeleteShader( ret->vertex_shader ); ret->vertex_shader = 0; }
	if( ret->program_shader ) { glDeleteShader( ret->program_shader ); ret->program_shader = 0; }
	ret = 0;
finalexit:
	free( fragmentShader_text );
	free( vertexShader_text );
	if( geometryShader_text ) free( geometryShader_text );
	return ret;
}

SpreadShader * SpreadLoadShader( Spreadgine * spr, const char * shadername, const char * fragmentShader, const char * vertexShader, const char * geometryShader )
{
	int i;
	int shaderindex = 0;
	int retval;
	SpreadShader * ret;

	//First see if there are any free shaders available in the parent...
	for( i = 0; i < spr->setshaders; i++ )
	{
		if( spr->shaders[i]->shadername == 0 )
			break;
	}
	if( i == spr->setshaders )
	{
		spr->shaders = realloc( spr->shaders, (spr->setshaders+1)* sizeof( SpreadShader* ) );
		ret = spr->shaders[spr->setshaders] = calloc( sizeof( SpreadShader), 1 );
	}
	else
	{
		ret = spr->shaders[i];
	}

	memset( ret, 0, sizeof( SpreadShader ) );

	ret->shader_in_parent = i;
	ret->parent = spr;
	ret->shadername = strdup( shadername );
	ret->fragment_shader_source = strdup( fragmentShader );
	ret->vertex_shader_source = strdup( vertexShader );
	ret->geometry_shader_source = geometryShader?strdup( geometryShader ):0;
	ret->known_uniform_slots = 0;
	LoadShaderAtPlace( ret, spr );
	spr->setshaders++;
	return ret;
}

int SpreadGetUniformSlot( SpreadShader * shd, const char * slotname )
{
	int i;
	glUseProgram(shd->program_shader);
	shd->parent->current_shader = shd->shader_in_parent;
	int slot = glGetUniformLocation( shd->program_shader, slotname );
	if( slot >= shd->known_uniform_slots )
	{
		shd->known_uniform_slots = slot+1;
		shd->uniform_slot_names = realloc( shd->uniform_slot_names, shd->known_uniform_slots * sizeof( char* ) );
		shd->uniform_slot_name_lens = realloc( shd->uniform_slot_name_lens, shd->known_uniform_slots * sizeof( int ) );
		shd->size_of_uniforms = realloc( shd->size_of_uniforms, shd->known_uniform_slots * sizeof( int ) );
		shd->uniforms_in_slots = realloc( shd->uniforms_in_slots, shd->known_uniform_slots * sizeof( float * ) );

		shd->uniform_slot_names[slot] = strdup( slotname );
		shd->uniform_slot_name_lens[slot] = strlen( slotname );
		shd->size_of_uniforms[slot] = 0;
		shd->uniforms_in_slots[slot] = 0;
	}

	return slot;
}

void SpreadUniform4f( SpreadShader * shd, int slot, const float * uni )
{
	int stlen = shd->uniform_slot_name_lens[slot];
	char outputarray[stlen+sizeof(float)*4];
	glUniform4fv( slot, 1, uni );
	memcpy( outputarray, uni, sizeof(float)*4 );
	memcpy( outputarray + sizeof(float)*4, shd->uniform_slot_names[slot], stlen );
	SpreadPushMessage(shd->parent, 81, sizeof(float)*4+stlen, outputarray );
}

void SpreadUniform16f( SpreadShader * shd, int slot, const float * uni )
{
	int stlen = shd->uniform_slot_name_lens[slot];
	char outputarray[stlen+sizeof(float)*16];
	glUniformMatrix4fv( slot, 1, 0, uni );
	memcpy( outputarray, uni, sizeof(float)*16 );
	memcpy( outputarray + sizeof(float)*16, shd->uniform_slot_names[slot], stlen );
	SpreadPushMessage(shd->parent, 82, sizeof(float)*16+stlen, outputarray );
}

void SpreadApplyShader( SpreadShader * shd )
{
	glUseProgram(shd->program_shader);
	uint32_t sip = htonl( shd->shader_in_parent );
	//SpreadPushMessage(shd->parent, 80, 4, &sip );
	SpreadMessage( shd->parent, "curshader", "bi", 80, shd->shader_in_parent );
}

void SpreadFreeShader( SpreadShader * shd )
{
	if( shd->fragment_shader ) glDeleteShader( shd->fragment_shader );
	if( shd->vertex_shader ) glDeleteShader( shd->vertex_shader );
	if( shd->program_shader ) glDeleteShader( shd->program_shader );
	if( shd->uniform_slot_names ) 		{ free( shd->uniform_slot_names ) ;  	shd->uniform_slot_names = 0; }
	if( shd->uniform_slot_name_lens ) 	{ free( shd->uniform_slot_name_lens ); 	shd->uniform_slot_name_lens = 0; }
	if( shd->size_of_uniforms ) 		{ free( shd->size_of_uniforms ); 		shd->size_of_uniforms = 0; }
	if( shd->uniforms_in_slots ) 		{ free( shd->uniforms_in_slots );		shd->uniforms_in_slots = 0; }
	if( shd->shadername )				{ free( shd->shadername );				shd->shadername = 0; }
	if( shd->fragment_shader_source )	{ free( shd->fragment_shader_source );	shd->fragment_shader_source = 0; }
	if( shd->vertex_shader_source )		{ free( shd->vertex_shader_source );	shd->vertex_shader_source = 0; }
	if( shd->geometry_shader_source )	{ free( shd->geometry_shader_source );	shd->geometry_shader_source = 0; }

	SpreadMessage( shd->parent, 0, "bb", 70, shd->shader_in_parent );
	SpreadHashRemove( shd->parent, "shader#", shd->shader_in_parent );

}

void SpreadCheckShaders( Spreadgine * spr )
{
	int i;
	for( i = 0; i < spr->setshaders; i++ )
	{
		SpreadShader * shd = spr->shaders[i];
		if( !shd->shadername ) continue;
		double ft = OGGetFileTime( shd->fragment_shader_source );
		double vt = OGGetFileTime( shd->vertex_shader_source );
		double gt = shd->geometry_shader_source?OGGetFileTime( shd->geometry_shader_source ):0;

		if( ft != shd->fragment_shader_time ||  vt != shd->vertex_shader_time || gt != shd->geometry_shader_time )
		{
			printf( "Recompiling shader %s\n", shd->shadername );
			LoadShaderAtPlace( shd, spr );
		}
	}
}

SpreadGeometry * SpreadCreateGeometry( Spreadgine * spr, const char * geoname, int render_type, int indices, uint16_t * indexarray, int verts, int nr_arrays, const void ** arrays, int * strides, int * types )
{

	int i;
	int retval;
	SpreadGeometry * ret;


	//First see if there are any free geos available in the parent...
	for( i = 0; i < spr->setgeos; i++ )
	{
		if( spr->geos[i]->geoname == 0 )
			break;
	}
	if( i == spr->setgeos )
	{
		spr->geos = realloc( spr->geos, (spr->setgeos+1)* sizeof( SpreadGeometry * ) );
		i = spr->setgeos;
		spr->setgeos++;
		ret = spr->geos[i] = calloc( sizeof( SpreadGeometry ), 1 );
	}
	else
	{
		ret = spr->geos[i];
	}

	ret->indices = indices;
	ret->indexarray = malloc(indices*sizeof(uint16_t));
	memcpy( ret->indexarray, indexarray, sizeof(uint16_t)*indices );

	ret->geo_in_parent = i;
	ret->geoname = strdup( geoname );
	ret->render_type = render_type;
	ret->verts = verts;
	ret->parent = spr;
	ret->numarrays = nr_arrays;
	ret->arrays = malloc( sizeof(void*) * nr_arrays );
	ret->types = malloc( sizeof(uint8_t) * nr_arrays );
	ret->strides = malloc( sizeof(uint8_t) * nr_arrays );
	ret->user = 0;
	
	glGenBuffers(1, &ret->ibo );
 	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*indices, ret->indexarray, GL_STATIC_DRAW);
	ret->laststartv = 0;

	ret->vbos = malloc( sizeof( uint32_t ) * nr_arrays );
	glGenBuffers(nr_arrays, ret->vbos );


	uint16_t SendIndexArray[ret->indices];
	for( i = 0; i < ret->indices; i++ )
	{
		SendIndexArray[i] = htons( ret->indexarray[i] );
	}

	SpreadMessage( ret->parent, "geometry#", "bbsiibvvv", ret->geo_in_parent, 87, ret->geo_in_parent, ret->geoname, ret->render_type, ret->verts, ret->numarrays,
		sizeof(uint8_t)*ret->numarrays, ret->strides, 
		sizeof(uint8_t)*ret->numarrays, ret->types,
		sizeof(uint16_t)*ret->indices, SendIndexArray );

	for( i = 0; i < nr_arrays; i++ )
	{
		int typesize = 0;
		if( types[i] == GL_FLOAT )
		{
			ret->types[i] = 0;
		}
		else if( types[i] == GL_UNSIGNED_BYTE )
		{
			ret->types[i] = 1;
		}
		else
		{
			fprintf( spr->fReport, "Error: bad 'type' passed into SpreadCreateGeometry. Assuming GL_FLOAT.\n" );
			ret->types[i] = 0;
		}
		int stride = ret->strides[i] = strides[i];
			typesize = SpreadTypeSizes[ret->types[i]];
		ret->arrays[i] = malloc( stride * typesize * verts );
		memcpy( ret->arrays[i], arrays[i], stride * typesize * verts );

	 	glBindBuffer(GL_ARRAY_BUFFER, ret->vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, stride * typesize * verts, ret->arrays[i], GL_STATIC_DRAW);

		SpreadMessage( spr, "geodata#_#", "bbbv", ret->geo_in_parent, i, 88, ret->geo_in_parent, i, stride * typesize * verts, ret->arrays[i] );
	}

	UpdateSpreadGeometry( ret, -1, 0 );
	
	int err = glGetError();
	if( err )
	{
		fprintf( spr->fReport, "Hanging error on geometry compile %d (0x%02x)\n", err, err );
	}

	return ret;
}

void UpdateSpreadGeometry( SpreadGeometry * geo, int arrayno, void * arraydata )
{
	if( arrayno == -1 || arrayno == -2 )
	{
		int i;

		uint16_t SendIndexArray[geo->indices];
		for( i = 0; i < geo->indices; i++ )
		{
			SendIndexArray[i] = htons( geo->indexarray[i] );
		}

		SpreadMessage( geo->parent, "geometry#", "bbsiibvvv", geo->geo_in_parent, 87, geo->geo_in_parent, geo->geoname, geo->render_type, geo->verts, geo->numarrays,
			sizeof(uint8_t)*geo->numarrays, geo->strides, 
			sizeof(uint8_t)*geo->numarrays, geo->types,
			sizeof(uint16_t)*geo->indices, SendIndexArray );

		if( arrayno == -2 )
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->ibo);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint16_t)*geo->indices, geo->indexarray );
			for( i = 0; i < geo->numarrays; i++ )
			{
			 	glBindBuffer(GL_ARRAY_BUFFER, geo->vbos[i]);
				int typesize = SpreadTypeSizes[geo->types[i]];
				glBufferSubData(GL_ARRAY_BUFFER, 0, geo->strides[i] * typesize * geo->verts, geo->arrays[i]);
			}
		}

		for( arrayno = 0; arrayno < geo->numarrays; arrayno++ )
		{
			int arraysize = geo->strides[arrayno] * SpreadTypeSizes[ geo->types[arrayno] ] * geo->verts;
			SpreadMessage( geo->parent, "geodata#_#", "bbbv", geo->geo_in_parent, arrayno, 88, geo->geo_in_parent, arrayno, arraysize, geo->arrays[arrayno] );
		}

	}
	else
	{
		int arraysize = geo->strides[arrayno] * SpreadTypeSizes[ geo->types[arrayno] ] * geo->verts;
		memcpy( geo->arrays[arrayno], arraydata, arraysize );
		SpreadMessage( geo->parent, "geodata#_#", "bbbv", geo->geo_in_parent, arrayno, 88, geo->geo_in_parent, arrayno, arraysize, arraydata );

		int i = arrayno;
		int stride = geo->strides[i];
		int typesize = SpreadTypeSizes[geo->types[i]];
		memcpy( geo->arrays[i], arraydata, stride * typesize * geo->verts );
	 	glBindBuffer(GL_ARRAY_BUFFER, geo->vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, geo->strides[i] * typesize * geo->verts, geo->arrays[i], GL_STATIC_DRAW);
	}
}

void SpreadRenderGeometry( SpreadGeometry * geo, const float * modelmatrix, int startv, int numv )
{
	Spreadgine * parent = geo->parent;
	SpreadShader * ss = parent->shaders[parent->current_shader];
	int vmatpos = ss->view_index;
	int pmatpos = ss->perspective_index;
	int mmatpos = ss->model_index;

	glUniformMatrix4fv( mmatpos, 1, 0, modelmatrix );
	//tdPrint( modelmatrix );

	int i;
	for( i = 0; i < geo->numarrays; i++ )
	{
		glBindBuffer(GL_ARRAY_BUFFER, geo->vbos[i]);
		int attribindex = ss->attribute_slots[i];
		glVertexAttribPointer( attribindex, geo->strides[i], (geo->types[i]==0)?GL_FLOAT:GL_UNSIGNED_BYTE, GL_FALSE, 0, /*geo->arrays[i]*/0 );
	    glEnableVertexAttribArray(attribindex);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geo->ibo);
	if( startv != geo->laststartv )
	{
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)*geo->indices-startv, geo->indexarray+startv, GL_STATIC_DRAW);
		geo->laststartv = startv;
	}

	for( i = 0; i < parent->setvps; i++ )
	{
		int * vpedges = parent->vpedges[i];
		glUniformMatrix4fv( vmatpos, 1, 0, parent->vpviews[i] );
		glUniformMatrix4fv( pmatpos, 1, 0, parent->vpperspectives[i] );
		glViewport(vpedges[0],  vpedges[1], vpedges[2], vpedges[3]); 
		glDrawElements(geo->render_type, (numv==-1)?geo->indices:numv, GL_UNSIGNED_SHORT, 0);
	}

	uint16_t SpreadGeoInfo[3+32];
	SpreadGeoInfo[0] = htons( geo->geo_in_parent );
	SpreadGeoInfo[1] = htons( startv );
	SpreadGeoInfo[2] = htons( numv );
	memcpy( &SpreadGeoInfo[3], modelmatrix, sizeof( float ) * 16 );
	SpreadPushMessage(geo->parent, 89, sizeof(SpreadGeoInfo), SpreadGeoInfo );
}


void SpreadFreeGeometry( SpreadGeometry * geo )
{
	if( geo->strides ) free( geo->strides );
	if( geo->types ) free( geo->types );
	if( geo->geoname ) free( geo->geoname );
	geo->geoname = 0;

	int i;
	for( i = 0; i < geo->numarrays; i++ )
	{
		free( geo->arrays[i] );
		SpreadHashRemove( geo->parent, "geodata#_#", geo->geo_in_parent, i );
	}

	SpreadMessage( geo->parent, "geometry#", "bb", geo->geo_in_parent, 90, geo->geo_in_parent );
	SpreadHashRemove( geo->parent, "geometry#", geo->geo_in_parent );
}

static const int chanmode[] = { 0, GL_RED, GL_RG, GL_RGB, GL_RGBA };

SpreadTexture * SpreadCreateTexture( Spreadgine * spr, const char * texname, int w, int h, int chan, int mode )
{
	int i;

	if( mode != GL_FLOAT && mode != GL_UNSIGNED_BYTE )
	{
		fprintf( stderr, "Error: mode must be GL_FLOAT or GL_UNSIGNED_BYTE\n" );
		return 0;
	}
	if( chan < 1 || chan > 4 )
	{
		fprintf( stderr, "Error: chan must be between 1 and 4\n" );
		return 0;
	}

	SpreadTexture * ret;

	//First see if there are any free textures available in the parent...
	for( i = 0; i < spr->settexs; i++ )
	{
		if( spr->textures[i]->texname == 0 )
			break;
	}
	if( i == spr->settexs )
	{
		spr->textures = realloc( spr->textures, (spr->settexs+1)* sizeof( SpreadTexture * ) );
		i = spr->settexs;
		spr->settexs++;
		ret = spr->textures[i] = calloc( sizeof( SpreadTexture ), 1 );
	}
	else
	{
		ret = spr->textures[i];
	}

	ret->w = w;
	ret->h = h;
	ret->parent = spr;
	ret->texture_in_parent = i;
	ret->channels = chan;
	ret->type = mode;
	ret->texname = strdup( texname );
	int pxsiz = ret->pixwid = chan*((mode==GL_FLOAT)?4:1);
	ret->pixeldata = calloc( w*h,pxsiz );
	ret->min_lin = 0;
	ret->mag_lin = 0;
	ret->clamp = 0;

	glGenTextures(1, &ret->textureID);
	glBindTexture(GL_TEXTURE_2D, ret->textureID);
	glTexImage2D( GL_TEXTURE_2D, 0, chanmode[chan], w, h, 0, chanmode[chan], mode, ret->pixeldata );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	SpreadMessage( ret->parent, "-texture#", "bbsiiii", ret->texture_in_parent, 97, ret->texture_in_parent, ret->texname, ret->type, ret->channels, ret->w, ret->h);



	return ret;
}

void SpreadChangeTextureProperties( SpreadTexture * tex, int min_lin, int mag_lin, int clamp, int max_miplevel )
{
	glBindTexture(GL_TEXTURE_2D, tex->textureID);

	if( min_lin < 0 || min_lin > 2 ) min_lin = 0;
	if( mag_lin < 0 || mag_lin > 2 ) mag_lin = 0;
	int mmmode[3] = { GL_NEAREST, GL_LINEAR, GL_NEAREST_MIPMAP_NEAREST };
	int mamode[3] = { GL_NEAREST, GL_LINEAR, GL_LINEAR };

	tex->min_lin = min_lin;
	tex->mag_lin = mag_lin;
	tex->clamp = clamp;
#ifdef RASPI_GPU
#define GL_GENERATE_MIPMAP 0x8191
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#endif
	if( min_lin == 2 )
	{
//#ifndef RASPI_GPU
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE ); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, max_miplevel);
//#endif
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mamode[mag_lin] );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mmmode[min_lin] );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp?GL_CLAMP_TO_EDGE:GL_REPEAT);
 
	SpreadMessage( tex->parent, "texture#props", "bbiiii", tex->texture_in_parent, 96, tex->texture_in_parent,  min_lin, mag_lin, clamp, max_miplevel);
}


void SpreadUpdateSubTexture( SpreadTexture * tex, void * texdat, int x, int y, int w, int h )
{
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, w, h, chanmode[tex->channels], tex->type, texdat );
	int csz = tex->pixwid;
	int l;
	//Copy data one line-at-a-time.
	for( l = 0; l < h; l++ )
	{
		memcpy( tex->pixeldata + csz*x + tex->w*csz * ( l + y ), ((uint8_t*)texdat) + l * w * csz, w * csz ); 
	}

	if( x == 0 && y == 0 )
	{
		//Is the whole image... Need to store it.
		SpreadMessage( tex->parent, "+texture#data", "bbiiiivb", tex->texture_in_parent, 98, tex->texture_in_parent, x, y, w, h, w*h*csz, texdat, 0 );
	}
	else
	{
		if( w*h*csz < 65500 )
		{
			SpreadMessage( tex->parent, 0, "bbiiiivb", 98, tex->texture_in_parent, x, y, w, h, w*h*csz, texdat, 0 );
		}
	}

#if 0
	uint8_t * mapin = texdat;
	if( tex->minmag_lin == 2 )
	{
		//glGenerateMipmap( GL_TEXTURE_2D );
		//Cook up some mipmaps.  (Old GL systems need this code)
		int level = 1;
		int newx = x / 2;
		int newy = y / 2;
		int neww = w / 2;
		int newh = h / 2;
		while( neww && newh )
		{
			uint8_t * mapout = malloc( neww*newh*csz );
			int lx, ly, c;
			for( ly = 0; ly < newh; ly++ )
			for( lx = 0; lx < neww; lx++ )
			{
				int mlt = 0;
				for( c = 0; c < csz; c++ )
				{
					mlt += ((uint8_t*)mapin)[(lx*2+ly*2*w)*csz + c];
					mlt += ((uint8_t*)mapin)[(lx*2+ly*2*w+1)*csz + c];
					mlt += ((uint8_t*)mapin)[(lx*2+(ly*2+1)*w)*csz + c];
					mlt += ((uint8_t*)mapin)[(lx*2+(ly*2+1)*w+1)*csz + c];
					mapout[(lx+ly*neww)*csz+c] = (mlt+2)/4;
				}
			}

			//memcpy( tex->pixeldata + csz*x + tex->w*csz * ( l + y ), ((uint8_t*)texdat) + l * w * csz, w * csz ); 
			glTexSubImage2D( GL_TEXTURE_2D, level++, newx, newy, neww, newh, chanmode[tex->channels], tex->type, mapout );
			printf( "Adding %d: %d %d %d %d %p\n", level, newx, newy, neww, newh, mapout );
			x = newx;
			y = newy;
			w = neww;
			h = newh;
			newx /= 2;
			newy /= 2;
			neww /= 2;
			newh /= 2;
			if( mapin != texdat ) free( mapin );
			mapin = mapout;
		}
		if( mapin != texdat ) free( mapin );

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
	}
#endif
}


void SpreadApplyTexture( SpreadTexture * tex, int slot )
{
//	glEnable(GL_TEXTURE_2D);
	glActiveTexture( GL_TEXTURE0 + slot );
	glBindTexture(GL_TEXTURE_2D, tex->textureID);
	SpreadMessage( tex->parent, 0, "bbb", 99, tex->texture_in_parent, slot );
}

void SpreadFreeTexture( SpreadTexture * tex )
{
	SpreadMessage( tex->parent, "texture#", "b", tex->texture_in_parent, 100 );
	SpreadHashRemove( tex->parent, "texture#", tex->texture_in_parent );
	glDeleteTextures( 1, &tex->textureID );
}





