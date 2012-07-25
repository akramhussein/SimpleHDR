HDR PROJECT
===========

Last updated: 23/07/2012

TO DO
=====

Extract shutter/exposure etc data from Camera
---------------------------------------------
See Jan's MetaData functions
Need to save to Exif

Write EXIF Data 
-----------------------
Need to extract EXIF data from camera per frame and write to JPG version.
See http://libexif.sourcearchive.com/documentation/0.6.18-1/cam__features_8c-source.html

Camera Response Function 
------------------------
Use Debevec technique
Need to investigate more

Extract Radiance Map
------------------------
See notes/Ask Jan

Tone map
------------------------
pfs

Motion correction
------------------------
flowlib

Automatic Exposure Control
-------------------------
extract histogram and adjust

Fix errors
------------------------
Xcode highlighting errors, fix them.

OPTIMISATIONS
=============

Save to JPEG 
------------
Currently: unsigned char * buffer -> PPM, PPM to ImageMagick object, ImageMagick -> JPEG
Should be: unsigned char * buffer -> JPEG

Threading
---------
Add threading for all file writing operations


REFACTORING
===========

SaveFile()
----------
Find appropriate implement functionality to save files and for better workflow
Currently running grab frame --> save frame in loop, a bit inefficient


NICE TO HAVES
=============

Better Error Handling for my functions
--------------------------------------
Currently have a mixed method to handle errors

COMPLETED
=========

Create PPM & JPG Directories
----------------------------
Use POSIX functions to check and create
Also, delete before re-using?
* Use boost because its cross compatible with Windows
NOTES: added script to remove before build and create new folders after build. 
Can addmkdir("ppm", 0755);
mkdir("jpg", 0755);
for POSIX version to main or boost::filesystem for universal method