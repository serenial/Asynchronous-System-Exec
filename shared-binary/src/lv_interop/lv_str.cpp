#include <cstddef>
#include <vector>
#include <optional>

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

    #ifdef _WIN32
    std::optional<UINT> to_code_page_flag(LV_StringHandle_t::multibyte_conversion_t conversion_type){

        switch(conversion_type){
            case LV_StringHandle_t::multibyte_conversion_t::NO_CONVERSION:{
                return {};
            }
            case LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_ACP:{
                return CP_ACP;
            }
            case LV_StringHandle_t::multibyte_conversion_t::UTF8_TO_CP_OEMCP:{
                return CP_OEMCP;
            }
        }

            return {};
    }
    #endif


    bool convert_and_copy_utf8(const char* c, size_t length ,LV_StringHandle_t dest, LV_StringHandle_t::multibyte_conversion_t conversion_type)
    {
        const bool success = true;
#ifdef _WIN32

        auto cpf_opt = to_code_page_flag(conversion_type);

        if(!cpf_opt.has_value()){
            return !success;
        }
        
        auto n_wide_chars = MultiByteToWideChar(CP_UTF8, 0, c, length, nullptr, 0);

        if (n_wide_chars <= 0)
        {
            // the widestring would have zero length so size_to_fit and return
            dest.size_to_fit(0);
            return success;
        }
        // convert to wide-string
        std::wstring wide(n_wide_chars, 0);
        MultiByteToWideChar(CP_UTF8, 0, c, length, wide.data(), n_wide_chars);

        // convert wide-string to ANSI
        auto n_ansi_chars = WideCharToMultiByte(*cpf_opt, 0, wide.data(), n_wide_chars, nullptr, 0, nullptr, nullptr);

        // resize string handle
        dest.size_to_fit(n_ansi_chars);

        if (n_ansi_chars == 0)
        {
            return success;
        }

        // copy ANSI into the string handle
        WideCharToMultiByte(*cpf_opt, 0, wide.data(), n_wide_chars, dest.begin(), n_ansi_chars, nullptr, nullptr);

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

    if (convert_and_copy_utf8(&(*boost::asio::buffers_begin(cb)), bytes, *this, conversion))
    {
        return;
    }

    size_to_fit(bytes);
    std::copy_n(boost::asio::buffers_begin(cb), bytes, begin());
}

void LV_StringHandle_t::copy_from_char_ptr(const char* c, LV_StringHandle_t::multibyte_conversion_t conversion)
{
    auto length = std::strlen(c);
    if (convert_and_copy_utf8(c, length, *this, conversion))
    {
        return;
    }

    size_to_fit(length);
    std::memcpy(begin(), c, length);
}

void LV_StringHandle_t::copy_from_char_ptr(const wchar_t* c, LV_StringHandle_t::multibyte_conversion_t conversion)
{
    auto length = std::wcslen(c);

    if (conversion!=multibyte_conversion_t::NO_CONVERSION)
    {
        auto cpf_opt = to_code_page_flag(conversion);

        // convert wide-string to ANSI
        auto n_ansi_chars = WideCharToMultiByte(*cpf_opt, 0, c, length, nullptr, 0, nullptr, nullptr);

        // resize string handle
        size_to_fit(n_ansi_chars);

        if (n_ansi_chars == 0)
        {
            return;
        }

        // copy ANSI into the string handle
        WideCharToMultiByte(*cpf_opt, 0, c, length, begin(), n_ansi_chars, nullptr, nullptr);

        return;
    }

    // not converting - just copy wide-string bytes into string handle
    auto n_bytes = length * sizeof(wchar_t);
    size_to_fit(n_bytes);
    std::memcpy(begin(), c, n_bytes);
}

LV_StringHandle_t::operator std::filesystem::path() const
{
    return std::filesystem::path{std::string{begin(), size()}};
}