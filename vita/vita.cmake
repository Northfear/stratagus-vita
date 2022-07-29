include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Wargus")
set(VITA_TITLEID  "WARG00001")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXECUTABLE_NAME stratagus-dbg)
else()
    set(EXECUTABLE_NAME stratagus)
endif()

set(VITA_VERSION "03.30")

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")
vita_create_self(${EXECUTABLE_NAME}.self ${EXECUTABLE_NAME})

vita_create_vpk(${EXECUTABLE_NAME}.vpk ${VITA_TITLEID} ${EXECUTABLE_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/pic0.png sce_sys/pic0.png
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/livearea/contents/wc1.png sce_sys/livearea/contents/wc1.png
    FILE ${CMAKE_SOURCE_DIR}/vita/sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
)
