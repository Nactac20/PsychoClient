cmake_minimum_required(VERSION 3.20)
project(PsychoClient VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.11.0/msvc2022_64")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

set(CLIENT_HEADERS
    src/network/NetworkManager.h
    src/auth/AuthWindow.h
    src/auth/EditProfileDialog.h
    src/client/ClientWindow.h
    src/client/PsychologistListWidget.h
    src/client/BookSlotDialog.h
    src/client/SlotCalendarWidget.h
    src/psychologist/PsychologistWindow.h
    src/psychologist/ScheduleWidget.h
    src/chat/ChatWindow.h
    src/diary/DiaryWindow.h
    src/common/DashboardCard.h
    src/common/SessionCalendarDialog.h
    src/common/SessionListDialog.h
    src/common/NotificationListDialog.h
)

set(CLIENT_SOURCES
    src/main.cpp
    src/network/NetworkManager.cpp
    src/auth/AuthWindow.cpp
    src/auth/EditProfileDialog.cpp
    src/client/ClientWindow.cpp
    src/client/PsychologistListWidget.cpp
    src/client/BookSlotDialog.cpp
    src/client/SlotCalendarWidget.cpp
    src/psychologist/PsychologistWindow.cpp
    src/psychologist/ScheduleWidget.cpp
    src/chat/ChatWindow.cpp
    src/diary/DiaryWindow.cpp
    src/common/DashboardCard.cpp
    src/common/SessionCalendarDialog.cpp
    src/common/SessionListDialog.cpp
    src/common/NotificationListDialog.cpp
)

add_executable(psycho_client ${CLIENT_SOURCES} ${CLIENT_HEADERS})

target_include_directories(psycho_client PRIVATE src)

enable_testing()
add_subdirectory(tests)

target_link_libraries(psycho_client
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

if(WIN32)
    set_target_properties(psycho_client PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
    
    add_custom_command(TARGET psycho_client POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "C:/Qt/6.11.0/msvc2022_64/bin"
            $<TARGET_FILE_DIR:psycho_client>
        COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:psycho_client>/platforms
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "C:/Qt/6.11.0/msvc2022_64/plugins/platforms/qwindows.dll"
            $<TARGET_FILE_DIR:psycho_client>/platforms/
    )
endif()