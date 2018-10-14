cmake_minimum_required(VERSION 3.5)
project(fake.token VERSION 1.0.0)

option(GEN_ABI "Generate ABI" OFF)

find_package(eosio.cdt)

set(SRC "src/fake.token.cpp")

if(GEN_ABI)
    add_contract(${PROJECT_NAME} ${PROJECT_NAME} ${SRC})
else()
    add_executable(${PROJECT_NAME}.wasm  ${SRC})
endif()

