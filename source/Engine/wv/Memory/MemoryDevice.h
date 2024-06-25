#pragma once

#include <stdint.h>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////

namespace wv
{

///////////////////////////////////////////////////////////////////////////////////////

	class Mesh;
	class Pipeline;

///////////////////////////////////////////////////////////////////////////////////////

	struct Memory
	{
		uint8_t* data = nullptr;
		unsigned int size = 0;
	};

///////////////////////////////////////////////////////////////////////////////////////

	struct TextureMemory : public Memory
	{
		int width = 0;
		int height = 0;
		int numChannels = 0;
	};

///////////////////////////////////////////////////////////////////////////////////////

	class cFileSystem
	{

	public:

		~cFileSystem();

		Memory* loadMemory( const std::string& _path );
		void unloadMemory( Memory* _memory );

		std::string loadString( const std::string& _path );

		TextureMemory* loadTextureData( const std::string& _path );
		
		bool fileExists( const std::string& _path );

///////////////////////////////////////////////////////////////////////////////////////

	private:

		std::vector<Memory*> m_loadedMemory;

	};

}