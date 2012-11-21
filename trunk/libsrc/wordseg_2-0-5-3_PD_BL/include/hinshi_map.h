#ifndef HINSHI_MAP_H
#define HINSHI_MAP_H

extern int hinshi_info[1096][4];

// �����λ�ԭ��ɾ�����δʣ��ӻ����ʺ�׺
extern char *kihongkei[];

extern int kihonglen[34][64];

extern int hyoji_level[34][64];

#define 	MIDASI_MAX	129		/* maximum length of MIDASI */
#define		TABLE_NUM_MAX	1718		/* max table number */
#define		MATRIX_NUM_MAX	50787		/* max matrix number */

/* �B�ӱ� */
typedef         struct          _RENSETU_PAIR {
     int   i_pos;
     int   j_pos;
     int   hinsi;
     int   bunrui;
     int   type;
     int   form;
     char  goi[MIDASI_MAX];
} RENSETU_PAIR;

extern char *table_string;

#endif
