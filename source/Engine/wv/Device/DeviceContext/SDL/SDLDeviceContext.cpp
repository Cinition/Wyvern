#include "SDLDeviceContext.h"

#include <stdio.h>

#include <wv/Application/Application.h>
#include <wv/Events/IInputListener.h>
#include <wv/Events/IMouseListener.h>

#include <wv/Math/Vector2.h>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#include <GLES2/gl2.h>
#endif

#include <wv/Debug/Print.h>

///////////////////////////////////////////////////////////////////////////////////////

void keyCallback( wv::DeviceContext* _device, SDL_KeyboardEvent* _event )
{
	/// TODO: move to application?
	if ( _event->keysym.sym == SDL_KeyCode::SDLK_ESCAPE && _event->type == SDL_KEYDOWN )
		_device->close();

	wv::InputEvent inputEvent;
	inputEvent.buttondown = _event->type == SDL_KEYDOWN;
	inputEvent.buttonup = _event->type == SDL_KEYUP;
	inputEvent.repeat = _event->repeat != 0;
	inputEvent.scancode = _event->keysym.scancode;
	inputEvent.mods = _event->keysym.mod;

	inputEvent.key = _event->keysym.sym;
	if ( inputEvent.key >= 'a' && inputEvent.key <= 'z' )
		inputEvent.key -= 32;

	wv::IInputListener::invoke( inputEvent );
}

///////////////////////////////////////////////////////////////////////////////////////

void mouseCallback( wv::DeviceContext* _device, SDL_MouseMotionEvent* _event )
{
	wv::MouseEvent mouseEvent;
	
	SDL_GetMouseState( &mouseEvent.position.x, &mouseEvent.position.y );
	SDL_GetRelativeMouseState( &mouseEvent.delta.x, &mouseEvent.delta.y );

	wv::IMouseListener::invoke( mouseEvent );
}

///////////////////////////////////////////////////////////////////////////////////////

void mouseButtonCallback( wv::DeviceContext* _device, SDL_MouseButtonEvent* _event )
{
	wv::MouseEvent mouseEvent;

	SDL_GetMouseState( &mouseEvent.position.x, &mouseEvent.position.y );
	SDL_GetRelativeMouseState( &mouseEvent.delta.x, &mouseEvent.delta.y );

	switch ( _event->button )
	{
	case 1: mouseEvent.button = wv::MouseEvent::WV_MOUSE_BUTTON_LEFT;   break;
	case 3: mouseEvent.button = wv::MouseEvent::WV_MOUSE_BUTTON_RIGHT;  break;
	case 2: mouseEvent.button = wv::MouseEvent::WV_MOUSE_BUTTON_MIDDLE; break;
	}

	mouseEvent.buttondown = _event->type == SDL_MOUSEBUTTONDOWN;
	mouseEvent.buttonup   = _event->type == SDL_MOUSEBUTTONUP;

	wv::IMouseListener::invoke( mouseEvent );
}

///////////////////////////////////////////////////////////////////////////////////////

void windowCallback( SDL_Window* _window, SDL_WindowEvent* _event )
{
	switch ( _event->event )
	{
		case SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED: 
		{
			int w, h;
			SDL_GetWindowSize( _window, &w, &h );
			wv::Application::get()->onResize( w, h );
		} break;
	}
}

///////////////////////////////////////////////////////////////////////////////////////

wv::SDLDeviceContext::SDLDeviceContext( ContextDesc* _desc ) : m_windowContext{ nullptr }
{
	// glfwSetErrorCallback( glfwErrorCallback );
	
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		Debug::Print( Debug::WV_PRINT_FATAL, "Failed to initialize Device Context\n" );
		return;
	}

	switch ( _desc->graphicsApi )
	{

	case WV_GRAPHICS_API_OPENGL:
	{
		SDL_GL_LoadLibrary( NULL );
		
		SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, _desc->graphicsApiVersion.major );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, _desc->graphicsApiVersion.minor );

		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	} break; // OpenGL 1.0 - 4.6

	case WV_GRAPHICS_API_OPENGL_ES1: case WV_GRAPHICS_API_OPENGL_ES2:
	{
		SDL_GL_LoadLibrary( NULL );

		SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, _desc->graphicsApiVersion.major );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, _desc->graphicsApiVersion.minor );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );

		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	} break; // OpenGL ES 1 & 2

	}

