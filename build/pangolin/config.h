#ifndef PANGOLIN_CONFIG_H
#define PANGOLIN_CONFIG_H

/*
 * Configuration Header for Pangolin
 */

/// Version
#define PANGOLIN_VERSION_MAJOR 0
#define PANGOLIN_VERSION_MINOR 1

/// Pangolin options
#define BUILD_PANGOLIN_VARS
#define BUILD_PANGOLIN_VIDEO

/// Configured libraries
/* #undef HAVE_CUDA */
/* #undef HAVE_CVARS */
/* #undef HAVE_EIGEN */
/* #undef HAVE_TOON */
#define HAVE_DC1394
/* #undef HAVE_V4L */
/* #undef HAVE_FFMPEG */
/* #undef HAVE_OPENNI */
#define HAVE_GLUT
#define HAVE_FREEGLUT
/* #undef HAVE_APPLE_OPENGL_FRAMEWORK */

/// Platform
#define _UNIX_
/* #undef _WIN_ */
#define _OSX_
/* #undef _LINUX_ */

/// Compiler
#define _GCC_
/* #undef _MSVC_ */

#endif //PANGOLIN_CONFIG_H
