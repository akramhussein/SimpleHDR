# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# For each target create a dummy rule so the target does not have to exist
/Users/akram/Developer/pangolin-hdr/build/pangolin/Debug/libpangolin.a:
/opt/local/lib/libGLEW.dylib:
/usr/local/lib/libboost_thread-mt.dylib:
/opt/local/lib/libglut.dylib:
/usr/local/lib/libdc1394.dylib:
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
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Debug/SimpleHDR


PostBuild.pangolin.Release:
PostBuild.SimpleHDR.Release:
PostBuild.pangolin.Release: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/Release/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/Release/SimpleHDR


PostBuild.pangolin.MinSizeRel:
PostBuild.SimpleHDR.MinSizeRel:
PostBuild.pangolin.MinSizeRel: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/MinSizeRel/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/MinSizeRel/SimpleHDR


PostBuild.pangolin.RelWithDebInfo:
PostBuild.SimpleHDR.RelWithDebInfo:
PostBuild.pangolin.RelWithDebInfo: /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR
/Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR:\
	/Users/akram/Developer/pangolin-hdr/build/pangolin/RelWithDebInfo/libpangolin.a\
	/opt/local/lib/libGLEW.dylib\
	/usr/local/lib/libboost_thread-mt.dylib\
	/opt/local/lib/libglut.dylib\
	/usr/local/lib/libdc1394.dylib
	/bin/rm -f /Users/akram/Developer/pangolin-hdr/build/SimpleHDR/RelWithDebInfo/SimpleHDR


