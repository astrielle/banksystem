#include "tokenizer.h"

/*
 * TKCreate creates a new TokenizerT object for a given token stream
 * (given as a string).
 * 
 * TKCreate should copy the arguments so that it is not dependent on
 * them staying immutable after returning.  (In the future, this may change
 * to increase efficiency.)
 *
 * If the function succeeds, it returns a non-NULL TokenizerT.
 * Else it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

 TokenizerT *TKCreate( char * ts ) { 	
 	TokenizerT *tokenizer = (TokenizerT*)malloc(sizeof(TokenizerT));
 	tokenizer->str = (char*)malloc((strlen(ts)+1)*sizeof(char));
 	strcpy (tokenizer->str, ts);
 	tokenizer->index = 0;
 	return tokenizer;
 }

/*
 * TKDestroy destroys a TokenizerT object.  It should free all dynamically
 * allocated memory that is part of the object being destroyed.
 *
 * You need to fill in this function as part of your implementation.
 */

 void TKDestroy( TokenizerT * tk ) {
 	free(tk->str);
 	free(tk);
 }

/*
 * TKGetNextToken returns the next token from the token stream as a
 * character string.  Space for the returned token should be dynamically
 * allocated.  The caller is responsible for freeing the space once it is
 * no longer needed.
 *
 * If the function succeeds, it returns a C string (delimited by '\0')
 * containing the token.  Else it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

 char *TKGetNextToken( TokenizerT * tk ) {

	/*find length of input*/
 	int inputLen = (strlen(tk->str));
 	int start = 0;
 	int curr = 0;
 	int tokenLen = 0;

	/*reached end of str*/
 	if (tk->index == inputLen){
 		//printf("END OF STR\n");
 		return NULL;
 	}

 	start = tk->index;
 	curr = start;

	/*increment through entry whitespace*/
 	while (isspace(tk->str[curr])){
 		curr++;

 		if (curr >= inputLen){
 			//printf("ALL WHITE SPACE\n");
 			return NULL;
 		}
 	}

 	start = curr;

 	/*word*/
 	if(isalnum(tk->str[curr])){
 		while(isalnum(tk->str[curr]) || (tk->str[curr] == '.')){
 			curr++;
 		}
 		tokenLen = curr-start;
 		char *token = malloc((tokenLen+1)*sizeof(char));
 		strncpy(token, &tk->str[start], tokenLen+1);
 		token[tokenLen] = '\0';
 		tk->index = curr;

 		//printf("TOKEN: %s LENGTH: %d\n", token, tokenLen);
 		return token;
 	}

 	return NULL;

}