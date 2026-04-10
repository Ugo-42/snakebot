#ifndef RECORDER_H
#define RECORDER_H

#include <sys/types.h>

#define H 22
#define W 27

void recorder_init(void);
void recorder_feed(unsigned char *buf, ssize_t n);

int recorder_frame_ready(void);
void recorder_consume_frame(char out[H][W]);

#endif
