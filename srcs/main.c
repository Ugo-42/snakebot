#include "recorder.h"
#include <unistd.h>

void bot_update(char frame[H][W]);

int main(void)
{
	unsigned char buf[4096];
	ssize_t n;

	recorder_init();

	char frame[H][W];

	while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
	{
		write(STDOUT_FILENO, buf, n);
		recorder_feed(buf, n);

		if (recorder_frame_ready())
		{
			recorder_consume_frame(frame);
			bot_update(frame);
		}
	}

	return 0;
}
