if (OPENZL_BUILD_PROTOBUF_TOOLS)
    cmake_minimum_required(VERSION 3.24)
    # Configure protobuf options before fetching
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(protobuf_WITH_ZLIB_DEFAULT OFF CACHE BOOL "" FORCE)
    set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    # Enable position-independent code for static linking into shared libs/PIE executables
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    # Get the protobuf library
    FetchContent_Declare(protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v32.0
        FIND_PACKAGE_ARGS NAMES protobuf
    )
    FetchContent_MakeAvailable(protobuf)
    target_compile_options(libprotobuf PUBLIC
        -Wno-deprecated-declarations
        -Wno-stringop-overread
        -Wno-strict-aliasing
        -Wno-attributes
    )
    find_package(protobuf CONFIG REQUIRED)

    # Make the protobuf_serializer library
    file(GLOB protobuf_serializer_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )
    list(REMOVE_ITEM protobuf_serializer_sources "${CMAKE_CURRENT_SOURCE_DIR}/cli.cpp")

    add_library(protobuf_serializer ${protobuf_serializer_sources})
    set_target_properties(protobuf_serializer PROPERTIES POSITION_INDEPENDENT_CODE ON)

    # Generate protobuf files for the serializer library
    include(FindProtobuf)
    protobuf_generate(TARGET protobuf_serializer
        PROTOS
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlp_metrics.proto"
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlp_traces.proto"
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_otap.proto"
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_tpch.proto"
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlpmetricsdict.proto"
            "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlptracesdict.proto"
    )

    target_link_libraries(protobuf_serializer PUBLIC
        openzl_cpp
        protobuf::libprotobuf
    )
    target_include_directories(protobuf_serializer PUBLIC ${PROJECT_BINARY_DIR})
    apply_openzl_compile_options_to_target(protobuf_serializer)

    # Make the protobuf_cli tool
    file(GLOB protobuf_cli_sources CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/cli.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlp_metrics.proto"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlp_traces.proto"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_otap.proto"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_tpch.proto"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlpmetricsdict.proto"
        "${CMAKE_CURRENT_SOURCE_DIR}/schema_otlptracesdict.proto")
    add_executable(protobuf_cli ${protobuf_cli_sources})

    include(FindProtobuf)
    protobuf_generate(TARGET protobuf_cli)

    target_link_libraries(protobuf_cli PRIVATE
        protobuf_serializer
        arg
        tools_io
        tools_training
        custom_parsers
    )
    target_include_directories(protobuf_cli PRIVATE ${PROJECT_BINARY_DIR})

    apply_openzl_compile_options_to_target(protobuf_cli)
endif()
