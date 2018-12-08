add_library(appimage_core_impl_fallback OBJECT traversal_fallback.cpp streambuf_fallback.h)
configure_appimage_module(appimage_core_impl_fallback)

add_library(appimage_core_impl_type_1 OBJECT streambuf_type1.cpp traversal_type_1.cpp)
configure_appimage_module(appimage_core_impl_type_1)

add_library(appimage_core_impl_type_2 OBJECT streambuf_type_2.cpp traversal_type_2.cpp)
configure_appimage_module(appimage_core_impl_type_2)

add_library(appimage_core_impl OBJECT
    $<TARGET_OBJECTS:appimage_core_impl_fallback>
    $<TARGET_OBJECTS:appimage_core_impl_type_1>
    $<TARGET_OBJECTS:appimage_core_impl_type_2>
)
