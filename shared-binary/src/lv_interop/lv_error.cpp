#include <cstring>
#include <string>
#include <sstream>

#include "ase/lv_interop/lv_error.hpp"

using namespace ase;
using namespace lv_interop;

void LV_ErrorClusterPtr_t::copy_from_exception(std::exception_ptr ex, const char * caller_name){

    std::stringstream ss;

    ss << caller_name << "<ERR>";

    try
    {
        if (ex)
        {
            std::rethrow_exception(ex);
        }
    }
    catch (LV_MemoryManagerException const&e)
    {
        ss << e.what();
        m_err->code = e.err;
    }
    catch (std::system_error const&e){
        ss << e.what();
        m_err->code = e.code().value();
    }
    catch (std::exception const&e)
    {
        ss << e.what();
        m_err->code = LV_ERR_bogusError;
    }
    catch (...)
    {
        ss << "An undefined exception occurred.";
        m_err->code = LV_ERR_bogusError;
    }

    m_err->status = m_err->code != 0;

    m_err->source.copy_from(ss.str());
}