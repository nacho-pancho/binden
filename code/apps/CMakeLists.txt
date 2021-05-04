set(TARGETS 
	classic_nlm
	bin_nlm_1
	bin_nlm_2
	quorum_den
)
foreach (aux ${TARGETS})
 add_executable (${aux} ${aux}.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
