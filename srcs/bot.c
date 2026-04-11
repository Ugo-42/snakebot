#include "recorder.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* ---------------- X11 ---------------- */

static Display *dpy = NULL;

static void init_x11(void)
{
    if (!dpy)
        dpy = XOpenDisplay(NULL);
}

static void send_key(char k)
{
    if (!dpy)
        return;

    char keystr[2] = {k, 0};
    KeyCode code = XKeysymToKeycode(dpy, XStringToKeysym(keystr));

    XTestFakeKeyEvent(dpy, code, True, 0);
    XTestFakeKeyEvent(dpy, code, False, 0);
    XFlush(dpy);
}

/* ---------------- GAME STATE ---------------- */

static int hx, hy;
static int ax, ay;

/* ---------------- PATH BUFFER ---------------- */

static char path[H * W];
static int path_len = 0;
static int path_i = 0;

/* ---------------- BFS ---------------- */

typedef struct s_node {
    int x, y;
} node_t;

static int visited[H][W];
static node_t parent[H][W];

static int dx[4] = {1, -1, 0, 0};
static int dy[4] = {0, 0, 1, -1};

static int is_blocked(char c)
{
    return (c == '#' || c == '%');
}

/* ---------------- FIND ---------------- */

static int find_head(char f[H][W])
{
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            if (f[y][x] == '^' || f[y][x] == 'v' ||
                f[y][x] == '<' || f[y][x] == '>')
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

/* ---------------- BFS (FULL PATH) ---------------- */

static int bfs(char f[H][W], char *out)
{
    int qx[H * W];
    int qy[H * W];
    int qh = 0, qt = 0;

    memset(visited, 0, sizeof(visited));

    qx[qt] = hx;
    qy[qt++] = hy;
    visited[hy][hx] = 1;

    while (qh < qt)
    {
        int x = qx[qh];
        int y = qy[qh++];

        if (x == ax && y == ay)
        {
            int len = 0;

            while (!(x == hx && y == hy))
            {
                node_t p = parent[y][x];

                if (x > p.x) out[len++] = 'd';
                else if (x < p.x) out[len++] = 'a';
                else if (y > p.y) out[len++] = 's';
                else out[len++] = 'w';

                x = p.x;
                y = p.y;
            }

            for (int i = 0; i < len / 2; i++)
            {
                char t = out[i];
                out[i] = out[len - 1 - i];
                out[len - 1 - i] = t;
            }

            return len;
        }

        for (int i = 0; i < 4; i++)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];

            if (nx < 0 || ny < 0 || nx >= W || ny >= H)
                continue;

            if (visited[ny][nx])
                continue;

            if (is_blocked(f[ny][nx]))
                continue;

            visited[ny][nx] = 1;
            parent[ny][nx] = (node_t){x, y};

            qx[qt] = nx;
            qy[qt++] = ny;
        }
    }

    return 0;
}

/* ---------------- BOT LOOP STATE ---------------- */

static int path_valid = 0;

/* ---------------- MAIN UPDATE ---------------- */

void bot_update(char frame[H][W])
{
    init_x11();

    if (!find_head(frame) || !find_apple(frame))
        return;

    /* refill path only when needed */
    if (path_i >= path_len)
    {
        path_len = bfs(frame, path);
        path_i = 0;
        path_valid = (path_len > 0);
    }

    char move;

    if (path_valid)
        move = path[path_i++];
    else
        move = bfs(frame, path); /* fallback immediate recompute */

    send_key(move);
}
