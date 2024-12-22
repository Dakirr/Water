cmake_minimum_required(VERSION 3.10)	 
project(FluidSim)		 
#set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_EXTENSIONS FALSE)
##########################################################################
#set(FIXED_LIB ./Fixed/Fixed.h ./Fixed/Fixed.cpp )
set(MAIN_SRC ./Fixed/Fixed.h ./Fixed/FastFixed.h ./Fixed/Fixed.cpp ./LiquidSim/LiquidSimDynamic.h  ./LiquidSim/ThreadPool.h ./LiquidSim/LiquidSimDynamic.h  ./LiquidSim/LiquidSim.cpp ./fluid.cpp)
###########################################################################
#add_library(FixedLib STATIC ${FIXED_LIB})
##########################################################################

add_executable (MAIN ${MAIN_SRC})
set_target_properties(
	MAIN PROPERTIES
	CXX_STANDARD 20 
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
target_compile_definitions(MAIN PRIVATE "TYPES=DOUBLE,FLOAT,FAST_FIXED(13,7),FIXED(32,5)" "SIZES=S(1920,1080),S(10,10),S(42,1337),S(4, 5)")
#target_compile_options(MAIN PRIVATE -DTYPES=DOUBLE,FLOAT,FAST_FIXED\(13,7\),FIXED\(32,5\))



