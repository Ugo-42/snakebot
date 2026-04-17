#include "recorder.h"
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#define REACTION_TIME_FRAMES 5
#define ABS(x) ((x) < 0 ? -(x) : (x))

static vec_t known_apple = {-1, -1};
static int frames_since_apple_spawn = 0;

static int get_free_space(char map[H][W], int y, int x, int visited[H][W])
{
	if (y < 0 || y >= H || x < 0 || x >= W || 
			map[y][x] == '#' || map[y][x] == '%' || visited[y][x])
		return 0;

	visited[y][x] = 1;
	return 1 + get_free_space(map, y + 1, x, visited) +
		get_free_space(map, y - 1, x, visited) +
		get_free_space(map, y, x + 1, visited) +
		get_free_space(map, y, x - 1, visited);
}

static void send_move(int fd, char move)
{
	write(fd, &move, 1);
}

static void best_move(int fd, char map[H][W], vec_t head, vec_t real_apple)
{
	if (real_apple.x != known_apple.x || real_apple.y != known_apple.y)
	{
		if (frames_since_apple_spawn >= REACTION_TIME_FRAMES) {
			known_apple = real_apple;
		}
		else
		{
			frames_since_apple_spawn++;
		}
	}
	else
	{
		frames_since_apple_spawn = 0; 
	}

	int dx[] = {0, 0, -1, 1};
	int dy[] = {-1, 1, 0, 0};
	char keys[] = {'w', 's', 'a', 'd'};

	int best_score = -1;
	char final_move = 'w';

	for (int i = 0; i < 4; i++)
	{
		int nx = head.x + dx[i];
		int ny = head.y + dy[i];

		if (ny < 0 || ny >= H || nx < 0 || nx >= W || 
				map[ny][nx] == '#' || map[ny][nx] == '%') 
			continue;

		int visited[H][W] = {0};
		int space = get_free_space(map, ny, nx, visited);

		int dist_score = 0;
		if (known_apple.x != -1)
		{
			dist_score = 1000 - (ABS(nx - known_apple.x) + ABS(ny - known_apple.y));
		}

		int total_score = (space * 10) + dist_score;

		if (total_score > best_score)
		{
			best_score = total_score;
			final_move = keys[i];
		}
	}

	send_move(fd, final_move);
}

void bot_update(int fd, char map[H][W])
{
	vec_t apple, head;

	if (!recorder_get_apple(&apple) || !recorder_get_head(&head))
		return;

	best_move(fd, map, head, apple);
}
