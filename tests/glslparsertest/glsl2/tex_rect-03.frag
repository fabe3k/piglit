/* PASS */
#ifndef GL_ARB_texture_rectangle
/* Since there is no #error in GLSL, this is a sure way to generate an
 * error from the preprocessor.
 */
#extension FAIL: require
#endif

void main() { }
