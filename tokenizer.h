#ifndef TOKENIZER_H
#define TOKENIZER_H

struct TokenizerT_ {
	char *str; /*contains entire null term input string*/
	int index; /*index of current position*/
 };

 typedef struct TokenizerT_ TokenizerT;

 TokenizerT *TKCreate( char * ts );
 void TKDestroy( TokenizerT * tk );
 char *TKGetNextToken( TokenizerT * tk );

#endif