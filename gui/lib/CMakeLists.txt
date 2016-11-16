cmake_minimum_required (VERSION 3.4)

set (GUI_LIB_SOURCES
	AppConfig.cpp
	Common.cpp
	ConfigFileManager.cpp
	ConfigMessageConvertor.cpp
	CoreInterface.cpp
	DebugLevel.cpp
	DeviceManager.cpp
	DirectoryManager.cpp
	Hostname.cpp
	IpcClient.cpp
	Ipc.cpp
	IpcReader.cpp
	IpManager.cpp
	LogManager.cpp
	Multicast.cpp
	MulticastManager.cpp
	MulticastMessage.cpp
	ProcessCommand.cpp
	ProcessManager.cpp
	ScreenBBArrangement.cpp
	Screen.cpp
	ScreenManager.cpp
	ScreenModel.cpp
	ServerInfo.cpp
)

add_library (gui_lib STATIC ${GUI_LIB_SOURCES})
qt5_use_modules (gui_lib Quick Widgets)
set_target_properties (gui_lib PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
