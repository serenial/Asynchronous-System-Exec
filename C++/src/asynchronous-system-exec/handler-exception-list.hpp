//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASYNCHRONOUS_SYSTEM_EXEC_HANDLER_EXCEPTION_LIST
#define ASYNCHRONOUS_SYSTEM_EXEC_HANDLER_EXCEPTION_LIST

#include <stdlib.h>
#include <vector>
#include <mutex>

// thread-safe list to store exceptions from callback functions in
// boost.asio handlers will be called in io_context thread but we might
// access the exception list at other times

class HandlerExceptionList
{
    public:
        struct handlerException;

    private:
        std::vector<handlerException> handlerExceptionList;
        std::mutex mutex;

    public:
        struct handlerException
        {
            std::string functionName, what;
            handlerException(std::string functionName, std::string what) : functionName(functionName), what(what){};
        };
        void add(std::string, std::string);

        size_t size();

        handlerException operator[](int);
};
#endif
