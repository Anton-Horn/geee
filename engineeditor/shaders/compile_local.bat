glslc.exe -fshader-stage=vert VertexShader.glsl -o VertexShader.spv
glslc.exe -fshader-stage=frag FragmentShader.glsl -o FragmentShader.spv
glslc.exe -fshader-stage=vert bezier_vertex.glsl -o bezier_vertex.spv
glslc.exe -fshader-stage=frag bezier_fragment.glsl -o bezier_fragment.spv
glslc.exe -fshader-stage=vert gooch_vertex.glsl -o gooch_vertex.spv
glslc.exe -fshader-stage=frag gooch_fragment.glsl -o gooch_fragment.spv
glslc.exe -fshader-stage=vert mandelbrotset_vertex.glsl -o mandelbrotset_vertex.spv
glslc.exe -fshader-stage=frag mandelbrotset_fragment.glsl -o mandelbrotset_fragment.spv

PAUSE