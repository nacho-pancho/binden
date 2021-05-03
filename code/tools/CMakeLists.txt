set(TOOLS 
	gather_stats
)
foreach (aux ${TOOLS})
 add_executable (${aux} ${aux}.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
