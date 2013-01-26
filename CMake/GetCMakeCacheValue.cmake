# GetCMakeCacheValue.cmake
#
# Cory Quammen <cquammen@cs.unc.edu>
#
# This function can be used to retrieve variable values from
# a CMakeCache.txt file located in the build directory of a
# project configured with CMake. It is useful for finding the
# values of variables not exported the project of interest via
# a <package>Config.cmake file.
#
# Usage:
#
# GetCMakeCacheValue( ${BINARY_PATH} VARIABLE ... )
#
# The BINARY_PATH argument is the path to the build directory
# containing the CMakeCache.txt file. Additional arguments to
# the functions are treated as variables that you want to query
# from the CMakeCache.txt file. Assuming the variable you want
# to query has the name VARIABLE, this function will define
# two variables in the scope in which GetCMakeCacheValue was
# called:
#
# VARIABLE      - Value of VARIABLE in the CMakeCache.txt file.
#                 If the variable is not defined in the file,
#                 this will have the value VARIABLE-NOT_FOUND.
#
# VARIABLE_TYPE - Type of the VARIABLE.
#                 If the variable is not defined in the file,
#                 this will have the value VARIABLE_TYPE-NOT_FOUND
#
# Any number of variable names can be passed in, e.g.,
#
# GetCMakeCacheValue( ${BINARY_PATH} VARIABLE1 VARIABLE2 VARIABLE3 ... )
#

function( GetCMakeCacheValue BINARY_PATH )

  file( READ "${BINARY_PATH}/CMakeCache.txt" CMAKE_CACHE_CONTENTS )

  foreach( VAR ${ARGN} )
    string( FIND "${CMAKE_CACHE_CONTENTS}" "${VAR}:" start )
    if ( NOT ${start} EQUAL -1 )
      string( SUBSTRING "${CMAKE_CACHE_CONTENTS}" ${start} -1 contents_tail )
      string( FIND "${contents_tail}" ":" type_start_pos )
      string( FIND "${contents_tail}" "=" type_end_pos )
      math( EXPR type_start_pos "${type_start_pos} + 1" )
      math( EXPR type_length "${type_end_pos} - ${type_start_pos}" )
      string( SUBSTRING "${contents_tail}" ${type_start_pos} ${type_length} var_type )

      string( FIND "${contents_tail}" "=" val_start_pos )
      string( FIND "${contents_tail}" "\n" val_end_pos )
      math( EXPR val_start_pos "${val_start_pos} + 1" )
      math( EXPR val_length "${val_end_pos} - ${val_start_pos}" )
      string( SUBSTRING "${contents_tail}" ${val_start_pos} ${val_length} var_value )

      set( ${VAR} "${var_value}" PARENT_SCOPE )
      set( ${VAR}_TYPE "${var_type}" PARENT_SCOPE )
    else()
      set( ${VAR} "${VAR}-NOT_FOUND" PARENT_SCOPE )
      set( ${VAR}_TYPE "${VAR}_TYPE-NOT_FOUND" PARENT_SCOPE )
    endif()
  endforeach()

endfunction()
