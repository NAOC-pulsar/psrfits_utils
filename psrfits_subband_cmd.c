/*****
  command line parser -- generated by clig
  (http://wsd.iitb.fhg.de/~kir/clighome/)

  The command line parser `clig':
  (C) 1995-2004 Harald Kirsch (clig@geggus.net)
*****/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "psrfits_subband_cmd.h"

char *Program;

/*@-null*/

static Cmdline cmd = {
  /***** -dm: Dispersion measure to use for the subband de-dispersion */
  /* dmP = */ 1,
  /* dm = */ 0.0,
  /* dmC = */ 1,
  /***** -nsub: Number of output frequency subbands */
  /* nsubP = */ 0,
  /* nsub = */ (int)0,
  /* nsubC = */ 0,
  /***** -dstime: Power-of-2 number of samples to average in time */
  /* dstimeP = */ 1,
  /* dstime = */ 1,
  /* dstimeC = */ 1,
  /***** -startfile: Starting file number of sequence */
  /* startfileP = */ 1,
  /* startfile = */ 1,
  /* startfileC = */ 1,
  /***** -numfiles: Number of files to process */
  /* numfilesP = */ 0,
  /* numfiles = */ (int)0,
  /* numfilesC = */ 0,
  /***** -outbits: Number of output bits desired */
  /* outbitsP = */ 0,
  /* outbits = */ (int)0,
  /* outbitsC = */ 0,
  /***** -filetime: Desired length of the resulting files in sec */
  /* filetimeP = */ 0,
  /* filetime = */ (float)0,
  /* filetimeC = */ 0,
  /***** -filelen: Desired length of the resulting files in GB */
  /* filelenP = */ 0,
  /* filelen = */ (float)0,
  /* filelenC = */ 0,
  /***** -tgtstd: Target stdev. If 0, set in code based on outbits */
  /* tgtstdP = */ 1,
  /* tgtstd = */ 0.0,
  /* tgtstdC = */ 1,
  /***** -tgtavg: Target avg for UNSIGNED data. If 0, set in code based on outbits */
  /* tgtavgP = */ 1,
  /* tgtavg = */ 0.0,
  /* tgtavgC = */ 1,
  /***** -onlyI: Only output total intensity data */
  /* onlyIP = */ 0,
  /***** -adjustlevels: Adjust output scales and offsets every row (default is only row 0) */
  /* adjustlevelsP = */ 0,
  /***** -weights: Filename containing ASCII list of channels and weights to use */
  /* wgtsfileP = */ 0,
  /* wgtsfile = */ (char*)0,
  /* wgtsfileC = */ 0,
  /***** -bandpass: Filename containing ASCII list of channels, avgs, stdevs to use */
  /* bandpassfileP = */ 0,
  /* bandpassfile = */ (char*)0,
  /* bandpassfileC = */ 0,
  /***** -o: Basename for the output files */
  /* outputbasenameP = */ 0,
  /* outputbasename = */ (char*)0,
  /* outputbasenameC = */ 0,
  /***** uninterpreted rest of command line */
  /* argc = */ 0,
  /* argv = */ (char**)0,
  /***** the original command line concatenated */
  /* full_cmd_line = */ NULL
};

/*@=null*/

/***** let LCLint run more smoothly */
/*@-predboolothers*/
/*@-boolops*/


/******************************************************************/
/*****
 This is a bit tricky. We want to make a difference between overflow
 and underflow and we want to allow v==Inf or v==-Inf but not
 v>FLT_MAX. 

 We don't use fabs to avoid linkage with -lm.
*****/
static void
checkFloatConversion(double v, char *option, char *arg)
{
  char *err = NULL;

  if( (errno==ERANGE && v!=0.0) /* even double overflowed */
      || (v<HUGE_VAL && v>-HUGE_VAL && (v<0.0?-v:v)>(double)FLT_MAX) ) {
    err = "large";
  } else if( (errno==ERANGE && v==0.0) 
	     || (v!=0.0 && (v<0.0?-v:v)<(double)FLT_MIN) ) {
    err = "small";
  }
  if( err ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to %s to represent\n",
	    Program, arg, option, err);
    exit(EXIT_FAILURE);
  }
}

