cmake_minimum_required(VERSION 3.25)

project(text_editor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EDITOR_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(EDITOR_BUILD_FUZZERS "Build libFuzzer targets (Clang only)" OFF)
option(EDITOR_BUILD_QT "Build the Qt Widgets editor when Qt 6 is available" ON)

add_library(editor_core STATIC
    src/core/piece_table.cpp
    src/core/piece_scanner.cpp
    src/core/undo_history.cpp
    src/core/line_index.cpp
    src/core/editor_model.cpp
)

target_include_directories(editor_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(editor_core PUBLIC cxx_std_23)

function(editor_enable_sanitizers target)
    if(NOT EDITOR_ENABLE_SANITIZERS)
        return()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    else()
        message(WARNING "Sanitizers are not configured for ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()

editor_enable_sanitizers(editor_core)

include(CTest)

if(BUILD_TESTING)
    include(FetchContent)

    find_package(Catch2 3 QUIET)
    if(NOT Catch2_FOUND)
        FetchContent_Declare(
            Catch2
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_TAG v3.7.1
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(Catch2)
    endif()

    add_executable(editor_core_tests
        tests/core/piece_table_test.cpp
        tests/core/piece_scanner_test.cpp
        tests/core/editor_model_test.cpp
    )

    target_link_libraries(editor_core_tests
        PRIVATE
            editor_core
            Catch2::Catch2WithMain
    )

    editor_enable_sanitizers(editor_core_tests)

    include(Catch)
    catch_discover_tests(editor_core_tests)
endif()

if(EDITOR_BUILD_FUZZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "libFuzzer targets require Clang")
    endif()

    add_executable(edit_sequence_fuzzer fuzz/edit_sequence_fuzzer.cpp)
    target_link_libraries(edit_sequence_fuzzer PRIVATE editor_core)
    target_compile_options(edit_sequence_fuzzer PRIVATE -fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer)
    target_link_options(edit_sequence_fuzzer PRIVATE -fsanitize=fuzzer,address,undefined)
    if(APPLE AND EXISTS "/usr/local/opt/llvm/lib/c++")
        target_link_directories(edit_sequence_fuzzer PRIVATE /usr/local/opt/llvm/lib/c++)
        target_link_options(edit_sequence_fuzzer PRIVATE -Wl,-rpath,/usr/local/opt/llvm/lib/c++)
    endif()
endif()

if(EDITOR_BUILD_QT)
    find_package(Qt6 6.2 COMPONENTS Widgets Test QUIET)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        add_library(editor_view STATIC src/view/editor_view.cpp)
        target_link_libraries(editor_view PUBLIC editor_core Qt6::Widgets)
        target_include_directories(editor_view PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
        target_compile_features(editor_view PUBLIC cxx_std_23)
        editor_enable_sanitizers(editor_view)

        add_executable(editor_app src/app/main.cpp)
        target_link_libraries(editor_app PRIVATE editor_view)
        editor_enable_sanitizers(editor_app)

        if(BUILD_TESTING)
            add_executable(editor_view_tests tests/view/editor_view_test.cpp)
            target_link_libraries(editor_view_tests PRIVATE editor_view Qt6::Test)
            editor_enable_sanitizers(editor_view_tests)
            add_test(NAME editor_view_tests COMMAND editor_view_tests)
            set_tests_properties(editor_view_tests PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
        endif()
    else()
        message(STATUS "Qt 6 Widgets/Test not found; editor_view and editor_app are skipped")
    endif()
endif()
