set log x
set log y
set title "Time (s)"
set output "scatterplot.ps"
set ylabel "Facotrised"
set xlabel "Original"
set xrange [0.0001:12000]
set yrange [0.0001:12000]
set term postscript enhanced colour
set datafile missing "?"
plot "ebtcs.dat" using 1:2 with points title "EBTCS", "dispose.dat" using 1:2 with points title "Dispose", "logistics.dat" using 1:2 with points title "Logistics", "slidingdoors.dat" using 1:2 with points title "Slidingdoors", "find_key.dat" using 1:2 with points title "Find key", x with lines notitle lt rgb "#000000"
