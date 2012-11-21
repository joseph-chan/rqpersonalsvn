#!/usr/bin/perl 

#ÔÚÊ¹ÓÃÇĞ´ÊµÄÈË¹¤¿ØÖÆ¹¦ÄÜÖ®Ç°£¬ÇëÏÈÓÃcheckworddictman.pl½Å±¾À´ÅĞ¶ÏÒ»ÏÂÄãµÄÊäÈëÊÇ·ñÊÇºÏÀíºÏ·¨µÄ¡£
#
#ÊäÈëÒªÇóÈçÏÂ:
#(1)²»ÄÜÓĞ¿ÕµÄterm»òÕßÊµÀıÆ¬¶Î
#(2)ĞÂ´ÊÔÚÇ°£¬ÊµÀıÆ¬¶ÎÔÚºó
#(3)ÊµÀıÆ¬¶ÎµÄÇĞ·Ö½á¹û±ØĞë³¬¹ı1¸ö£¬Èç¹ûÖ»ÓĞÒ»¸öÄÇÊÇĞÂ´Ê¡£
#(4)ÇĞ·Ö½á¹û×éºÏµ½Ò»Æğ»¹ÊÇÊµÀıÆ¬¶Î¡
#(5)µÚÒ»ĞĞÊÇ°æ±¾ºÅ
#(6)ÒÑ¾­Ìí¼Ó¹ıµÄ²»ÔÊĞíÖØ¸´Ìí¼Ó
#

if(!open(fp, "worddict.man"))
{
   die "¿ØÖÆÎÄ¼şÃû×Ö±ØĞëÊÇworddict.man£¬Çë¼ì²é!"."\n";
}

my %hash;
my $line = <fp>;
if($line !~ /^ver:/)
{
   die "µÚÒ»ĞĞ±ØĞëÊÇ°æ±¾ºÅ£¬Çë¼ì²é!"."\n";
}

my $Nflag = 0;#-NflagÈç¹ûÊÇ1£¬ÔòËµÃ÷ĞÂ´ÊÒÑ¾­Ìí¼ÓÍê³É¡£
while($line = <fp>)
{
   chomp($line);
   my @array = split(/ /, $line);
   my @temp = split(/\[/, $array[1]);
   my @word = split(/\]/, $temp[1]);
   if($word[0] eq "")
   {
	   print "²»ÄÜÌí¼Ó¿ÕµÄterm"."\n";
	   exit;
   }
   if(exists $hash{$word[0]})
   {
	  print "ÒÑ¾­Ìí¼Ó¹ıÕâ¸ö´Ê,ÇëÈ¥³ı".":".$word[0]."\n";
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
	   print "Æ¬¶ÎµÄ×éºÏÓ¦¸ÃºÍÌí¼ÓµÄtermÏàÍ¬"."\n";
	   print "term:".$word[0]."\t"."fragment:".$str."\n";
	   exit;
   }
   if($array[0] eq "-N")
   {
       if($Nflag == 1)
       {
            print "ĞÂ´ÊµÄÌí¼Ó(-N)Ó¦¸ÃÔÚÊµÀıÆ¬¶Î(-M)Ìí¼ÓÖ®Ç°£¬Çë¼ì²é!"."\n";
	    exit;
       }
       if(@array != 6)
       {
	       print "Ìí¼ÓµÄĞÂ´Ê¸ñÊ½²»¶Ô"."\t".$line."\n";
	       print "Ìí¼ÓĞÂ´Ê¸ñÊ½ÈçÏÂ:-N [newword] [0(basic1)1(basic2)..] [0(subphrase1)1(subphrase2)] prop weight"."\n";
	       exit;
       }
   }
   elsif($array[0] eq "-M")
   {
	   $Nflag = 1;
	   if(@array != 4)
	   {
		   print "Ìí¼ÓµÄÊµÀıÆ¬¶Î¸ñÊ½²»¶Ô"."\t".$line."\n";
		   print "Ìí¼ÓÊµÀıÆ¬¶Î¸ñÊ½ÈçÏÂ:-M [seg fragment] [0(seg1)1(seg2)...] -M"."\n";
		   exit;
	   }
	   
   }
}

print "Äã³É¹¦µÄÍ¨¹ıÁË¼ì²é£¬¿ÉÒÔÌí¼Ó¸ÃÅäÖÃÎÄ¼şÁË£¬good luck :-)\n";
