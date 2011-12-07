if [ $# -lt 2 ]
then
	echo "usage:<infile> <outfile>"
	exit 1
fi
time ./commonclient localhost 23202 $1 "\n" $2 2>log2.txt
