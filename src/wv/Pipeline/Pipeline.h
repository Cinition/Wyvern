#pragma once

#include <wv/Shader/UniformBlock.h>

namespace wv
{
	struct ShaderSource;

	typedef void( *PipelineUniformCallback )( wv::UniformBlockMap& _uniformBlocks );

	enum PipelineType
	{
		WV_PIPELINE_GRAPHICS
	};

	enum PipelineTopology
	{
		WV_PIPELINE_TOPOLOGY_TRIANGLES
	};

	struct PipelineDesc
	{
		PipelineType type;
		PipelineTopology topology;
		int layout;

		ShaderSource* shaders;
		unsigned int numShaders;

		UniformBlockDesc* uniformBlocks;
		unsigned int numUniformBlocks;

		PipelineUniformCallback pipelineCallback;
		PipelineUniformCallback instanceCallback;
	};


	/// TODO: make not public >:(((
	class Pipeline
	{
	public:
		wv::Handle program;
		uint32_t mode;
		UniformBlockMap uniformBlocks;

		PipelineUniformCallback pipelineCallback;
		PipelineUniformCallback instanceCallback;
	private:

	};
}