/* 2011 ISO C Lexer */

%e  1019
%p  2807
%n  371
%k  284
%a  1213
%o  1117

O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{
#include <stdio.h>
#include <stdbool.h>
#include "iso2011c.tab.h"

extern void yyerror(const char *);      /* prints grammar violation message */

static void comment_multiple_line( void );
static void comment_single_line( void );
static void count( void );

bool G_FlagDeclare;

#define COPY_STRING { yylval.string=(char*)malloc((yyleng+1)*(sizeof(char))); strncpy(yylval.string, yytext, yyleng + 1); }
%}

%%
"/*"                    { comment_multiple_line(); }
"//".*                  { comment_single_line();   }
"#".*					{ comment_single_line();   }

"auto"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(AUTO); } else comment_single_line(); }
"break"					{ count(); return(BREAK);    }
"case"					{ count(); return(CASE);     }
"char"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(CHAR); } else comment_single_line(); }
"const"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(CONST);} else comment_single_line(); }
"continue"				{ count(); return(CONTINUE); }
"default"				{ count(); return(DEFAULT);  }
"do"					{ count(); return(DO);       }
"double"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(DOUBLE);} else comment_single_line();}
"else"					{ count(); return(ELSE);     }
"enum"					{ count(); return(ENUM);     }
"extern"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(EXTERN);} else comment_single_line();}
"float"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(FLOAT);}  else comment_single_line();}
"for"					{ count(); return(FOR);      }
"goto"					{ count(); return(GOTO);     }
"if"					{ count(); return(IF);       }
"inline"				{ count(); return(INLINE);   }
"int"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(INT);     } else comment_single_line(); }
"long"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(LONG);    } else comment_single_line(); }
"register"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(REGISTER);} else comment_single_line(); }
"restrict"				{ count(); return(RESTRICT); }
"return"				{ /*count(); return(RETURN);*/ comment_single_line(); }
"short"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(SHORT); } else comment_single_line(); }
"signed"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(SIGNED);} else comment_single_line(); }
"sizeof"				{ count(); return(SIZEOF);   }
"static"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(STATIC);} else comment_single_line(); }
"struct"				{ count(); return(STRUCT);   }
"switch"				{ count(); return(SWITCH);   }
"typedef"				{ count(); return(TYPEDEF);  }
"union"					{ count(); return(UNION);    }
"unsigned"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(UNSIGNED);} else comment_single_line(); }
"void"					{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(VOID);    } else comment_single_line(); }
"volatile"				{ if(!G_FlagDeclare) {count(); G_FlagDeclare = true; return(VOLATILE);} else comment_single_line(); }
"while"					{ count(); return(WHILE);    }
"_Alignas"              { count(); return ALIGNAS;       }
"_Alignof"              { count(); return ALIGNOF;       }
"_Atomic"               { count(); return ATOMIC;        }
"_Bool"                 { count(); return BOOL;          }
"_Complex"              { count(); return COMPLEX;       }
"_Generic"              { count(); return GENERIC;       }
"_Imaginary"            { count(); return IMAGINARY;     }
"_Noreturn"             { count(); return NORETURN;      }
"_Static_assert"        { count(); return STATIC_ASSERT; }
"_Thread_local"         { count(); return THREAD_LOCAL;  }
"__func__"              { count(); return FUNC_NAME;     }

{L}{A}*					{ count(); COPY_STRING return(IDENTIFIER);  }

