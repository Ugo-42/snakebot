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
    if (c == '#')
        return 1;
    if (c == '%')
        return 1;
    return 0;
}

/* ---------------- HEAD / APPLE ---------------- */

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

/* ---------------- BFS SEARCH ---------------- */

static char bfs(char f[H][W])
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
            /* reconstruct first step */
            int cx = x;
            int cy = y;

            while (!(parent[cy][cx].x == hx &&
                     parent[cy][cx].y == hy))
            {
                int px = parent[cy][cx].x;
                int py = parent[cy][cx].y;
                cx = px;
                cy = py;
            }

            if (cx > hx) return 'd';
            if (cx < hx) return 'a';
            if (cy > hy) return 's';
            return 'w';
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
            parent[ny][nx].x = x;
            parent[ny][nx].y = y;

            qx[qt] = nx;
            qy[qt++] = ny;
        }
    }

    /* fallback greedy */
    if (ax > hx) return 'd';
    if (ax < hx) return 'a';
    if (ay > hy) return 's';
    return 'w';
}

/* ---------------- MAIN UPDATE ---------------- */

void bot_update(char frame[H][W])
{
    init_x11();

    if (!find_head(frame) || !find_apple(frame))
        return;

    char move = bfs(frame);

    send_key(move);
}
