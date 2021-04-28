#set(TESTS
#	test
#)
set(TESTS 
  test_pnm
  test_image
  test_templates
  test_patches
  test_stats
)

foreach (aux ${TESTS})
 add_executable (${aux} ${aux}.c)
 if (WIN32)
   target_link_libraries(${aux} binden)
 else() # UNIX
   target_link_libraries(${aux} binden -lm)
 endif()
endforeach (aux)