#ifdef EMSCRIPTEN
	EmscriptenWebGLContextAttributes attrs;
	attrs.antialias = true;
	attrs.majorVersion = 3;
	attrs.minorVersion = 0;
	attrs.alpha = true;
	attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;

	// The following lines must be done in exact order, or it will break!
	emscripten_webgl_init_context_attributes( &attrs ); // you MUST init the attributes before creating the context
	attrs.majorVersion = 3; // you MUST set the version AFTER the above line
	EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webgl_context = emscripten_webgl_create_context( "#canvas", &attrs );
	emscripten_webgl_make_context_current( webgl_context );
#endif

	m_windowContext = SDL_CreateWindow( _desc->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _desc->width, _desc->height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE );
	m_glContext = SDL_GL_CreateContext( m_windowContext );
	
	SDL_version version;
	SDL_GetVersion( &version );
	Debug::Print( Debug::WV_PRINT_INFO, "Initialized Context Device\n" );
	Debug::Print( Debug::WV_PRINT_INFO, "  SDL %i.%i.%i\n", version.major, version.minor, version.patch );


	//glfwSetFramebufferSizeCallback( m_windowContext, onResizeCallback );
	//glfwSetKeyCallback( m_windowContext, keyCallback );
	//
	//glfwSetCursorPosCallback( m_windowContext, mouseCallback );
	//glfwSetMouseButtonCallback( m_windowContext, mouseButtonCallback );

	if ( !m_windowContext )
	{
		Debug::Print( Debug::WV_PRINT_FATAL, "Failed to create Context\n" );
		return;
	}
	// glfwMakeContextCurrent( m_windowContext );

	SDL_GL_SetSwapInterval( 0 );

	SDL_GetWindowSize( m_windowContext, &m_width, &m_height );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::SDLDeviceContext::terminate()
{
	SDL_Quit();
}

///////////////////////////////////////////////////////////////////////////////////////

wv::GraphicsDriverLoadProc wv::SDLDeviceContext::getLoadProc()
{
	return (GraphicsDriverLoadProc)SDL_GL_GetProcAddress;
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::SDLDeviceContext::pollEvents()
{
	// process input
	SDL_Event ev;
	while ( SDL_PollEvent( &ev ) )
	{
		switch ( ev.type )
		{
		case SDL_EventType::SDL_QUIT: m_alive = false; break;
		
		case SDL_EventType::SDL_KEYDOWN: case SDL_EventType::SDL_KEYUP: keyCallback( this, &ev.key ); break;

		case SDL_EventType::SDL_MOUSEBUTTONDOWN: mouseButtonCallback( this, &ev.button ); break;
		case SDL_EventType::SDL_MOUSEBUTTONUP:   mouseButtonCallback( this, &ev.button ); break;
		case SDL_EventType::SDL_MOUSEMOTION:     mouseCallback( this, &ev.motion ); break;

		case SDL_EventType::SDL_WINDOWEVENT: windowCallback( m_windowContext, &ev.window ); break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::SDLDeviceContext::swapBuffers()
{
	// if opengl
	SDL_GL_SwapWindow( m_windowContext );
	
	m_time = SDL_GetTicks64() / 1000.0;

	uint64_t last = m_performanceCounter;
	m_performanceCounter = static_cast<double>( SDL_GetPerformanceCounter() );

	m_deltaTime = (double)( ( m_performanceCounter - last ) / (double)SDL_GetPerformanceFrequency() );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::SDLDeviceContext::onResize( int _width, int _height )
{
	DeviceContext::onResize( _width, _height );
}

///////////////////////////////////////////////////////////////////////////////////////

void wv::SDLDeviceContext::setMouseLock( bool _lock )
{
	SDL_SetRelativeMouseMode( (SDL_bool)_lock );
}

void wv::SDLDeviceContext::setTitle( const char* _title )
{
	SDL_SetWindowTitle( m_windowContext, _title );
}
