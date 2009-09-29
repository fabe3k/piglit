/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file scissor-copypixels.c
 *
 * Tests that glScissor properly affects glCopyPixels().
 */

#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static GLboolean Automatic = GL_FALSE;
static int win_width, win_height;

static GLboolean
check_red_box_surrounded_by_green(int x, int y, int w, int h)
{
	GLboolean pass = GL_TRUE;
	int probe_x, probe_y;
	const float red[] = {1.0, 0.0, 0.0, 0.0};
	const float green[] = {0.0, 1.0, 0.0, 0.0};

	for (probe_y = y - 1; probe_y <= y + h; probe_y++) {
		for (probe_x = x - 1; probe_x <= x + w; probe_x++) {
			const float *expected;

			if (probe_y < y || probe_y >= y + h ||
			    probe_x < x || probe_x >= x + w)
				expected = green;
			else
				expected = red;

			pass &= piglit_probe_pixel_rgb(probe_x, probe_y,
						       expected);
		}
	}

	return pass;
}

static void display(void)
{
	GLboolean pass = GL_TRUE;
	int dst_x = win_width / 2 + 10, dst_y;
	int dst_w = 10, dst_h = 10;

	/* whole window red */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* right half green */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(win_width / 2, 0, win_width / 2, win_height);

	/* Copy a 10x10 square from left to right */
	glEnable(GL_SCISSOR_TEST);
	dst_y = 10;
	glScissor(dst_x, dst_y, dst_w, dst_h);
	glRasterPos2i(dst_x - 5, dst_y - 5);
	glCopyPixels(10, 10, 20, 20, GL_COLOR);

	/* Don't copy a 10x10 square from left to right */
	dst_y = 30;
	glScissor(dst_x, dst_y, 0, 0);
	glRasterPos2i(dst_x - 5, dst_y - 5);
	glCopyPixels(10, 10, 20, 20, GL_COLOR);

	/* Copy an unscissored 10x10 square from left to right */
	glDisable(GL_SCISSOR_TEST);
	dst_y = 50;
	glRasterPos2i(dst_x, dst_y);
	glCopyPixels(10, 10, dst_w, dst_h, GL_COLOR);

	pass &= check_red_box_surrounded_by_green(dst_x, 10, dst_w, dst_h);
	pass &= check_red_box_surrounded_by_green(dst_x, 30, 0, 0);
	pass &= check_red_box_surrounded_by_green(dst_x, 50, dst_w, dst_h);

	glutSwapBuffers();

	if (Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
	}
}


static void reshape(int width, int height)
{
	win_width = width;
	win_height = height;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void
init(void)
{
	reshape(WIN_WIDTH, WIN_HEIGHT);
}

int main(int argc, char**argv)
{
	int i;
	glutInit(&argc, argv);

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			Automatic = 1;
		else
			printf("Unknown option: %s\n", argv[i]);
	}

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("scissor-copypixels");
	glutKeyboardFunc(piglit_escape_exit_key);
	init();
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
