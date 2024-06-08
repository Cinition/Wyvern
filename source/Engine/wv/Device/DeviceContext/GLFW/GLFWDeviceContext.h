#pragma once

#include <GLFW/glfw3.h>
#include <wv/Types.h>

#include <wv/Device/DeviceContext.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////

namespace wv
{

///////////////////////////////////////////////////////////////////////////////////////

	class GLFWDeviceContext : public DeviceContext
	{

	public:

		void terminate() override;

		GraphicsDriverLoadProc getLoadProc() override;

		void pollEvents() override;
		void swapBuffers() override;

		void onResize( int _width, int _height ) override;

		void setMouseLock( bool _lock ) override;
		void setTitle( const char* _title ) override;

///////////////////////////////////////////////////////////////////////////////////////

	protected:
		friend class DeviceContext;
		GLFWDeviceContext( ContextDesc* _desc );
		
		GLFWwindow* m_windowContext;

	};
}