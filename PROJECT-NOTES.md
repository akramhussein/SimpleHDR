HDR PROJECT
===========

Last updated: 01/08/2012

TO DO
=====

Extract Radiance Map
------------------------
See notes/Ask Jan

Tone map
------------------------
pfstools/mo/calibrate

Motion correction
------------------------
flowlib

Automatic Exposure Control
-------------------------
extract histogram and adjust

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
Find appropriate  way to implement functionality to save files and for better workflow
Currently running grab frame --> save frame in loop, a bit inefficient


NICE TO HAVES
=============

Better Error Handling for my functions
--------------------------------------
Currently have a mixed method to handle errors

COMPLETED
=========


Camera Response Function 
------------------------
Use Debevec technique
Need to investigate more


Create PPM & JPG Directories
----------------------------
Use POSIX functions to check and create
Also, delete before re-using?
* Use boost because its cross compatible with Windows
NOTES: added script to remove before build and create new folders after build. 
Can addmkdir("ppm", 0755);
mkdir("jpg", 0755);
for POSIX version to main or boost::filesystem for universal method


Write EXIF Data 
-----------------------
Exiv2 -- works but not all tags write 

Extract shutter/exposure etc data from Camera
---------------------------------------------
See Jan's MetaData functions
Need to save to Exif