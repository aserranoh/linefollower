
#ifndef FOLLOWEXCEPTION_HPP
#define FOLLOWEXCEPTION_HPP

#include <exception>
#include <string>

using namespace std;

class FollowException: public exception {

    public:

        FollowException(const string& msg): msg(msg) {}
        virtual ~FollowException() throw() {}
        virtual const char * what() const throw() {return msg.c_str();}

    private:

        string msg;
};

#endif

