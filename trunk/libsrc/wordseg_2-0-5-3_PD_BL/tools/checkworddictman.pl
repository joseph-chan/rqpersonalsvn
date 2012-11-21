#!/usr/bin/perl 

#��ʹ���дʵ��˹����ƹ���֮ǰ��������checkworddictman.pl�ű����ж�һ����������Ƿ��Ǻ���Ϸ��ġ�
#
#����Ҫ������:
#(1)�����пյ�term����ʵ��Ƭ��
#(2)�´���ǰ��ʵ��Ƭ���ں�
#(3)ʵ��Ƭ�ε��зֽ�����볬��1�������ֻ��һ�������´ʡ�
#(4)�зֽ����ϵ�һ����ʵ��Ƭ�Ρ
#(5)��һ���ǰ汾��
#(6)�Ѿ���ӹ��Ĳ������ظ����
#

if(!open(fp, "worddict.man"))
{
   die "�����ļ����ֱ�����worddict.man������!"."\n";
}

my %hash;
my $line = <fp>;
if($line !~ /^ver:/)
{
   die "��һ�б����ǰ汾�ţ�����!"."\n";
}

my $Nflag = 0;#-Nflag�����1����˵���´��Ѿ������ɡ�
while($line = <fp>)
{
   chomp($line);
   my @array = split(/ /, $line);
   my @temp = split(/\[/, $array[1]);
   my @word = split(/\]/, $temp[1]);
   if($word[0] eq "")
   {
	   print "������ӿյ�term"."\n";
	   exit;
   }
   if(exists $hash{$word[0]})
   {
	  print "�Ѿ���ӹ������,��ȥ��".":".$word[0]."\n";
	  exit;
   }
   $hash{$word[0]} = 1;

   @temp = split(/\[/, $array[2]);
   @temp1 = split(/\]/, $temp[1]);
   my @fragment = split(/\(/, $temp1[0]);
   my @jointfragment = 0;
   for(my $i = 0; $i < @fragment - 1; $i++)
   {   
       @temp = split(/\)/, $fragment[$i+1]);
       $jointfragment[$i] = $temp[0];
   }
   my $str = join("", @jointfragment);
   if($word[0] ne $str)
   {
	   print "error !\n";
	   print "Ƭ�ε����Ӧ�ú���ӵ�term��ͬ"."\n";
	   print "term:".$word[0]."\t"."fragment:".$str."\n";
	   exit;
   }
   if($array[0] eq "-N")
   {
       if($Nflag == 1)
       {
            print "�´ʵ����(-N)Ӧ����ʵ��Ƭ��(-M)���֮ǰ������!"."\n";
	    exit;
       }
       if(@array != 6)
       {
	       print "��ӵ��´ʸ�ʽ����"."\t".$line."\n";
	       print "����´ʸ�ʽ����:-N [newword] [0(basic1)1(basic2)..] [0(subphrase1)1(subphrase2)] prop weight"."\n";
	       exit;
       }
   }
   elsif($array[0] eq "-M")
   {
	   $Nflag = 1;
	   if(@array != 4)
	   {
		   print "��ӵ�ʵ��Ƭ�θ�ʽ����"."\t".$line."\n";
		   print "���ʵ��Ƭ�θ�ʽ����:-M [seg fragment] [0(seg1)1(seg2)...] -M"."\n";
		   exit;
	   }
	   
   }
}

print "��ɹ���ͨ���˼�飬������Ӹ������ļ��ˣ�good luck :-)\n";
