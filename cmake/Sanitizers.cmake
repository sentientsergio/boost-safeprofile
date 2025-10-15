# Sanitizers.cmake - ASan/UBSan/TSan support for self-conformance

function(enable_sanitizers target_name)
    if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(SANITIZERS "")

        option(ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer" ON)
        if(ENABLE_SANITIZER_ADDRESS)
            list(APPEND SANITIZERS "address")
        endif()

        option(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR "Enable UndefinedBehaviorSanitizer" ON)
        if(ENABLE_SANITIZER_UNDEFINED_BEHAVIOR)
            list(APPEND SANITIZERS "undefined")
        endif()

        option(ENABLE_SANITIZER_THREAD "Enable ThreadSanitizer" OFF)
        if(ENABLE_SANITIZER_THREAD)
            if("address" IN_LIST SANITIZERS OR "leak" IN_LIST SANITIZERS)
                message(WARNING "ThreadSanitizer cannot be used with Address or Leak sanitizer")
            else()
                list(APPEND SANITIZERS "thread")
            endif()
        endif()

        list(JOIN SANITIZERS "," SANITIZER_LIST)
        if(SANITIZER_LIST)
            message(STATUS "Enabling sanitizers: ${SANITIZER_LIST}")
            target_compile_options(${target_name} PRIVATE
                -fsanitize=${SANITIZER_LIST}
                -fno-omit-frame-pointer
            )
            target_link_options(${target_name} PRIVATE
                -fsanitize=${SANITIZER_LIST}
            )
        endif()
    elseif(MSVC)
        option(ENABLE_SANITIZER_ADDRESS "Enable AddressSanitizer" ON)
        if(ENABLE_SANITIZER_ADDRESS)
            message(STATUS "Enabling MSVC AddressSanitizer")
            target_compile_options(${target_name} PRIVATE /fsanitize=address)
        endif()
    endif()
endfunction()
