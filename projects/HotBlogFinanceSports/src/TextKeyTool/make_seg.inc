######################################
#         �ִ�����ͨ�ñ�������       #
######################################
#�ִ�����ͷ�ļ�����
SEG_INC=./TextKeyTool/inc_seg/
#�ִ����Ŀ��ļ�����
SEG_LIB=./TextKeyTool/lib_seg/

#���ļ�
SEG_LIB_FILE=-lcommon 
SEG_LIB_FILE+=-llexicon 
SEG_LIB_FILE+=-lustr 
SEG_LIB_FILE+=-ltouni 
SEG_LIB_FILE+=-lfromuni 
SEG_LIB_FILE+=-lslib 
SEG_LIB_FILE+=-lstdc++ 
SEG_LIB_FILE+=-ldl

#����������м��ļ�
SEG_O_FILE=$(SEG_LIB)/*.a