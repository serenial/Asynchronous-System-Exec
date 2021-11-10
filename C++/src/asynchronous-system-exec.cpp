//          Copyright serenial.io and contributors.
// Distributed under the Boost Software License, Version 1.0.
//    (See https://www.boost.org/LICENSE_1_0.txt)

#ifndef ASE_VERSION
#define ASE_VERSION "X.Y.Z"
#endif

// std libs
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <vector>
#include <exception>
#include <future>
#include <thread>
#include <mutex>
#include <chrono>

// boost libs
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/process.hpp>
#include <boost/process/extend.hpp>
#include <boost/process/search_path.hpp>
#include <boost/process/group.hpp>
#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

// labVIEW Libs
#include <extcode.h>

// local includes
#include "asynchronous-system-exec/types.hpp"
#include "asynchronous-system-exec/memory-manager-utils.hpp"
#include "asynchronous-system-exec/handler-exception-list.hpp"

#ifdef BOOST_WINDOWS_API
#include <boost/winapi/process.hpp>
#include <boost/winapi/show_window.hpp>
#endif

// Wrap functionality into a class to persists between calls
class Interface
{
    // class to manage string event generation
    class OutputEventGenerator
    {
    private:
        const LStrHandle id;
        LVUserEventRef eventRef;
        outEventData eventData;
        size_t eventStringMaxSize;
        MgErr err = 0;
        void resizeEventDataHandle(size_t newSize)
        {
            // use numeric array resize as a string is really just an LV byte array
            // and it uses the correct number of bytes for string length
            throw_any_lv_memory_manager_errors(NumericArrayResize(uB, 1, (UHandle *)&(eventData.data), newSize));
            // update max size
            eventStringMaxSize = newSize;
        };

    public:
        OutputEventGenerator(const LStrHandle id, LVUserEventRef eventRef, size_t eventStringStartingSize)
            : id(id), eventData({0, id}), eventRef(eventRef)
        {
            // set eventStringHandle initial size
            resizeEventDataHandle(eventStringStartingSize);
        };

        ~OutputEventGenerator()
        {
            DSDisposeHandle(eventData.data);
        };

        void generateEvent(std::string message)
        {
            if (!err && eventRef && !message.empty())
            {
                if (message.length() > eventStringMaxSize)
                {
                    // resize
                    resizeEventDataHandle(message.length());
                }

                // Write message into StringHandle Buffer
                MoveBlock(message.c_str(), LHStrBuf(eventData.data), message.length());
                // Write string length to handle
                LStrLen(*eventData.data) = message.length();

                // Generate Event
                err = PostLVUserEvent(eventRef, (void *)&eventData);
            }
        };
    };

    // class to override default handler behaviour
    class ExtendedHandlers : public boost::process::extend::async_handler
    {
    private:
        const LStrHandle id;
        LVUserEventRef didExitEventRef;
        HandlerExceptionList *handlerExceptionListPtr;
        std::promise<int> exitCodePromise;

    public:
        ExtendedHandlers(const LStrHandle id, LVUserEventRef didExitEventRef, HandlerExceptionList *handlerExceptionListPtr)
            : id(id),
              didExitEventRef(didExitEventRef),
              handlerExceptionListPtr(handlerExceptionListPtr){/* nothing else to construct */};

// these have to be wrapped with an ifdef <platform> because they call boost::winapi which won't be defined for non-windows platforms
#ifdef BOOST_WINDOWS_API

        // on_setup for windows system
        template <typename Executor>
        void on_setup(boost::process::extend::windows_executor<char, Executor> &exec)
        {
            // set the flags to hide the process window
            exec.startup_info.dwFlags |= boost::winapi::STARTF_USESHOWWINDOW_;
            exec.startup_info.wShowWindow |= boost::winapi::SW_HIDE_;
        }

        // on_setup for windows system with wchar
        template <typename Executor>
        void on_setup(boost::process::extend::windows_executor<wchar_t, Executor> &exec)
        {
            // set the flags to hide the process window
            exec.startup_info.dwFlags |= boost::winapi::STARTF_USESHOWWINDOW_;
            exec.startup_info.wShowWindow |= boost::winapi::SW_HIDE_;
        }
#endif

