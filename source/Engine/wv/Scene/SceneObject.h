#pragma once

#include <wv/Math/Transform.h>

#include <stdint.h>
#include <vector>
#include <string>

///////////////////////////////////////////////////////////////////////////////////////

namespace wv
{

///////////////////////////////////////////////////////////////////////////////////////

	class iDeviceContext;
	class iGraphicsDevice;

///////////////////////////////////////////////////////////////////////////////////////

	class iSceneObject
	{

	public:

		 iSceneObject( const uint64_t& _uuid, const std::string& _name );
		~iSceneObject();

		void addChild( iSceneObject* _node );
		void removeChild( iSceneObject* _node );
		void moveChild( iSceneObject* _node, iSceneObject* _newParent );

		void onLoad()
		{
			onLoadImpl();
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->onLoad();
		}
		
		void onUnload()
		{
			onUnloadImpl();
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->onUnload();
		}
		
		void onCreate()
		{
			onCreateImpl();
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->onCreate();
		}

		void onDestroy()
		{
			onDestroyImpl();
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->onDestroy();
		}

		void update( double _deltaTime ) 
		{
			updateImpl( _deltaTime );
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->update( _deltaTime );
		}

		void draw( iDeviceContext* _context, iGraphicsDevice* _device ) 
		{
			drawImpl( _context, _device );
			for ( size_t i = 0; i < m_children.size(); i++ )
				m_children[ i ]->draw( _context, _device );
		}

		Transformf m_transform;
		Transformf m_worldTransform;

///////////////////////////////////////////////////////////////////////////////////////

	protected:

		virtual void onLoadImpl() = 0;
		virtual void onUnloadImpl() = 0;

		virtual void onCreateImpl() = 0;
		virtual void onDestroyImpl() = 0;

		virtual void updateImpl( double _deltaTime ) = 0;
		virtual void drawImpl( iDeviceContext* _context, iGraphicsDevice* _device ) = 0;

		uint64_t    m_uuid;
		std::string m_name;
		
		iSceneObject* m_parent = nullptr;
		std::vector<iSceneObject*> m_children{};

	};

}

// REFLECTNODE( INode )