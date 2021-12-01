set(TARGETS 
	quorum_den
	original_nlm
	semibin_nlm
	bin_nlm
	bin_nlm_tree
	bin_epll
	bin_dude
)
foreach (aux ${TARGETS})
 add_executable (${aux} ${aux}.c nlm_options.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
