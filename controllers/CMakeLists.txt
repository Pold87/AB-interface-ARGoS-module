include_directories(${CMAKE_SOURCE_DIR}/controllers)

add_library(controllers SHARED 
	    epuck_environment_classification.h 
            epuck_environment_classification.cpp
	    helpers.h	
	    helpers.cpp
	    ../interface/generic_interface.h
	    ../interface/generic_interface.cpp)

target_link_libraries(controllers
  argos3core_simulator
  argos3plugin_simulator_epuck
  argos3plugin_simulator_genericrobot)
