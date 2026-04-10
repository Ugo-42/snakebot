#include "recorder.h"
#include <stdio.h>

void bot_update(char frame[H][W])
{
	FILE *f = fopen("frame.txt", "a");

	if (!f)
		return;

	fprintf(f, "FRAME\n");

	for (int y = 0; y < H; y++)
	{
		fwrite(frame[y], 1, W, f);
		fputc('\n', f);
	}

	fputc('\n', f);
	fclose(f);
}
