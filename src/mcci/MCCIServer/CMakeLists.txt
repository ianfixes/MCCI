PROJECT(MCCIServer)
 
#what files are needed?
SET(MCCIServer_SRCS
  packets.h
  MCCITime.h
  MCCITime.cpp
  MCCIRevisionSet.h
  MCCIRevisionSet.cpp
  MCCIServer.h
  MCCIServer.cpp
  MCCISchema.h
  MCCISchema.cpp
  MCCIServerMain.cpp
)

# Add executable 
ADD_EXECUTABLE( MCCIServer ${MCCIServer_SRCS})
 
# indicate how to link
# if rhash and dl don't come at the beginning, it will fail
TARGET_LINK_LIBRARIES(MCCIServer lua5.1 sqlite3 crypto)
