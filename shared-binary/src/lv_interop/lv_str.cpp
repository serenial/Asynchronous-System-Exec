#include <cstddef>

#ifdef _WIN32
// for UTF8 to acii conversion on windows
#include <cstring>
#include <Windows.h>
#endif

#include "ase/lv_interop/lv_str.hpp"
#include "ase/lv_interop/lv_functions.hpp"

using namespace ase;
using namespace lv_interop;

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

void LV_StringHandle_t::copy_from_streambuf(const boost::asio::streambuf &buffer, size_t bytes, bool convert_utf8)
{

    boost::asio::streambuf::const_buffers_type cb = buffer.data();

#ifdef _WIN32
    if (convert_utf8)
    {

        std::string utf8_chars{boost::asio::buffers_begin(cb), boost::asio::buffers_begin(cb) + bytes};

        auto n_bytes_wide_string = MultiByteToWideChar(CP_UTF8, 0, &utf8_chars[0], static_cast<int>(bytes), NULL, 0);

        if (n_bytes_wide_string <= 0)
        {
            // the widestring would have zero length so size_to_fit and return
            size_to_fit(0);
            return;
        }
        // convert to wide-string
        std::wstring wide(n_bytes_wide_string, 0);
        MultiByteToWideChar(CP_UTF8, 0, &utf8_chars[0], static_cast<int>(bytes), &wide[0], n_bytes_wide_string);

        // convert wide-string to ANSI
        auto n_bytes_ansi = WideCharToMultiByte(CP_ACP, 0, &wide[0], n_bytes_wide_string, NULL, 0, NULL, NULL);

        // resize string handle
        size_to_fit(n_bytes_ansi);

        if (n_bytes_ansi == 0)
        {
            return;
        }

        // copy ANSI into the string handle
        WideCharToMultiByte(CP_ACP, 0, &wide[0], n_bytes_wide_string, begin(), n_bytes_ansi, NULL, NULL);
    }
#endif

    size_to_fit(bytes);
    std::copy_n(boost::asio::buffers_begin(cb), bytes, begin());
}

void LV_StringHandle_t::copy_from_string(const std::string &str)
{
    size_to_fit(str.length());
    std::memcpy(begin(), str.data(), str.length());
}