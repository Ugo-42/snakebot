#include "recorder.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* ---------------- STATE ---------------- */

static int hx, hy;
static int ax, ay;
static int dir = 0;

/* ---------------- FIND ---------------- */

static int find_head(char f[H][W])
{
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			if (strchr("^v<>", f[y][x]))
			{
				hx = x;
				hy = y;
				return 1;
			}
	return 0;
}

static int find_apple(char f[H][W])
{
	for (int y = 0; y < H; y++)
		for (int x = 0; x < W; x++)
			if (f[y][x] == '@')
			{
				ax = x;
				ay = y;
				return 1;
			}
	return 0;
}

/* ---------------- MOVE ---------------- */

static int dx[4] = {0, 1, 0, -1};
static int dy[4] = {-1, 0, 1, 0};

static char dir_to_key(int d)
{
	return "wdsa"[d];
}

static int is_safe(char f[H][W], int x, int y)
{
	if (x < 0 || y < 0 || x >= W || y >= H)
		return 0;

	if (f[y][x] == '#' || f[y][x] == '%')
		return 0;

	return 1;
}

/* ---------------- FLOOD FILL ---------------- */

static int flood_fill(char f[H][W], int x, int y, int visited[H][W])
{
	if (!is_safe(f, x, y) || visited[y][x])
		return 0;

	visited[y][x] = 1;

	int size = 1;

	for (int d = 0; d < 4; d++)
		size += flood_fill(f, x + dx[d], y + dy[d], visited);

	return size;
}

/* ---------------- SCORING ---------------- */

static int score_dir(char f[H][W], int d)
{
	int nx = hx + dx[d];
	int ny = hy + dy[d];

	if (!is_safe(f, nx, ny))
		return -100000;

	int dist = abs(nx - ax) + abs(ny - ay);

	int visited[H][W] = {0};
	int space = flood_fill(f, nx, ny, visited);

	return -dist + space;
}

/* ---------------- DECISION ---------------- */

static int choose_direction(char f[H][W])
{
	int best = dir;
	int best_score = -1000000;

	for (int d = 0; d < 4; d++)
	{
		int score = score_dir(f, d);

		if (d == dir)
			score += 2;

		if (score > best_score)
		{
			best_score = score;
			best = d;
		}
	}

	return best;
}

/* ---------------- OUTPUT ---------------- */

static void send_key(int fd, char k)
{
	write(fd, &k, 1);
}

/* ---------------- MAIN ---------------- */

void bot_update(int fd, char frame[H][W])
{
	if (!find_head(frame) || !find_apple(frame))
		return;

	dir = choose_direction(frame);

	send_key(fd, dir_to_key(dir));
}
