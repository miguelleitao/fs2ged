/*
 * fs2ged.c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>

#define MAX_LINE_LEN (860)
#define MAX_PID_SIZE (10)

typedef char pId_t[MAX_PID_SIZE+2];
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
    

// not used
char *fieldTable[][2] = {
    { "given", "NAME" },
    { "surname", "NAME" },
    { "gender", "SEX" },
    { "birtdate", "" }
};

FILE *fout = NULL;

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
    if (p->givenName) 	printString("given", p->givenName);
    if (p->surName) 	printString("last", p->surName);
    if (p->birthDate)	printString("birth date", p->birthDate);
    if (p->birthPlace) 	printString("birth place", p->birthPlace);
    if (p->baptDate)	printString("batism date", p->baptDate);
    if (p->baptPlace) 	printString("batism place", p->baptPlace);
    if (p->deadDate)	printString("dead date", p->deadDate);
    if (p->deadPlace) 	printString("dead place", p->deadPlace);
    printString("father", p->parents[0]);
    printString("mother", p->parents[1]);
}

void setPersonFieldString(char **fname, char *fval) {
    *fname = strdup(fval);
}

void setPersonFieldPid(pId_t *fname, char *fval) {
    strncpy(*fname,fval,MAX_PID_SIZE);
    assert( strlen(*fname)>7 );
}

void setPersonFieldCouple(pId_t (*fname)[2], char *fval) {
    if ( ! strcmp(fval,"null") ) {
	strcpy( (*fname)[0], "" );
	strcpy( (*fname)[1], "" );
	return;
    }
    char *pend = parseString(fval);
//printf("**father1:'%s'\n", fval+1);
    strncpy((*fname)[0], fval+1, MAX_PID_SIZE);
//printf("**father:'%s'\n", *fname[0]);
    fval = pend+2;
//printf("**mother:'%s'\n", fval);
    parseString(fval);
    strncpy((*fname)[1], fval+1, MAX_PID_SIZE);
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

    setPersonField("parents",	parents,	Couple);

    //setPersonField("ch",id,String);

 

   // date_t	baptDate;
    
    return fend;
}

void freePerson(person_t *p) {
    if ( ! p ) return;
    if ( p->givenName ) free( p->givenName );
    if ( p->surName ) free( p->surName );
    if ( p->birthDate ) free( p->birthDate );
    if ( p->birthPlace ) free( p->birthPlace );
    if ( p->baptDate ) free( p->baptDate );
    if ( p->baptPlace ) free( p->baptPlace );
    if ( p->deadDate ) free( p->deadDate );
    if ( p->deadPlace ) free( p->deadPlace );
    free(p);
}

person_t *newPerson() {
    person_t *newP;
    newP = malloc(sizeof(person_t));
    if ( ! newP ) return newP;

    strcpy(newP->id, "");
    strcpy(newP->parents[0], "");
    strcpy(newP->parents[1], "");

    newP->birthDate = NULL;
    newP->birthPlace = NULL;
    newP->baptDate = NULL;
    newP->baptPlace = NULL;
    newP->deadDate = NULL;
    newP->deadPlace = NULL;
    return newP;
}

person_t *parsePerson(char *line) {
    person_t *newP;
    newP = newPerson();
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
   printHeader(fout);
   int n=0;  // number of individuals
   while( fgets(line, MAX_LINE_LEN, fp)!=NULL ) {
      /* writing content to stdout */
      fprintf(fout,"@I%d@\n",n+1);
      person_t *p = parsePerson(line);
      if ( p ) {
	printPerson(p);
      	freePerson(p);
      }
      n++;
   }
   fclose(fp);
   return 0;
}


int main(int argc, char **argv) {
    char *fname = NULL;
    fout = stdout;
    if ( argc>1 ) fname = argv[1];
    if ( argc>2 ) {
	fout = fopen(argv[2],"w");
	if ( ! fout )
	    error("Opening output file");
    }
    parseFile(fname);
    fclose(fout);
    exit(0);
}

