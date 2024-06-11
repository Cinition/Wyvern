function load_platform_wasm()
    add_defines("WV_PLATFORM_WASM")
    add_defines("MA_NO_MP3") -- .mp3 audio file decoding seems to cause a stack overflow, so it has been disabled completely
    
    add_requires( "glm", "libsdl", "assimp" )
    
    add_ldflags( "-sALLOW_MEMORY_GROWTH=1" )
    add_ldflags( "-sUSE_SDL=2" )
    -- gles
    add_ldflags( "-sMAX_WEBGL_VERSION=2" )
    add_ldflags( "-sFULL_ES3=1" )
end

function target_platform_wasm( target )
    print( "target WASM" )
    
    -- preload resource files
    target:add( "ldflags", "--embed-file game/res@/res" )

    -- add supports
    target:add( "deps", "GLAD" )
    import(".platform.support.glm"   )(target)
    import(".platform.support.libsdl")(target)

    target:add( "packages", "assimp" )
end