int
getIntOpt(int argc, char **argv, int i, int *value, int force)
{
  char *end;
  long v;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  v = strtol(argv[i], &end, 0);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace((int) *end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check if it fits into an int */
  if( errno==ERANGE || v>(long)INT_MAX || v<(long)INT_MIN ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to large to represent\n",
	    Program, argv[i], argv[i-1]);
    exit(EXIT_FAILURE);
  }
  *value = (int)v;

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr, 
	  "%s: missing or malformed integer value after option `%s'\n",
	  Program, argv[i-1]);
    exit(EXIT_FAILURE);
}
/**********************************************************************/

int
getIntOpts(int argc, char **argv, int i, 
	   int **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;
  long v;
  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
  alloced = cmin + 4;
  *values = (int*)calloc((size_t)alloced, sizeof(int));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (int *) realloc(*values, alloced*sizeof(int));
      if( !*values ) goto outMem;
    }

    errno = 0;
    v = strtol(argv[used+i+1], &end, 0);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace((int) *end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE || v>(long)INT_MAX || v<(long)INT_MIN ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to large to represent\n",
	      Program, argv[i+used+1], argv[i]);
      exit(EXIT_FAILURE);
    }

    (*values)[used] = (int)v;

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be an "
	    "integer value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getLongOpt(int argc, char **argv, int i, long *value, int force)
{
  char *end;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  *value = strtol(argv[i], &end, 0);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace((int) *end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  if( errno==ERANGE ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to large to represent\n",
	    Program, argv[i], argv[i-1]);
    exit(EXIT_FAILURE);
  }
  return i;

nothingFound:
  /***** !force means: this parameter may be missing.*/
  if( !force ) return i-1;

  fprintf(stderr, 
	  "%s: missing or malformed value after option `%s'\n",
	  Program, argv[i-1]);
    exit(EXIT_FAILURE);
}
/**********************************************************************/

int
getLongOpts(int argc, char **argv, int i, 
	    long **values,
	    int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values. It does not hurt to have room
    for a bit more values than cmax.
  *****/
  alloced = cmin + 4;
  *values = (long int *)calloc((size_t)alloced, sizeof(long));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (long int*) realloc(*values, alloced*sizeof(long));
      if( !*values ) goto outMem;
    }

    errno = 0;
    (*values)[used] = strtol(argv[used+i+1], &end, 0);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace((int) *end) ) end+=1; 
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to large to represent\n",
	      Program, argv[i+used+1], argv[i]);
      exit(EXIT_FAILURE);
    }

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be an "
	    "integer value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getFloatOpt(int argc, char **argv, int i, float *value, int force)
{
  char *end;
  double v;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  v = strtod(argv[i], &end);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace((int) *end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  checkFloatConversion(v, argv[i-1], argv[i]);

  *value = (float)v;

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr,
	  "%s: missing or malformed float value after option `%s'\n",
	  Program, argv[i-1]);
  exit(EXIT_FAILURE);
 
}
/**********************************************************************/

