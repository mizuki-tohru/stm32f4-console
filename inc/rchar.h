
#define SC_TXT 1
#define SC_NUM 2

long axtol(char *,unsigned char *,unsigned int);
int ltohex(unsigned long ,unsigned char *,unsigned char);
void clr(char_type);
void clr_all(char_type, char_flg);
void rchar(FILE *,char_type,char_flg);
void cgetchar(char_type);
void next(FILE *,char_type,char_flg);

/*----�p�[�U�f�R�[�h���L�����N�^��ԍ\����--*/
struct char_type{
	char text[256];
	char flg;	/*�����񂩁A���񂩁A�L����*/
	char level;	/*�l�X�g���x���L���p*/
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
�g����:
�@�Ƃ肠���� chdata * CHD = new(chdata);
�@�X�N���v�g�t�@�C����open(filename)���āA�߂��Ă����t�@�C���n���h����
�@CHD->F�ɑ���B
�@���Ƃ�rchar()���ACHD->C_C.flg �� EOF�������Ă���܂œǂ݂Â���B
�@�L��#�ȍ~�̂��̍s�̕����Ɋւ���rchar()�̉����͖����B�ǂݔ�΂����B
�@���p�󔒋y�у^�u��������y�ѐ���̋�؂�ɂȂ��Ă���B���ɂ����Z�q�⊇��
�ł���؂�ɂȂ�B�t�ɂ����ƕ�����ɋ󔒂≉�Z�q�L�����g�����͂ł��Ȃ��B
���݂̂Ƃ���G�X�P�[�v�V�[�P���X�͗p�ӂ��Ă��Ȃ��B
�@�A���t�@�x�b�g��Shift-JIS��2�o�C�g�L�����A_����n�܂��ċ󔒂ŏI���܂�
������Ƃ��Ĉ�����B
�@������������-�Ŏn�܂鐔���݂̂̋L�����10�i����Ƃ��Ĉ�����B
�@���̑��A���ʂ����L���͈ȉ��̂Ƃ���B
  =,==,!,!=,>,<,>=,<=,(,),{,},&,&&,|,||,-,+,*,/,%,",����Ɖ��s������EOF�B
�@"�ň͂�ꂽ�L���͑S�Ė������ŕ�����Ǝ��ʂ����B�����ɂ͋󔒂≉�Z�q��
�܂�ł��ǂ��B
�@CHD->C_C.level���Q�Ƃ���΁A���̉��Z�q�̃l�X�g���l�������D�揇�ʂ�
�ǂݏo�������ł���B�D�揇�ʂ͕W���I�Ȃ��̂ł���B
�@������͍Œ�256�����܂ł����ێ����Ȃ��B
*/
/*--------------------------------------------------------------------------*/
