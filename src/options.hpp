
#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include <map>
#include <string>

using namespace std;

typedef enum {
    OPTIONS_OK,
    OPTIONS_PARSING_ERROR
} options_err_t;

class Options {

    public:

        Options();
        Options(const char* options_file, const map<string, string>& defaults);

        // Get the value of an option and convert it to int
        int get_int(const string& option) const;

        // Get the value of an option and convert it to float
        float get_float(const string& option) const;

        // Get the value of an option as string
        const string& get_string(const string& option) const;

    private:

        map<string, string> options;

};

#endif

