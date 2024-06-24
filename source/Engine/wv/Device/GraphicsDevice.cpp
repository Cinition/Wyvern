#include "GraphicsDevice.h"

#include <wv/Assets/Texture.h>
#include <wv/Assets/Materials/Material.h>
#include <wv/Debug/Print.h>
#include <wv/Decl.h>
#include <wv/Memory/MemoryDevice.h>
#include <wv/Pipeline/Pipeline.h>
#include <wv/Primitive/Mesh.h>
#include <wv/Primitive/Primitive.h>
#include <wv/RenderTarget/RenderTarget.h>

#include <glad/glad.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////

wv::GraphicsDevice::GraphicsDevice( GraphicsDeviceDesc* _desc )
{
	/// TODO: make configurable

	m_graphicsApi = _desc->graphicsApi;
	m_graphicsApiVersion = _desc->graphicsApiVersion;

	int initRes = 0;
	switch ( _desc->graphicsApi )
	{
	case WV_GRAPHICS_API_OPENGL:     initRes = gladLoadGLLoader( _desc->loadProc ); break;
	case WV_GRAPHICS_API_OPENGL_ES1: initRes = gladLoadGLES1Loader( _desc->loadProc ); break;
	case WV_GRAPHICS_API_OPENGL_ES2: initRes = gladLoadGLES2Loader( _desc->loadProc ); break;
	}

	if ( !initRes )
	{
		Debug::Print( Debug::WV_PRINT_FATAL, "Failed to initialize Graphics Device\n" );
		return;
	}

	Debug::Print( Debug::WV_PRINT_INFO, "Intialized Graphics Device\n" );
	Debug::Print( Debug::WV_PRINT_INFO, "  %s\n", glGetString( GL_VERSION ) );

	/// TEMPORARY---
	/// TODO: add to pipeline configuration
	//glEnable( GL_MULTISAMPLE );
	//glEnable( GL_BLEND );
	//glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE );
	/// ---TEMPORARY
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::terminate()
{

}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::onResize( int _width, int _height )
{
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::setViewport( int _width, int _height )
{
	glViewport( 0, 0, _width, _height );
}

///////////////////////////////////////////////////////////////////////////////////////

wv::RenderTarget* wv::GraphicsDevice::createRenderTarget( RenderTargetDesc* _desc )
{
	RenderTarget* target = new RenderTarget();
	
	glGenFramebuffers( 1, &target->fbHandle );
	glBindFramebuffer( GL_FRAMEBUFFER, target->fbHandle );

	target->numTextures = _desc->numTextures;
	GLenum* drawBuffers = new GLenum[ _desc->numTextures ];
	target->textures = new Texture * [ _desc->numTextures ];
	for ( int i = 0; i < _desc->numTextures; i++ )
	{
		_desc->textureDescs[ i ].width = _desc->width;
		_desc->textureDescs[ i ].height = _desc->height;

		target->textures[ i ] = createTexture( &_desc->textureDescs[ i ] );
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, target->textures[ i ]->handle, 0 );

		drawBuffers[ i ] = GL_COLOR_ATTACHMENT0 + i;
	}

	glGenRenderbuffers( 1, &target->rbHandle );
	glBindRenderbuffer( GL_RENDERBUFFER, target->rbHandle );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _desc->width, _desc->height );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target->rbHandle );
	
	glDrawBuffers( _desc->numTextures, drawBuffers );
	delete[] drawBuffers;
#ifndef WV_PACKAGE
	int errcode = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( errcode != GL_FRAMEBUFFER_COMPLETE )
	{
		const char* err = "";
		switch ( errcode )
		{
		case GL_FRAMEBUFFER_UNDEFINED:                     err = "GL_FRAMEBUFFER_UNDEFINED"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         err = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: err = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        err = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        err = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
		case GL_FRAMEBUFFER_UNSUPPORTED:                   err = "GL_FRAMEBUFFER_UNSUPPORTED"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        err = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"; break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      err = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
		case GL_INVALID_ENUM:                              err = "GL_INVALID_ENUM "; break;
		case GL_INVALID_OPERATION:                         err = "GL_INVALID_OPERATION "; break;
		}

		Debug::Print( Debug::WV_PRINT_ERROR, "Failed to create RenderTarget\n" );
		Debug::Print( Debug::WV_PRINT_ERROR, "  %s\n", err );

		destroyRenderTarget( &target );
		delete target;
		return nullptr;
	}
