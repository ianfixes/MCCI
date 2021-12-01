PROJECT(rhashtest)
 
#what files are needed?
SET(rhashtest_SRCS
  test.cpp
)
# Add executable called myLua
ADD_EXECUTABLE( rhashtest ${rhashtest_SRCS})
 
# indicate how to link
TARGET_LINK_LIBRARIES(rhashtest rhash dl sqlite3)
