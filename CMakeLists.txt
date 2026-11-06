cmake_minimum_required(VERSION 3.21)
project(Munin VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_APP "Compilar aplicativo principal Munin" ON)
option(BUILD_CLI "Compilar utilitario de linha de comando munin-app-cli" ON)
option(BUILD_BENCHMARK "Compilar executável de benchmark MuninBench" OFF)
option(BUILD_MOCKS "Compilar utilitarios e mocks do projeto" OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/O2 /Oi /Ot /Gy /W3 /permissive- /Zc:__cplusplus /utf-8)
    add_compile_options("$<$<CONFIG:Release>:/GL>")
    add_link_options("$<$<CONFIG:Release>:/LTCG>")
else()
    add_compile_options(-O3 -Wall -Wextra)
endif()

add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)

if(BUILD_APP OR BUILD_BENCHMARK OR BUILD_CLI)
    # ----------------------------------------------------------------------------
    # 1. Target Estático da Engine C++ / Win32 (Sem dependência de Qt)
    # ----------------------------------------------------------------------------
    add_library(MuninEngine STATIC
        source/engine/MemoryMappedFile.hpp
        source/engine/MemoryMappedFile.cpp
        source/engine/SimdUtils.hpp
        source/engine/SimdUtils.cpp
        source/engine/ChunkIndex.hpp
        source/engine/ChunkIndex.cpp
        source/engine/LiveTailWatcher.hpp
        source/engine/LiveTailWatcher.cpp
        source/engine/ParallelSearch.hpp
        source/engine/ParallelSearch.cpp
        source/engine/TimeSliceScanner.hpp
        source/engine/TimeSliceScanner.cpp
    )

    target_include_directories(MuninEngine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )
endif()

