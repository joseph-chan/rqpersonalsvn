killall testServer
sleep 1

###记录重启server的时间###
year=`date -d "0 days" +%Y`
month=`date -d "0 days" +%m`
day=`date -d "0 days" +%d`
hour=`date -d "0 days" +%H`
minute=`date -d "0 days" +%M`
time=$year$month$day-$hour:$minute
echo "the server restart time is $time"

mv CommonServer testServer
./testServer -d 20230 1>log1.txt 2>err 
