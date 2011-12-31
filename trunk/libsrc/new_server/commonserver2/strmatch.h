#ifndef __STRING_MATCH__PURE_C_VERSION____
#define __STRING_MATCH__PURE_C_VERSION____

#define MATCH_DEBUG

#ifdef MATCH_DEBUG
#define MTRACE( fmt, args... ) printf( fmt, ##args )
#else
#define MTRACE( fmt, args... )
#endif

#define KN 	  256  //the input character is 8 bits.
#define STATE  int

#define F_TERMINATE 0x80000000
#define FIND_MATCHCASE 0x01

#define ID_NOID			  -2	
#define ID_SPECIAL1		  -3
#define ID_SPECIAL2		  -4

typedef STATE (*STATESLIST)[KN+1]; 

typedef struct {
	unsigned char  *word;
	int gram;
	int	length;
	int   	findnum;
	int   	type;
	int	id;
}KEYWORD; 

typedef struct {
	STATESLIST states;
	KEYWORD    *wordlist;
	int	   wordlistlength;
	int	   wordsnum;
	STATE      wordslength;
	int	   totalfind;
	STATE      usedstates;	
}MATCHENTRY;

MATCHENTRY * strMatchInit(int nSize);

int refreshMachine(char* file, int idx);
char *findKeyWord(char *data_buf, int idx);

int Addword(const char *word,int len, MATCHENTRY *entry, int gram);
int AddwordWithID(int id, const char *word,int len, MATCHENTRY *entry);
char *FindUntilNull(const char *p, int *word, STATE state, MATCHENTRY *entry);
char *FindUntilLen(int num, const char *p, int *word, STATE state, MATCHENTRY *entry);
STATE FindAllUntilNull(const char *p,STATE state, MATCHENTRY *entry);
STATE FindAllUntilLen(int num, const char *p,STATE state, MATCHENTRY *entry);
int Prepare(int model, MATCHENTRY *entry);
void RemoveEntry(MATCHENTRY *entry);
char * Getword(int word, MATCHENTRY *entry);
int Getwordid(int word, MATCHENTRY *entry);
int Getwordsnum(MATCHENTRY *entry);
void Clear(int num, MATCHENTRY *entry);

#endif
