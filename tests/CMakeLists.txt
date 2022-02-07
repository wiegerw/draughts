project(DRAUGHTS_TEST)
  
file(GLOB SOURCES "*.cpp")
foreach( OBJ ${SOURCES} )
  get_filename_component(result "${OBJ}" NAME_WE)
  add_executable("${result}" "${OBJ}"  )
  target_link_libraries("${result}" ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} Threads::Threads)
endforeach( OBJ )
