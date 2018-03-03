
#include <err.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>

// Flag that indicates if the syslog facility must be used or not
bool use_syslog = false;

/* Initializes the logging facility
   Parameters:
     * progname: program name to add to each logged message.
*/
void
log_init(const char *progname)
{
    if (!isatty(2)) {
        use_syslog = true;
        openlog(progname, LOG_ODELAY, LOG_DAEMON);
    }
}

/* Log an error message.
   Parameters:
     * msg: the user's error message.
*/
void
log_err(const char *msg, ...)
{
    va_list vl;

    va_start(vl, msg);
    if (use_syslog) {
        vsyslog(LOG_ERR, msg, vl);
    } else {
        vwarnx(msg, vl);
    }
}

/* Log an info message.
   Parameters:
     * msg: the user's info message.
*/
void
log_info(const char *msg, ...)
{
    va_list vl;

    va_start(vl, msg);
    if (use_syslog) {
        vsyslog(LOG_INFO, msg, vl);
    } else {
        vwarnx(msg, vl);
    }
}

/* Log an warning message.
   Parameters:
     * msg: the user's warning message.
*/
void
log_warn(const char *msg, ...)
{
    va_list vl;

    va_start(vl, msg);
    if (use_syslog) {
        vsyslog(LOG_WARNING, msg, vl);
    } else {
        vwarnx(msg, vl);
    }
}

