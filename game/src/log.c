#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "game/log.h"
#include "game/game.h"
#include "util/memory.h"

#ifdef ENABLE_LOG_TIMESTAMPS
#   include <time.h>
#endif

/* ------------------------------------------------------------------------- */
char
log_global_open(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */
void
log_global_close(void)
{

}

/* ------------------------------------------------------------------------- */
char
log_init(struct game_t* game)
{
    /* TODO open log file */

    return 1;
}

/* ------------------------------------------------------------------------- */
void
log_message(log_level_e level,
     const struct game_t* game,
     const char* fmt,
     ...)
{
    /* variables required to generate a timestamp string */
#ifdef ENABLE_LOG_TIMESTAMPS
    time_t rawtime;
    struct tm* timeinfo;
    char timestamp[12];
#endif

    /* more local variables because C89 */
    va_list ap;
    uint32_t total_length = 0;
    char* buffer = NULL;
    char* tag = NULL;

    /*
     * Get timestamp string.
     * NOTE also sets the total length to the length of the timestamp string.
     */
#ifdef ENABLE_LOG_TIMESTAMPS
    rawtime = time(NULL); /* get system time */
    timeinfo = localtime(&rawtime); /* convert to local time */
    total_length = strftime(timestamp, 12, "[%X] ", timeinfo);
#endif

    /* determine tag string */
    switch(level)
    {
        case LOG_INFO:
            tag = "[INFO] ";
            break;
        case LOG_WARNING:
            tag = "[WARNING] ";
            break;
        case LOG_ERROR:
            tag = "[ERROR] ";
            break;
        case LOG_FATAL:
            tag = "[FATAL] ";
            break;
        case LOG_USER:
            tag = "[USER] ";
            break;
        default:
            tag = "";
            break;
    }
    total_length += strlen(tag);

    /* add length of game string, plus three characters for [] and space */
    if(game)
    {
        total_length += strlen(game->name) + 3;
    }

    /*
     * Get total length of all strings combined and allocate a buffer large
     * enough to hold them, including a null terminator.
     */
    va_start(ap, fmt);
    total_length += vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);

    /* null terminator and newline */
    total_length += 2;

    /* allocate buffer and copy all strings into it */
    buffer = (char*)MALLOC(sizeof(char) * total_length, "log_message()");
    *buffer = '\0'; /* so strcat() works */

    /* copy timestamp into buffer */
#ifdef ENABLE_LOG_TIMESTAMPS
    strcat(buffer, timestamp);
#endif

    /* copy tag */
    strcat(buffer, tag);

    /* copy game name */
    if(game)
    {
        strcat(buffer, "[");
        strcat(buffer, game->name);
        strcat(buffer, "] ");
    }

    /* copy varargs into buffer and end with newline */
    {
        int off = strlen(buffer);
        va_start(ap, fmt);
        vsnprintf(buffer + off, total_length - off, fmt, ap);
        va_end(ap);
    }

    /* null terminator and newline */
    buffer[total_length-2] = '\n';
    buffer[total_length-1] = '\0';

    /* fire event and output message */

    /* output message with the appropriate colours */
    switch(level)
    {
        default:
        case LOG_INFO:      fprintf(stdout, "%s", buffer);             break;
        case LOG_WARNING:   fprintf(stderr, KYEL "%s" RESET, buffer);  break;
        case LOG_ERROR:     fprintf(stderr, KMAG "%s" RESET, buffer);  break;
        case LOG_FATAL:     fprintf(stderr, KRED "%s" RESET, buffer);  break;
        case LOG_USER:      fprintf(stdout, KCYN "%s" RESET, buffer);  break;
    }

    FREE(buffer);
}

/* ------------------------------------------------------------------------- */
void
log_critical_use_no_memory(const char* message)
{
    printf("[FATAL] %s\n", message);
}