        // add a function called on_exit_handler which will return the on_exit function when called at childProcess Launch
        template <typename Executor>
        std::function<void(int, const std::error_code &)> on_exit_handler(Executor &exec)
        {
            return [&](int exit_code, const std::error_code &ec)
            {
                didExitEventData data{exit_code, id};
                if (didExitEventRef)
                {
                    PostLVUserEvent(didExitEventRef, (void *)&data);
                }
                if (ec)
                {
                    handlerExceptionListPtr->add(__func__, ec.message());
                }
                exitCodePromise.set_value(exit_code);
            };
        }

        std::shared_future<int> get_child_process_exit_future()
        {
            return exitCodePromise.get_future();
        }
    };

    // class members
    const std::string cmd;
    const boost::filesystem::path workingDir;
    const LStrHandle idLStrHandle;
    boost::process::child childProcess;
    boost::process::group group;
    boost::asio::io_context io_context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work;
    const boost::regex stdOutSplitRegex, stdErrSplitRegex;
    boost::process::async_pipe stdInPipe, stdOutPipe, stdErrPipe;
    boost::asio::streambuf stdOutBuffer, stdErrBuffer;
    OutputEventGenerator stdOutEventGenerator, stdErrorEventGenerator;
    std::thread workerThread;
    std::shared_future<int> childProcessExitCodeFuture;
    const std::function<void(const boost::system::error_code, std::size_t)> stdOutHandler;
    const std::function<void(const boost::system::error_code, std::size_t)> stdErrHandler;
    ExtendedHandlers extendedHandlers;
    std::mutex stdInOperation;

public:
    HandlerExceptionList handlerExceptionList;

    // constructor
    Interface(std::string cmd, boost::filesystem::path workingDir, std::string id, LVUserEventRef stdOutEventRef, LVUserEventRef stdErrEventRef, LVUserEventRef didExitEventRef, std::string stdOutSplitRegexString, std::string stdErrSplitRegexString)
        : cmd(cmd),
          workingDir(workingDir),
          idLStrHandle([](std::string id)
                       {
                            // initialize handle with lambda
                            LStrHandle h = 0;
                            // allocate LStrHandle
                            throw_any_lv_memory_manager_errors(NumericArrayResize(uB, 1, (UHandle *)&(h), id.length()));
                            // copy string data into handle data
                            MoveBlock(id.c_str(), LHStrBuf(h), id.length());
                            // update handle data size
                            LStrLen(*h) = id.length();
                            return h; }(id)),
          handlerExceptionList(),
          io_context(),
          work(io_context.get_executor()), // use work_guard to prevent io_context.run() from exiting before childProcess is launched
          stdInPipe(io_context),
          stdOutPipe(io_context),
          stdErrPipe(io_context),
          stdOutEventGenerator(idLStrHandle, stdOutEventRef, 4096),
          stdErrorEventGenerator(idLStrHandle, stdErrEventRef, 4096),
          extendedHandlers(idLStrHandle, didExitEventRef, &handlerExceptionList),
          stdOutSplitRegex(stdOutSplitRegexString),
          stdErrSplitRegex(stdErrSplitRegexString),
          stdOutHandler([&](const boost::system::error_code &ec, std::size_t size)
                        {
        // check error-handler-code is non-zero
        if (!ec)
        {
            try
            {
                // get buffer data
                boost::asio::streambuf::const_buffers_type bufferData = stdOutBuffer.data();

                // move the received data out of the buffer
                stdOutBuffer.consume(size);

                // convert buffer to string - snip off delimiter
                std::string line(
                    boost::asio::buffers_begin(bufferData),
                    boost::asio::buffers_begin(bufferData) + size);

                // generate events
                stdOutEventGenerator.generateEvent(line);

                // reattach handler to go again
                boost::asio::async_read_until(stdOutPipe, stdOutBuffer, stdOutSplitRegex, stdOutHandler);
            }
            catch (std::exception &e)
            {
                handlerExceptionList.add(__func__, e.what());
            }
        } }),
          stdErrHandler([&](const boost::system::error_code &ec, std::size_t size)
                        {
        // check error-handler-code is non-zero
        if (!ec)
        {
            try
            {
                // get buffer data
                boost::asio::streambuf::const_buffers_type bufferData = stdErrBuffer.data();

                // move the received data out of the buffer
                stdErrBuffer.consume(size);

                // convert buffer to string - snip off delimiter
                std::string line(
                    boost::asio::buffers_begin(bufferData),
                    boost::asio::buffers_begin(bufferData) + size);

                // generate events
                stdErrorEventGenerator.generateEvent(line);

                // reattach handler to go again
                boost::asio::async_read_until(stdErrPipe, stdErrBuffer, stdErrSplitRegex, stdErrHandler);
            }
            catch (std::exception &e)
            {
                handlerExceptionList.add(__func__, e.what());
            }
        } }),
          workerThread([=]()
                       { io_context.run(); })
    {
        // shared_future which will wait until the exit handler has been called
        childProcessExitCodeFuture = extendedHandlers.get_child_process_exit_future();
        // attach Pipe-Readers
        boost::asio::async_read_until(stdOutPipe, stdOutBuffer, stdOutSplitRegex, stdOutHandler);
        boost::asio::async_read_until(stdErrPipe, stdErrBuffer, stdErrSplitRegex, stdErrHandler);

        // interface is now ready to have startCall called.
        // do not call here as any exceptions thrown by childProcess will lead to the interface
        // being de-initialized which will likely cause more exceptions
    };

