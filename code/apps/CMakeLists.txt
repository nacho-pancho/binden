set(TARGETS 
	classic_nlm
	bin_nlm_1
	bin_nlm_2
	bin_nlm_3
	bin_nlm_4
	quorum_den
)
foreach (aux ${TARGETS})
 add_executable (${aux} ${aux}.c nlm_options.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
