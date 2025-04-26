#include <cstddef>
#include <vector>

#ifdef _WIN32
// for UTF8 to ansi conversion on windows
#include <cstring>
#include <Windows.h>
#endif

#include "ase/lv_interop/lv_str.hpp"
#include "ase/lv_interop/lv_functions.hpp"

using namespace ase;
using namespace lv_interop;

namespace
{
    bool convert_and_copy_utf8(const std::string &utf8_chars, LV_StringHandle_t dest, LV_StringHandle_t::multibyte_conversion_t conversion_type)
    {
        const bool success = true;
#ifdef _WIN32

        UINT code_page;

        switch(conversion_type){
            case LV_StringHandle_t::multibyte_conversion_t::NO_CONVERSION:{
                return !success;
            }
            case LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_ACP:{
                code_page = CP_ACP;
                break;
            }
            case LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_OEMCP:{
                code_page = CP_OEMCP;
                break;
            }
        }

        auto n_wide_chars = MultiByteToWideChar(CP_UTF8, 0, &utf8_chars[0], static_cast<int>(utf8_chars.length()), nullptr, 0);

        if (n_wide_chars <= 0)
        {
            // the widestring would have zero length so size_to_fit and return
            dest.size_to_fit(0);
            return success;
        }
        // convert to wide-string
        std::wstring wide(n_wide_chars, 0);
        MultiByteToWideChar(CP_UTF8, 0, &utf8_chars[0], static_cast<int>(utf8_chars.length()), wide.data(), n_wide_chars);

        // convert wide-string to ANSI
        auto n_ansi_chars = WideCharToMultiByte(code_page, 0, wide.data(), n_wide_chars, nullptr, 0, nullptr, nullptr);

        // resize string handle
        dest.size_to_fit(n_ansi_chars);

        if (n_ansi_chars == 0)
        {
            return success;
        }

        // copy ANSI into the string handle
        WideCharToMultiByte(code_page, 0, wide.data(), n_wide_chars, dest.begin(), n_ansi_chars, nullptr, nullptr);

        return success;
#endif

        return !success;
    }
}

size_t LV_StringHandle_t::capacity() const
{
    if (m_handle == nullptr)
    {
        return 0;
    }

    return DSGetHandleSize(reinterpret_cast<LV_UHandle_t>(m_handle)) - offsetof(LV_Str, data);
}

void LV_StringHandle_t::size_to_fit(size_t size)
{
    if (capacity() < size)
    {

        auto err = NumericArrayResize(0x01, 1, reinterpret_cast<LV_UHandlePtr_t>(&m_handle), size);

        if (err != LV_ERR_noError)
        {
            throw LV_MemoryManagerException(err);
        }
    }

    (*m_handle)->len = static_cast<int32_t>(size);
}

size_t LV_StringHandle_t::size() const
{
    if (m_handle == nullptr)
    {
        return 0;
    }

    return static_cast<size_t>((*m_handle)->len);
}

char *LV_StringHandle_t::begin() const
{
    if (m_handle == nullptr)
    {
        return nullptr;
    }

    return &((*m_handle)->data[0]);
}

char *LV_StringHandle_t::end() const
{
    if (m_handle == nullptr)
    {
        return nullptr;
    }

    return begin() + size();
}

LV_StringHandle_t::operator boost::string_view() const
{
    return boost::string_view{begin(), size()};
}

LV_StringHandle_t::operator boost::regex() const
{
    return boost::regex{std::string{begin(), size()}};
}

LV_StringHandle_t &LV_StringHandle_t::operator=(boost::string_view string)
{
    size_to_fit(string.length());
    std::memcpy(begin(), string.begin(), string.length());

    return *this;
}

LV_StringHandle_t &LV_StringHandle_t::operator=(const char *c)
{
    size_to_fit(std::strlen(c));
    std::memcpy(begin(), c, size());

    return *this;
}

LV_StringHandle_t::LV_StringHandle_t() : m_handle(nullptr)
{
    // nothing else to init
}

LV_StringHandle_t *LV_StringHandle_t::create(size_t starting_capacity)
{
    auto p = new LV_StringHandle_t();
    NumericArrayResize(0x01, 1, reinterpret_cast<LV_UHandlePtr_t>(&(p->m_handle)), starting_capacity);
    (*(p->m_handle))->len = 0;

    return p;
}

void LV_StringHandle_t::destroy(LV_StringHandle_t *p)
{
    DSDisposeHandle(reinterpret_cast<LV_UHandle_t>(p->m_handle));
}

void LV_StringHandle_t::consume_from_streambuf(std::shared_ptr<boost::asio::streambuf> buffer, size_t bytes, LV_StringHandle_t::multibyte_conversion_t conversion)
{

    boost::asio::streambuf::const_buffers_type cb = buffer->data();
    buffer->consume(bytes);

    if (conversion!=multibyte_conversion_t::NO_CONVERSION && convert_and_copy_utf8({boost::asio::buffers_begin(cb), boost::asio::buffers_begin(cb) + bytes}, *this, conversion))
    {
        return;
    }

    size_to_fit(bytes);
    std::copy_n(boost::asio::buffers_begin(cb), bytes, begin());
}

void LV_StringHandle_t::copy_from_string(const std::string& str, LV_StringHandle_t::multibyte_conversion_t conversion)
{
    if (conversion!=multibyte_conversion_t::NO_CONVERSION && convert_and_copy_utf8(str, *this, conversion))
    {
        return;
    }

    size_to_fit(str.length());
    std::memcpy(begin(), str.data(), str.length());
}

LV_StringHandle_t::operator std::filesystem::path() const
{
    return std::filesystem::path{std::string{begin(), size()}};
}