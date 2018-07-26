/* Copyright (c) 2018 Trollforge. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Trollforge's name may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define R	0
#define G	1
#define B	2

#define DIST(x, y)	fabs(sqrtf((x[R] - y[R]) * (x[R] - y[R]) + \
				   (x[G] - y[G]) * (x[G] - y[G]) + \
				   (x[B] - y[B]) * (x[B] - y[B])))

#define ANSI_FMT	0
#define MIRC_FMT	1
#define EMOJI_FMT	2

#define VGA_PAL		0
#define MIRC_PAL	1

typedef struct block_s {
	int color;
} block_t;

void usage(void);
int nearestcolor(float *pixel, int palette);
float huetorgb(float p, float q, float t);
void tweak(float *pixel, float sat, float lum);

int
main(int argc, char *argv[])
{
	int width = 0;
	int height = 0;
	int channels = 0;
	block_t *block = NULL;

	int format = ANSI_FMT;
	int palette = VGA_PAL;
	bool cp437 = false;
	bool useice = false;
	bool resize = false;
	long resize_width = 0;
	long resize_height = 0;

	int ch = 0;
	int fg = 0;
	int bg = 0;
	int lfg = 0;
	int lbg = 0;

	float *pixel = NULL;
	float *resized = NULL;

	float brightness = 100.0f;
	float saturation = 100.0f;

	while((ch = getopt(argc, argv, "b:f:p:w:s:")) != -1) {
		switch (ch) {
			case 'b':
				brightness = strtof(optarg, NULL);
				break;
			case 'f':
				switch (optarg[0]) {
					case 'a':
						format = ANSI_FMT;
						break;
					case 'd':
						format = ANSI_FMT;
						cp437 = true;
						useice = true;
						break;
					case 'm':
						format = MIRC_FMT;
						break;
					case 'e':
						format = EMOJI_FMT;
						break;
					default:
						usage();
						break;
				}
				break;
			case 'p':
				switch (optarg[0]) {
					case 'm':
						palette = MIRC_PAL;
						break;
					case 'v':
						palette = VGA_PAL;
						break;
					default:
						usage();
						break;
				}
				break;
			case 's':
				saturation = strtof(optarg, NULL);
				break;
			case 'w':
				resize_width = strtol(optarg, NULL, 10);
				resize = true;
				break;
			default:
				usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage();
	}

	/* handle alpha eventually */
	pixel = stbi_loadf(argv[0], &width, &height, &channels, STBI_rgb);

	if (!pixel) {
		usage();
	}

	if (resize) {
		resize_height = height * resize_width / width;

		resized = malloc(sizeof(float) * resize_width * resize_height * channels);

		stbir_resize_float(pixel, width, height, 0,
				   resized, resize_width, resize_height, 0,
				   channels);

		free(pixel);

		pixel = resized;
		width = resize_width;
		height = resize_height;
	}

	if (brightness != 100.0f || saturation != 100.0f) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				tweak(&pixel[((width * i) + j) * channels],
				      saturation, brightness);
			}
		}
	}

	block = malloc(sizeof(block_t) * height * width);

	if (format == EMOJI_FMT) {
		palette = VGA_PAL;
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			block[(width * i) + j].color =
			nearestcolor(&pixel[((width * i) + j) * channels],
				     palette);
		}
	}
	free(pixel);

	if (format == EMOJI_FMT) {
		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				switch (block[(width * i) + j].color) {
					case 0:
						printf("⬛");
						break;
					case 1:
						printf("🔴");
						break;
					case 2:
						printf("💚");
						break;
					case 3:
						printf("💩");
						break;
					case 4:
						printf("💙");
						break;
					case 5:
						printf("💜");
						break;
					case 6:
						printf("📫");
						break;
					case 7:
						printf("👽");
						break;
					case 8:
						printf("💣");
						break;
					case 9:
						printf("🧠");
						break;
					case 10:
						printf("🎾");
						break;
					case 11:
						printf("🌞");
						break;
					case 12:
						printf("♿");
						break;
					case 13:
						printf("🐷");
						break;
					case 14:
						printf("💦");
						break;
					case 15:
						printf("💭");
						break;
				}
			}
			printf("\n");
		}
		return 0;
	}

	for (int i = 0; i + 1 < height; i += 2) {
		for (int j = 0; j < width; j++) {
			fg = block[(width * i) + j].color;
			bg = block[(width * (i + 1)) + j].color;

			/* dont print color codes if we dont have to */
			if (j != 0 && lbg == bg && lfg == fg) {
				/* try to save bytes */
				if (bg == fg) {
					printf(" ");
				} else {
					cp437 ? printf("\xdf") : printf("▀");
				}
			} else {
				/* XXX we dont really have to print both attrs */
				if (format == ANSI_FMT) {
					if (useice) {
						printf("\x1b[%s%d;%dm%s",
							/* bold and ice */
							(fg > 7 && bg > 7) ? "1;5;" :
							/* bold only */
							(fg > 7 && bg < 8) ? "1;" :
							/* ice only */
							(fg < 8 && bg > 7) ? "5;" :
							/* neither */
							"",
							fg > 7 ? fg - 8 + 30 : fg + 30,
							bg > 7 ? bg - 8 + 40 : bg + 40,
							bg == fg ? " " : cp437 ? "\xdf" : "▀");
					} else {
					printf("\x1b[%d;%dm%s",
						fg < 8 ? fg + 30 : fg - 8 + 90,
						bg < 8 ? bg + 40 : bg - 8 + 100,
						bg == fg ? " " : cp437 ? "\xdf" : "▀");
					}
				} else {
					printf("\x03%d,%d%s", fg, bg,
					       bg == fg ? " " : cp437 ? "\xdf" : "▀");
				}
			}
			lbg = bg;
			lfg = fg;
		}
		/* reset to prevent line bleeding on terms */
		if (format == ANSI_FMT) {
			printf("\x1b[0m\n");
		} else {
			printf("\n");
		}
	}
	return 0;
}

