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
 * \file get.c
 *
 * Test limits and default state.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


static bool
test_limits(void)
{
	bool pass = true;
	int i, v;
	static const struct {
		GLenum name;
		int min;
	} limits[] = {
		{GL_MAX_TESS_GEN_LEVEL, 64},
		{GL_MAX_PATCH_VERTICES, 32},
		{GL_MAX_TESS_CONTROL_UNIFORM_COMPONENTS, 1024},
		{GL_MAX_TESS_EVALUATION_UNIFORM_COMPONENTS, 1024},
		{GL_MAX_TESS_CONTROL_TEXTURE_IMAGE_UNITS, 16},
		{GL_MAX_TESS_EVALUATION_TEXTURE_IMAGE_UNITS, 16},
		{GL_MAX_TESS_CONTROL_OUTPUT_COMPONENTS, 128},
		{GL_MAX_TESS_PATCH_COMPONENTS, 120},
		{GL_MAX_TESS_CONTROL_TOTAL_OUTPUT_COMPONENTS, 4096},
		{GL_MAX_TESS_EVALUATION_OUTPUT_COMPONENTS, 128},
		{GL_MAX_TESS_CONTROL_INPUT_COMPONENTS, 128},
		{GL_MAX_TESS_EVALUATION_INPUT_COMPONENTS, 128},
		{GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS, 12},
		{GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS, 12},
		{GL_MAX_COMBINED_TESS_CONTROL_UNIFORM_COMPONENTS, 1024 + 12 * (16384/4)},
		{GL_MAX_COMBINED_TESS_EVALUATION_UNIFORM_COMPONENTS, 1024 + 12 * (16384/4)},
	};

	for (i = 0; i < ARRAY_SIZE(limits); ++i) {
		glGetIntegerv(limits[i].name, &v);
		if (v < limits[i].min) {
			fprintf(stderr, "%s to small: %d < %d\n",
				piglit_get_gl_enum_name(limits[i].name),
				v, limits[i].min);
			pass = false;
		}
	}

	return pass;
}


static bool
test_default_state(void)
{
	bool pass = true;
	int v;
	float f[4];

	glGetIntegerv(GL_PATCH_VERTICES, &v);
	if (v != 3) {
		fprintf(stderr, "default of GL_PATCH_VERTICES wrong: %d "
			"(expected 3)\n", v);
		pass = false;
	}

	memset(f, 0, sizeof(f));
	glGetFloatv(GL_PATCH_DEFAULT_OUTER_LEVEL, f);
	if (f[0] != 1.0 || f[1] != 1.0 || f[2] != 1.0 || f[3] != 1.0) {
		fprintf(stderr, "default of GL_PATCH_DEFAULT_OUTER_LEVEL wrong: "
			"(%f, %f, %f, %f) (expected (1.0, 1.0, 1.0, 1.0))\n",
			f[0], f[1], f[2], f[3]);
		pass = false;
	}

	memset(f, 0, sizeof(f));
	/* Set some random values to check for excess writes. */
	f[2] = 0.125; f[3] = 4000.0;
	glGetFloatv(GL_PATCH_DEFAULT_INNER_LEVEL, f);
	if (f[2] != 0.125 || f[3] != 4000.0) {
		fprintf(stderr, "glGetFloatv(GL_PATCH_DEFAULT_INNER_LEVEL) "
			"returned excess data\n");
		pass = false;
	}
	if (f[0] != 1.0 || f[1] != 1.0) {
		fprintf(stderr, "default of GL_PATCH_DEFAULT_INNER_LEVEL wrong: "
			"(%f, %f) (expected (1.0, 1.0))\n",
			f[0], f[1]);
		pass = false;
	}

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_tessellation_shader");

	pass = test_limits() && pass;

	pass = test_default_state() && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}

