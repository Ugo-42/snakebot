#include "recorder.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <stdlib.h>

static Display *dpy;

static int hx, hy;
static int ax, ay;

static void init_x11(void)
{
	if (!dpy)
		dpy = XOpenDisplay(NULL);
}

static void send_key(char k)
{
	if (!dpy)
		return;

	KeyCode code = XKeysymToKeycode(dpy, XStringToKeysym((char[]){k, 0}));

	XTestFakeKeyEvent(dpy, code, True, 0);
	XTestFakeKeyEvent(dpy, code, False, 0);
	XFlush(dpy);
}

static int find_head(char f[H][W])
{
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			if (f[y][x] == '^' || f[y][x] == 'v' ||
					f[y][x] == '<' || f[y][x] == '>')
			{
				hx = x;
				hy = y;
				return 1;
			}
		}
	}
	return 0;
}

static int find_apple(char f[H][W])
{
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			if (f[y][x] == '@')
			{
				ax = x;
				ay = y;
				return 1;
			}
		}
	}
	return 0;
}

static char decide(void)
{
	int dx = ax - hx;
	int dy = ay - hy;

	if (abs(dx) > abs(dy))
		return (dx > 0) ? 'd' : 'a';
	return (dy > 0) ? 's' : 'w';
}

void bot_update(char frame[H][W])
{
	init_x11();

	if (!find_head(frame) || !find_apple(frame))
		return;

	char move = decide();

	send_key(move);
}
