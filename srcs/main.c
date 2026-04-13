#include "recorder.h"
#include <unistd.h>
#include <pty.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <termios.h>
#include <string.h>

void bot_update(int fd, char frame[H][W]);

/* ---------- STATE MACHINE ---------- */

enum {
	ST_MENU,
	ST_START,
	ST_GAME,
	ST_DEAD
};

static int state = ST_MENU;

static char history[512];
static size_t hlen = 0;

static void update_history(unsigned char *buf, size_t n)
{
	size_t copy = n;

	if (copy > sizeof(history) - hlen)
		copy = sizeof(history) - hlen;

	memcpy(history + hlen, buf, copy);
	hlen += copy;

	if (hlen > 256)
	{
		memmove(history, history + (hlen - 256), 256);
		hlen = 256;
	}
}

static int extract_score(void)
{
	for (ssize_t i = hlen - 6; i >= 0; i--)
	{
		if (memcmp(&history[i], "Score:", 6) == 0)
		{
			char *p = &history[i + 6];

			while (*p == ' ')
				p++;

			return atoi(p);
		}
	}
	return -1;
}

static void handle_prompts(int fd)
{
	if (state == ST_MENU && memmem(history, hlen, "(y/n)", 5))
	{
#if UBUNTU
		write(fd, "n\n", 2);
#endif
		write(fd, "y\n", 2);
		state = ST_START;
	}
	else if (state == ST_START && memmem(history, hlen, "Press ENTER", 11))
	{
		write(fd, "\n", 1);
		write(fd, "w", 1);
		state = ST_GAME;
	}
	else if (state == ST_GAME && memmem(history, hlen, "GAME OVER", 9))
	{
		state = ST_DEAD;
		if (extract_score() >= 900)
			write(fd, "UgoBot\n", 7);
		else
			write(fd, "\n", 1);
		write(fd, "n\n", 2);
	}
}

/* ---------- TERM ---------- */

static void set_raw_mode(int fd)
{
	struct termios tio;

	if (tcgetattr(fd, &tio) == -1)
		return;

	tio.c_lflag &= ~(ICANON | ECHO);
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	tcsetattr(fd, TCSANOW, &tio);
}

/* ---------- MAIN ---------- */

#if DEBUG
static void print_frame(char frame[H][W])
{
	char line[W + 1];

	write(STDOUT_FILENO, "\033[H\033[J", 6);

	for (int y = 0; y < H; y++)
	{
		for (int x = 0; x < W; x++)
			line[x] = frame[y][x];
		line[W] = '\n';

		write(STDOUT_FILENO, line, W + 1);
	}
}
#endif

int main(void)
{
	int master_fd;
	pid_t pid;

	unsigned char buf[4096];
	char frame[H][W];

	recorder_init();

	pid = forkpty(&master_fd, NULL, NULL, NULL);
	if (pid < 0)
	{
		perror("forkpty");
		return 1;
	}

	if (pid == 0)
	{
		execl("./bin/minisnake", "minisnake", NULL);
		perror("execl");
		exit(1);
	}

	set_raw_mode(master_fd);

	while (1)
	{
		ssize_t n = read(master_fd, buf, sizeof(buf));
		if (n <= 0)
			break;

		#if !DEBUG
			write(STDOUT_FILENO, buf, n);
		#endif
		update_history(buf, n);
		handle_prompts(master_fd);

		recorder_feed(buf, n);

		if (state == ST_GAME && recorder_frame_ready())
		{
			#if DEBUG
				print_frame(frame);
			#endif
			recorder_consume_frame(frame);
			bot_update(master_fd, frame);
		}
	}

	wait(NULL);
	return 0;
}
