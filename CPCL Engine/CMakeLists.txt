cmake_minimum_required(VERSION 3.20)
project(CPCLEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL2_DIR    ${CMAKE_SOURCE_DIR}/ThirdParty/SDL2)

# ---------- Engine (bibliothèque) ----------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS Engine/*.cpp)
add_library(Engine STATIC ${ENGINE_SOURCES})
target_include_directories(Engine PUBLIC Engine)

# ---------- App (exécutable) ----------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS App/*.cpp)
add_executable(App ${APP_SOURCES})

target_include_directories(App PRIVATE ${SDL2_DIR}/include)
target_link_libraries(App PRIVATE
    Engine
    ${SDL2_DIR}/lib/x64/SDL2main.lib
    ${SDL2_DIR}/lib/x64/SDL2.lib
)

# Copie SDL2.dll à côté de App.exe après chaque build
add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_DIR}/lib/x64/SDL2.dll
        $<TARGET_FILE_DIR:App>
)

