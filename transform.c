/*
 * Compute a touchscreen Transformation Matrix based on a pre-set
 * multi-screen configuration.
 *
 * Copyright © 2018 SICOM Systems Inc.
 *
 * Author: Zoltán Böszörményi <zboszormenyi@sicom.com>
 *
 * Source was based on xinput-1.6.2/src/transform.c
 *
 * The original copyright is:
 *
 * Copyright © 2011 Red Hat, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <getopt.h>

#define MAX_SCREENS	(4)
#define MAX_ROTATIONS (4)

#define DEBUG 0

typedef struct Matrix {
    float m[9];
} Matrix;

enum rotations_t {
	RR_Rotate_0,
	RR_Rotate_90,
	RR_Rotate_180,
	RR_Rotate_270
};

static const char *rotations[MAX_ROTATIONS] = { "normal", "left", "inverted", "right" };

static int rotation_index(const char *rot_name) {
	int i;

	for (i = 0; i < MAX_ROTATIONS; i++)
		if (strcasecmp(rot_name, rotations[i]) == 0) {
			fprintf(stderr, "rotation %d\n", i);
			return i;
		}

	fprintf(stderr, "rotation -1\n");
	return -1;
}

static void matrix_set(Matrix *m, int row, int col, float val) {
	m->m[row * 3 + col] = val;
}

static void matrix_set_unity(Matrix *m) {
	memset(m, 0, sizeof(m->m));
	matrix_set(m, 0, 0, 1);
	matrix_set(m, 1, 1, 1);
	matrix_set(m, 2, 2, 1);
}

#if DEBUG
static void matrix_print(const Matrix *m) {
	printf("[ %3.3f %3.3f %3.3f ]\n", m->m[0], m->m[1], m->m[2]);
	printf("[ %3.3f %3.3f %3.3f ]\n", m->m[3], m->m[4], m->m[5]);
	printf("[ %3.3f %3.3f %3.3f ]\n", m->m[6], m->m[7], m->m[8]);
}
#endif

static void matrix_s4(Matrix *m, float x02, float x12, float d1, float d2, int main_diag) {
	matrix_set(m, 0, 2, x02);
	matrix_set(m, 1, 2, x12);

	if (main_diag) {
		matrix_set(m, 0, 0, d1);
		matrix_set(m, 1, 1, d2);
	} else {
		matrix_set(m, 0, 0, 0);
		matrix_set(m, 1, 1, 0);
		matrix_set(m, 0, 1, d1);
		matrix_set(m, 1, 0, d2);
	}
}

#define RR_Reflect_All	(RR_Reflect_X|RR_Reflect_Y)

static void set_transformation_matrix(int width, int height,
									Matrix *m, int offset_x, int offset_y,
									int screen_width, int screen_height,
									int rotation) {
	/* offset */
	float x = 1.0 * offset_x/width;
	float y = 1.0 * offset_y/height;

	/* mapping */
	float w = 1.0 * screen_width/width;
	float h = 1.0 * screen_height/height;

	matrix_set_unity(m);

	/*
	 * There are 16 cases:
	 * Rotation X Reflection
	 * Rotation: 0 | 90 | 180 | 270
	 * Reflection: None | X | Y | XY
	 *
	 * They are spelled out instead of doing matrix multiplication to avoid
	 * any floating point errors.
	 */
	switch (rotation) {
	case RR_Rotate_0:
#if 0
	case RR_Rotate_180 | RR_Reflect_All:
#endif
		matrix_s4(m, x, y, w, h, 1);
		break;
#if 0
	case RR_Reflect_X|RR_Rotate_0:
	case RR_Reflect_Y|RR_Rotate_180:
		matrix_s4(m, x + w, y, -w, h, 1);
		break;
	case RR_Reflect_Y|RR_Rotate_0:
	case RR_Reflect_X|RR_Rotate_180:
		matrix_s4(m, x, y + h, w, -h, 1);
		break;
#endif
	case RR_Rotate_90:
#if 0
	case RR_Rotate_270 | RR_Reflect_All: /* left limited - correct in working zone. */
#endif
		matrix_s4(m, x + w, y, -w, h, 0);
		break;
	case RR_Rotate_270:
#if 0
	case RR_Rotate_90 | RR_Reflect_All: /* left limited - correct in working zone. */
#endif
		matrix_s4(m, x, y + h, w, -h, 0);
		break;
#if 0
	case RR_Rotate_90 | RR_Reflect_X: /* left limited - correct in working zone. */
	case RR_Rotate_270 | RR_Reflect_Y: /* left limited - correct in working zone. */
		matrix_s4(m, x, y, w, h, 0);
		break;
	case RR_Rotate_90 | RR_Reflect_Y: /* right limited - correct in working zone. */
	case RR_Rotate_270 | RR_Reflect_X: /* right limited - correct in working zone. */
		matrix_s4(m, x + w, y + h, -w, -h, 0);
		break;
#endif
	case RR_Rotate_180:
#if 0
	case RR_Reflect_All|RR_Rotate_0:
#endif
		matrix_s4(m, x + w, y + h, -w, -h, 1);
		break;
	}

