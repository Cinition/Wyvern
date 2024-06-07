#include "MemoryDevice.h"

#include <wv/Application/Application.h>
#include <wv/Device/GraphicsDevice.h>
#include <wv/Primitive/Mesh.h>

#define STB_IMAGE_IMPLEMENTATION
#include <wv/Auxiliary/stb_image.h>

#include <wv/Auxiliary/json.hpp>

#include <fstream>
#include <vector>

wv::Memory wv::MemoryDevice::loadFromFile( const char* _path )
{
	Memory mem;
	
	std::ifstream in( _path, std::ios::binary );
	std::vector<char> buf{ std::istreambuf_iterator<char>( in ), {} };

	mem.data = new unsigned char[ buf.size() ];
	mem.size = static_cast<unsigned int>( buf.size() );

	memcpy( mem.data, buf.data(), buf.size() );

	numLoadedFiles++;
	return mem;
}

void wv::MemoryDevice::freeMemory( Memory* _memory )
{
	if ( !_memory->data )
		return;

	delete _memory->data;
	*_memory = {};
	numLoadedFiles--;
}

wv::TextureMemory wv::MemoryDevice::loadTextureData( const char* _path )
{
	TextureMemory mem;

	numLoadedFiles++;
	stbi_set_flip_vertically_on_load( 0 );
	mem.data = reinterpret_cast<uint8_t*>( stbi_load( _path, &mem.width, &mem.height, &mem.numChannels, 0 ) );

	if ( !mem.data )
	{
		//printf( "Failed to load texture %s\n", _path );
		unloadTextureData( &mem );
		return {}; // empty memory object
	}
	
	printf( "Loaded texture '%s' (%ix%i @ %ibpp)\n", _path, mem.width, mem.height, mem.numChannels * 8 );
	mem.size = mem.height * mem.numChannels * mem.width * mem.numChannels;
	return mem;
}

void wv::MemoryDevice::unloadTextureData( TextureMemory* _memory )
{
	stbi_image_free( _memory->data );
	*_memory = {};
	numLoadedFiles--;
}

wv::Mesh* wv::MemoryDevice::loadModel( const char* _path, bool _binary )
{
	wv::Application* app = wv::Application::get();
	wv::GraphicsDevice* device = app->device;

	Memory gltf = loadFromFile( "res/meshes/cube.gltf" );
	std::string source{ (char*)gltf.data, gltf.size };

	nlohmann::json json = nlohmann::json::parse( source );

	std::vector<Memory> buffers;
	for ( auto& buffer : json["buffers"] )
	{
		std::string path = "res/meshes/" + (std::string)buffer[ "uri" ];
		buffers.push_back( loadFromFile( path.c_str() ) );
	}

	for ( auto& scene : json["scenes"] )
	{
		for ( int node : scene[ "nodes" ] )
		{
			
		}
	}

    return nullptr;
}
