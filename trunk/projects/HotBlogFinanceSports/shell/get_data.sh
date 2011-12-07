#!/bin/sh 

######variable definition#########
prehours=-4
date=`date -d "$prehours hours" "+%Y%m%d"`
start_hour=`date -d "$prehours hours" "+%H"`
hour_num=2
start_time=`date -d "0 hour" "+%Y-%m-%d %H:%M:%S"`

project=/data4/ruqiang/projects/HotBlogFinanceSports/
data_dir=$project/data/


classified_data=172.16.42.32::guibin3/mbInfo/
original_data=172.16.42.32::guibin3/mbContent/
forward_data=172.16.42.32::guibin3/mbTransmitNum/

echo start time is : $start_time
##################################################
######download classified data##############
##################################################

hour_num_got=0
hour=$start_hour
while [ $hour_num_got -ne $hour_num ]
do
	rsync -av $classified_data/h_${date}_${hour}* $data_dir/temp
	hour=`date -d "+1 hour $hour" "+%H"`
	if [ $hour -eq 00 ]
	then
		date=`date -d "+1 day $date" "+%Y%m%d"`
	fi
	let hour_num_got=$hour_num_got+1
done


cat $data_dir/temp/h_${date}_* > $data_dir/weibo_classified.txt
rm $data_dir/temp/h_${date}_*


##################################################
#######download original json data#################
##################################################
hour_num_got=0
hour=$start_hour
while [ $hour_num_got -ne $hour_num ]
do
	rsync -av $original_data/$date/$hour.txt $data_dir/temp
	hour=`date -d "+1 hour $hour" "+%H"`
	if [ $hour -eq 00 ]
	then
		date=`date -d "+1 day $date" "+%Y%m%d"`
	fi
	let hour_num_got=$hour_num_got+1
done


cat $data_dir/temp/*.txt > $data_dir/weibo_original.txt
rm $data_dir/temp/*.txt

##################################################
#######download forward data#################
##################################################
hour=$start_hour
hour_num_got=0
while [ $hour_num_got -ne $hour_num ]
do
	rsync -av $forward_data/minibloghot_$date$hour* $data_dir/temp
	hour=`date -d "+1 hour $hour" "+%H"`
	if [ $hour -eq 00 ]
	then
		date=`date -d "+1 day $date" "+%Y%m%d"`
	fi
	let hour_num_got=$hour_num_got+1
done

cat $data_dir/temp/minibloghot_$date* > $data_dir/weibo_forward.txt
rm $data_dir/temp/minibloghot_$date*

##########to be implemented#######################





############################################################



############################################################
#########################获取评论数据###########################
############################################################
remote_cmt=172.16.235.167::yaowei/formatdata/
remote_cmt_backup=172.16.105.75::yaowei/formatdata/
data_cmt=$data_dir/comment.txt
rsync -av $remote_cmt/"$date"_backupplum/"$date"_plum $data_cmt &
rsync -av $remote_cmt_backup/"$date"_backupplum/"$date"_plum_slave $data_cmt"_slave"
count=0
while [ true ]
do
	if [ -e $data_cmt ]
	then
		break
	fi
	sleep 5
	#超时20分钟则跳过
	let count=$count+1
	echo 等待第$count个5秒
	if [ $count -gt 240 ]
	then
		break;
	fi
done
if [ -e $data_cmt"_slave" ]
then
	cat $data_cmt"_slave" >> $data_cmt
	rm -f $data_cmt"_slave"
fi
cur_hour=`date -d "0 hour" "+%H"`
if [ $cur_hour -lt $start_hour ]
then
	this_date=`date -d "0 hour" "+%Y%m%d"`
	echo $this_date
	rsync -av $remote_cmt/"$this_date"_backupplum/"$this_date"_plum $data_dir/$this_date.tmp &
	rsync -av $remote_cmt_backup/"$this_date"_backupplum/"$this_date"_plum_slave $data_dir/$this_date.tmp_slave
	#等待其中一个rsync结束
	count=0;
	while [ true ]
	do
		if [ -e $data_dir/$this_date.tmp ]
		then
			break
		fi
		sleep 5
		#超时20分钟则跳过
		let count=$count+1
		if [ $count -gt 240 ]
		then
			break;
		fi
	done
	if [ -e $data_dir/$this_date.tmp_slave ]
	then
		cat $data_dir/$this_date.tmp_slave >> $data_dir/$this_date.tmp
		rm -f $data_dir/$this_date.tmp_slave
	fi
fi

##################提取评论时间在指定的时间段内的评论############
echo 提取评论时间在指定的时间段内的评论
cur_hour=`date -d "0 hour" "+%H"`
const1=\"created_at\":\"
hour_tmp=$start_hour
grep_expr=""
if [ $hour_tmp -gt $cur_hour ]
then
	let cur_hour=23
fi
while [ $hour_tmp -le $cur_hour ]
do
	curDay=`echo "$date"|sed -r 's/(....)(..)(..)/\1-\2-\3/'`
	grep_expr=`echo $grep_expr$const1$curDay $hour_tmp`
	hour_tmp=`date -d "+1 Hour $hour_tmp" +%H`
	if [ $hour_tmp -gt $cur_hour ] || [ $hour_tmp -eq 00 ]
	then
		break;
	fi
	grep_expr=`echo $grep_expr"|"`
done
#echo $grep_expr
egrep "$grep_expr" $data_cmt > $data_cmt.tmp
#echo $data_dir/$this_date.tmp
if [ -e $data_dir/$this_date.tmp ]
then
	cat $data_cmt.tmp $data_dir/$this_date.tmp > $data_cmt
	sh $shell_dir/remove.sh $data_dir/$this_date.tmp
else
	mv $data_cmt.tmp $data_cmt
fi