#if DEBUG
	matrix_print(m);
#endif
}

#if 0
static int
map_output_xrandr(Display *dpy, int deviceid, const char *output_name)
{
	int rc = EXIT_FAILURE;
	XRRScreenResources *res;
	XRROutputInfo *output_info;

	res = XRRGetScreenResources(dpy, DefaultRootWindow(dpy));
	output_info = find_output_xrandr(dpy, output_name);

	/* crtc holds our screen info, need to compare to actual screen size */
	if (output_info)
	{
		XRRCrtcInfo *crtc_info;
		Matrix m;
		matrix_set_unity(&m);
		crtc_info = XRRGetCrtcInfo (dpy, res, output_info->crtc);
		set_transformation_matrix(dpy, &m, crtc_info->x, crtc_info->y,
								  crtc_info->width, crtc_info->height,
								  crtc_info->rotation);
		rc = apply_matrix(dpy, deviceid, &m);
		XRRFreeCrtcInfo(crtc_info);
		XRRFreeOutputInfo(output_info);
	} else
		printf("Unable to find output '%s'. "
				"Output may not be connected.\n", output_name);

	XRRFreeScreenResources(res);

	return rc;
}

static int
map_output_xinerama(Display *dpy, int deviceid, const char *output_name)
{
	const char *prefix = "HEAD-";
	XineramaScreenInfo *screens = NULL;
	int rc = EXIT_FAILURE;
	int event, error;
	int nscreens;
	int head;
	Matrix m;

	if (!XineramaQueryExtension(dpy, &event, &error))
	{
		fprintf(stderr, "Unable to set screen mapping. Xinerama extension not found\n");
		goto out;
	}

	if (strlen(output_name) < strlen(prefix) + 1 ||
		strncmp(output_name, prefix, strlen(prefix)) != 0)
	{
		fprintf(stderr, "Please specify the output name as HEAD-X,"
				"where X is the screen number\n");
		goto out;
	}

	head = output_name[strlen(prefix)] - '0';

	screens = XineramaQueryScreens(dpy, &nscreens);

	if (nscreens == 0)
	{
		fprintf(stderr, "Xinerama failed to query screens.\n");
		goto out;
	} else if (nscreens <= head)
	{
		fprintf(stderr, "Found %d screens, but you requested %s.\n",
				nscreens, output_name);
		goto out;
	}

	matrix_set_unity(&m);
	set_transformation_matrix(dpy, &m,
							  screens[head].x_org, screens[head].y_org,
							  screens[head].width, screens[head].height,
							  RR_Rotate_0);
	rc = apply_matrix(dpy, deviceid, &m);

out:
	XFree(screens);
	return rc;
}

			XRRFreeOutputInfo(output_info);
			if (strncmp("HEAD-", output_name, strlen("HEAD-")) == 0)
				return map_output_xinerama(dpy, info->deviceid, output_name);
		}
	} else
		XRRFreeOutputInfo(output_info);

	return map_output_xrandr(dpy, info->deviceid, output_name);
}
#endif

