cmake_minimum_required(VERSION 3.20)

# Современный CMake 4.x больше не принимает старый минимум политики
# в некоторых зависимостях LibreDWG (в частности, miniz). Это значение
# задаёт совместимый минимум для вложенных сторонних проектов.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING
    "Minimum CMake policy version for third-party subprojects")

project(GeoReport LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# LibreDWG
# -----------------------------------------------------------------------------
# LibreDWG используется только как внешний инструмент для создания DWG.
# ВАЖНО: не запускаем autogen/configure/make во время конфигурации CMake.
# Ранее именно это блокировало Reload CMake в CLion.
# LibreDWG имеет собственный CMakeLists.txt, поэтому используем его напрямую.

set(GEOREPORT_LIBREDWG_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../CppLibs/LibreDWG"
    CACHE PATH "Path to LibreDWG")

option(GEOREPORT_FETCH_LIBREDWG
       "Download LibreDWG automatically when it is missing"
       ON)

if (NOT EXISTS "${GEOREPORT_LIBREDWG_DIR}/CMakeLists.txt")
    if (NOT GEOREPORT_FETCH_LIBREDWG)
        message(FATAL_ERROR
            "LibreDWG was not found at ${GEOREPORT_LIBREDWG_DIR}. "
            "Enable GEOREPORT_FETCH_LIBREDWG or set GEOREPORT_LIBREDWG_DIR.")
    endif()

    find_package(Git REQUIRED)

    message(STATUS "LibreDWG is missing. Cloning official repository...")

    execute_process(
        COMMAND "${GIT_EXECUTABLE}" clone --depth 1
                https://github.com/LibreDWG/libredwg.git
                "${GEOREPORT_LIBREDWG_DIR}"
        RESULT_VARIABLE LIBREDWG_CLONE_RESULT
        OUTPUT_VARIABLE LIBREDWG_CLONE_OUTPUT
        ERROR_VARIABLE LIBREDWG_CLONE_ERROR
    )

    if (NOT LIBREDWG_CLONE_RESULT EQUAL 0)
        message(FATAL_ERROR
            "Failed to clone LibreDWG.\n"
            "Git output: ${LIBREDWG_CLONE_OUTPUT}\n"
            "Git error: ${LIBREDWG_CLONE_ERROR}")
    endif()
endif()

if (NOT EXISTS "${GEOREPORT_LIBREDWG_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "LibreDWG source exists but its CMakeLists.txt was not found: "
        "${GEOREPORT_LIBREDWG_DIR}")
endif()

# Не включаем тесты и LTO LibreDWG: нам нужен только dwgwrite и его библиотека.
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(LIBREDWG_LIBONLY OFF CACHE BOOL "" FORCE)
set(LIBREDWG_DISABLE_WRITE OFF CACHE BOOL "" FORCE)
set(LIBREDWG_DISABLE_JSON ON CACHE BOOL "" FORCE)
set(ENABLE_LTO OFF CACHE BOOL "" FORCE)
set(DISABLE_WERROR ON CACHE BOOL "" FORCE)

# macOS does not provide GNU memmem(). LibreDWG uses memmem() in generated
# parser code. Force-include a tiny compatibility implementation only while
# LibreDWG is being compiled. This avoids modifying the downloaded repository.
set(GEOREPORT_MEMMEM_COMPAT_HEADER
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/georeport_memmem_compat.h")

if (APPLE)
    set(GEOREPORT_OLD_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS
        "${CMAKE_C_FLAGS} -include ${GEOREPORT_MEMMEM_COMPAT_HEADER}")
endif()

add_subdirectory(
    "${GEOREPORT_LIBREDWG_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}/LibreDWG"
    EXCLUDE_FROM_ALL)

if (APPLE)
    set(CMAKE_C_FLAGS "${GEOREPORT_OLD_C_FLAGS}")
    unset(GEOREPORT_OLD_C_FLAGS)
endif()

if (NOT TARGET dwgwrite)
    message(FATAL_ERROR "LibreDWG configured, but target 'dwgwrite' was not created.")
endif()

# OpenXLSX можно подключить локально или автоматически скачать
# при конфигурации CMake. Локальный вариант предпочтителен.
set(OPENXLSX_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../CppLibs/OpenXLSX"
    CACHE PATH "Path to OpenXLSX")

option(GEOREPORT_FETCH_OPENXLSX
       "Download OpenXLSX automatically when it is not found locally"
       ON)

if (EXISTS "${OPENXLSX_DIR}/CMakeLists.txt")
    set(OPENXLSX_CREATE_DOCS OFF CACHE BOOL "" FORCE)
    set(OPENXLSX_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

    add_subdirectory(
        "${OPENXLSX_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}/OpenXLSX")
else()
    if (NOT GEOREPORT_FETCH_OPENXLSX)
        message(FATAL_ERROR
            "OpenXLSX was not found at: ${OPENXLSX_DIR}. "
            "Set OPENXLSX_DIR or enable GEOREPORT_FETCH_OPENXLSX.")
    endif()

    include(FetchContent)

    FetchContent_Declare(
        OpenXLSX
        GIT_REPOSITORY https://github.com/troldal/OpenXLSX.git
        GIT_TAG v0.5.1
    )

    set(OPENXLSX_CREATE_DOCS OFF CACHE BOOL "" FORCE)
    set(OPENXLSX_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(OpenXLSX)
endif()

add_executable(GeoReport
    main.cpp

        headers/utils/ParameterValue.h
        sources/utils/ParameterValue.cpp

        headers/excel/ExcelCellUtils.h
        sources/excel/ExcelCellUtils.cpp

        headers/domain/Sample.h
        sources/domain/Sample.cpp

        headers/domain/Borehole.h
        sources/domain/Borehole.cpp

        headers/domain/Block.h
        sources/domain/Block.cpp

        headers/domain/Deposit.h
        sources/domain/Deposit.cpp

        headers/excel/ExcelReader.h
        sources/excel/ExcelReader.cpp

        headers/results/WeightedAverageResult.h
        sources/results/WeightedAverageResult.cpp

        headers/results/BoreholeResult.h
        sources/results/BoreholeResult.cpp

        headers/calculations/WeightedAverageCalculator.h
        sources/calculations/WeightedAverageCalculator.cpp
        sources/excel/ExcelWriter.cpp
        headers/excel/ExcelWriter.h
        sources/excel/ExcelWriteUtils.cpp
        headers/excel/ExcelWriteUtils.h
        sources/writers/BoreholeResultWriter.cpp
        headers/writers/BoreholeResultWriter.h
        sources/writers/BlockResultWriter.cpp
        headers/writers/BlockResultWriter.h
        sources/calculations/GranulometricCompositionCalculator.cpp
        headers/calculations/GranulometricCompositionCalculator.h
        sources/writers/GranulometricCompositionWriter.cpp
        headers/writers/GranulometricCompositionWriter.h
        sources/utils/SandClassification.cpp
        headers/utils/SandClassification.h
        sources/settings/MainIndicatorsSettings.cpp
        headers/settings/MainIndicatorsSettings.h
        sources/domain/BoreholeSection.cpp
        headers/domain/BoreholeSection.h
        sources/geology/SectionClassifier.cpp
        headers/geology/SectionClassifier.h
        sources/geology/SectionDecision.cpp
        headers/geology/SectionDecision.h
        sources/geology/BoreholeSectionResolver.cpp
        headers/geology/BoreholeSectionResolver.h
        sources/calculations/BoreholeSectionAnalysis.cpp
        headers/calculations/BoreholeSectionAnalysis.h
        sources/calculations/BoreholeSectionAnalyzer.cpp
        headers/calculations/BoreholeSectionAnalyzer.h
        sources/results/MainIndicators.cpp
        headers/results/MainIndicators.h
        sources/calculations/MainIndicatorsCalculator.cpp
        headers/calculations/MainIndicatorsCalculator.h
        sources/writers/MainIndicatorsResultWriter.cpp
        headers/writers/MainIndicatorsResultWriter.h
        sources/writers/SplitLaboratoryWriter.cpp
        headers/writers/SplitLaboratoryWriter.h
        sources/domain/GeologicalSection.cpp
        headers/domain/GeologicalSection.h
        sources/domain/Overburden.cpp
        headers/domain/Overburden.h
        sources/domain/DryMineralSection.cpp
        headers/domain/DryMineralSection.h
        sources/domain/WaterBearingMineralSection.cpp
        headers/domain/WaterBearingMineralSection.h
        sources/domain/UnderlyingRock.cpp
        headers/domain/UnderlyingRock.h
        sources/domain/UsefulMineral.cpp
        headers/domain/UsefulMineral.h
        sources/calculations/GroundwaterAnalysis.cpp
        headers/calculations/GroundwaterAnalysis.h
        sources/settings/GroundwaterSettings.cpp
        headers/settings/GroundwaterSettings.h
        sources/geology/OverburdenResolver.cpp
        headers/geology/OverburdenResolver.h
        sources/domain/SurfaceSoil.cpp
        headers/domain/SurfaceSoil.h
        sources/geology/SurfaceSoilResolver.cpp
        headers/geology/SurfaceSoilResolver.h
        sources/geology/UsefulMineralResolver.cpp
        headers/geology/UsefulMineralResolver.h
        sources/geology/UnderlyingRockResolver.cpp
        headers/geology/UnderlyingRockResolver.h
        sources/geology/BoreholeGeology.cpp
        headers/geology/BoreholeGeology.h
        sources/geology/BoreholeGeologyResolver.cpp
        headers/geology/BoreholeGeologyResolver.h
        sources/geology/SectionDecisionProvider.cpp
        headers/geology/SectionDecisionProvider.h
        sources/application/GeologicalAnalysisResult.cpp
        headers/application/GeologicalAnalysisResult.h
        sources/application/GeologicalAnalysisService.cpp
        headers/application/GeologicalAnalysisService.h
        sources/application/BoreholeAnalysisResult.cpp
        headers/application/BoreholeAnalysisResult.h
        sources/graphics/dxf/DxfLayer.cpp
        headers/graphics/dxf/DxfLayer.h
        sources/graphics/dxf/DxfDocument.cpp
        headers/graphics/dxf/DxfDocument.h
        sources/graphics/dxf/DxfWriter.cpp
        headers/graphics/dxf/DxfWriter.h
        sources/graphics/dxf/DxfColumnWriter.cpp
        headers/graphics/dxf/DxfColumnWriter.h
        sources/graphics/dxf/DxfGeologicalSectionWriter.cpp
        headers/graphics/dxf/DxfGeologicalSectionWriter.h
        sources/graphics/cad/BoreholePlanDrawingBuilder.cpp
        headers/graphics/cad/BoreholePlanDrawingBuilder.h
        sources/graphics/cad/CadStandardsWriter.cpp
        headers/graphics/cad/CadStandardsWriter.h
        sources/graphics/dwg/DwgPlanWriter.cpp
        headers/graphics/dwg/DwgPlanWriter.h
        sources/graphics/dwg/DwgGeologicalSectionWriter.cpp
        headers/graphics/dwg/DwgGeologicalSectionWriter.h
        sources/settings/GroundwaterCalculationSettings.cpp
        headers/settings/GroundwaterCalculationSettings.h
        sources/settings/DevelopmentSettings.cpp
        headers/settings/DevelopmentSettings.h
        sources/calculations/SampleSplitter.cpp
        headers/calculations/SampleSplitter.h
        sources/calculations/MineralCalculationPart.cpp
        headers/calculations/MineralCalculationPart.h
        sources/settings/ConsoleSettingsReader.cpp
        headers/settings/ConsoleSettingsReader.h
        sources/settings/SectionSettings.cpp
        headers/settings/SectionSettings.h
        sources/settings/ReserveSummarySettings.cpp
        headers/settings/ReserveSummarySettings.h
        sources/writers/ReserveSummaryWriter.cpp
        headers/writers/ReserveSummaryWriter.h
        sources/normative/NormativeAnalysis.cpp
        headers/normative/NormativeAnalysis.h
        sources/writers/AnaliticsWriter.cpp
        headers/writers/AnaliticsWriter.h
        sources/writers/Gost23735Writer.cpp
        headers/writers/Gost23735Writer.h
        sources/writers/IndustrialTypesWriter.cpp
        headers/writers/IndustrialTypesWriter.h
        sources/writers/BlockDesignation.cpp
        headers/writers/BlockDesignation.h
        sources/writers/SandGroupDistributionWriter.cpp
        headers/writers/SandGroupDistributionWriter.h
        sources/writers/Gost8736RequirementsWriter.cpp
        headers/writers/Gost8736RequirementsWriter.h
        sources/statistics/DepositStatistics.cpp
        headers/statistics/DepositStatistics.h
)

target_include_directories(GeoReport PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/headers"
)

# GeoReport можно собрать только после того, как CMake соберёт dwgwrite.
add_dependencies(GeoReport dwgwrite)

# Передаём приложению точный путь к dwgwrite из текущей CMake-сборки.
# Это надёжнее, чем искать бинарник в исходниках LibreDWG.
target_compile_definitions(GeoReport PRIVATE
    GEOREPORT_DWGWRITE_PATH="$<TARGET_FILE:dwgwrite>"
)

target_link_libraries(GeoReport PRIVATE
    OpenXLSX::OpenXLSX
)