int
nearestcolor(float *pixel, int palette)
{

	/* vga palette, maybe add more */
	float vga_palette[16][3] = {{0.00f, 0.00f, 0.00f},
				    {0.66f, 0.00f, 0.00f},
				    {0.00f, 0.66f, 0.00f},
				    {0.66f, 0.33f, 0.00f},
				    {0.00f, 0.00f, 0.66f},
				    {0.66f, 0.00f, 0.66f},
				    {0.00f, 0.66f, 0.66f},
				    {0.66f, 0.66f, 0.66f},
				    {0.33f, 0.33f, 0.33f},
				    {1.00f, 0.85f, 0.85f},
				    {0.33f, 1.00f, 0.33f},
				    {1.00f, 1.00f, 0.33f},
				    {0.33f, 0.33f, 1.00f},
				    {1.00f, 0.33f, 1.00f},
				    {0.33f, 1.00f, 1.00f},
				    {1.00f, 1.00f, 1.00f}};

	float mirc_palette[16][3] = {{1.00f, 1.00f, 1.00f},
				     {0.00f, 0.00f, 0.00f},
				     {0.00f, 0.00f, 0.50f},
				     {0.00f, 0.57f, 0.00f},
				     {1.00f, 0.00f, 0.00f},
				     {0.50f, 0.00f, 0.00f},
				     {0.61f, 0.00f, 0.61f},
				     {0.98f, 0.50f, 0.00f},
				     {1.00f, 1.00f, 0.00f},
				     {0.00f, 0.98f, 0.00f},
				     {0.00f, 0.57f, 0.57f},
				     {0.00f, 1.00f, 1.00f},
				     {0.00f, 0.33f, 0.98f},
				     {1.00f, 0.00f, 1.00f},
				     {0.50f, 0.50f, 0.50f},
				     {0.82f, 0.82f, 0.82f}};

	float delta = 10;
	int color = 0;

	if (palette == MIRC_PAL) {
		for (int i = 0; i < 16; i++) {
			if (DIST(pixel, mirc_palette[i]) < delta) {
				delta = DIST(pixel, mirc_palette[i]);
				color = i;
			}
		}
	} else {
		for (int i = 0; i < 16; i++) {
			if (DIST(pixel, vga_palette[i]) < delta) {
				delta = DIST(pixel, vga_palette[i]);
				color = i;
			}
		}
	}
	return color;
}

float
huetorgb(float p, float q, float t)
{
	if (t < 0.0f) {
		t += 1.0f;
	} else if (t > 1.0f) {
		t -= 1.0f;
	}

	if (t < 1.0f/6.0f) {
		return p + (q - p) * 6.0f * t;
	}

	if (t < 0.5f) {
		return q;
	}

	if (t < 2.0f/3.0f) {
		return p + (q - p) * ((2.0f/3.0f) - t) * 6.0f;
	}
	return p;
}

/* sat and lum are percentages */
void
tweak(float *pixel, float sat, float lum)
{
	/* convert rgb to hsl */
	float r = pixel[R];
	float g = pixel[G];
	float b = pixel[B];

	float max = r > g ? r > b ? r : b : g > b ? g : b;
	float min = r < g ? r < b ? r : b : g < b ? g : b;

	float h = (min + max) / 2.0f;
	float s = (min + max) / 2.0f;
	float l = (min + max) / 2.0f;

	float d = max - min;

	float q = 0.0f;
	float p = 0.0f;

	if (max == min) {
		s = l = 0.0f;
	} else {
		s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);
		if (max == r) {
			h = (g - b) / d + (g < b ? 6.0f : 0.0f);
		} else if (max == g) {
			h = (b - r) / d + 2.0f;
		} else { /* max == b */
			h = (r - g) / d + 4.0f;
		}
	}
	h /= 6.0f;

	/* apply tweaks */
	s *= sat * 0.01f;
	l *= lum * 0.01f;

	/* convert from hsl to rgb */
	if (s == 0.0f) {
		r = g = b = l;
	} else {
		q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
		p = 2 * l - q;
		r = huetorgb(p, q, h + 1.0f/3.0f);
		g = huetorgb(p, q, h);
		b = huetorgb(p, q, h - 1.0f/3.0f);
	}

	/* clamp values */
	pixel[R] = r < 0.0f ? 0.0f : r > 1.0f ? 1.0f : r;
	pixel[G] = g < 0.0f ? 0.0f : g > 1.0f ? 1.0f : g;
	pixel[B] = b < 0.0f ? 0.0f : b > 1.0f ? 1.0f : b;
}

void
usage(void)
{
	fprintf(stderr, "usage: p2u [options] input\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "-b percent     Adjust brightness levels, default is 100.\n");
	fprintf(stderr, "-f a|d|e|m     Specify output format ANSI, DOS (ANSI with");
	fprintf(stderr, "               CP437 characters), emoji or mirc.  Default is ANSI.\n");
	fprintf(stderr, "-p m|v         Specify palette to use, mirc or VGA, default is VGA.\n");
	fprintf(stderr, "-s percent     Adjust saturation levels, default is 100.\n");
	fprintf(stderr, "-w width       Specify output width, default is the image width.\n");
	fprintf(stderr, "\n");
	exit(1);
}
