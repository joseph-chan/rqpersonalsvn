if [ $# -lt 2 ]
then
	echo "usage:<infile> <outfile>"
	exit 1
fi
time ./test localhost 20241 $1 "\n" $2 2>log2.txt
