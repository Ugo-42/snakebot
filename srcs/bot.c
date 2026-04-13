#include "recorder.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

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

/* ================= FLOOD FILL SAFETY ================= */

static int flood_fill_space(char f[H][W], int sx, int sy)
{
    if (!can_move(f, sx, sy))
        return 0;

    char visited[H][W];
    memset(visited, 0, sizeof(visited));

    int qx[H * W];
    int qy[H * W];
    int qh = 0, qt = 0;

    qx[qt] = sx;
    qy[qt] = sy;
    qt++;

    visited[sy][sx] = 1;

    int count = 0;

    while (qh < qt)
    {
        int x = qx[qh];
        int y = qy[qh];
        qh++;

        count++;

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || ny < 0 || nx >= W || ny >= H)
                continue;

            if (visited[ny][nx])
                continue;

            if (!can_move(f, nx, ny))
                continue;

            visited[ny][nx] = 1;
            qx[qt] = nx;
            qy[qt] = ny;
            qt++;
        }
    }

    return count;
}

/* ================= SAFETY FILTER ================= */

static int is_safe_move(char f[H][W], int x, int y)
{
    int space = flood_fill_space(f, x, y);

    /* IMPORTANT THRESHOLD:
       prevents self-trapping corridors */
    if (space < 8)
        return 0;

    return 1;
}

/* ================= CHOOSE DIRECTION ================= */

static int choose_direction(char f[H][W])
{
    int best_dir = dir;
    int best_score = INT_MIN;

    for (int d = 0; d < 4; d++)
    {
        int nx = hx + dx[d];
        int ny = hy + dy[d];

        if (!can_move(f, nx, ny))
            continue;

        if (!is_safe_move(f, nx, ny))
            continue;

        int dist = abs(nx - ax) + abs(ny - ay);

        /* combine:
           - safety (primary)
           - apple attraction (secondary)
        */
        int score = -dist;

        /* slight preference bonus for stability */
        score += flood_fill_space(f, nx, ny) / 10;

        if (score > best_score)
        {
            best_score = score;
            best_dir = d;
        }
    }

    return best_dir;
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
