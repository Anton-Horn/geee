glslc.exe -fshader-stage=vert VertexShader.glsl -o VertexShader.spv
glslc.exe -fshader-stage=frag FragmentShader.glsl -o FragmentShader.spv
glslc.exe -fshader-stage=vert bezier_vertex.glsl -o bezier_vertex.spv
glslc.exe -fshader-stage=frag bezier_fragment.glsl -o bezier_fragment.spv

PAUSE