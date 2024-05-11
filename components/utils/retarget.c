#include "retarget.h"
#include <_ansi.h>
#include <_syslist.h>
#include <errno.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/times.h>

#if !defined(OS_USE_SEMIHOSTING)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

void usart1_init(void)
{
    // TODO
}

void RetargetInit()
{
    usart1_init();
    /* Disable I/O buffering for STDOUT stream, so that
     * chars are sent out as soon as they are printed. */
    setvbuf(stdout, NULL, _IONBF, 0);
}
int _isatty(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 1;
    errno = EBADF;
    return 0;
}

int _write(int fd, char* ptr, int len)
{
    uint8_t ustatus;
    if (fd == STDOUT_FILENO || fd == STDERR_FILENO) {
        // TODO
        // ustatus = usart1_transmit(ptr, len);
        if (ustatus == 0U)
            return len;
        else
            return EIO;
    }
    errno = EBADF;
    return -1;
}

int _close(int fd)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
        return 0;
    errno = EBADF;
    return -1;
}
int _lseek(int fd, int ptr, int dir)
{
    (void)fd;
    (void)ptr;
    (void)dir;
    errno = EBADF;
    return -1;
}

// FIXME: implement _read
// int _read(int fd, char* ptr, int len)
// {
//     HAL_StatusTypeDef hstatus;
//     if (fd == STDIN_FILENO) {
//         hstatus = HAL_UART_Receive(gHuart, (uint8_t*)ptr, 1, HAL_MAX_DELAY);
//         if (hstatus == HAL_OK)
//             return 1;
//         else
//             return EIO;
//     }
//     errno = EBADF;
//     return -1;
// }

int _fstat(int fd, struct stat* st)
{
    if (fd >= STDIN_FILENO && fd <= STDERR_FILENO) {
        st->st_mode = S_IFCHR;
        return 0;
    }
    errno = EBADF;
    return 0;
}

#endif // #if !defined(OS_USE_SEMIHOSTING)