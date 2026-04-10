#include "recorder.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

static char grid[H][W];

/* cursor */
static int cx = 0;
static int cy = 0;

/* escape parsing */
static int in_esc = 0;
static char esc[128];
static int ei = 0;

/* sync */
static int seen_top = 0;
static int seen_bottom = 0;
static int recording = 0;

/* ---------- GRID ---------- */

static void clear_grid(void)
{
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			grid[y][x] = ' ';
}

/* ---------- ANSI ---------- */

static void handle_escape(void)
{
	esc[ei] = 0;

	int r, c;
	if (sscanf(esc, "[%d;%dH", &r, &c) == 2)
	{
		cy = r - 1;
		cx = c - 1;
	}

	if (strstr(esc, "[2J"))
		clear_grid();

	ei = 0;
}

/* ---------- UTF8 ---------- */

static char convert_utf8(unsigned char *buf, ssize_t *i)
{
	unsigned char c = buf[*i];

	if (c < 128)
	{
		(*i)++;
		return c;
	}

	if (c == 0xE2)
	{
		unsigned char c2 = buf[*i + 1];
		unsigned char c3 = buf[*i + 2];

		*i += 3;

		if (c2 == 0x96 && c3 == 0x91) /* ░ */
			return '#';

		if (c2 == 0x96 && c3 == 0x9A) /* ▚ */
			return '%';

		return '?';
	}

	if (c == 0xF0)
	{
		unsigned char c3 = buf[*i + 3];

		if (c3 == 0xA9) { *i += 4; return '^'; }
		if (c3 == 0xAB) { *i += 4; return 'v'; }
		if (c3 == 0xA8) { *i += 4; return '<'; }
		if (c3 == 0xAA) { *i += 4; return '>'; }

		*i += 4;
		return 'X';
	}

	(*i)++;
	return '?';
}

/* ---------- BORDER ---------- */

static int is_full_border(char *line)
{
	for (int i = 0; i < W; i++)
		if (line[i] != '#')
			return 0;
	return 1;
}

/* ---------- PROCESS ---------- */

void recorder_init(void)
{
	clear_grid();
	cx = cy = 0;
	in_esc = 0;
	recording = 0;
	seen_top = seen_bottom = 0;
}

void recorder_feed(unsigned char *buf, ssize_t n)
{
	for (ssize_t i = 0; i < n;)
	{
		unsigned char c = buf[i];

		if (c == 27)
		{
			in_esc = 1;
			ei = 0;
			i++;
			continue;
		}

		if (in_esc)
		{
			if (ei < (int)sizeof(esc) - 1)
				esc[ei++] = c;

			if ((c >= 'A' && c <= 'Z') || c == 'm')
			{
				in_esc = 0;
				handle_escape();
			}

			i++;
			continue;
		}

		char outc = convert_utf8(buf, &i);

		if (outc == '\n')
		{
			cy++;
			cx = 0;
			continue;
		}

		if (cy >= 0 && cy < H && cx >= 0 && cx < W)
			grid[cy][cx] = outc;

		cx++;
	}
}

/* ---------- FRAME ---------- */

int is_valid_frame(void)
{
	int top = is_full_border(grid[0]);
	int bottom = is_full_border(grid[H - 1]);

	if (!top || !bottom)
		return 0;

	return 1;
}

int recorder_frame_ready(void)
{
    if (!is_valid_frame())
        return 0;

    if (!recording)
        recording = 1;

    return recording;
}

void recorder_consume_frame(char out[H][W])
{
	for (int y = 0; y < H; y++)
		memcpy(out[y], grid[y], W);
}
