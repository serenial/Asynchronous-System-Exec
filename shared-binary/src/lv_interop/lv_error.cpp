#include <cstring>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <boost/regex.hpp>
#include <boost/process.hpp>

#include "ase/lv_interop/lv_error.hpp"

using namespace ase;
using namespace lv_interop;

namespace{
    const int32_t ERR_UNKNOWN_ERROR= -200;
    const int32_t ERR_STD_EXCEPTION= -201;
    
    // Defined Errors
    const int32_t ERR_BAD_REGEX_EXPRESSION= -300;
    const int32_t ERR_UNABLE_TO_LAUNCH_EXE= -301;
}

void LV_ErrorClusterPtr_t::copy_from_exception(std::exception_ptr ex, const char *caller_name)
{

    std::stringstream ss;

    ss << caller_name << "<ERR>";

    try
    {
        if (ex)
        {
            std::rethrow_exception(ex);
        }
    }
    catch(boost::regex_error const &e){
        ss << "Bad regular-expression for the std-out or std-err match (" << e.what() << ")";
        m_err->code = ERR_BAD_REGEX_EXPRESSION;
    }
    catch(boost::system::system_error const &e){
        ss << "Invalid executable or argument supplied (" << e.what() << ")";
        m_err->code = ERR_UNABLE_TO_LAUNCH_EXE;
    }
    catch (LV_MemoryManagerException const &e)
    {
        ss << e.what();
        m_err->code = e.err;
    }
    catch (std::system_error const &e)
    {
        ss << e.what();
        m_err->code = e.code().value();
    }
    catch (std::exception const &e)
    {
        ss << e.what();
        m_err->code = ERR_STD_EXCEPTION;
        ;
    }
    catch (...)
    {
        ss << "An undefined exception occurred.";
        m_err->code = ERR_UNKNOWN_ERROR;
    }

    m_err->status = m_err->code != 0;

    m_err->source = ss.str();
}

LV_ErrorClusterPtr_t* LV_ErrorClusterPtr_t::create(std::exception_ptr ex, const char *caller_name){

    auto p = new LV_ErrorClusterPtr_t();
    p->m_err = reinterpret_cast<LV_Error_t*>(std::malloc(sizeof(LV_Error_t)));

    if(p->m_err == nullptr){
        throw std::runtime_error("Unable to allocate memory");
    }

    // allocate the string handle - this will need to be destroyed when the error ptr is destroyed
    p->m_err->source = *LV_StringHandle_t::create(1024);

    p->copy_from_exception(ex, caller_name);
    
    return p;
}

void LV_ErrorClusterPtr_t::destroy(LV_ErrorClusterPtr_t *p){

    if(p == nullptr){
        return;
    }

    // deallocate source string
    LV_StringHandle_t::destroy(&(p->m_err->source));

    // deallocate the rest of the memory
    std::free(p->m_err);

    p = nullptr;
}

LV_ErrorClusterPtr_t::operator void *() const{
    return m_err;
}