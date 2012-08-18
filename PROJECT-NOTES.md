HDR PROJECT
===========

Last updated: 16/08/2012

TO DO
=====

Automatic Exposure Control
-------------------------
extract histogram and adjust
--> nearly done

EV
---
calculate EV values based on ticks not by calling getters each time
--> doesn't seem to work

Video Mode
----------
Move video mode to constructor

OPTIMISATIONS
=============

REFACTORING
===========

NICE TO HAVES
=============

COMPLETED
=========

Config file for output
-------------------------
can't use dropdowns, so use config file for video format, tmo etc


SaveFile()
----------
Find appropriate  way to implement functionality to save files and for better workflow
Currently running grab frame --> save frame in loop, a bit inefficient

Better Error Handling for my functions
--------------------------------------
Currently have a mixed method to handle errors


Tone map
------------------------
pfstools/mo/calibrate

Threading
---------
Add threading for all file writing operations


Save to JPEG 
------------
Currently: unsigned char * buffer -> PPM, PPM to ImageMagick object, ImageMagick -> JPEG
Should be: unsigned char * buffer -> JPEG

Camera Response Function 
------------------------
Use robertson (change to debevec)
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