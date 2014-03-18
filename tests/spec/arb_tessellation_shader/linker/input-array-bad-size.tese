// [config]
// expect_result: fail
// glsl_version: 1.50
// check_link: true
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// From the ARB_tessellation_shader spec (Section 2.X.3.2):
//
//  "User-defined per-vertex input varying variables have a value for each
//  vertex and thus need to be declared as arrays or inside input blocks
//  declared as arrays. [...]  If a size is specified, it must match the
//  maximum patch size; otherwise, a link error will occur."
//
// This is only true for per-vertex inputs (as opposed to per-patch inputs).

#version 150
#extension GL_ARB_tessellation_shader: require

layout(triangles) in;

in vec4 wrong_size[3];

void main() {
	gl_Position = vec4(0);
}

