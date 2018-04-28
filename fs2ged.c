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
typedef char *date_t;
typedef char *place_t;

typedef struct {
    pId_t	id;
    //pId_t	father;
    //pId_t	mother;
    pId_t	parents[2];
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

    date_t	deadDate;
    place_t	deadPlace;
} person_t;
    
char *fieldTable[][2] = {
    { "given", "NAME" },
    { "surname", "NAME" },
    { "gender", "SEX" },
    { "birtdate", "" }
};

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
    if ( val )
        printf("%s: '%s'\n", fname, val);
}

void printPerson(person_t *p) {
    printString("id", p->id);
    printString("given", p->givenName);
    printString("last", p->surName);
    printString("birth date", p->birthDate);
    printString("birth place", p->birthPlace);
    printString("batism date", p->baptDate);
    printString("batism place", p->baptPlace);
    printString("dead date", p->deadDate);
    printString("dead place", p->deadPlace);
}

void setPersonFieldString(char **fname, char *fval) {
    *fname = strdup(fval);
}

void setPersonFieldPid(pId_t *fname, char *fval) {
    strncpy(*fname,fval,8);
}

void setPersonFieldCouple(pId_t *fname, char *fval) {
    strncpy(*fname,fval,8);
}

#define setPersonField(name,fld,type) \
    if ( ! strcmp(fname,name) ) \
	setPersonField##type(&(newP->fld),fval);

char *parsePersonField(person_t *newP, char *fname, char *p) {
    char *fval = p;
    if ( *p=='"' || *p=='[' ) fval += 1;
    char *fend = parseField(p);
    printf("fname:'%s' fval:'%s'\n",fname,fval);

    setPersonField("fid",	id,		Pid);
    setPersonField("given",	givenName,	String);
    setPersonField("surname",	surName,	String);
 
    setPersonField("birtdate",  birthDate, 	String);
    setPersonField("birtplac",  birthPlace, 	String);
    setPersonField("chrdate",   baptDate, 	String);
    setPersonField("chrplac",   baptPlace, 	String);
    setPersonField("deatdate",	deadDate,	String);
    setPersonField("deatplac",	deadPlace,	String);

    //setPersonField("ch",id,String);

 

   // date_t	baptDate;
    
    return fend;
}

person_t *parsePerson(char *line) {
    person_t *newP;
    newP = malloc(sizeof(person_t));
    if ( ! newP ) return newP;

    char *p = line;
    if ( *p=='{' ) p++;
    char *fname = NULL;
    //char *fval = NULL;
    while( *p && *p!='}' && *p!='\n' ) {
	if ( *p==' ' || *p==',' ) {
	    p++;
	    continue;
	}
        fname = p+1;
	p = parseString(p);
	assert( *p==':' );
	p += 2;
        p = parsePersonField(newP,fname,p);
/*
	if ( *p=='"' || *p=='[' ) fval = p+1;
	else fval = p;
	p = parseField(p);
        printf("fname:'%s' fval:'%s'\n",fname,fval);
*/
    }
    return newP;
}

void printHeader(FILE *fout) {
   fprintf(fout, "0 HEAD\n");
   fprintf(fout, "1 SOUR fs2ged\n");
   fprintf(fout, "2 NAME Converted from FamilySearch exported records\n");
   fprintf(fout, "2 VERS 0.1\n");
   fprintf(fout, "1 DEST FILE\n");
   fprintf(fout, "1 DATE 25 MAR 2018\n");
   fprintf(fout, "1 GEDC\n");
   fprintf(fout, "2 VERS 5.5\n");
   fprintf(fout, "2 FORM LINEAGE-LINKED\n");
   fprintf(fout, "1 CHAR UTF-8\n");
   fprintf(fout, "1 PLAC\n");
   fprintf(fout, "2 FORM Lugar, Freguesia, Conselho, District/Province, Country\n");
   fprintf(fout, "1 SUBM @SUBM@\n");
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
   FILE *fout = stdout;
   printHeader(fout);
   int n=0;  // number of individuals
   while( fgets(line, MAX_LINE_LEN, fp)!=NULL ) {
      /* writing content to stdout */
      fprintf(fout,"@I%d@\n",n+1);
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

