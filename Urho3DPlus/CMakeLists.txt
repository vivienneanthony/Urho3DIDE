#
# Copyright (c) 2008-2014 the Urho3D project.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
        LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()



# Define target name
set (TARGET_NAME Urho3DPlus)

# Define source files
SUBDIRLIST(Urho3DIDE_SUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/Source)

FOREACH(SOURCE ${Urho3DIDE_SUBDIRS})  

		file (GLOB CPP_FILES_ Source/${SOURCE}/*.cpp Source/${SOURCE}/*.c )
		file (GLOB H_FILES_ Source/${SOURCE}/*.h Source/${SOURCE}/*.hpp Source/${SOURCE}/*.inl)

		source_group ("Source Files\\${SOURCE}" FILES ${CPP_FILES_})
    		source_group ("Header Files\\${SOURCE}" FILES ${H_FILES_})

		set (H_FILES ${H_FILES} ${H_FILES_})
		set (CPP_FILES ${CPP_FILES} ${CPP_FILES_})

	SUBDIRLIST(Urho3DIDE_SUBSUBDIRS ${CMAKE_CURRENT_SOURCE_DIR}/Source/${SOURCE})
	FOREACH(SUBSOURCE ${Urho3DIDE_SUBSUBDIRS}) 
		file (GLOB CPP_FILES_ Source/${SOURCE}/${SUBSOURCE}/*.cpp Source/${SOURCE}/${SUBSOURCE}/*.c )
		file (GLOB H_FILES_ Source/${SOURCE}/${SUBSOURCE}/*.h Source/${SOURCE}/${SUBSOURCE}/*.hpp Source/${SOURCE}/${SUBSOURCE}/*.inl)

		source_group ("Source Files\\${SOURCE}\\${SUBSOURCE}" FILES ${CPP_FILES_})
    		source_group ("Header Files\\${SOURCE}\\${SUBSOURCE}" FILES ${H_FILES_})

		set (H_FILES ${H_FILES} ${H_FILES_})
		set (CPP_FILES ${CPP_FILES} ${CPP_FILES_})

		include_directories(Source/${SOURCE}/${SUBSOURCE})
	ENDFOREACH()

	include_directories(Source/${SOURCE})
ENDFOREACH()

file (GLOB CPP_FILES_ Source/*.cpp Source/*.c )
file (GLOB H_FILES_ Source/*.h Source/*.hpp Source/*.inl)
set (H_FILES ${H_FILES} ${H_FILES_})
set (CPP_FILES ${CPP_FILES} ${CPP_FILES_})

set (CPP_FILES ${CPP_FILES} ${CPP_FILES_})
set (SOURCE_FILES ${CPP_FILES} ${H_FILES})

# Setup target with resource copying
setup_library ()

target_include_directories (Urho3DPlus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source)



