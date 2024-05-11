#ifndef RETARGET_H
#define RETARGET_H

#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

void usart1_init(void);
void usart_printf(char* s);
uint8_t usart1_transmit(uint8_t* pdata, uint16_t size);

void RetargetInit(void);
int _isatty(int fd);
int _write(int fd, char* ptr, int len);
int _close(int fd);
int _lseek(int fd, int ptr, int dir);
int _read(int fd, char* ptr, int len);
int _fstat(int fd, struct stat* st);
#endif