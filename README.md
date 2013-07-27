Simple HDR
=========

This project incorporates additional functionality to Pangolin developed by 
Steven Lovegrove (@stevenlovegrove) to allow for greater control of firewire 
cameras and to enable the capture HDR images and video.

An example of these additions can be found in the examples/SimpleHDR.

Examples of HDR Images and Video can be found [here](http://www.akramhussein.co.uk/simplehdr)

What is Pangolin
----------------

Pangolin is a lightweight rapid development library for managing OpenGL
display / interaction and video input. At its heart is a simple OpenGl
viewport manager which can help to modularise 3D visualisation without
adding to its complexity, and offers an advanced but intuitive 3D
navigation handler. Pangolin also provides a mechanism for manipulating
program variables through config files and ui integration.

The ethos of Pangolin is to reduce the boilerplate code that normally
gets written to visualise and interact with (typically image and 3D
based) systems, without compromising performance.

Required Dependencies
---------------------

__OpenGL__

__Boost__ 

* (win) http://www.boost.org/users/download/
* (deb) sudo apt-get install libboost-dev libboost-thread-dev
* (mac) sudo port install boost

__CMake__ (for build environment)

* (win) http://www.cmake.org/cmake/resources/software.html
* (deb) sudo apt-get install cmake
* (mac) sudo port install cmake

__FreeGlut / GLU / Glew__ (required for drawing text and windowing)

* (win) http://www.transmissionzero.co.uk/software/freeglut-devel/
* (deb) sudo apt-get install freeglut3-dev libglu-dev libglew-dev
* (mac) sudo port install freeglut glew

Optional Dependencies
---------------------

__FFMPEG__ (for video decoding and image rescaling)

* (deb) sudo apt-get install ffmpeg libavcodec-dev libavutil-dev libavformat-dev libswscale-dev

__DC1394__ (for firewire input)

* (deb) sudo apt-get install libdc1394-22-dev libraw1394-dev
* (mac) sudo port install libdc1394

__ImageMagick / Magick++__ (for ppm to jpg conversion and general image manipulation)
 
* (win) http://www.imagemagick.org/script/magick++.php
* (deb) sudo apt-get install imagemagick libmagick++-dev
* (mac) sudo port install ImageMagick
	
__Exiv2__ (for writing exif data to jpgs)

* (win) http://www.exiv2.org/download.html
* (deb) sudo apt-get install exiv2 (may need to install from source)
* (mac) sudo port install exiv2

__JPEG__ (for generating JPEGS directly from the camera)

_note: may come installed with Magick++_

* (win) http://gnuwin32.sourceforge.net/packages/jpeg.htm _(untested)_
* (deb) sudo apt-get install libjpeg8
* (mac) sudo port install jpeg

__PFSTools__ (for generating response functions and HDR images)

_note: quite tricky to install on mac/win_

* (win) http://sourceforge.net/projects/pfstools/files/
* (deb) sudo apt-get install pfstools
* (mac) http://sourceforge.net/projects/pfstools/files/

Very Optional Dependencies
--------------------------

__CUDA Toolkit__ (>= 3.2)

* http://developer.nvidia.com/object/cuda_3_2_downloads.html

__Cg Library__ (some small Cg utils)

* (deb) sudo apt-get install nvidia-cg-toolkit

__gnuplot__ (for plotting response curve)

* (win) http://www.gnuplot.info/
* (deb) sudo apt-get install gnuplot
* (mac) sudo port install gnuplot
