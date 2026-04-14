#include "recorder.h"
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>

typedef struct {
	int dx;
	int dy;
	char cmd;
} dir_t;

static dir_t g_dir[4] = {
	{0, -1, 'w'},
	{0,  1, 's'},
	{-1, 0, 'a'},
	{1,  0, 'd'}
};

/* ============================================================
   BASIC VALIDATION
   ============================================================ */

static int in_bounds(int x, int y)
{
	return (x >= 0 && y >= 0 && x < W && y < H);
}

static int is_blocked(char c)
{
	return (c == '#' || c == '%');
}

static int is_valid(char map[H][W], int x, int y)
{
	if (!in_bounds(x, y))
		return 0;
	return !is_blocked(map[y][x]);
}

/* ============================================================
   DISTANCE
   ============================================================ */

static int manhattan(int x1, int y1, int x2, int y2)
{
	return (x1 > x2 ? x1 - x2 : x2 - x1)
		+ (y1 > y2 ? y1 - y2 : y2 - y1);
}

/* ============================================================
   LOCAL FREEDOM
   ============================================================ */

static int local_space(char map[H][W], int sx, int sy)
{
	vec_t stack[64];
	char visited[H][W];
	int size = 0;

	memset(visited, 0, sizeof(visited));

	stack[size++] = (vec_t){sx, sy};
	visited[sy][sx] = 1;

	int count = 0;

	while (size > 0 && count < 30)
	{
		vec_t cur = stack[--size];
		count++;

		for (int i = 0; i < 4; i++)
		{
			int nx = cur.x + g_dir[i].dx;
			int ny = cur.y + g_dir[i].dy;

			if (!is_valid(map, nx, ny))
				continue;
			if (visited[ny][nx])
				continue;

			visited[ny][nx] = 1;
			stack[size++] = (vec_t){nx, ny};
		}
	}

	return count;
}

/* ============================================================
   ADJACENCY
   ============================================================ */

static int adjacency_bonus(char map[H][W], int x, int y)
{
	int score = 0;

	for (int i = 0; i < 4; i++)
	{
		int nx = x + g_dir[i].dx;
		int ny = y + g_dir[i].dy;

		if (!in_bounds(nx, ny))
			continue;

		char c = map[ny][nx];

		if (c == '#')
			score += 2;
		if (c == '%' || c == '^' || c == 'v' || c == '<' || c == '>')
			score += 3;
	}

	return score;
}

/* ============================================================
   MOVE SCORING 
   ============================================================ */

static int score_move(char map[H][W], vec_t head, vec_t apple, vec_t next)
{
	int old_dist = manhattan(head.x, head.y, apple.x, apple.y);
	int new_dist = manhattan(next.x, next.y, apple.x, apple.y);

	int score = 0;

	score += (old_dist - new_dist) * 10;
	score += adjacency_bonus(map, next.x, next.y) * 5;

	int space = local_space(map, next.x, next.y);
	score += space * 2;

	if (space < 10)
		score -= 50;

	return score;
}

/* ============================================================
   SOFTMAX SELECTION
   ============================================================ */

static char softmax_pick(int scores[4], char cmds[4])
{
	double max = -1e9;

	for (int i = 0; i < 4; i++)
		if (scores[i] > max)
			max = scores[i];

	double sum = 0.0;
	double prob[4];

	double temperature = 4.0;

	for (int i = 0; i < 4; i++)
	{
		prob[i] = exp((scores[i] - max) / temperature);
		sum += prob[i];
	}

	for (int i = 0; i < 4; i++)
		prob[i] /= sum;

	double r = (double)rand() / (double)RAND_MAX;

	double acc = 0.0;
	for (int i = 0; i < 4; i++)
	{
		acc += prob[i];
		if (r <= acc)
			return cmds[i];
	}

	return cmds[3];
}

/* ============================================================
   DECISION LOOP
   ============================================================ */

static int best_move(int fd, char map[H][W], vec_t head, vec_t apple)
{
	int scores[4];
	char cmds[4];

	for (int i = 0; i < 4; i++)
	{
		int nx = head.x + g_dir[i].dx;
		int ny = head.y + g_dir[i].dy;

		cmds[i] = g_dir[i].cmd;

		if (!is_valid(map, nx, ny))
		{
			scores[i] = -100000;
			continue;
		}

		vec_t next = {nx, ny};
		scores[i] = score_move(map, head, apple, next);
	}

	char chosen = softmax_pick(scores, cmds);

	write(fd, &chosen, 1);
	return 1;
}

/* ============================================================
   MAIN BOT ENTRY
   ============================================================ */

void bot_update(int fd, char map[H][W])
{
	vec_t apple, head;

	if (!recorder_get_apple(&apple) || !recorder_get_head(&head))
		return;

	best_move(fd, map, head, apple);
}
