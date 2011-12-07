######################################
#         分词器的通用变量定义       #
######################################
#分词器的头文件所在
SEG_INC=./TextKeyTool/inc_seg/
#分词器的库文件所在
SEG_LIB=./TextKeyTool/lib_seg/

#库文件
SEG_LIB_FILE=-lcommon 
SEG_LIB_FILE+=-llexicon 
SEG_LIB_FILE+=-lustr 
SEG_LIB_FILE+=-ltouni 
SEG_LIB_FILE+=-lfromuni 
SEG_LIB_FILE+=-lslib 
SEG_LIB_FILE+=-lstdc++ 
SEG_LIB_FILE+=-ldl

#链接所需的中间文件
SEG_O_FILE=$(SEG_LIB)/*.a
