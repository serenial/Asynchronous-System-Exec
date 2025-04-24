#include <cstddef>

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

void LV_StringHandle_t::ensure_sized_to_fit(size_t size)
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
    ensure_sized_to_fit(string.length());
    std::memcpy(begin(), string.begin(), string.length());

    return *this;
}

LV_StringHandle_t &LV_StringHandle_t::operator=(const char* c)
{
    ensure_sized_to_fit(std::strlen(c));
    std::memcpy(begin(), c, size());

    return *this;
}

LV_StringHandle_t::LV_StringHandle_t() : m_handle(nullptr){
    // nothing else to init
}

LV_StringHandle_t *LV_StringHandle_t::create(size_t starting_capacity)
{
    auto p = new LV_StringHandle_t();
    NumericArrayResize(0x01, 1, reinterpret_cast<LV_UHandlePtr_t>(&(p->m_handle)), starting_capacity);
    (*(p->m_handle))->len = 0;

    return p;
}

void LV_StringHandle_t::destroy(LV_StringHandle_t* p){
    DSDisposeHandle(reinterpret_cast<LV_UHandle_t>(p->m_handle));
}