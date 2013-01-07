function( GetCMakeCacheValue CMAKE_CACHE_FILE_PATH )

  file( READ "${CMAKE_CACHE_FILE_PATH}" CMAKE_CACHE_CONTENTS )

  foreach( VAR ${ARGN} )
    string( FIND "${CMAKE_CACHE_CONTENTS}" "${VAR}" start )
    if ( NOT ${start} EQUAL -1 )
      string( SUBSTRING "${CMAKE_CACHE_CONTENTS}" ${start} -1 contents_tail )
      string( FIND "${contents_tail}" "=" start )
      string( FIND "${contents_tail}" "\n" end_pos )
      math( EXPR start_pos "${start} + 1" )
      math( EXPR length "${end_pos} - ${start_pos}" )
      string( SUBSTRING "${contents_tail}" ${start_pos} ${length} var_value )

      set( ${VAR} "${var_value}" PARENT_SCOPE )
    else()
      set( ${VAR} "${VAR}-NOT_FOUND" PARENT_SCOPE )
    endif()
  endforeach()

endfunction()