int
getFloatOpts(int argc, char **argv, int i, 
	   float **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;
  double v;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values.
  *****/
  alloced = cmin + 4;
  *values = (float*)calloc((size_t)alloced, sizeof(float));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (float *) realloc(*values, alloced*sizeof(float));
      if( !*values ) goto outMem;
    }

    errno = 0;
    v = strtod(argv[used+i+1], &end);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace((int) *end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    checkFloatConversion(v, argv[i], argv[i+used+1]);
    
    (*values)[used] = (float)v;
  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be a "
	    "floating-point value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

int
getDoubleOpt(int argc, char **argv, int i, double *value, int force)
{
  char *end;

  if( ++i>=argc ) goto nothingFound;

  errno = 0;
  *value = strtod(argv[i], &end);

  /***** check for conversion error */
  if( end==argv[i] ) goto nothingFound;

  /***** check for surplus non-whitespace */
  while( isspace((int) *end) ) end+=1;
  if( *end ) goto nothingFound;

  /***** check for overflow */
  if( errno==ERANGE ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of option `%s' to %s to represent\n",
	    Program, argv[i], argv[i-1],
	    (*value==0.0 ? "small" : "large"));
    exit(EXIT_FAILURE);
  }

  return i;

nothingFound:
  if( !force ) return i-1;

  fprintf(stderr,
	  "%s: missing or malformed value after option `%s'\n",
	  Program, argv[i-1]);
  exit(EXIT_FAILURE);
 
}
/**********************************************************************/

int
getDoubleOpts(int argc, char **argv, int i, 
	   double **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;
  char *end;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  /***** 
    alloc a bit more than cmin values.
  *****/
  alloced = cmin + 4;
  *values = (double*)calloc((size_t)alloced, sizeof(double));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory while parsing option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (double *) realloc(*values, alloced*sizeof(double));
      if( !*values ) goto outMem;
    }

    errno = 0;
    (*values)[used] = strtod(argv[used+i+1], &end);

    /***** check for conversion error */
    if( end==argv[used+i+1] ) break;

    /***** check for surplus non-whitespace */
    while( isspace((int) *end) ) end+=1;
    if( *end ) break;

    /***** check for overflow */
    if( errno==ERANGE ) {
      fprintf(stderr, 
	      "%s: parameter `%s' of option `%s' to %s to represent\n",
	      Program, argv[i+used+1], argv[i],
	      ((*values)[used]==0.0 ? "small" : "large"));
      exit(EXIT_FAILURE);
    }

  }
    
  if( used<cmin ) {
    fprintf(stderr, 
	    "%s: parameter `%s' of `%s' should be a "
	    "double value\n",
	    Program, argv[i+used+1], argv[i]);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

/**
  force will be set if we need at least one argument for the option.
*****/
int
getStringOpt(int argc, char **argv, int i, char **value, int force)
{
  i += 1;
  if( i>=argc ) {
    if( force ) {
      fprintf(stderr, "%s: missing string after option `%s'\n",
	      Program, argv[i-1]);
      exit(EXIT_FAILURE);
    } 
    return i-1;
  }
  
  if( !force && argv[i][0] == '-' ) return i-1;
  *value = argv[i];
  return i;
}
/**********************************************************************/

int
getStringOpts(int argc, char **argv, int i, 
	   char*  **values,
	   int cmin, int cmax)
/*****
  We want to find at least cmin values and at most cmax values.
  cmax==-1 then means infinitely many are allowed.
*****/
{
  int alloced, used;

  if( i+cmin >= argc ) {
    fprintf(stderr, 
	    "%s: option `%s' wants at least %d parameters\n",
	    Program, argv[i], cmin);
    exit(EXIT_FAILURE);
  }

  alloced = cmin + 4;
    
  *values = (char**)calloc((size_t)alloced, sizeof(char*));
  if( ! *values ) {
outMem:
    fprintf(stderr, 
	    "%s: out of memory during parsing of option `%s'\n",
	    Program, argv[i]);
    exit(EXIT_FAILURE);
  }

  for(used=0; (cmax==-1 || used<cmax) && used+i+1<argc; used++) {
    if( used==alloced ) {
      alloced += 8;
      *values = (char **)realloc(*values, alloced*sizeof(char*));
      if( !*values ) goto outMem;
    }

    if( used>=cmin && argv[used+i+1][0]=='-' ) break;
    (*values)[used] = argv[used+i+1];
  }
    
  if( used<cmin ) {
    fprintf(stderr, 
    "%s: less than %d parameters for option `%s', only %d found\n",
	    Program, cmin, argv[i], used);
    exit(EXIT_FAILURE);
  }

  return i+used;
}
/**********************************************************************/

void
checkIntLower(char *opt, int *values, int count, int max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%d\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkIntHigher(char *opt, int *values, int count, int min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%d\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkLongLower(char *opt, long *values, int count, long max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%ld\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkLongHigher(char *opt, long *values, int count, long min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%ld\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkFloatLower(char *opt, float *values, int count, float max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%f\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkFloatHigher(char *opt, float *values, int count, float min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%f\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkDoubleLower(char *opt, double *values, int count, double max)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]<=max ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' greater than max=%f\n",
	    Program, i+1, opt, max);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

void
checkDoubleHigher(char *opt, double *values, int count, double min)
{
  int i;

  for(i=0; i<count; i++) {
    if( values[i]>=min ) continue;
    fprintf(stderr, 
	    "%s: parameter %d of option `%s' smaller than min=%f\n",
	    Program, i+1, opt, min);
    exit(EXIT_FAILURE);
  }
}
/**********************************************************************/

static char *
catArgv(int argc, char **argv)
{
  int i;
  size_t l;
  char *s, *t;

  for(i=0, l=0; i<argc; i++) l += (1+strlen(argv[i]));
  s = (char *)malloc(l);
  if( !s ) {
    fprintf(stderr, "%s: out of memory\n", Program);
    exit(EXIT_FAILURE);
  }
  strcpy(s, argv[0]);
  t = s;
  for(i=1; i<argc; i++) {
    t = t+strlen(t);
    *t++ = ' ';
    strcpy(t, argv[i]);
  }
  return s;
}
/**********************************************************************/

void
usage(void)
{
  fprintf(stderr,"%s","   [-dm dm] [-nsub nsub] [-dstime dstime] [-startfile startfile] [-numfiles numfiles] [-outbits outbits] [-filetime filetime] [-filelen filelen] [-tgtstd tgtstd] [-tgtavg tgtavg] [-onlyI] [-adjustlevels] [-weights wgtsfile] [-bandpass bandpassfile] [-o outputbasename] [--] infile ...\n");
  fprintf(stderr,"%s","      \n");
  fprintf(stderr,"%s","      Partially de-disperse and subband PSRFITS search-mode data.\n");
  fprintf(stderr,"%s","      \n");
  fprintf(stderr,"%s","              -dm: Dispersion measure to use for the subband de-dispersion\n");
  fprintf(stderr,"%s","                   1 double value between 0.0 and 10000.0\n");
  fprintf(stderr,"%s","                   default: `0.0'\n");
  fprintf(stderr,"%s","            -nsub: Number of output frequency subbands\n");
  fprintf(stderr,"%s","                   1 int value between 1 and 4096\n");
  fprintf(stderr,"%s","          -dstime: Power-of-2 number of samples to average in time\n");
  fprintf(stderr,"%s","                   1 int value between 1 and 128\n");
  fprintf(stderr,"%s","                   default: `1'\n");
  fprintf(stderr,"%s","       -startfile: Starting file number of sequence\n");
  fprintf(stderr,"%s","                   1 int value between 1 and 2000\n");
  fprintf(stderr,"%s","                   default: `1'\n");
  fprintf(stderr,"%s","        -numfiles: Number of files to process\n");
  fprintf(stderr,"%s","                   1 int value between 1 and 2000\n");
  fprintf(stderr,"%s","         -outbits: Number of output bits desired\n");
  fprintf(stderr,"%s","                   1 int value between 2 and 8\n");
  fprintf(stderr,"%s","        -filetime: Desired length of the resulting files in sec\n");
  fprintf(stderr,"%s","                   1 float value between 0.0 and 100000.0\n");
  fprintf(stderr,"%s","         -filelen: Desired length of the resulting files in GB\n");
  fprintf(stderr,"%s","                   1 float value between 0.0 and 1000.0\n");
  fprintf(stderr,"%s","          -tgtstd: Target stdev. If 0, set in code based on outbits\n");
  fprintf(stderr,"%s","                   1 float value between 0.0 and 100000.0\n");
  fprintf(stderr,"%s","                   default: `0.0'\n");
  fprintf(stderr,"%s","          -tgtavg: Target avg for UNSIGNED data. If 0, set in code based on outbits\n");
  fprintf(stderr,"%s","                   1 float value between 0.0 and 100000.0\n");
  fprintf(stderr,"%s","                   default: `0.0'\n");
  fprintf(stderr,"%s","           -onlyI: Only output total intensity data\n");
  fprintf(stderr,"%s","    -adjustlevels: Adjust output scales and offsets every row (default is only row 0)\n");
  fprintf(stderr,"%s","         -weights: Filename containing ASCII list of channels and weights to use\n");
  fprintf(stderr,"%s","                   1 char* value\n");
  fprintf(stderr,"%s","        -bandpass: Filename containing ASCII list of channels, avgs, stdevs to use\n");
  fprintf(stderr,"%s","                   1 char* value\n");
  fprintf(stderr,"%s","               -o: Basename for the output files\n");
  fprintf(stderr,"%s","                   1 char* value\n");
  fprintf(stderr,"%s","           infile: Input file name(s) of the PSRFITs datafiles\n");
  fprintf(stderr,"%s","                   1...10000 values\n");
  fprintf(stderr,"%s","  version: 05Nov19\n");
  fprintf(stderr,"%s","  ");
  exit(EXIT_FAILURE);
}
/**********************************************************************/
Cmdline *
parseCmdline(int argc, char **argv)
{
  int i;

  Program = argv[0];
  cmd.full_cmd_line = catArgv(argc, argv);
  for(i=1, cmd.argc=1; i<argc; i++) {
    if( 0==strcmp("--", argv[i]) ) {
      while( ++i<argc ) argv[cmd.argc++] = argv[i];
      continue;
    }

    if( 0==strcmp("-dm", argv[i]) ) {
      int keep = i;
      cmd.dmP = 1;
      i = getDoubleOpt(argc, argv, i, &cmd.dm, 1);
      cmd.dmC = i-keep;
      checkDoubleLower("-dm", &cmd.dm, cmd.dmC, 10000.0);
      checkDoubleHigher("-dm", &cmd.dm, cmd.dmC, 0.0);
      continue;
    }

    if( 0==strcmp("-nsub", argv[i]) ) {
      int keep = i;
      cmd.nsubP = 1;
      i = getIntOpt(argc, argv, i, &cmd.nsub, 1);
      cmd.nsubC = i-keep;
      checkIntLower("-nsub", &cmd.nsub, cmd.nsubC, 4096);
      checkIntHigher("-nsub", &cmd.nsub, cmd.nsubC, 1);
      continue;
    }

    if( 0==strcmp("-dstime", argv[i]) ) {
      int keep = i;
      cmd.dstimeP = 1;
      i = getIntOpt(argc, argv, i, &cmd.dstime, 1);
      cmd.dstimeC = i-keep;
      checkIntLower("-dstime", &cmd.dstime, cmd.dstimeC, 128);
      checkIntHigher("-dstime", &cmd.dstime, cmd.dstimeC, 1);
      continue;
    }

    if( 0==strcmp("-startfile", argv[i]) ) {
      int keep = i;
      cmd.startfileP = 1;
      i = getIntOpt(argc, argv, i, &cmd.startfile, 1);
      cmd.startfileC = i-keep;
      checkIntLower("-startfile", &cmd.startfile, cmd.startfileC, 2000);
      checkIntHigher("-startfile", &cmd.startfile, cmd.startfileC, 1);
      continue;
    }

    if( 0==strcmp("-numfiles", argv[i]) ) {
      int keep = i;
      cmd.numfilesP = 1;
      i = getIntOpt(argc, argv, i, &cmd.numfiles, 1);
      cmd.numfilesC = i-keep;
      checkIntLower("-numfiles", &cmd.numfiles, cmd.numfilesC, 2000);
      checkIntHigher("-numfiles", &cmd.numfiles, cmd.numfilesC, 1);
      continue;
    }

    if( 0==strcmp("-outbits", argv[i]) ) {
      int keep = i;
      cmd.outbitsP = 1;
      i = getIntOpt(argc, argv, i, &cmd.outbits, 1);
      cmd.outbitsC = i-keep;
      checkIntLower("-outbits", &cmd.outbits, cmd.outbitsC, 8);
      checkIntHigher("-outbits", &cmd.outbits, cmd.outbitsC, 2);
      continue;
    }

    if( 0==strcmp("-filetime", argv[i]) ) {
      int keep = i;
      cmd.filetimeP = 1;
      i = getFloatOpt(argc, argv, i, &cmd.filetime, 1);
      cmd.filetimeC = i-keep;
      checkFloatLower("-filetime", &cmd.filetime, cmd.filetimeC, 100000.0);
      checkFloatHigher("-filetime", &cmd.filetime, cmd.filetimeC, 0.0);
      continue;
    }

    if( 0==strcmp("-filelen", argv[i]) ) {
      int keep = i;
      cmd.filelenP = 1;
      i = getFloatOpt(argc, argv, i, &cmd.filelen, 1);
      cmd.filelenC = i-keep;
      checkFloatLower("-filelen", &cmd.filelen, cmd.filelenC, 1000.0);
      checkFloatHigher("-filelen", &cmd.filelen, cmd.filelenC, 0.0);
      continue;
    }

    if( 0==strcmp("-tgtstd", argv[i]) ) {
      int keep = i;
      cmd.tgtstdP = 1;
      i = getFloatOpt(argc, argv, i, &cmd.tgtstd, 1);
      cmd.tgtstdC = i-keep;
      checkFloatLower("-tgtstd", &cmd.tgtstd, cmd.tgtstdC, 100000.0);
      checkFloatHigher("-tgtstd", &cmd.tgtstd, cmd.tgtstdC, 0.0);
      continue;
    }

    if( 0==strcmp("-tgtavg", argv[i]) ) {
      int keep = i;
      cmd.tgtavgP = 1;
      i = getFloatOpt(argc, argv, i, &cmd.tgtavg, 1);
      cmd.tgtavgC = i-keep;
      checkFloatLower("-tgtavg", &cmd.tgtavg, cmd.tgtavgC, 100000.0);
      checkFloatHigher("-tgtavg", &cmd.tgtavg, cmd.tgtavgC, 0.0);
      continue;
    }

    if( 0==strcmp("-onlyI", argv[i]) ) {
      cmd.onlyIP = 1;
      continue;
    }

    if( 0==strcmp("-adjustlevels", argv[i]) ) {
      cmd.adjustlevelsP = 1;
      continue;
    }

    if( 0==strcmp("-weights", argv[i]) ) {
      int keep = i;
      cmd.wgtsfileP = 1;
      i = getStringOpt(argc, argv, i, &cmd.wgtsfile, 1);
      cmd.wgtsfileC = i-keep;
      continue;
    }

    if( 0==strcmp("-bandpass", argv[i]) ) {
      int keep = i;
      cmd.bandpassfileP = 1;
      i = getStringOpt(argc, argv, i, &cmd.bandpassfile, 1);
      cmd.bandpassfileC = i-keep;
      continue;
    }

    if( 0==strcmp("-o", argv[i]) ) {
      int keep = i;
      cmd.outputbasenameP = 1;
      i = getStringOpt(argc, argv, i, &cmd.outputbasename, 1);
      cmd.outputbasenameC = i-keep;
      continue;
    }

    if( argv[i][0]=='-' ) {
      fprintf(stderr, "\n%s: unknown option `%s'\n\n",
              Program, argv[i]);
      usage();
    }
    argv[cmd.argc++] = argv[i];
  }/* for i */


  /*@-mustfree*/
  cmd.argv = argv+1;
  /*@=mustfree*/
  cmd.argc -= 1;

  if( 1>cmd.argc ) {
    fprintf(stderr, "%s: there should be at least 1 non-option argument(s)\n",
            Program);
    exit(EXIT_FAILURE);
  }
  if( 10000<cmd.argc ) {
    fprintf(stderr, "%s: there should be at most 10000 non-option argument(s)\n",
            Program);
    exit(EXIT_FAILURE);
  }
  /*@-compmempass*/  return &cmd;
}

