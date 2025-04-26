#pragma once

#include <memory>
#include <string>
#include <filesystem>

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
            LV_StringHandle_t();

            public:
            enum class multibyte_conversion_t {
                NO_CONVERSION,
                UTF8_TO_CP_ACP,
                UTF8_TO_CP_OEMCP
            };
            char* begin() const;
            char* end() const;
            size_t size() const;
            void size_to_fit(size_t size);
            static LV_StringHandle_t* create(size_t stating_capacity);
            static void destroy(LV_StringHandle_t* to_destroy);
            operator boost::string_view()  const;
            operator std::filesystem::path()  const;
            LV_StringHandle_t& operator=(const char* c);
            LV_StringHandle_t& operator=(boost::string_view str);
            void consume_from_streambuf(std::shared_ptr<boost::asio::streambuf> buffer, size_t bytes, multibyte_conversion_t conversion);
            void copy_from_string(const std::string& str, multibyte_conversion_t conversion);
        };
    }
}