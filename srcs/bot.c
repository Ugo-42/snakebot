#include "recorder.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* ================= STATE ================= */

static int hx, hy;
static int ax, ay;

static int dir = 0;

/* up right down left */
static int dx[4] = {0, 1, 0, -1};
static int dy[4] = {-1, 0, 1, 0};

static char dir_to_key(int d)
{
	return "wdsa"[d];
}

/* ================= FINDERS ================= */

static int find_head(char f[H][W])
{
	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
		{
			if (f[y][x] == '^' || f[y][x] == 'v'
					|| f[y][x] == '<' || f[y][x] == '>')
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

static int can_move(char f[H][W], int x, int y)
{
	if (x < 0 || y < 0 || x >= W || y >= H)
		return 0;
	if (f[y][x] == '#' || f[y][x] == '%')
		return 0;
	return 1;
}

/* ================= CHOOSE DIRECTION ================= */

static int choose_direction(char f[H][W])
{
	int best = dir;
	int best_dist = 100000;

	for (int d = 0; d < 4; d++)
	{
		int nx = hx + dx[d];
		int ny = hy + dy[d];

		if (!can_move(f, nx, ny))
			continue;

		int dist = abs(nx - ax) + abs(ny - ay);

		if (dist < best_dist)
		{
			best_dist = dist;
			best = d;
		}
	}

	return best;
}

/* ================= OUTPUT ================= */

static void send_key(int fd, char k)
{
	write(fd, &k, 1);
}

/* ================= MAIN ================= */

void bot_update(int fd, char frame[H][W])
{
	if (!find_head(frame) || !find_apple(frame))
		return;

	dir = choose_direction(frame);

	send_key(fd, dir_to_key(dir));
}
