#ifndef SUB_SERVER_H_
#define SUB_SERVER_H_

void *startSubserver(void *arg);
void error_handling(char *message);
void *thr_read(void *arg);
void *thr_write(void *arg);

#endif
