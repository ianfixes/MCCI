PROJECT(MCCIServer)
 
#what files are needed?
SET(MCCIServer_SRCS
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
TARGET_LINK_LIBRARIES(MCCIServer rhash dl lua5.1 sqlite3)
