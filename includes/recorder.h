#ifndef RECORDER_H
#define RECORDER_H

#include <sys/types.h>

#define H 22
#define W 27

typedef struct 
{
	int x;
	int y;
} vec_t;

void recorder_init(void);
void recorder_feed(unsigned char *buf, ssize_t n);

int recorder_frame_ready(void);
void recorder_consume_frame(char out[H][W]);

int recorder_get_head(vec_t *head);
int recorder_get_apple(vec_t *apple);

#endif
