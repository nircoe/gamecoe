function(check_compiler_version)
    message(STATUS "[gamecoe] Using compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
            message(FATAL_ERROR "[gamecoe] GCC 10+ required for C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11")
            message(WARNING "[gamecoe] GCC 11+ recommended for full C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        endif()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "10")
            message(FATAL_ERROR "[gamecoe] Clang 10+ required for basic C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12")
            message(WARNING "[gamecoe] Clang 12+ recommended for good C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        endif()
    endif()
    
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.28")
            message(FATAL_ERROR "[gamecoe] MSVC 2019 16.8+ required for C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "19.29")
            message(WARNING "[gamecoe] MSVC 2019 16.11+ recommended for full C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
            message(FATAL_ERROR "[gamecoe] Apple Clang 13+ (Xcode 13+) required for C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        elseif(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "14.0")
            message(WARNING "[gamecoe] Apple Clang 14+ (Xcode 14+) recommended for better C++20 support (you're using ${CMAKE_CXX_COMPILER_VERSION})")
        endif()
    endif()
endfunction()

function(test_cpp20_features)
    include(CheckCXXSourceCompiles)
    
    # Test concepts
    check_cxx_source_compiles("
        #include <concepts>
        template<std::integral T> void test(T) {}
        int main() { test(42); return 0; }
    " HAVE_CPP20_CONCEPTS)
    
    if(NOT HAVE_CPP20_CONCEPTS)
        message(WARNING "C++20 concepts not available")
    endif()
    
    # Test ranges (if you plan to use them)
    check_cxx_source_compiles("
        #include <ranges>
        #include <vector>
        int main() { 
            std::vector<int> v{1,2,3};
            auto r = v | std::views::filter([](int i){ return i > 1; });
            return 0; 
        }
    " HAVE_CPP20_RANGES)
    
    if(NOT HAVE_CPP20_RANGES)
        message(WARNING "C++20 ranges not available")
    endif()
endfunction()