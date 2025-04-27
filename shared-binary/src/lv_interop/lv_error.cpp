#include <cstring>
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