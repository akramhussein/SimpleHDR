FIND_PACKAGE(FREEGLUT QUIET)
FIND_PACKAGE(GLUT QUIET)

IF(FREEGLUT_FOUND OR GLUT_FOUND)

    ## These samples require Pangolin Var support
    IF(BUILD_PANGOLIN_VARS)

      ## Video Samples require Pangolin Video support
      IF(BUILD_PANGOLIN_VIDEO)
	ADD_SUBDIRECTORY(SimpleHDR)
	ADD_SUBDIRECTORY(testApp)
      ENDIF()

    ENDIF()
ENDIF()
