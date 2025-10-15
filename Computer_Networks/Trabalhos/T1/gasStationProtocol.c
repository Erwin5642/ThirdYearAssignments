#include "gasStationProtocol.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Simula um erro no arquivo 
char simulateError(){
    return ((float)rand() / RAND_MAX) > 0.5;
}

static const char *type[LOG_LEVEL_LEN] = {
    "INFO",
    "SUCC",
    "WARN",
    "ERR"
};

static const char *colors[LOG_LEVEL_LEN] = {
    "\x1b[0m",      // INFO
    "\x1b[32m",     // SUCCESS
    "\x1b[1;33m",   // WARNING
    "\x1b[31m"      // ERROR
};

// Exibe uma mensagem formatada seguindo padrao de log
void logMessage(LOG_LEVEL level, const char *fmt, ...)
{   
    time_t current_time;
    struct tm *m_time;
    pthread_mutex_lock(&lock);
    time(&current_time);
    m_time = localtime(&current_time);

    va_list args;
    va_start(args, fmt);

    printf("%s[%02d/%02d/%d] [%02d:%02d:%02d] [%s] ",
        colors[level],
        m_time->tm_mday,
        m_time->tm_mon + 1,
        m_time->tm_year + 1900,
        m_time->tm_hour,
        m_time->tm_min,
        m_time->tm_sec,
        type[level]);
    vprintf(fmt, args);
    printf("%s\n", colors[LOG_INFO]); 
    va_end(args);
    pthread_mutex_unlock(&lock);
}
