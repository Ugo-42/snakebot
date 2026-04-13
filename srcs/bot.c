#include "recorder.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* ================= STATE ================= */

static int hx, hy;
static int ax, ay;

static int dir = 0;

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
		for (int x = 0; x < W; x++)
			if (f[y][x] == '^' || f[y][x] == 'v'
				|| f[y][x] == '<' || f[y][x] == '>')
			{
				hx = x; hy = y;
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
				ax = x; ay = y;
				return 1;
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

/* ================= FLOOD FILL ================= */

static int flood(char f[H][W], int sx, int sy, int mark_tail)
{
	(void)mark_tail;
	int vis[H][W];
	memset(vis, 0, sizeof(vis));

	int qx[H * W], qy[H * W];
	int qh = 0, qt = 0;

	qx[qt] = sx;
	qy[qt] = sy;
	qt++;
	vis[sy][sx] = 1;

	int count = 0;

	while (qh < qt)
	{
		int x = qx[qh];
		int y = qy[qh];
		qh++;
		count++;

		for (int d = 0; d < 4; d++)
		{
			int nx = x + dx[d];
			int ny = y + dy[d];

			if (!can_move(f, nx, ny))
				continue;
			if (vis[ny][nx])
				continue;

			vis[ny][nx] = 1;
			qx[qt] = nx;
			qy[qt] = ny;
			qt++;
		}
	}

	return count;
}

/* ================= SIMULATION SAFETY ================= */

static int safe_move(char f[H][W], int nx, int ny)
{
	if (!can_move(f, nx, ny))
		return 0;

	/* simulate occupancy */
	char tmp[H][W];
	memcpy(tmp, f, sizeof(tmp));

	tmp[hy][hx] = '#'; /* treat old head as body */
	tmp[ny][nx] = '^';

	/* flood from new head */
	int space = flood(tmp, nx, ny, 0);

	/* critical heuristic:
	   must have enough space to not trap self */
	if (space < 6)
		return 0;

	return 1;
}

/* ================= CHOOSE MOVE ================= */

static int choose_direction(char f[H][W])
{
	int best = dir;
	int best_score = 1e9;

	/* 1. try apple chase */
	for (int d = 0; d < 4; d++)
	{
		int nx = hx + dx[d];
		int ny = hy + dy[d];

		if (!safe_move(f, nx, ny))
			continue;

		int dist = abs(nx - ax) + abs(ny - ay);

		if (dist < best_score)
		{
			best_score = dist;
			best = d;
		}
	}

	/* 2. fallback: if no safe apple move → tail escape mode */
	if (best_score == 1e9)
	{
		for (int d = 0; d < 4; d++)
		{
			int nx = hx + dx[d];
			int ny = hy + dy[d];

			if (can_move(f, nx, ny))
			{
				best = d;
				break;
			}
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
