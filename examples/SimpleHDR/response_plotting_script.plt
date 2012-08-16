# response function plotting script
set terminal jpeg
set output "response_function.jpeg"
set title "Camera Response Function"
set xlabel "Log Irradiance" font ",15"
set ylabel "Intensity" font ",15"
plot "camera.response" with dots