##################################################
#		�ı�key�����ɹ��ߵ�ͨ�ñ�������          #
##################################################

#�ִ��齨ͨ�ñ�������
include ./TextKeyTool/make_seg.inc

#���ļ�����·��
KEY_INC=./TextKeyTool/inc_key

#ͷ�ļ�����·��
KEY_LIB=./TextKeyTool/lib_obj

#���ļ�
KEY_O_FILE=$(KEY_LIB)/CTokenizer.o 
KEY_O_FILE+=$(KEY_LIB)/jenkins.o
KEY_O_FILE+=$(KEY_LIB)/paichong.o
KEY_O_FILE+=$(KEY_LIB)/CMBTextTool.o
KEY_O_FILE+=$(KEY_LIB)/CTextKeyTool.o