    void startCall(){
        if (workingDir.empty())
            {
                childProcess = boost::process::child{
                    boost::process::cmd = cmd,
                    boost::process::std_out > stdOutPipe,
                    boost::process::std_err > stdErrPipe,
                    boost::process::std_in < stdInPipe,
                    extendedHandlers,
                    io_context,
                    group};
            }
            else
            {
                childProcess = boost::process::child{
                    boost::process::cmd = cmd,
                    boost::process::start_dir = workingDir.string(),
                    boost::process::std_out > stdOutPipe,
                    boost::process::std_err > stdErrPipe,
                    boost::process::std_in < stdInPipe,
                    extendedHandlers,
                    io_context,
                    group};
            }
        // reset work_guard so io_context.run() will finish when boost::process::child exits
        work.reset();
    }

    bool writeToStdIn(std::string data)
    {
        const std::lock_guard<std::mutex> lock(stdInOperation);
        if (!stdInPipe.is_open())
        {
            return true;
        }
        boost::asio::write(stdInPipe, boost::asio::buffer(data));
        return false;
    };

    bool closeStdIn()
    {
        const std::lock_guard<std::mutex> lock(stdInOperation);
        if (!stdInPipe.is_open())
        {
            return true;
        }
        stdInPipe.async_close();
        return false;
    }

    bool sendTerminate()
    {
        if (!childProcess.running())
        {
            return true;
        }

        group.terminate();
        return false;
    }

    std::thread startWaitOnChildProcessExitThread(LVUserEventRef exitTimeoutEventRef, int timeout_ms)
    {
        return std::thread([=](std::shared_future<int> future)
                           {
        auto status = std::future_status::ready;
        if (timeout_ms < 0)
        {
            future.wait();
        }
        else
        {
            status = future.wait_for(std::chrono::milliseconds(timeout_ms));
        }

        LVBoolean data = (status == std::future_status::timeout) ? LVBooleanTrue : LVBooleanFalse;
        if(exitTimeoutEventRef){
            PostLVUserEvent(exitTimeoutEventRef, (void *)&data);
        } },
                           childProcessExitCodeFuture);
    }

    int waitForChildProcessExit()
    {
        // return exit code from future
        return childProcessExitCodeFuture.get();
    }

