
#ifndef LOG_HPP
#define LOG_HPP

/* Initializes the logging facility
   Parameters:
     * progname: program name to add to each logged message.
*/
void
log_init(const char *progname);

/* Log an error message.
   Parameters:
     * msg: the user's error message.
*/
void
log_err(const char *msg, ...);

/* Log an info message.
   Parameters:
     * msg: the user's info message.
*/
void
log_info(const char *msg, ...);

/* Log an warning message.
   Parameters:
     * msg: the user's warning message.
*/
void
log_warn(const char *msg, ...);

#endif