#endif
	target->width = _desc->width;
	target->height = _desc->height;

	glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return target;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::destroyRenderTarget( RenderTarget** _renderTarget )
{
	RenderTarget* rt = *_renderTarget;

	glDeleteFramebuffers( 1, &rt->fbHandle );
	glDeleteRenderbuffers( 1, &rt->rbHandle );

	for ( int i = 0; i < rt->numTextures; i++ )
		glDeleteTextures( 1, &rt->textures[ i ]->handle );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::setRenderTarget( RenderTarget* _target )
{
	unsigned int handle = 0;
	if ( _target )
		handle = _target->fbHandle;

	glBindFramebuffer( GL_FRAMEBUFFER, handle );
	if ( _target )
		glViewport( 0, 0, _target->width, _target->height );
	else
		glViewport( 0, 0, 640, 480 );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::setClearColor( const wv::Color& _color )
{
	glClearColor( _color.r, _color.g, _color.b, _color.a );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::clearRenderTarget( bool _color, bool _depth )
{
	glClear( (GL_COLOR_BUFFER_BIT * _color) | (GL_DEPTH_BUFFER_BIT * _depth) );
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Pipeline* wv::GraphicsDevice::createPipeline( PipelineDesc* _desc )
{
	wv::Pipeline* pipeline = new wv::Pipeline();

	if ( _desc->name != "" )
	{
		if ( m_pipelines.count( _desc->name ) )
			return m_pipelines[ _desc->name ];
	}

	switch ( _desc->type )
	{
	case WV_PIPELINE_GRAPHICS:
	{
		std::vector<wv::Handle> shaders;

		// create shaders
		for ( int i = 0; i < (int)_desc->shaders.size(); i++ )
		{
			wv::Handle shader = createShader( &_desc->shaders[ i ] );
			if ( shader != 0 )
				shaders.push_back( shader );
		}

		// create program
		{
			wv::ShaderProgramDesc programDesc;
			programDesc.shaders = shaders.data();
			programDesc.numShaders = (unsigned int)_desc->shaders.size();
			pipeline->program = createProgram( &programDesc );
		}

		// clean up shaders
		for ( int i = 0; i < (int)_desc->shaders.size(); i++ )
			glDeleteShader( shaders[ i ] );
		
		for ( int i = 0; i < (int)_desc->uniformBlocks.size(); i++ )
			createUniformBlock( pipeline, &_desc->uniformBlocks[ i ] );

		glUseProgram( pipeline->program );

		// required for OpenGL < 4.2
		// could probably be skipped for OpenGL >= 4.2 but would require layout(binding=i) in the shader source
		for ( int i = 0; i < (int)_desc->textureUniforms.size(); i++ )
		{
			unsigned int loc = glGetUniformLocation( pipeline->program, _desc->textureUniforms[ i ].name.c_str() );
			
			glUniform1i( loc, _desc->textureUniforms[ i ].index );
		}

		glUseProgram( 0 );

	} break;
	}

	glUseProgram( 0 );

	if ( _desc->name != "" )
		m_pipelines[ _desc->name ] = pipeline;
	
	Debug::Print( Debug::WV_PRINT_INFO, "Created pipeline '%s'\n", _desc->name.c_str() );

	return pipeline;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::destroyPipeline( Pipeline** _pipeline )
{
	
	std::string key = "";
	for ( auto& p : m_pipelines )
	{
		if ( p.second == *_pipeline )
		{
			key = p.first;
			break;
		}
	}
	if( key != "" )
		m_pipelines.erase( key );

	Debug::Print( Debug::WV_PRINT_DEBUG, "Destroyed texture '%s'\n", key.c_str() );

	glDeleteProgram( ( *_pipeline )->program );

	delete* _pipeline;
	*_pipeline = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Pipeline* wv::GraphicsDevice::getPipeline( const char* _name )
{
	if ( !m_pipelines.count( _name ) )
		return nullptr;
	
	return m_pipelines[ _name ];
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Mesh* wv::GraphicsDevice::createMesh( MeshDesc* _desc )
{
	Mesh* mesh = new Mesh();
	/// TODO: remove?
	return mesh;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::destroyMesh( Mesh** _mesh )
{
	Debug::Print( Debug::WV_PRINT_DEBUG, "Destroyed mesh\n" );

	Mesh* mesh = *_mesh;
	for ( int i = 0; i < mesh->primitives.size(); i++ )
		destroyPrimitive( &mesh->primitives[ i ] );
	mesh->primitives.clear();
	*_mesh = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::setActivePipeline( Pipeline* _pipeline )
{
	if ( _pipeline == m_activePipeline )
		return;
	if ( m_activePipeline && _pipeline->program == m_activePipeline->program )
		return;


	glUseProgram( _pipeline->program );
	m_activePipeline = _pipeline;
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Primitive* wv::GraphicsDevice::createPrimitive( PrimitiveDesc* _desc, Mesh* _mesh )
{
	Primitive* primitive = new Primitive();
	glGenVertexArrays( 1, &primitive->vaoHandle );
	glBindVertexArray( primitive->vaoHandle );
	
	// create vertex buffer object
	glGenBuffers( 1, &primitive->vboHandle );
	glBindBuffer( GL_ARRAY_BUFFER, primitive->vboHandle );

	GLenum usage = GL_NONE;
	switch ( _desc->type ) /// TODO: more primitive types
	{
	case wv::WV_PRIMITIVE_TYPE_STATIC: usage = GL_STATIC_DRAW; break;
	}

	// buffer data
	glBufferData( GL_ARRAY_BUFFER, _desc->vertexBufferSize, _desc->vertexBuffer, usage );


	if ( _desc->indexBufferSize > 0 )
	{
		// create element buffer object
		primitive->drawType = WV_PRIMITIVE_DRAW_TYPE_INDICES;

		wv::Handle ebo;
		glGenBuffers( 1, &ebo );
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, _desc->indexBufferSize, _desc->indexBuffer, usage );

		primitive->eboHandle = ebo;
		primitive->numIndices = _desc->numIndices;
	}
	else
	{
		primitive->drawType = WV_PRIMITIVE_DRAW_TYPE_VERTICES;
	}



	int offset = 0;
	int stride = 0;
	for ( unsigned int i = 0; i < _desc->layout->numElements; i++ )
	{
		InputLayoutElement& element = _desc->layout->elements[ i ];
		stride += element.size;
	}

	for ( unsigned int i = 0; i < _desc->layout->numElements; i++ )
	{
		InputLayoutElement& element = _desc->layout->elements[ i ];

		GLenum type = GL_FLOAT;
		switch ( _desc->layout->elements[ i ].type )
		{
		case WV_BYTE:           type = GL_BYTE;           break;
		case WV_UNSIGNED_BYTE:  type = GL_UNSIGNED_BYTE;  break;
		case WV_SHORT:          type = GL_SHORT;          break;
		case WV_UNSIGNED_SHORT: type = GL_UNSIGNED_SHORT; break;
		case WV_INT:            type = GL_INT;            break;
		case WV_UNSIGNED_INT:   type = GL_UNSIGNED_INT;   break;
		case WV_FLOAT:          type = GL_FLOAT;          break;
		#ifndef EMSCRIPTEN // WebGL does not support GL_DOUBLE
		case WV_DOUBLE:         type = GL_DOUBLE; break;
		#endif
		}

		glVertexAttribPointer( i, element.num, type, element.normalized, stride, VPTRi32( offset ) );
		glEnableVertexAttribArray( i );

		offset += element.size;
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	primitive->numVertices = _desc->numVertices; 
	primitive->stride = stride;
	
	_mesh->primitives.push_back( primitive );

	return primitive;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::destroyPrimitive( Primitive** _primitive )
{
	Primitive* pr = *_primitive;
	glDeleteBuffers( 1, &pr->eboHandle );
	glDeleteBuffers( 1, &pr->vboHandle );
	glDeleteVertexArrays( 1, &pr->vaoHandle );
	delete pr;
	*_primitive = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Texture* wv::GraphicsDevice::createTexture( TextureDesc* _desc )
{
	Texture* texture = new Texture();

	GLenum internalFormat = GL_NONE;
	GLenum format = GL_NONE;

	unsigned char* data = nullptr;
	if ( _desc->memory )
	{
		data = _desc->memory->data;
		_desc->width = _desc->memory->width;
		_desc->height = _desc->memory->height;
		_desc->channels = static_cast<wv::TextureChannels>( _desc->memory->numChannels );
	}

	switch ( _desc->channels )
	{
	case wv::WV_TEXTURE_CHANNELS_R:
		format = GL_RED;
		switch ( _desc->format )
		{
		case wv::WV_TEXTURE_FORMAT_BYTE:  internalFormat = GL_R8;  break;
		case wv::WV_TEXTURE_FORMAT_FLOAT: internalFormat = GL_R32F; break;
		case wv::WV_TEXTURE_FORMAT_INT:   internalFormat = GL_R32I; format = GL_RED_INTEGER; break;
		}
		break;
	case wv::WV_TEXTURE_CHANNELS_RG:
		format = GL_RG;
		switch ( _desc->format )
		{
		case wv::WV_TEXTURE_FORMAT_BYTE:  internalFormat = GL_RG8;    break;
		case wv::WV_TEXTURE_FORMAT_FLOAT: internalFormat = GL_RG32F; break;
		case wv::WV_TEXTURE_FORMAT_INT:   internalFormat = GL_RG32I; format = GL_RG_INTEGER; break;
		}
		break;
	case wv::WV_TEXTURE_CHANNELS_RGB:
		format = GL_RGB;
		glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // 3 (channels) is not divisible by 4. Set pixel alignment to 1
		switch ( _desc->format )
		{
		case wv::WV_TEXTURE_FORMAT_BYTE:  internalFormat = GL_RGB8;   break;
		case wv::WV_TEXTURE_FORMAT_FLOAT: internalFormat = GL_RGB32F; break;
		case wv::WV_TEXTURE_FORMAT_INT:   internalFormat = GL_RGB32I; format = GL_RGB_INTEGER; break;
		}
		break;
	case wv::WV_TEXTURE_CHANNELS_RGBA:
		format = GL_RGBA;
		switch ( _desc->format )
		{
		case wv::WV_TEXTURE_FORMAT_BYTE:  internalFormat = GL_RGBA8;    break;
		case wv::WV_TEXTURE_FORMAT_FLOAT: internalFormat = GL_RGBA32F; break;
		case wv::WV_TEXTURE_FORMAT_INT:   internalFormat = GL_RGBA32I; format = GL_RGBA_INTEGER; break;
		}
		break;
	}

	glGenTextures( 1, &texture->handle );
	glBindTexture( GL_TEXTURE_2D, texture->handle );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	GLenum filter = GL_NONE;
	switch ( _desc->filtering )
	{
	case WV_TEXTURE_FILTER_NEAREST: filter = GL_NEAREST; break;
	case WV_TEXTURE_FILTER_LINEAR:  filter = GL_LINEAR; break;
	}
	
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
	
	GLenum type = GL_UNSIGNED_BYTE;
	switch ( _desc->format )
	{
	case wv::WV_TEXTURE_FORMAT_FLOAT: type = GL_FLOAT; break;
	case wv::WV_TEXTURE_FORMAT_INT:   type = GL_INT; break;
	}

	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, _desc->width, _desc->height, 0, format, type, data );
	

	if( _desc->generateMipMaps )
		glGenerateMipmap( GL_TEXTURE_2D );

	GLenum err;
	while ( ( err = glGetError() ) != GL_NO_ERROR )
	{
		Debug::Print( Debug::WV_PRINT_ERROR, "Failed to create Texture:\n" );
		switch ( err )
		{
		case GL_INVALID_ENUM:      Debug::Print( "  GL_INVALID_ENUM" ); break;
		case GL_INVALID_VALUE:     Debug::Print( "  GL_INVALID_VALUE" ); break;
		case GL_INVALID_OPERATION: Debug::Print( "  GL_INVALID_OPERATION" ); break;
		}
	}

	texture->width  = _desc->width;
	texture->height = _desc->height;
	return texture;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::destroyTexture( Texture** _texture )
{
	Debug::Print( Debug::WV_PRINT_DEBUG, "Destroyed texture\n" );

	glDeleteTextures( 1, &( *_texture )->handle );
	delete *_texture;
	*_texture = nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::bindTextureToSlot( Texture* _texture, unsigned int _slot )
{
	/// TODO: some cleaner way of checking version/supported features
	if ( m_graphicsApiVersion.major == 4 && m_graphicsApiVersion.minor >= 5 ) // if OpenGL 4.5 or higher
	{
		glBindTextureUnit( _slot, _texture->handle );
	}
	else 
	{
		glActiveTexture( GL_TEXTURE0 + _slot );
		glBindTexture( GL_TEXTURE_2D, _texture->handle );
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::draw( Mesh* _mesh )
{
	if ( !_mesh )
		return;

	for ( size_t i = 0; i < _mesh->primitives.size(); i++ )
	{
		if ( _mesh->primitives[ i ]->material )
		{
			_mesh->primitives[ i ]->material->setAsActive( this );
			_mesh->primitives[ i ]->material->instanceCallback( _mesh );
		}

		drawPrimitive( _mesh->primitives[ i ] );
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::drawPrimitive( Primitive* _primitive )
{
	glBindVertexArray( _primitive->vaoHandle );
	
	for ( auto& block : m_activePipeline->uniformBlocks )
	{
		glUniformBlockBinding( m_activePipeline->program, block.second.m_index, block.second.m_bindingIndex );
		glBindBuffer( GL_UNIFORM_BUFFER, block.second.m_bufferHandle );
		glBufferData( GL_UNIFORM_BUFFER, block.second.m_bufferSize, block.second.m_buffer, GL_DYNAMIC_DRAW );
	}

	/// TODO: change GL_TRIANGLES
	if ( _primitive->drawType == WV_PRIMITIVE_DRAW_TYPE_INDICES )
		glDrawElements( GL_TRIANGLES, _primitive->numIndices, GL_UNSIGNED_INT, 0 );
	else
	{ 
	#ifndef EMSCRIPTEN
		/// this does not work on WebGL
		glBindVertexBuffer( 0, _primitive->vboHandle, 0, _primitive->stride );
		glDrawArrays( GL_TRIANGLES, 0, _primitive->numVertices );
	#else
		Debug::Print( Debug::WV_PRINT_FATAL, "glBindVertexBuffer is not supported on WebGL. Index Buffer is required\n" );
	#endif
	}
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Handle wv::GraphicsDevice::createShader( ShaderSource* _desc )
{
	int  success;
	char infoLog[ 512 ];

	MemoryDevice md;
	std::string source = md.loadString( _desc->path.c_str() );
	if ( source == "" )
	{
		Debug::Print( Debug::WV_PRINT_ERROR, "Could not find shader source\n" );
		return 0;
	}

	// GLSL specification (chapter 3.3) requires that #version be the first thing in a shader source
	// therefore #if GL_ES cannot be used in the shader itself
#ifdef EMSCRIPTEN
	source = "#version 300 es\n" + source;
#else
	source = "#version 460 core\n" + source;
#endif
	const char* sourcePtr = source.c_str();

	GLenum type = GL_NONE;
	{
		switch ( _desc->type )
		{
		case wv::WV_SHADER_TYPE_FRAGMENT: type = GL_FRAGMENT_SHADER; break;
		case wv::WV_SHADER_TYPE_VERTEX: type = GL_VERTEX_SHADER; break;
		}
	}

	wv::Handle shader;
	shader = glCreateShader( type );
	glShaderSource( shader, 1, &sourcePtr, NULL );
	glCompileShader( shader );

	glGetShaderiv( shader, GL_COMPILE_STATUS, &success );
	if ( !success )
	{
		glGetShaderInfoLog( shader, 512, NULL, infoLog );
		Debug::Print( Debug::WV_PRINT_ERROR, "Failed to compile shader '%s'\n %s \n", _desc->path.c_str(), infoLog);
	}

	return shader;
}

///////////////////////////////////////////////////////////////////////////////////////

wv::Handle wv::GraphicsDevice::createProgram( ShaderProgramDesc* _desc )
{
	int  success;
	char infoLog[ 512 ];

	wv::Handle program;
	program = glCreateProgram();

	for ( int i = 0; i < _desc->numShaders; i++ )
		glAttachShader( program, _desc->shaders[ i ] );

	glLinkProgram( program );

	glGetProgramiv( program, GL_LINK_STATUS, &success );
	if ( !success )
	{
		glGetProgramInfoLog( program, 512, NULL, infoLog );
		Debug::Print( Debug::WV_PRINT_ERROR, "Failed to create program\n %s \n", infoLog );
	}

	return program;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::GraphicsDevice::createUniformBlock( Pipeline* _pipeline, UniformBlockDesc* _desc )
{
	UniformBlock block;

	block.m_bindingIndex = m_numTotalUniformBlocks;
	
	block.m_index = glGetUniformBlockIndex( _pipeline->program, _desc->name.c_str() );;
	glGetActiveUniformBlockiv( _pipeline->program, block.m_index, GL_UNIFORM_BLOCK_DATA_SIZE, &block.m_bufferSize );

	block.m_buffer = new char[ block.m_bufferSize ];
	memset( block.m_buffer, 0, block.m_bufferSize );
	
	glGenBuffers( 1, &block.m_bufferHandle );
	glBindBuffer( GL_UNIFORM_BUFFER, block.m_bufferHandle );
	glBufferData( GL_UNIFORM_BUFFER, block.m_bufferSize, 0, GL_DYNAMIC_DRAW );
	glBindBuffer( GL_UNIFORM_BUFFER, 0 );
	
	glBindBufferBase( GL_UNIFORM_BUFFER, block.m_bindingIndex, block.m_bufferHandle );

	std::vector<unsigned int> indices( _desc->uniforms.size() );
	std::vector<int> offsets( _desc->uniforms.size() );
	
	std::vector<const char*> uniformNames;
	for ( size_t i = 0; i < _desc->uniforms.size(); i++ )
		uniformNames.push_back( _desc->uniforms[ i ].name.c_str() ); // lifetime issues?
	
	glGetUniformIndices( _pipeline->program, (GLsizei)_desc->uniforms.size(), uniformNames.data(), indices.data());
	glGetActiveUniformsiv( _pipeline->program, (GLsizei)_desc->uniforms.size(), indices.data(), GL_UNIFORM_OFFSET, offsets.data());

	for ( int o = 0; o < _desc->uniforms.size(); o++ )
	{
		Uniform u;
		u.index = indices[ o ];
		u.offset = offsets[ o ];
		u.name = _desc->uniforms[ o ].name;

		block.m_uniforms[ u.name ] = u;
	}

	_pipeline->uniformBlocks[ _desc->name ] = block;
	m_numTotalUniformBlocks++;
}
