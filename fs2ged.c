

/* fs2ged
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

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
    fprintf (stderr,"%s\n",msg);
    exit(1);
}

char *parseString(char *field, char *fval) {
    assert( *fval == '"' );
    char *val = fval+1;
    char *val_end = strchr(val,'"');
    if ( val_end ) *val_end = 0;
    else error("Parsing field value");
    strcpy(field, val);
    return val_end + 2;
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
    while( *p && *p!='}' ) {
	if ( *p==' ' || *p==',' ) {
	    p++;
	    continue;
	}
	if ( *p == '"' ) {
	    char *fname = p+1;
	    char *fn_end = strchr(fname,'"');
	    if ( fn_end ) *fn_end = 0;
	    else error("Parsing field name");
	    assert(fn_end[1]==':');
	    char *fval = fn_end+2;
	    if ( ! strcmp(fname,"fid") ) {
		p = parseString(newP->id, fval);
	    }
	}
    }
}

int parseFile(char *fname) {
   FILE *fp;
   char line[260];

   /* opening file for reading */
   fp = fopen(fname , "r");
   if (fp == NULL) {
      perror("Error opening file");
      return(-1);
   }
   while( fgets(line, 260, fp)!=NULL ) {
      /* writing content to stdout */
      person_t *p = parsePerson(line);
      if ( p ) printPerson(p);
   }
   fclose(fp);
}


int main(int argc, char **argv) {

    parseFile("out1.fs");

}

