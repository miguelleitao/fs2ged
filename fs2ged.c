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
    unsigned int num;
    pId_t	id;
    //pId_t	father;
    //pId_t	mother;
    pId_t	parents[2];

    unsigned int familyNum;
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
    
typedef struct {
    unsigned int num;
    pId_t	id;

    unsigned int husbandNum;
    unsigned int wifeNum;

    pId_t	husbandId;
    pId_t	wifeId;

    date_t	marrDate;
    place_t	marrPlace;
    
    short int 	 children;
    unsigned int *child;
} family_t;

// not used
char *fieldTable[][2] = {
    { "given", "NAME" },
    { "surname", "NAME" },
    { "gender", "SEX" },
    { "birtdate", "" }
};


unsigned int nPersons = 0;
person_t **Person = NULL;

unsigned int nFamilies = 0;
family_t **Family = NULL;

FILE *fout = NULL;

int error(char *msg) {
    fprintf (stderr,"Error %s\n",msg);
    exit(1);
}

int isFamily(char *line) {
    if ( strstr(line,"\"gender\": ") ) return 0;
    return 1;
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

void outputTag(char *fname) {
    fprintf(fout, "%s", fname);
}

void outputString(char *fname, char *val) {
    if ( val )
        printf("%s %s\n", fname, val);
}

void outputPersonNum(char *fname, unsigned int val) {
     fprintf(fout,"%s @I%u@\n", fname, val);
}

void outputPerson(person_t *p) {
    static int personNum = 1;
    fprintf(fout,"0 @I%d@ INDI\n",personNum);
    personNum += 1;
    outputTag("1 NAME ");
    if (p->givenName) {
	outputTag(p->givenName);
	outputTag(" ");
    }
    if (p->surName) 	outputTag(p->surName);    
    outputTag("\n");
    if (p->givenName) 	outputString("2 GIVN", p->givenName);
    if (p->surName) 	outputString("2 SURN", p->surName);

    char sex[4];
    sex[0] = p->gender;
    sex[1] = 0;
    outputString("1 SEX", sex);
    
    outputTag("1 BIRT\n");
    if (p->birthDate)	outputString("2 DATE", p->birthDate);
    if (p->birthPlace) 	outputString("2 PLAC", p->birthPlace);
   
    outputTag("1 CHAN\n");
    if (p->birthDate)	outputString("2 DATE", p->baptDate);
    if (p->birthPlace) 	outputString("2 PLAC", p->baptPlace);
   
    outputTag("1 DEAD\n");
    if (p->deadDate)	outputString("2 DATE", p->deadDate);
    if (p->deadPlace) 	outputString("2 PLAC", p->deadPlace);

    outputString("father", p->parents[0]);
    outputString("mother", p->parents[1]);
}

void outputFamily(family_t *f) {
    static int familyNum = 1;
    fprintf(fout,"0 @F%d@ FAM\n",familyNum);
    familyNum += 1;

    outputPersonNum("1 HUSB", f->husbandNum);
    outputPersonNum("1 WIFE", f->wifeNum);
    // 1 MARR
    for( int i=0 ; i<f->children ; i++ ) 
	outputPersonNum("1 CHIL", f->child[i]);
    outputTag("0 TRLR\n");

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

void setPersonFieldInt(unsigned int *fld, char *fval) {
    *fld = atoi(fval);
}

void setPersonFieldListLen(short int *fld, char *fval) {
    if ( strlen(fval)==0 ) {
	*fld = 0;
	return;
    }
    *fld = 1;
    while( *fval ) {
	if ( *fval==',' ) (*fld)++;
	fval++;
    }
}

void setPersonFieldIntList(unsigned int **fld, char *fval) {
    int i=0;
    while( *fval ) {
        if ( *fval==',' || *fval==' ' ) {
	    fval++;
	    continue;
	}
        (*fld)[i] = atoi(fval);
	i++;
        while( *fval && *fval!=',' ) fval++;
    }
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

char *parseFamilyField(family_t *newP, char *fname, char *p) {
    char *fval = p;
    if ( *p=='"' || *p=='[' ) fval += 1;
    char *fend = parseField(p);
    printf("fname:'%s' fval:'%s'\n",fname,fval);

    setPersonField("fid",	id,		Pid);

    setPersonField("husb_fid",	husbandId,	Pid);
    setPersonField("wife_fid",	wifeId,	Pid);

    setPersonField("husb_num",	husbandNum,	Int);
    setPersonField("wife_num",	wifeNum,	Int);
 
    setPersonField("marrdate",  marrDate, 	String);
    setPersonField("marrplac",  marrPlace, 	String);

    setPersonField("chil_num",  children,	ListLen);
    newP->child = malloc(newP->children*sizeof(unsigned int));
    setPersonField("chil_num",  child, 		IntList);
    // "chil_num": [9, 10]
    // "chil_fid": ["LTDY-WFF", "LTFH-MQS"]
   
    return fend;
}

void linkPersonsToFamilies() {
    for( unsigned int p=0 ; p<nPersons ; p++ ) {
	person_t *pi = Person[p];
	if ( ( ! pi->parents[0][0] ) && ( ! pi->parents[1][0] ) )
	    continue;
	for( unsigned int f=0 ; f<nFamilies ; f++ ) {
	    family_t *fi = Family[f];
	    if ( ! strcmp(pi->parents[0],fi->husbandId) &&
		 ! strcmp(pi->parents[1],fi->wifeId) ) {
			pi->familyNum = f;
			break;
	    }
	}
    }
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

void freeFamily(family_t *p) {
    if ( ! p ) return;
    if ( p->marrDate ) free( p->marrDate );
    if ( p->marrPlace ) free( p->marrPlace );

    if ( p->child ) {
        free( p->child );
    }
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

family_t *newFamily() {
    family_t *newP;
    newP = malloc(sizeof(family_t));
    if ( ! newP ) return newP;

    strcpy(newP->id, "");
    strcpy(newP->husbandId, "");
    strcpy(newP->wifeId, "");

    newP->husbandNum = 0;
    newP->wifeNum = 0;

    newP->marrDate = NULL;
    newP->marrPlace = NULL;

    newP->children = 0;
    newP->child = NULL;
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

family_t *parseFamily(char *line) {
    family_t *newF;
    newF = newFamily();
    if ( ! newF ) return newF;
   
    char *p = line;
    if ( *p=='{' ) p++;
    char *fname = NULL;
    while( *p && *p!='}' && *p!='\n' ) {
	if ( *p==' ' || *p==',' ) {
	    p++;
	    continue;
	}
        fname = p+1;
	p = parseString(p);
	assert( *p==':' );
	p += 2;
        p = parseFamilyField(newF,fname,p);
    }
    return newF;
}

void printHeader(FILE *fout) {
   fprintf(fout, "0 HEAD\n");
   fprintf(fout, "1 SOUR fs2ged\n");
   fprintf(fout, "2 NAME Converted from FamilySearch exported records\n");
   fprintf(fout, "2 VERS 0.1\n");
   fprintf(fout, "1 DEST FILE\n");
   fprintf(fout, "1 DATE 25 MAR 2018\n");
   fprintf(fout, "1 GEDC\n");
   fprintf(fout, "2 VERS 5.5.1\n");
   fprintf(fout, "2 FORM LINEAGE-LINKED\n");
   fprintf(fout, "1 CHAR UTF-8\n");
   fprintf(fout, "1 PLAC\n");
   fprintf(fout, "2 FORM Lugar, Freguesia, Conselho, District/Province, Country\n");
   fprintf(fout, "1 SUBM @SUBM@\n");
}


int fileNumLines(char *fname) {
    char buffer[1024] = "";
    FILE *fin;
    fin = fopen(fname, "r");
    int bytes = 0;
    if ( fin == NULL ) {
        perror(fname);
        return 0;
    }
    char lastchar = 0;
    int lines = 0;
    while ((bytes = fread(buffer, 1, sizeof(buffer) - 1, fin))) {
	lastchar = buffer[bytes - 1];
        for (char *c = buffer; (c = memchr(c, '\n', bytes - (c - buffer))); c++) {
            lines++;
        }
    }
    fclose(fin);
    if (lastchar != '\n') {
        lines++;  /* Count the last line even if it lacks a newline */
    }
    return lines;
}

int parseFile(char *fname) {

   unsigned int maxPersons = fileNumLines(fname);
   Person = malloc(maxPersons*sizeof(person_t *));
   Family = malloc(maxPersons*sizeof(family_t *));
   nPersons = 0;
   nFamilies = 0;
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

   //printHeader(fout);
   int n=0;  // number of lines;
   while( fgets(line, MAX_LINE_LEN, fp)!=NULL ) {
      n += 1;
      if ( isFamily(line) ) {
	family_t *f = parseFamily(line);
	if ( f ) {
	    //outputFamily(f);
	    //freeFamily(f);
	    Family[nFamilies] = f;
	    nFamilies += 1;
	    assert(nFamilies<=maxPersons);
	}
	continue;
      }
      // Person
      person_t *p = parsePerson(line);
      /* writing content to stdout */
      if ( p ) {
	//printPerson(p);
	//outputPerson(p);
      	//freePerson(p);
	Person[nPersons] = p;
	nPersons += 1;
	assert(nPersons<=maxPersons);
      }
   }
   //outputTag("0 TRLR\n");
   fclose(fp);
   return 0;
}

void outputGedcom() {
   printHeader(fout);
   for( unsigned int i=0 ; i<nPersons ; i++ )
	outputPerson(Person[i]);
   for( unsigned int i=0 ; i<nFamilies ; i++ )
	outputFamily(Family[i]);
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
    linkPersonsToFamilies();
    outputGedcom();
    fclose(fout);
    for( unsigned int i=0 ; i<nPersons ; i++ )
	freePerson(Person[i]);
    free(Person);

    for( unsigned int i=0 ; i<nFamilies ; i++ )
	freeFamily(Family[i]);
    free(Family);
    exit(0);
}

