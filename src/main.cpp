
#include <err.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "linefollowerapp.hpp"

#define OPTSTRING   "c:dhv"

// TODO: Daemonize
// TODO: Capture signals to exit gracefully
// TODO: Use config.h defines
// TODO: Add logging facility

// Configuration file
const char *config_file = 0;

// Daemonize or not this process
bool is_daemon = false;

// Print help message and exits
void
print_help()
{
    printf("Usage: follow [options ...]\n"
           "Options:\n"
           "  -h, --help                    Show this message and exit.\n"
           "  -v, --version                 Show version information\n."
           "  -c=CONFIG, --config=CONFIG    Give the configuration file.\n"
           "  -d, --daemonize               Daemonize this process.\n\n"

           "Antonio Serrano Hernandez (toni.serranoh@gmail.com)\n");
    exit(0);
}

// Print version message and exits
void
print_version()
{
    printf("follow 0.1\n"
           "Copyright (C) 2017 Antonio Serrano\n");
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
            case '?':
                exit(1);
            default:
                break;
        }
    } while (o != -1);

    // Give an error if config_file is not given
    if (!config_file)
        errx(1, "missing config file");
}

// Transform this process into a daemon
void daemonize()
{
}

int
main(int argc, char **argv)
{
    parse_args(argc, argv);
    // Daemonize this process, if necessary
    if (is_daemon) {
        daemonize();
    }
    LineFollowerApp a(config_file);
    a.run();
    return 0;
}

