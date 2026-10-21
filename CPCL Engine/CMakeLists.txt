cmake_minimum_required(VERSION 3.20)
project(CPCLEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL3_DIR    ${CMAKE_SOURCE_DIR}/ThirdParty/SDL3)
set(SDL3_TTF_DIR ${SDL3_DIR}/SDL3_ttf)

# ---------- LibMath (header-only) ----------
add_library(LibMath INTERFACE)
target_include_directories(LibMath INTERFACE ${CMAKE_SOURCE_DIR}/ThirdParty/LibMath)

add_library(Stb INTERFACE)
target_include_directories(Stb INTERFACE ${CMAKE_SOURCE_DIR}/ThirdParty/Stb)


# ---------- Engine (bibliothèque) ----------
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS Engine/*.cpp)
add_library(Engine STATIC ${ENGINE_SOURCES})
target_include_directories(Engine PUBLIC
    Engine
    ${SDL3_DIR}/include
    ${SDL3_TTF_DIR}/include
)
target_link_libraries(Engine PUBLIC ${SDL3_TTF_DIR}/lib/x64/SDL3_ttf.lib)
target_link_libraries(Engine PUBLIC LibMath)
target_link_libraries(Engine PUBLIC Stb)


# ---------- App (exécutable) ----------
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS App/*.cpp)
add_executable(App ${APP_SOURCES})

target_include_directories(App PRIVATE ${SDL3_DIR}/include)
target_link_libraries(App PRIVATE
    Engine
    ${SDL3_DIR}/lib/x64/SDL3.lib   
)


add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL3_DIR}/lib/x64/SDL3.dll
        $<TARGET_FILE_DIR:App>
)

add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL3_TTF_DIR}/lib/x64/SDL3_ttf.dll
        $<TARGET_FILE_DIR:App>
)

# Copier les assets à côté de l'exécutable
add_custom_command(TARGET App POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:App>/Assets"
)


# Définir le répertoire de travail pour le débogueur Visual Studio
set_property(TARGET App PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:App>"
)