    ~Interface()
    {
        // close all the pipes
        stdInPipe.async_close();
        stdOutPipe.async_close();
        stdErrPipe.async_close();

        // remove the work_guard
        work.reset();

        // stop
        io_context.stop();

        // wait for worker thread to finish
        if (workerThread.joinable())
        {
            workerThread.join();
        }

        // clean-up the id LStrHandle
        DSDisposeHandle(idLStrHandle);
    }
};

extern "C"
{
    ASE_API MgErr ASE_version(LVErrorCluster *errorPtr, LStrHandle versionStrHandle)
    {
        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }
        try
        {
            std::string version = ASE_VERSION;

            lv_string_write_to_handle_ptr(&versionStrHandle, version);
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }

        return ERR_NO_ERROR;
    }

    ASE_API MgErr ASE_startCall(
        LVErrorCluster *errorPtr,
        Interface **interfaceHandle,
        LStrHandle idStrHandle,
        LStrHandle cmdStrHandle,
        Path workingDirectory,
        StreamSplitRegex *streamSplitRegexPtr,
        StartCallEventRefs *eventRefsPtr)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfaceHandle || !idStrHandle || !cmdStrHandle || !workingDirectory || !streamSplitRegexPtr || !eventRefsPtr)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        Interface* interface=0;

        try
        {
            // create strings from LStrHandles
            std::string id = lv_string_handle_to_string(idStrHandle);
            std::string cmd = lv_string_handle_to_string(cmdStrHandle);
            std::string stdOutStreamSplitRegexString = lv_string_handle_to_string(streamSplitRegexPtr->stdOutRegex);
            std::string stdErrStreamSplitRegexString = lv_string_handle_to_string(streamSplitRegexPtr->stdErrRegex);

            // create a LStrHandle of some length to allow for conversion from LVPath Type => std::string => boost::filesystem::path
            LStrHandle pathLStrHandle = 0;
            throw_any_lv_memory_manager_errors(NumericArrayResize(uB, 1, (UHandle *)&pathLStrHandle, 255));

            // determine if using a different working directory
            boost::filesystem::path pwd;
            int32 workingDirType = -1;

            if (!FGetPathType(workingDirectory, &workingDirType) && workingDirType == fAbsPath)
            {
                // convert Path to LStrHandle
                FPathToDSString(workingDirectory, &pathLStrHandle);
                // create string from LStrHandle and push into vector
                pwd = boost::filesystem::path(lv_string_handle_to_string(pathLStrHandle));
            }

            // dispose of pathLStrHandle
            throw_any_lv_memory_manager_errors(DSDisposeHandle(pathLStrHandle));

            // create interface
            interface = new Interface(cmd, pwd, id, eventRefsPtr->stdOutEventRef, eventRefsPtr->stdErrorEventRef, eventRefsPtr->didExitEventRef, stdOutStreamSplitRegexString, stdErrStreamSplitRegexString);

            // start call
            interface->startCall();
        }
        catch(boost::process::process_error &e){
            if(interface){
                delete interface;
            }
            return lv_error_cluster_write_err(errorPtr, ERR_UNABLE_TO_LAUNCH_EXE, __func__, "Unable to launch the executable as a child process.\n["+ boost::algorithm::trim_copy(std::string(e.what())) + "]");
        }
        catch (boost::regex_error)
        {
            if(interface){
                delete interface;
            }
            return lv_error_cluster_write_err(errorPtr, ERR_BAD_REGEX_EXPRESSION, __func__, "Invalid Std Output/Std Error stream split regular expression.");
        }
        catch (std::exception &e)
        {
            if(interface){
                delete interface;
            }
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            if(interface){
                delete interface;
            }
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }

        *interfaceHandle = interface;

        return ERR_NO_ERROR;
    }

    ASE_API MgErr ASE_destroy(LVErrorCluster *errorPtr, Interface *interfacePtr, int32 *exitCodePtr, LVArray<1, LVErrorCluster> ***handlerErrorListHandlePtr)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfacePtr || !handlerErrorListHandlePtr || !exitCodePtr)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        try
        {
            // copy handler errors into handlerErrorListHandle
            size_t errorListSize = *handlerErrorListHandlePtr ? (**handlerErrorListHandlePtr)->dims[0] : 0;
            size_t handlerErrorsCount = interfacePtr->handlerExceptionList.size();

            // dispose of any unnecessary incoming values
            for (int i = handlerErrorsCount; i < errorListSize; i++)
            {
                LVErrorCluster errorToDeallocate = (**handlerErrorListHandlePtr)->data[i];
                // deallocate source string
                throw_any_lv_memory_manager_errors(DSDisposeHandle(errorToDeallocate.source));
            }

            // resize handlerErrorList
            if (errorListSize < handlerErrorsCount)
            {
                throw_any_lv_memory_manager_errors(NumericArrayResize(LV_U8_TYPECODE, 1, (UHandle *)handlerErrorListHandlePtr, handlerErrorsCount * sizeof(LVErrorCluster)));
            }

            // copy each exception into the handlerErrorsList
            for (size_t i = 0; i < handlerErrorsCount; i++)
            {
                auto exception = interfacePtr->handlerExceptionList[i];
                lv_error_cluster_write_std_exception(&((**handlerErrorListHandlePtr)->data[i]), exception.functionName, exception.what);
            }

            // update the handlerErrorsList array size - handle may be null if there have been no exceptions
            if (*handlerErrorListHandlePtr)
            {
                (**handlerErrorListHandlePtr)->dims[0] = handlerErrorsCount;
            }

            // blocking wait for io_context to finish
            *exitCodePtr = interfacePtr->waitForChildProcessExit();

            // delete the interface
            delete interfacePtr;
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }
        return ERR_NO_ERROR;
    }

    ASE_API MgErr ASE_writeToStdIn(LVErrorCluster *errorPtr, Interface *interfacePtr, LStrHandle input, LVBoolean *alreadyClosedPtr)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfacePtr || !alreadyClosedPtr)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        try
        {
            *alreadyClosedPtr = interfacePtr->writeToStdIn(lv_string_handle_to_string(input));
        }
        catch (boost::system::system_error &e)
        {
            // pipe is closing/closed
            *alreadyClosedPtr = true;
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }
        return ERR_NO_ERROR;
    }

    ASE_API MgErr ASE_closeStdIn(LVErrorCluster *errorPtr, Interface *interfacePtr, LVBoolean *alreadyClosedPtr)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfacePtr || !alreadyClosedPtr)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        try
        {
            *alreadyClosedPtr = interfacePtr->closeStdIn();
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }
        return ERR_NO_ERROR;
    }

    // use cluster to pass event refs to ASE_startCall otherwise the call library function node
    // might pass a User Reference from a previous call when attempting to pass a pointer to
    // an "Invalid/Uninitialized" User Event Reference

    ASE_API MgErr ASE_startWaitOnCall(LVErrorCluster *errorPtr, Interface *interfacePtr, WaitOnCallEventRefs *eventRefsPtr, int32 timeout_ms)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfacePtr || !eventRefsPtr)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        try
        {
            if (eventRefsPtr->didTimeoutEventRef)
            {
                interfacePtr->startWaitOnChildProcessExitThread(eventRefsPtr->didTimeoutEventRef, timeout_ms).detach();
            }
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }
        return ERR_NO_ERROR;
    }

    ASE_API MgErr ASE_sendTerminate(LVErrorCluster *errorPtr, Interface *interfacePtr, LVBoolean *alreadyExited)
    {

        // check ability to write out error
        if (!errorPtr)
        {
            return ERR_INVALID_ERROR_PTR;
        }

        // check other handles
        if (!interfacePtr || !alreadyExited)
        {
            return lv_error_cluster_write_null_ptr_err(errorPtr, __func__);
        }

        try
        {
            *alreadyExited = interfacePtr->sendTerminate();
        }
        catch (std::exception &e)
        {
            return lv_error_cluster_write_std_exception(errorPtr, __func__, e.what());
        }
        catch (...)
        {
            return lv_error_cluster_write_unknown_err(errorPtr, __func__);
        }
        return ERR_NO_ERROR;
    }
}