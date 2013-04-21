PROJECT(MCCIServer)
 
#what files are needed?
SET(MCCI_SRCS
    interaction.cpp
)
# Add executable called myLua
ADD_EXECUTABLE( MCCIServer ${MCCI_SRCS})
 
# indicate how to link
TARGET_LINK_LIBRARIES(MCCIServer lua5.1)
