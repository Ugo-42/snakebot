#include "recorder.h"
#include <unistd.h>

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

static int is_valid(int x, int y, char map[H][W])
{
    if (x < 0 || y < 0 || x >= W || y >= H)
        return 0;
    if (map[y][x] == '#' || map[y][x] == '%')
        return 0;
    return 1;
}

/* ============================================================
   BFS STRUCTURES
   ============================================================ */

typedef struct {
    vec_t parent[H][W];
    char visited[H][W];
} bfs_t;

/* ============================================================
   BFS TO APPLE
   ============================================================ */

static int bfs(char map[H][W], vec_t start, vec_t goal, vec_t *next)
{
    vec_t q[H * W];
    int qh = 0, qt = 0;

    bfs_t ctx = {0};

    q[qt++] = start;
    ctx.visited[start.y][start.x] = 1;
    ctx.parent[start.y][start.x] = (vec_t){-1, -1};

    while (qh < qt)
    {
        vec_t cur = q[qh++];

        if (cur.x == goal.x && cur.y == goal.y)
            break;

        for (int i = 0; i < 4; i++)
        {
            int nx = cur.x + g_dir[i].dx;
            int ny = cur.y + g_dir[i].dy;

            if (!is_valid(nx, ny, map))
                continue;

            if (ctx.visited[ny][nx])
                continue;

            ctx.visited[ny][nx] = 1;
            ctx.parent[ny][nx] = cur;
            q[qt++] = (vec_t){nx, ny};
        }
    }

    if (!ctx.visited[goal.y][goal.x])
        return 0;

    vec_t cur = goal;

    while (!(ctx.parent[cur.y][cur.x].x == start.x &&
             ctx.parent[cur.y][cur.x].y == start.y))
    {
        cur = ctx.parent[cur.y][cur.x];
    }

    *next = cur;
    return 1;
}

/* ============================================================
   FLOOD FILL SAFETY CHECK
   ============================================================ */

static int flood_fill(char map[H][W], vec_t start)
{
    vec_t q[H * W];
    char visited[H][W] = {0};

    int qh = 0, qt = 0;
    int count = 0;

    q[qt++] = start;
    visited[start.y][start.x] = 1;

    while (qh < qt)
    {
        vec_t cur = q[qh++];
        count++;

        for (int i = 0; i < 4; i++)
        {
            int nx = cur.x + g_dir[i].dx;
            int ny = cur.y + g_dir[i].dy;

            if (!is_valid(nx, ny, map))
                continue;

            if (visited[ny][nx])
                continue;

            visited[ny][nx] = 1;
            q[qt++] = (vec_t){nx, ny};
        }
    }
    return count;
}

/* ============================================================
   SIMPLE SNAKE LENGTH ESTIMATION
   (fallback heuristic)
   ============================================================ */

static int estimate_len(char map[H][W])
{
    int len = 0;

    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (map[y][x] == '%' || map[y][x] == '^'
                || map[y][x] == 'v' || map[y][x] == '<'
                || map[y][x] == '>')
                len++;

    return len;
}

/* ============================================================
   SAFETY RULE
   ============================================================ */

static int is_safe(char map[H][W], vec_t next)
{
    int space = flood_fill(map, next);
    int len = estimate_len(map);

    return space > len;
}

/* ============================================================
   FALLBACK MOVE
   ============================================================ */

static int fallback_move(int fd, char map[H][W], vec_t head)
{
    for (int i = 0; i < 4; i++)
    {
        int nx = head.x + g_dir[i].dx;
        int ny = head.y + g_dir[i].dy;

        if (is_valid(nx, ny, map) &&
            is_safe(map, (vec_t){nx, ny}))
        {
            write(fd, &g_dir[i].cmd, 1);
            return 1;
        }
    }

    return 0;
}

/* ============================================================
   MAIN BOT LOOP
   ============================================================ */

void bot_update(int fd, char map[H][W])
{
    vec_t apple, head, next;

	if (!recorder_get_apple(&apple) || !recorder_get_head(&head))
		return;

    if (!bfs(map, head, apple, &next))
    {
        fallback_move(fd, map, head);
        return;
    }

    if (!is_safe(map, next))
    {
        fallback_move(fd, map, head);
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        if (head.x + g_dir[i].dx == next.x &&
            head.y + g_dir[i].dy == next.y)
        {
            write(fd, &g_dir[i].cmd, 1);
            return;
        }
    }

    fallback_move(fd, map, head);
}
