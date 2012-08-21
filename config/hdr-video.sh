#!/bin/sh

# HDR video frame conversion script
# Author: Akram Hussein (ah4811@doc.ic.ac.uk)
# Description: Takes every pair of frames and converts to tone mapped jpeg and then to video

# ARGUMENTS
# $1 = number of frames

#for img in image%*.jpeg; do pfsin ${img} | pfsout ${img%%.hdr}.exr; done

for ((i=0; i<=$1; ++i )) ; 
do
	@ j= 1
	
	echo "pfsinme ./hdr-video/jpeg/image000$i.jpeg ./hdr-video/jpeg/image000$i.jpeg \
	| pfshdrcalibrate -f ./config/camera.response \
    | pfsoutexr ./hdr-video/temp-exr/image000$j.exr && pfsinexr ./hdr-video/temp-exr/image000$j.exr \
    | pfstmo_drago03 | pfsout ./hdr-video/temp-jpeg/image000$j.jpeg"
	
	i = i+1

done

echo "convert ./hdr-video/temp-jpeg/*.jpeg -quality 100 ./hdr-video/hdr.mpeg"