
#define SC_TXT 1
#define SC_NUM 2

long axtol(char *,unsigned char *,unsigned int);
int ltohex(unsigned long ,unsigned char *,unsigned char);
void clr(char_type);
void clr_all(char_type, char_flg);
void rchar(FILE *,char_type,char_flg);
void cgetchar(char_type);
void next(FILE *,char_type,char_flg);

/*----パーザデコード時キャラクタ状態構造体--*/
struct char_type{
	char text[256];
	char flg;	/*文字列か、数列か、記号か*/
	char level;	/*ネストレベル記憶用*/
	long data;
};

struct char_flg{
	gyou = 0;
	minus_flg   = 0;
	scene_label = 0;
	nest_level = 0;
};
/*--------------------------------------------------------------------------*/
/*
使い方:
　とりあえず chdata * CHD = new(chdata);
　スクリプトファイルをopen(filename)して、戻ってきたファイルハンドラを
　CHD->Fに代入。
　あとはrchar()を、CHD->C_C.flg に EOFが入ってくるまで読みつづける。
　記号#以降のその行の文字に関するrchar()の応答は無い。読み飛ばされる。
　半角空白及びタブが文字列及び数列の区切りになっている。他にも演算子や括弧
でも区切りになる。逆にいうと文字列に空白や演算子記号を使う事はできない。
現在のところエスケープシーケンスは用意していない。
　アルファベットかShift-JISの2バイトキャラ、_から始まって空白で終わるまで
文字列として扱われる。
　数字もしくは-で始まる数字のみの記号列は10進数列として扱われる。
　その他、識別される記号は以下のとおり。
  =,==,!,!=,>,<,>=,<=,(,),{,},&,&&,|,||,-,+,*,/,%,",それと改行文字とEOF。
　"で囲われた記号は全て無条件で文字列と識別される。ここには空白や演算子を
含んでも良い。
　CHD->C_C.levelを参照すれば、その演算子のネストを考慮した優先順位を
読み出す事ができる。優先順位は標準的なものである。
　文字列は最長256文字までしか保持しない。
*/
/*--------------------------------------------------------------------------*/
