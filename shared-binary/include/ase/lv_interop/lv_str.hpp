#pragma once

#include <string>

#include <boost/utility/string_view.hpp>
#include <boost/asio.hpp>

#include "./lv_types.hpp"

namespace ase
{
    namespace lv_interop
    {
        class LV_StringHandle_t{
            private:
            #include "./set_packing.hpp"
            struct LV_Str{
                int32_t len;
                char data [1];
            };
            #include "./reset_packing.hpp"

            LV_Handle_t<LV_Str> m_handle;
            size_t capacity() const;
            void size_to_fit(size_t size);
            LV_StringHandle_t();

            public:
            char* begin() const;
            char* end() const;
            size_t size() const;
            static LV_StringHandle_t* create(size_t stating_capacity);
            static void destroy(LV_StringHandle_t* to_destroy);
            operator boost::string_view()  const;
            LV_StringHandle_t& operator=(const char* c);
            LV_StringHandle_t& operator=(boost::string_view str);
            void copy_from_streambuf(const boost::asio::streambuf &buffer, size_t bytes, bool convert_utf8);
            void copy_from_string(const std::string& str);
        };
    }
}