void usage(const char *progname) {
	printf("Usage: %s [-i index] { -r NxM [-R {normal|left|right|inverted}] ... }\n", progname);
	printf("-r, --resolution NxM\n\tscreen resolution\n");
	printf("-R, --rotation {normal|left|right|inverted}\n\tscreen rotation\n");
	printf("-i index\n\tFor multi-screen setup, the touchscreen is\n\tattached to the (zero based) indexth screen\n");
	printf("Options -r and -R are positional and can be repeated.\n");
}

int main(int argc, char **argv) {
	struct option opts[] = {
		{ "help",		no_argument,		NULL,	'h' },
		{ "index",		required_argument,	NULL,	't' },
		{ "resolution",	required_argument,	NULL,	'r' },
		{ "rotation",	required_argument,	NULL,	'R' },
		{ NULL,			0,					NULL,	0   },
	};
	int longindex;
	int ts_index_set = 0;
	int ts_index = 0;
	int nr_screens = 0;
	int rot_index;
	int cwidth = 0, cheight = 0;
	int x_org, y_org;
	int s_width, s_height;
	int i;
	struct {
		int width;
		int height;
		int rotation;
	} screens[MAX_SCREENS];
	Matrix m;

	while (1) {
		int opt = getopt_long(argc, argv, "hi:r:R:", opts, &longindex);

		if (opt < 0)
			break;

		switch (opt) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'i':
			if (ts_index_set) {
				fprintf(stderr, "Duplicate -i option\n");
				return 1;
			}
			ts_index = atoi(optarg);
			break;
		case 'r':
			if (nr_screens >= MAX_SCREENS) {
				fprintf(stderr, "Too many -r options\n");
				return 1;
			}
			if (sscanf(optarg, "%dx%d", &screens[nr_screens].width, &screens[nr_screens].height) != 2) {
				fprintf(stderr, "Invalid -r argument: %s\n", optarg);
				return 1;
			}
			screens[nr_screens].rotation = 0;
			nr_screens++;
			break;
		case 'R':
			rot_index = nr_screens - 1;
			if (rot_index < 0) {
				fprintf(stderr, "Option -R cannot appear before -r\n");
				return 1;
			}
			if (rot_index >= MAX_SCREENS) {
				fprintf(stderr, "Too many -r options\n");
				return 1;
			}
			screens[rot_index].rotation = rotation_index(optarg);
			if (screens[rot_index].rotation == -1) {
				fprintf(stderr, "Invalid rotation specification: %s\n", optarg);
				return 1;
			}

			break;
		default:
			return 1;
		}
	}

	if (nr_screens == 0) {
		usage(argv[0]);
		return 0;
	}

	x_org = 0;
	y_org = 0;
	for (i = 0; i < nr_screens; i++) {
		switch (screens[i].rotation) {
		case RR_Rotate_0:
		case RR_Rotate_180:
			cwidth += screens[i].width;
			if (i < ts_index)
				x_org += screens[i].width;
			if (cheight < screens[i].height)
				cheight = screens[i].height;
			break;
		case RR_Rotate_90:
		case RR_Rotate_270:
			cwidth += screens[i].height;
			if (i < ts_index)
				x_org += screens[i].width;
			if (cheight < screens[i].width)
				cheight = screens[i].width;
			break;
		default:
			break;
		}
	}

#if DEBUG
	printf("%d screens, canvas %dx%d ts index %d, ts_origin %dx%d\n", nr_screens, cwidth, cheight, ts_index, x_org, y_org);
#endif

	switch (screens[ts_index].rotation) {
	case RR_Rotate_0:
	case RR_Rotate_180:
		s_width	= screens[ts_index].width;
		s_height = screens[ts_index].height;
		break;
	case RR_Rotate_90:
	case RR_Rotate_270:
		s_width = screens[ts_index].height;
		s_height = screens[ts_index].width;
		break;
	}

	set_transformation_matrix(cwidth, cheight, &m, x_org, y_org, s_width, s_height, screens[ts_index].rotation);

	for (i = 0; i < 9; i++)
		printf("%s%.5f", (i ? " " : ""), m.m[i]);
	printf("\n");

	return 0;
}
