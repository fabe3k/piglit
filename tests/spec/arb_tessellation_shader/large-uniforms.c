/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file large-uniforms.c
 *
 * Test that shader with the maximum allowed uniforms links successfully
 * and test link error when using more uniforms slots than allowed.
 *
 * From the ARB_tessellation_shader spec (Sections 2.X.1.1 and 2.X.3.1):
 *
 *    A link error is generated if an attempt is made to utilize more than the
 *    space available for tessellation control shader uniform variables.
 *
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


static const char *const vs_source =
"#version 150\n"
"void main() { gl_Position = vec4(0.0); }\n";

static const char *const tcs_source =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = 3) out;\n"
"void main() {\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"}\n";

static const char *const tcs_source_uniform_array_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = 3) out;\n"
"uniform float large_array[%d];\n"
"void main() {\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"	for (int i = 0; i < large_array.length(); ++i)\n"
"		gl_TessLevelInner[0] += large_array[i];\n"
"}\n";

static const char *const tcs_source_uniform_block_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = 3) out;\n"
"uniform block {\n"
"	float large_array[%d];\n"
"} large_block[%d];\n"
"void main() {\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);\n"
"	gl_TessLevelInner = float[2](1.0, 1.0);\n"
"	for (int i = 0; i < large_block[0].large_array.length(); ++i)\n"
"		gl_TessLevelInner[0] += %s;\n"
"}\n";

static const char *const tes_source =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"void main() { gl_Position = vec4(0.0); }\n";

static const char *const tes_source_uniform_array_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"uniform float large_array[%d];\n"
"void main() {\n"
"	gl_Position = vec4(0.0);\n"
"	for (int i = 0; i < large_array.length(); ++i)\n"
"		gl_Position.x += large_array[i];\n"
"}\n";

static const char *const tes_source_uniform_block_template =
"#version 150\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"uniform block {\n"
"	float large_array[%d];\n"
"} large_block[%d];\n"
"void main() {\n"
"	gl_Position = vec4(0.0);\n"
"	for (int i = 0; i < large_block[0].large_array.length(); ++i)\n"
"		gl_Position.x += %s;\n"
"}\n";

static const char *const fs_source =
"#version 150\n"
"void main() { gl_FragColor = vec4(0.0); }\n";


static bool
test_uniform_array(const GLenum shader, const int n, const bool expect_fail)
{
	bool link_status;
	unsigned int prog;
	char *source_uniforms;
	const char *source_template = (shader == GL_TESS_CONTROL_SHADER) ?
				      tcs_source_uniform_array_template :
				      tes_source_uniform_array_template;

	source_uniforms = malloc(strlen(source_template) + 8);
	sprintf(source_uniforms, source_template, n);

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			shader, source_uniforms,
			//GL_TESS_EVALUATION_SHADER, tes_source,
			GL_FRAGMENT_SHADER, fs_source,
			0);
	glLinkProgram(prog);
	link_status = piglit_link_check_status_quiet(prog);

	if (link_status && expect_fail) {
		fprintf(stderr, "Program with %d uniform components in %s "
			"linked succesfully\n", n,
			piglit_get_gl_enum_name(shader));
		return false;
	}
	if (!link_status && !expect_fail) {
		fprintf(stderr, "Program with %d uniform components in %s "
			"failed to link\n", n,
			piglit_get_gl_enum_name(shader));
		return false;
	}
	glDeleteProgram(prog);

	free(source_uniforms);

	return true;
}


static bool
test_uniform_block(const GLenum shader, const int num_blocks, const int size,
		   const bool expect_fail)
{
	bool link_status;
	unsigned int prog;
	char *source_uniforms;
	const char *source_template = shader == GL_TESS_CONTROL_SHADER ?
				      tcs_source_uniform_block_template :
				      tes_source_uniform_block_template;
	const char *summand_template = " + large_block[%d].large_array[i]";
	char *sum;
	int i;


	/* From the GLSL 1.5 spec (chapter 4.3.7):
	 *
	 *    All indexes used to index a uniform block
	 *    array must be integral constant expressions.
	 *
	 * So we have to generate code from an unrolled loop.
	 */
	sum = malloc((strlen(summand_template) + 8) * num_blocks);
	sum[0] = '\0';
	for(i = 0; i < num_blocks; ++i) {
		char summand[40];
		sprintf(summand, summand_template, i);
		strcat(sum, summand);
	}

	source_uniforms = malloc(strlen(source_template) + 16 + strlen(sum));
	sprintf(source_uniforms, source_template, size, num_blocks, sum);
	free(sum);

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			shader, source_uniforms,
			//GL_TESS_EVALUATION_SHADER, tes_source,
			GL_FRAGMENT_SHADER, fs_source,
			0);
	glLinkProgram(prog);
	link_status = piglit_link_check_status_quiet(prog);

	if (link_status && expect_fail) {
		fprintf(stderr, "Program with %d uniform blocks of size %d "
			"in %s linked succesfully\n", num_blocks, size,
			piglit_get_gl_enum_name(shader));
		return false;
	}
	if (!link_status && !expect_fail) {
		fprintf(stderr, "Program with %d uniform blocks of size %d "
			"in %s failed to link\n", num_blocks, size,
			piglit_get_gl_enum_name(shader));
		return false;
	}
	glDeleteProgram(prog);

	free(source_uniforms);

	return true;
}


static bool
test_shader(const GLenum shader, const int max_uniform_components,
	    const int max_combined_uniform_components,
	    const int max_uniform_blocks)
{
	bool pass = true;
	int max_uniform_block_size;

	/* From the tessellation shader spec (New State section):
	 *
	 *    The minimum values for MAX_COMBINED_*_UNIFORM_COMPONENTS by
	 *    computing the value of:
	 *       MAX_*_UNIFORM_COMPONENTS + MAX_*_UNIFORM_BLOCKS *
	 *       (MAX_UNIFORM_BLOCK_SIZE/4)
	 *    using the minimum values of the corresponding terms.
	 */
	pass = max_combined_uniform_components >=
	       max_uniform_components +
	       max_uniform_blocks *
	       (max_uniform_block_size/4) && pass;

	pass = test_uniform_array(shader, max_uniform_components, false) && pass;
	pass = test_uniform_array(shader, max_uniform_components + 1, true) && pass;

	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &max_uniform_block_size);

	pass = test_uniform_block(shader, max_uniform_blocks, max_uniform_block_size/4, false) && pass;
	pass = test_uniform_block(shader, max_uniform_blocks + 1, max_uniform_block_size/4, true) && pass;
	pass = test_uniform_block(shader, max_uniform_blocks, max_uniform_block_size/4 + 1, true) && pass;

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int max_uniform_components;
	int max_combined_uniform_components;
	int max_uniform_blocks;

	piglit_require_extension("GL_ARB_tessellation_shader");

	glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS,
		      &max_uniform_components);
	glGetIntegerv(GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS,
		      &max_combined_uniform_components);
	glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS,
		      &max_uniform_blocks);

	pass = test_shader(GL_TESS_CONTROL_SHADER, max_uniform_components, max_combined_uniform_components, max_uniform_blocks) && pass;

	glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS,
		      &max_uniform_components);
	glGetIntegerv(GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS,
		      &max_combined_uniform_components);
	glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS,
		      &max_uniform_blocks);

	pass = test_shader(GL_TESS_EVALUATION_SHADER, max_uniform_components, max_combined_uniform_components, max_uniform_blocks) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