{HP}{H}+{IS}?				{ count(); COPY_STRING return(I_CONSTANT); }
{NZ}{D}*{IS}?				{ count(); COPY_STRING return(I_CONSTANT); }
"0"{O}*{IS}?				{ count(); COPY_STRING return(I_CONSTANT); }
{CP}?"'"([^'\\\n]|{ES})+"'"	{ count(); COPY_STRING return(I_CONSTANT); }

{D}+{E}{FS}?				{ count(); COPY_STRING return(F_CONSTANT); }
{D}*"."{D}+{E}?{FS}?		{ count(); COPY_STRING return(F_CONSTANT); }
{D}+"."{E}?{FS}?			{ count(); COPY_STRING return(F_CONSTANT); }
{HP}{H}+{P}{FS}?			{ count(); COPY_STRING return(F_CONSTANT); }
{HP}{H}*"."{H}+{P}{FS}?		{ count(); COPY_STRING return(F_CONSTANT); }
{HP}{H}+"."{P}{FS}?			{ count(); COPY_STRING return(F_CONSTANT); }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ count(); COPY_STRING return(STRING_LITERAL); }

"..."					{ count(); return ELLIPSIS;     }
">>="					{ count(); return RIGHT_ASSIGN; }
"<<="					{ count(); return LEFT_ASSIGN;  }
"+="					{ count(); return ADD_ASSIGN;   }
"-="					{ count(); return SUB_ASSIGN;   }
"*="					{ count(); return MUL_ASSIGN;   }
"/="					{ count(); return DIV_ASSIGN;   }
"%="					{ count(); return MOD_ASSIGN;   }
"&="					{ count(); return AND_ASSIGN;   }
"^="					{ count(); return XOR_ASSIGN;   }
"|="					{ count(); return OR_ASSIGN;    }
">>"					{ count(); return RIGHT_OP; }
"<<"					{ count(); return LEFT_OP;  }
"++"					{ count(); return INC_OP;   }
"--"					{ count(); return DEC_OP;   }
"->"					{ count(); return PTR_OP;   }
"&&"					{ count(); return AND_OP;   }
"||"					{ count(); return OR_OP;    }
"<="					{ count(); return LE_OP;    }
">="					{ count(); return GE_OP;    }
"=="					{ count(); return EQ_OP;    }
"!="					{ count(); return NE_OP;    }
";"						{ count(); return SEMIC;       }
("{"|"<%")				{ count(); return LEFT_CURBR;  }
("}"|"%>")				{ count(); return RIGHT_CURBR; }
","						{ count(); return COMMA;       }
":"						{ count(); return COLON;       }
"="						{ count(); return EQUAL;       }
"("						{ count(); return LEFT_BR;     }
")"						{ count(); return RIGHT_BR;    }
("["|"<:")				{ count(); return LEFT_SQBR;   }
("]"|":>")				{ count(); return RIGHT_SQBR;  }
"."						{ count(); return DOT;         }
"&"						{ count(); return BIT_AND;     }
"!"						{ count(); return NOT_OP;      }
"~"						{ count(); return BIT_NOT;     }
"-"						{ count(); return MINUS;       }
"+"						{ count(); return PLUS;        }
"*"						{ count(); return MULT;        }
"/"						{ count(); return DIV;         }
"%"						{ count(); return MOD;         }
"<"						{ count(); return LT_OP;       }
">"						{ count(); return GT_OP;       }
"^"						{ count(); return BIT_XOR;     }
"|"						{ count(); return BIT_OR;      }
"?"						{ count(); return TERNARY;     }

{WS}					{ /* whitespace separates tokens */ }
.						{ /* discard bad characters      */ }

%%

int yywrap(void)        /* called at end of input */
{
    return 1;           /* terminate now */
}


static void comment_multiple_line( void )
{
    int c;

    while ((c = input()) != 0)
        if (c == '*')
        {
            while ((c = input()) == '*')
                ;

            if (c == '/')
                return;

            if (c == 0)
                break;
        }
    yyerror("unterminated comment");
}


static void comment_single_line( void )
{
	//for handling //one line comments
	char c;

	while((c=input())!='\n' && c!=0)
		;
}



int column = 0;

static void count( void )
{
	int i;

	for (i = 0; yytext[i] != '\0'; i++)
		if (yytext[i] == '\n')
			column = 0;
		else if (yytext[i] == '\t')
			column += 8 - (column % 8);
		else
			column++;

	//Uncomment the following command to detect syntax errors
	//ECHO;
}
