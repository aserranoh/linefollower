
#include <getopt.h>
#include <stdio.h>

#include "daemon.h"
#include "linefollowerapp.hpp"
#include "log.hpp"

// Short options:
//   * h: help
//   * v: version
//   * c: configuration file
//   * d: daemonize
//   * p: pid file
#define OPTSTRING   "hvc:dp:"

// Name of the program, to use it in the version and help string
#define PROGNAME    "follow"

// Configuration file, that contains the application options
#define DEFAULT_CONFIGFILE  SYSCONFDIR "/follow.conf"

// Configuration file
const char *config_file = DEFAULT_CONFIGFILE;

// Flag that tells if this process must be daemonized
int is_daemon = 0;

// Pid file to identify the daemon
const char *pidfile = 0;

// LineFollowerApp instance
LineFollowerApp *app = 0;

/* Signals handler.
   This handler is executed upon reception of the signals SIGINT and SIGTERM.
   The stop flag it is set, which indicates that the process must finish.

   Parameters:
     * signum: the signal received (although it is not used).
*/
void
signal_handler(int signum)
{
    // Set the done flag
    if (app) {
        log_info("signal received");
        app->stop();
    };
}

// Print help message and exits
void
print_help()
{
    printf("Usage: " PROGNAME " [options]\n"
"Options:\n"
"  -h, --help                  Show this message and exit.\n"
"  -v, --version               Show version information.\n"
"  -c PATH, --config PATH      Give the configuration file.\n"
"  -d, --daemonize             Daemonize this process.\n"
"  -p PATH, --pidfile PATH     Create a pidfile.\n\n"

"Report bugs to:\n"
"Antonio Serrano Hernandez (" PACKAGE_BUGREPORT ")\n"
    );
    exit(0);
}

// Print version message and exits
void
print_version()
{
    printf(PROGNAME " (" PACKAGE_NAME ") " PACKAGE_VERSION "\n"
"Copyright (C) 2018 Antonio Serrano\n"
"This is free software; see the source for copying conditions.  There is NO\n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
    );
    exit(0);
}

// Parse the command line arguments
void
parse_args(int argc, char **argv)
{
    struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"config", required_argument, 0, 'c'},
        {"daemonize", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };
    int o;

    do {
        o = getopt_long(argc, argv, OPTSTRING, long_opts, 0);
        switch (o) {
            case 'h':
                print_help();
            case 'v':
                print_version();
            case 'c':
                config_file = optarg;
                break;
            case 'd':
                is_daemon = true;
                break;
            case 'p':
                pidfile = optarg;
                break;
            case '?':
                exit(1);
            default:
                break;
        }
    } while (o != -1);
}

// Set the handler for the signals SIGINT and SIGTERM, to stop this process.
void
set_signals()

{
    struct sigaction sa;
    sigset_t mask;

    sigemptyset(&mask);
    sa.sa_handler = signal_handler;
    sa.sa_mask = mask;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int
main(int argc, char **argv)
{
    int retcode = 0;

    try {
        parse_args(argc, argv);
        // Set a handler for the signals SIGINT and SIGTERM to have a mechanism
        // to stop this process.
        set_signals();
        // Daemonize, if demanded
        if (is_daemon) {
            if (daemonize(pidfile)) {
                return 1;
            }
        }
        // Initialize log facility
        log_init(PROGNAME);
        log_info("starting");
        // Start line following application
        LineFollowerApp a(config_file);
        app = &a;
        a.run();
    } catch (exception &e) {
        log_err(e.what());
        retcode = 1;
    }
    log_info("terminating");
    return retcode;
}

