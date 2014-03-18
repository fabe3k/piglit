// [config]
// expect_result: fail
// glsl_version: 1.50
// check_link: true
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// From the ARB_tessellation_shader spec (Section 2.X.1.2):
//
//  "Similarly to the built-in varying variables, each user-defined input
//  varying variable has a value for each vertex and thus needs to be
//  declared as arrays or inside input blocks declared as arrays."

#version 150
#extension GL_ARB_tessellation_shader: require

layout(vertices = 3) out;

in vec4 no_array;

void main() {
	gl_TessLevelOuter = float[4](1, 1, 1, 0);
	gl_TessLevelInner = float[2](0, 0);
}

