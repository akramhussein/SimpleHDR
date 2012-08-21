# response function plotting script
#set terminal pdf
#set output "response_function.pdf"
set terminal png
set output "response_function.png"

set title "Camera Response Function"

set xlabel "Pixel Irradiance" font ",15"
set ylabel "Pixel Brightness (Intensity)" font ",15"

#set xrange [0:5]
set yrange [0:300]

set xtics 0.5
set ytics 25

set style line 1 lt 1 lw 1 pt 1 linecolor rgb "red"
set style line 2 lt 1 lw 1 pt 1 linecolor rgb "green"
set style line 3 lt 1 lw 1 pt 1 linecolor rgb "blue"
#set style line 4 lt 1 lw 1 pt 1 linecolor rgb "purple"
unset key

#set key right bottom

#plot "camera.response" using 3:2 w l ls 1 
plot "< head -262 camera.response | tail +7" using 3:2 w l ls 1 title "Red", \
"< head -525 camera.response | tail +270" using 3:2 w l ls 2 title "Green", \
"< head -788 camera.response | tail +533" using 3:2 w l ls 3 title "Blue"
#"< head -1051 camera.response | tail +796" using 1:2 w l ls 4
