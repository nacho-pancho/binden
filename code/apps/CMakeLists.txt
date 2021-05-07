set(TARGETS 
	quorum_den
	nlm_original
	nlm_semibin
	nlm_fullbin_windowed_avg
	nlm_fullbin_windowed_bayes
	nlm_fullbin_tree_bayes
	bin_epll
)
foreach (aux ${TARGETS})
 add_executable (${aux} ${aux}.c nlm_options.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
