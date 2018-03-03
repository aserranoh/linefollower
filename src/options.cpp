
#include <fstream>
#include <stdlib.h>

#include "followexception.hpp"
#include "options.hpp"

Options::Options() {}

Options::Options(const char* options_file, const char** defaults)
{
    std::ifstream f(options_file);
    std::string line;
    size_t pos_eq;

    while (std::getline(f, line)) {
        // Jump comments and white lines
        if (line == "" || line[0] == '#') {
        } else {
            // Find the '=' character
            pos_eq = line.find('=');
            if (pos_eq == std::string::npos) {
                // '=' not found
                throw OPTIONS_PARSING_ERROR;
            } else {
                options[line.substr(0, pos_eq)] = line.substr(pos_eq + 1);
            }
        }
    }

    // Insert the default values
    for (int i = 0; defaults[i] != NULL; i += 2) {
        if (options.find(defaults[i]) == options.end()) {
            // The current element of the defaults is not in options.
            // Insert its default value
            options[defaults[i]] = defaults[i + 1];
        }
    }
}

// Get the value of an option and convert it to int
int
Options::get_int(const string& option) const
{
    return atoi(get_string(option).c_str());
}

// Get the value of an option and convert it to float
float
Options::get_float(const string& option) const
{
    return atof(get_string(option).c_str());
}

// Get the value of an option as string
const string&
Options::get_string(const string& option) const
{
    try {
        return options.at(option);
    } catch (std::out_of_range& e) {
        throw FollowException("missing option '" + option + "'");
    }
}

