file(GLOB zmc_motapp_http_client_source demo/*.cpp source/*.cpp)
add_executable(zmc_motapp_http_client ${zmc_motapp_http_client_source})

target_include_directories(zmc_motapp_http_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../rpc_client/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/fb/include)

target_compile_definitions(zmc_motapp_http_client PRIVATE ZML_EXPORTS)

install(TARGETS zmc_motapp_http_client DESTINATION bin/${BUILD_OPT})

if(ENABLE_DEV_UNITTEST)
    add_subdirectory(test)
endif()
