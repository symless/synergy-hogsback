cmake_minimum_required (VERSION 3.4)

# Remove additional compiler flags for third party libraries. 
# We don't care about their issues.

remove_definitions (${SYNERGY_WARNING_FLAGS})

#
# fmt library
#
add_subdirectory (fmt EXCLUDE_FROM_ALL)

#
# Bonefish (WAMP router)
#
add_subdirectory (bonefish EXCLUDE_FROM_ALL)

#
# Msgpack
#
add_subdirectory (msgpack-c EXCLUDE_FROM_ALL)
