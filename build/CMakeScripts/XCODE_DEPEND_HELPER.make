# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# For each target create a dummy rule so the target does not have to exist
/Users/akram/Developer/pangolin-hdr/build/pangolin/Debug/libpangolin.a:
/opt/local/lib/libGLEW.dylib:
/usr/local/lib/libboost_thread-mt.dylib:
/usr/local/lib/libMagick++.dylib:
/opt/local/lib/libglut.dylib:
/usr/local/lib/libdc1394.dylib:
/usr/local/lib/libexiv2.dylib:
/Users/akram/Developer/pangolin-hdr/build/pangolin/MinSizeRel/libpangolin.a:
/Users/akram/Developer/pangolin-hdr/build/pangolin/RelWithDebInfo/libpangolin.a:
/Users/akram/Developer/pangolin-hdr/build/pangolin/Release/libpangolin.a:


# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.pangolin.Debug:
PostBuild.SimpleHDR.Debug:
PostBuild.pangolin.Debug: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Debug/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Debug/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/Debug/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Debug/SimpleHDR


PostBuild.testApp.Debug:
PostBuild.pangolin.Debug: /Users/akram/Developer/pangolin-hdr/build/testApp/Debug/testApp
/Users/akram/Developer/pangolin-hdr/build/testApp/Debug/testApp:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/Debug/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/testApp/Debug/testApp


PostBuild.pangolin.Release:
PostBuild.SimpleHDR.Release:
PostBuild.pangolin.Release: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/Release/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR


PostBuild.testApp.Release:
PostBuild.pangolin.Release: /Users/akram/Developer/pangolin-hdr/build/testApp/Release/testApp
/Users/akram/Developer/pangolin-hdr/build/testApp/Release/testApp:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/Release/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/testApp/Release/testApp


PostBuild.pangolin.MinSizeRel:
PostBuild.SimpleHDR.MinSizeRel:
PostBuild.pangolin.MinSizeRel: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/MinSizeRel/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR


PostBuild.testApp.MinSizeRel:
PostBuild.pangolin.MinSizeRel: /Users/akram/Developer/pangolin-hdr/build/testApp/MinSizeRel/testApp
/Users/akram/Developer/pangolin-hdr/build/testApp/MinSizeRel/testApp:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/MinSizeRel/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/testApp/MinSizeRel/testApp


PostBuild.pangolin.RelWithDebInfo:
PostBuild.SimpleHDR.RelWithDebInfo:
PostBuild.pangolin.RelWithDebInfo: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/RelWithDebInfo/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR


PostBuild.testApp.RelWithDebInfo:
PostBuild.pangolin.RelWithDebInfo: /Users/akram/Developer/pangolin-hdr/build/testApp/RelWithDebInfo/testApp
/Users/akram/Developer/pangolin-hdr/build/testApp/RelWithDebInfo/testApp:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/RelWithDebInfo/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/usr/local/lib/libMagick++.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib\
	/usr/local/lib/libexiv2.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/testApp/RelWithDebInfo/testApp


