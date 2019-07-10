file(GLOB_RECURSE EXAMPLES "z-all-examples/*.rmt")

install (FILES ${EXAMPLES} DESTINATION legacy-examples)
