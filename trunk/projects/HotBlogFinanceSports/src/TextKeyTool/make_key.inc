##################################################
#		文本key串生成工具的通用变量定义          #
##################################################

#分词组建通用变量定义
include ./TextKeyTool/make_seg.inc

#库文件所在路径
KEY_INC=./TextKeyTool/inc_key

#头文件所在路径
KEY_LIB=./TextKeyTool/lib_obj

#库文件
KEY_O_FILE=$(KEY_LIB)/CTokenizer.o 
KEY_O_FILE+=$(KEY_LIB)/jenkins.o
KEY_O_FILE+=$(KEY_LIB)/paichong.o
KEY_O_FILE+=$(KEY_LIB)/CMBTextTool.o
KEY_O_FILE+=$(KEY_LIB)/CTextKeyTool.o
