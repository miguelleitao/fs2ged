/*
 * fs2ged.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

#define MAX_LINE_LEN (860)

typedef char pId_t[10];
typedef char date_t[20];
typedef char place_t[40];

typedef struct {
    pId_t	id;
    pId_t	father;
    pId_t	mother;
    short int	partners;
    pId_t	*partner;
    short int 	children;
    pId_t	*child;

    char	*givenName;
    char	*surName;
    char	gender;

    date_t	birthDate;
    place_t	birthPlace;

    date_t	baptDate;
    place_t	baptPlace;
} person_t;
    
int error(char *msg) {
    fprintf (stderr,"Error %s\n",msg);
    exit(1);
}

char *parseString(char *s) {
	//printf("parsing string: '%s'\n", s);
    assert( *s == '"' );
    char *val = s+1;
    char *val_end = strchr(val,'"');
    if ( val_end ) *val_end = 0;
    else error("Parsing field value");
    return val_end + 1;
}


char *parseField(char *s) {
    if ( *s == '"' ) 
	return parseString(s);
    if ( *s == '[' ) {
	    //printf("parsing field in '%s'\n",s);
	    int cntBrac = 0;
	    int outAsp = 1;
	    while( *s ) {
		switch (*s) {
		    case '"':
			outAsp = 1 - outAsp;
			break;
		    case '[': 
			cntBrac += outAsp;
			break;
		    case ']':
			cntBrac -= outAsp;
			break;
		}
		if ( cntBrac==0 ) {
		    *s = 0;
		    return s+1;
		}
		s++;
	    }
	    error("Parsing field");
	    return s;
    }
    // Must be an unenclosed number or word
    char *s_end = strchr(s,',');
    if ( s_end ) {
	*s_end = 0;
	return s_end+1;
    }
    s_end = strchr(s,'}');
    if ( s_end ) {
	*s_end = 0;
	return s_end+1;
    }
    return s+strlen(s);
}

void printString(char *fname, char *val) {
    printf("%s: '%s'\n", fname, val);
}

void printPerson(person_t *p) {
    printString("id", p->id);
    printString("given", p->givenName);
    printString("last", p->surName);
    printString("birth place", p->birthPlace);
}

person_t *parsePerson(char *line) {
    person_t *newP;
    newP = malloc(sizeof(person_t));
    if ( ! newP ) return newP;

    char *p = line;
    if ( *p=='{' ) p++;
    char *fname = NULL;
    char *fval = NULL;
    while( *p && *p!='}' && *p!='\n' ) {
	if ( *p==' ' || *p==',' ) {
	    p++;
	    continue;
	}
        fname = p+1;
	p = parseString(p);
	assert( *p==':' );
	p += 2;
	if ( *p=='"' || *p=='[' ) fval = p+1;
	else fval = p;
	p = parseField(p);
        printf("fname:'%s' fval:'%s'\n",fname,fval);

    	if ( ! strcmp(fname,"fid") ) {
		printf("  NAME:%s\n",fval);
	    }
    	else printf("unkown field %s\n",fname);
    }
    return newP;
}

int parseFile(char *fname) {
   FILE *fp;
   char line[MAX_LINE_LEN];

   /* opening file for reading */
   if ( fname ) {
   	fp = fopen(fname , "r");
	if (fp == NULL) {
	   perror("Error opening file");
	   return(-1);
	}
   }
   else fp = stdin;

   while( fgets(line, MAX_LINE_LEN, fp)!=NULL ) {
      /* writing content to stdout */
      person_t *p = parsePerson(line);
      if ( p ) {
	printPerson(p);
      	free(p);
      }
   }
   fclose(fp);
   return 0;
}


int main(int argc, char **argv) {
    char *fname = NULL;
    if ( argc>1 ) fname = argv[1];
    parseFile(fname);

}

