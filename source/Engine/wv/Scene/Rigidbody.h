#pragma once

#include <wv/Scene/SceneObject.h>

#include <wv/Physics/PhysicsBodyDescriptor.h>

#include <wv/Reflection/Reflection.h>

#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////

namespace wv
{

///////////////////////////////////////////////////////////////////////////////////////

	class Mesh; 

///////////////////////////////////////////////////////////////////////////////////////

	class cRigidbody : public iSceneObject
	{
	public:
		 cRigidbody( const UUID& _uuid, const std::string& _name, Mesh* _pMesh, iPhysicsBodyDesc* _bodyDesc );
		 cRigidbody( const UUID& _uuid, const std::string& _name, const std::string& _meshPath, iPhysicsBodyDesc* _bodyDesc );
		~cRigidbody();

		virtual nlohmann::json dataToJson( void ) override;

		static cRigidbody* createInstance();
		static cRigidbody* createInstanceJson( nlohmann::json& _json );

///////////////////////////////////////////////////////////////////////////////////////

	protected:
		void onLoadImpl() override;
		void onUnloadImpl() override;
		void onCreateImpl() override { };
		void onDestroyImpl() override { };

		virtual void updateImpl( double _deltaTime ) override;
		virtual void drawImpl  ( wv::iDeviceContext* _context, wv::iGraphicsDevice* _device ) override;

		wv::Mesh*   m_pMesh    = nullptr;
		std::string m_meshPath = "";

		wv::iPhysicsBodyDesc* m_pPhysicsBodyDesc = nullptr;
		wv::Handle m_physicsBodyHandle = 0;

	};

	REFLECT_CLASS( cRigidbody );
}

