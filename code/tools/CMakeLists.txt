set(TOOLS 
	gather_stats
	analyze_stats
	print_template
	create_template
	add_noise
	estimate_noise
	compare
)
foreach (aux ${TOOLS})
 add_executable (${aux} ${aux}.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
