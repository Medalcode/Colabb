
# Define test sources
set(TEST_SOURCES
    main_test.cpp
    unit/suggestion_cache_test.cpp
    unit/config_manager_test.cpp
    unit/profile_manager_test.cpp
    unit/context_service_test.cpp
    unit/translation_manager_test.cpp
    unit/prediction_service_queue_test.cpp
    # Add other test files here
)

# Core library sources needed for testing (exclude main.cpp)
set(CORE_SOURCES
    ../src/infrastructure/terminal/vte_terminal.cpp
    ../src/infrastructure/http/http_client.cpp
    ../src/infrastructure/config/config_manager.cpp
    ../src/domain/ai/groq_provider.cpp
    ../src/domain/ai/openai_provider.cpp
    ../src/application/prediction_service.cpp
    ../src/application/suggestion_cache.cpp
    ../src/ui/tab_manager.cpp
    ../src/infrastructure/config/profile_manager.cpp
    ../src/infrastructure/context/context_service.cpp
    ../src/infrastructure/i18n/translation_manager.cpp
    # Converting UI components to be testable might require mocking or refactoring
    # Converting UI components to be testable might require mocking or refactoring
    # For now, we focus on logic-heavy components
)

# Note: We need to handle GTK/VTE dependencies even for logic tests if includes pull them in
# Ideally, we should mock them or separate logic better. 
# For this first pass, we will link against GTK/VTE to satisfy symbols.

add_executable(colabb_tests ${TEST_SOURCES} ${CORE_SOURCES})

target_include_directories(colabb_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${GTK3_INCLUDE_DIRS}
    ${VTE_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${SECRET_INCLUDE_DIRS}
)

target_link_libraries(colabb_tests
    GTest::gtest_main
    ${GTK3_LIBRARIES}
    ${VTE_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SECRET_LIBRARIES}
    nlohmann_json::nlohmann_json
    pthread
)

# Register tests
include(GoogleTest)
gtest_discover_tests(colabb_tests)
