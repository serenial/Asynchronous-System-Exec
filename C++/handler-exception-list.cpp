//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)


// thread-safe list to store exceptions from callback functions
// boost.asio handlers will be called in io_context thread but we might
// access the exception list from other places

#include "./handler-exception-list.hpp"

void HandlerExceptionList::add(std::string functionName, std::string exceptionDetails)
{
    const std::lock_guard<std::mutex> lock(mutex);
    handlerExceptionList.emplace_back(functionName, exceptionDetails);
};

size_t HandlerExceptionList::size()
{
    const std::lock_guard<std::mutex> lock(mutex);
    return handlerExceptionList.size();
};

HandlerExceptionList::handlerException HandlerExceptionList::operator[](int index)
{
    const std::lock_guard<std::mutex> lock(mutex);
    return handlerExceptionList[index];
};