if(BUILD_APP)
    # ----------------------------------------------------------------------------
    # 2. Executável Gráfico Principal (Munin com Qt6)
    # ----------------------------------------------------------------------------
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

    # Configurar ícone nativo PE para Windows (Explorer, Desktop, Barra de Tarefas)
    if(WIN32)
        enable_language(RC)
        set(MUNIN_APP_ICON_RC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/assets/icons/munin.ico")
        file(TO_NATIVE_PATH "${MUNIN_APP_ICON_RC_PATH}" MUNIN_APP_ICON_RC_PATH_NATIVE)
        string(REPLACE "\\" "\\\\" MUNIN_APP_ICON_RC_PATH_ESCAPED "${MUNIN_APP_ICON_RC_PATH_NATIVE}")
        set(MUNIN_APP_ICON_RC_PATH "${MUNIN_APP_ICON_RC_PATH_ESCAPED}")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/cmake/windows_app_icon.rc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/munin_app_icon.rc"
            @ONLY
        )
        set(MUNIN_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/munin_app_icon.rc")
    else()
        set(MUNIN_RC_FILE "")
    endif()

    add_executable(Munin WIN32
        source/model/LogModel.hpp
        source/model/LogModel.cpp
        source/utils/LogManager.hpp
        source/utils/LogManager.cpp
        source/utils/CaminhoPersistenteUtils.hpp
        source/utils/CaminhoPersistenteUtils.cpp
        source/utils/ApplicationPathUtils.hpp
        source/utils/ApplicationPathUtils.cpp
        source/language/GerenciadorTraducao.hpp
        source/language/GerenciadorTraducao.cpp
        source/ui/OpcoesSobre.hpp
        source/ui/OpcoesSobre.cpp
        source/ui/GerenciadorDialogos.hpp
        source/ui/GerenciadorDialogos.cpp
        source/ui/SobreDialog.hpp
        source/ui/SobreDialog.cpp
        source/ui/Manual.hpp
        source/ui/Manual.cpp
        source/ui/Win32DwmHelper.hpp
        source/ui/Win32DwmHelper.cpp
        source/ui/StickyMenu.hpp
        source/ui/StickyMenu.cpp
        source/ui/ButtonUtils.hpp
        source/ui/ButtonUtils.cpp
        source/ui/ThemeManager.hpp
        source/ui/ThemeManager.cpp
        source/ui/LogItemDelegate.hpp
        source/ui/LogItemDelegate.cpp
        source/ui/MinimapScrollBar.hpp
        source/ui/MinimapScrollBar.cpp
        source/ui/LogView.hpp
        source/ui/LogView.cpp
        source/ui/TimeSliceWidget.hpp
        source/ui/TimeSliceWidget.cpp
        source/ui/JsonInspectorWidget.hpp
        source/ui/JsonInspectorWidget.cpp
        source/ui/SearchBarWidget.hpp
        source/ui/SearchBarWidget.cpp
        source/ui/LogGeneratorDialog.hpp
        source/ui/LogGeneratorDialog.cpp
        source/ui/MainWindow.hpp
        source/ui/MainWindow.cpp
        source/main.cpp
        ${MUNIN_RC_FILE}
    )

    target_link_libraries(Munin PRIVATE
        MuninEngine
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        dwmapi
        psapi
    )

    target_include_directories(Munin PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    # ----------------------------------------------------------------------------
    # 3. Deployment e Recursos (MuninDeploy)
    # ----------------------------------------------------------------------------
    get_filename_component(QT_PREFIX_DIR "${Qt6_DIR}/../../.." ABSOLUTE)
    set(QT_BIN_DIR "${QT_PREFIX_DIR}/bin")
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    # Garantir que assets e traduções sejam copiados no POST_BUILD do Munin
    add_custom_command(TARGET Munin POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/assets" "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations/munin_en_US.qm" "${CMAKE_BINARY_DIR}/translations/munin_en_US.qm"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations/munin_pt_BR.qm" "${CMAKE_BINARY_DIR}/translations/munin_pt_BR.qm"
        COMMENT "Copiando recursos e traduções do Munin..."
    )

    set(DEPLOY_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E echo "Deploying Qt dependencies, modern style plugins, translations, and assets for Munin..."
        COMMAND ${WINDEPLOYQT_EXECUTABLE} "$<TARGET_FILE:Munin>" --dir "${CMAKE_BINARY_DIR}" --plugindir "${CMAKE_BINARY_DIR}/plugins" --add-plugin-types styles,iconengines,imageformats,platforms --no-translations --no-compiler-runtime
        COMMAND ${CMAKE_COMMAND} -E rm -f "${CMAKE_BINARY_DIR}/vc_redist.x64.exe" "${CMAKE_BINARY_DIR}/dxcompiler.dll" "${CMAKE_BINARY_DIR}/dxil.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/platforms/qwindows.dll" "${CMAKE_BINARY_DIR}/plugins/platforms/qwindows.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/styles"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/styles/qmodernwindowsstyle.dll" "${CMAKE_BINARY_DIR}/plugins/styles/qmodernwindowsstyle.dll"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations/munin_en_US.qm" "${CMAKE_BINARY_DIR}/translations/munin_en_US.qm"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/source/language/translations/munin_pt_BR.qm" "${CMAKE_BINARY_DIR}/translations/munin_pt_BR.qm"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/source/assets" "${CMAKE_BINARY_DIR}/assets"
    )

    if(EXISTS "${QT_PREFIX_DIR}/plugins/iconengines/qsvgicon.dll")
        list(APPEND DEPLOY_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/plugins/iconengines"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${QT_PREFIX_DIR}/plugins/iconengines/qsvgicon.dll" "${CMAKE_BINARY_DIR}/plugins/iconengines/qsvgicon.dll"
        )
    endif()

    if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        set(GCC_RUNTIMES libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll)
        foreach(GCC_DLL IN LISTS GCC_RUNTIMES)
            if(EXISTS "${MINGW_BIN_DIR}/${GCC_DLL}")
                list(APPEND DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            elseif(EXISTS "${QT_BIN_DIR}/${GCC_DLL}")
                list(APPEND DEPLOY_COMMANDS
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${QT_BIN_DIR}/${GCC_DLL}"
                        "${CMAKE_BINARY_DIR}/${GCC_DLL}"
                )
            endif()
        endforeach()
    endif()

    if(MSVC)
        set(CURRENT_DIST_NAME "Munin_msvc")
    else()
        set(CURRENT_DIST_NAME "Munin_mingw")
    endif()

    find_program(POWERSHELL_EXECUTABLE NAMES powershell.exe pwsh.exe powershell pwsh)
    if(POWERSHELL_EXECUTABLE)
        list(APPEND DEPLOY_COMMANDS
            COMMAND ${CMAKE_COMMAND} -E echo "Sincronizando distribuicao limpa externa (${CURRENT_DIST_NAME}) em D:/MISCELANEAS/VSCode/Munin..."
            COMMAND ${POWERSHELL_EXECUTABLE} -ExecutionPolicy Bypass -File "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gerar_distribuicoes_limpas.ps1" -DistributionName "${CURRENT_DIST_NAME}"
        )
    endif()

    add_custom_target(MuninDeploy
        ${DEPLOY_COMMANDS}
        DEPENDS Munin
        COMMENT "Deploying dependencies and clean distribution for Munin"
    )
endif()

# ----------------------------------------------------------------------------
# 4. Executável de Benchmark / Testes de Performance da Engine (Build Isolada)
# ----------------------------------------------------------------------------
if(BUILD_BENCHMARK)
    add_executable(MuninBench
        source/engine/TestEngine.cpp
    )

    target_link_libraries(MuninBench PRIVATE
        MuninEngine
        psapi
    )

    target_include_directories(MuninBench PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )
endif()

# ----------------------------------------------------------------------------
# 5. Executável Standalone de Linha de Comando (munin-app-cli)
# ----------------------------------------------------------------------------
if(BUILD_CLI)
    add_executable(munin-app-cli
        source/cli/main_cli.cpp
        ${MUNIN_RC_FILE}
    )

    target_link_libraries(munin-app-cli PRIVATE
        MuninEngine
        psapi
    )

    target_include_directories(munin-app-cli PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
    )

    if(TARGET MuninDeploy)
        add_dependencies(MuninDeploy munin-app-cli)
    endif()
endif()

# ----------------------------------------------------------------------------
# 6. Mocks e Utilitários (Build Isolada)
# ----------------------------------------------------------------------------
if(BUILD_MOCKS AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/mocks/CMakeLists.txt")
    add_subdirectory(mocks)
endif()
