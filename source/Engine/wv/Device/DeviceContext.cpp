#include "DeviceContext.h"

#include <wv/Device/DeviceContext/GLFW/GLFWDeviceContext.h>
#include <wv/Device/DeviceContext/SDL/SDLDeviceContext.h>

///////////////////////////////////////////////////////////////////////////////////////

wv::DeviceContext* wv::DeviceContext::getDeviceContext( ContextDesc* _desc )
{
	switch ( _desc->deviceApi )
	{
	case WV_DEVICE_CONTEXT_API_GLFW: return new GLFWDeviceContext( _desc );
	case WV_DEVICE_CONTEXT_API_SDL:  return new SDLDeviceContext( _desc );
	}

	return nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::DeviceContext::onResize( int _width, int _height )
{
	m_width  = _width;
	m_height = _height;
}
