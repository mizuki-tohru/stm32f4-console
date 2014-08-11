#define TYPE_UCHAR  1
#define TYPE_UINT   2
#define TYPE_SINGLE 3
#define TYPE_DOUBLE 4
#define TYPE_EVENT  5
#define TYPE_CONST  6

struct parse_OP{
	struct parse_OP * NextP;
	unsigned int line_no;
	uisigned char code[32];
	unsigned char code_len;
};

