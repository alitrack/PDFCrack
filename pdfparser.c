/**
 * Copyright (C) 2006-2020 Henning Norén
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */

#include "pdfparser.h"
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 256

/** Please rewrite all of this file in a clean and stable way */

struct p_str {
  uint8_t *content;
  uint8_t len;
};

static int o_len, u_len;

typedef struct p_str p_str;

__attribute__ ((pure)) static bool
isWhiteSpace(const int ch) {
  return (ch == 0x20 || (ch >= 0x09 && ch <= 0x0d) || ch == 0x00);
}

__attribute__ ((pure)) static bool
isDelimiter(const int ch) {
  switch(ch) {
  case '(':
  case ')':
  case '<':
  case '>':
  case '[':
  case ']':
  case '{':
  case '}':
  case '/':
  case '%':
    return true;
  default:
    return false;
  }
}

__attribute__ ((pure)) static inline bool
isEndOfLine(const int ch) {
  return (ch == 0x0a || ch == 0x0d);
}

static int
parseIntWithC(FILE *file, const int c) {
  bool neg = false;
  int i = 0;
  int ch = c;

  if(ch == '-') {
    neg = true;
    ch = getc(file);
  }
  else if(ch == '+')
    ch = getc(file);
  while(ch >= '0' && ch <= '9') {
    i *= 10;
    i += ch-'0';
    ch = getc(file);
  }
  ungetc(ch,file);
  if(neg)
    i *= -1;

  return i;
}

static int
parseInt(FILE *file) {
  return parseIntWithC(file, getc(file));
}


static char
parseWhiteSpace(FILE *file) {
  int ch;
  do {
    ch = getc(file);
  } while(isWhiteSpace(ch));
  return ch;
}

static char
parseWhiteSpaceOrComment(FILE *file) {
  int ch;
  do {
    do {
      ch = getc(file);
    } while(isWhiteSpace(ch));
    if(ch == '%') {
      do {
	ch = getc(file);
      } while(!(ch >= 0x09 && ch <= 0x0d) && ch != EOF);
      ch = getc(file);
    }
  } while(isWhiteSpace(ch) || ch == '%');
  return ch;
}

static char*
parseName(FILE *file) {
  int ch;
  unsigned int i;
  char *ret;
  char buff[BUFFSIZE];

  ch = parseWhiteSpace(file);

  if(ch != '/') {
    ungetc(ch, file);
    return NULL;
  }
  ch = getc(file);
  for(i=0; i<BUFFSIZE-1 && !isWhiteSpace(ch) && 
	!isDelimiter(ch) && ch != EOF; ++i) {
    buff[i] = ch;
    ch = getc(file);
  }
  ungetc(ch, file);
  buff[i++] = '\0';
  ret = malloc(sizeof(char)*i);
  memcpy(ret, buff, i);
  return ret;
}

/**
static bool
isName(FILE *file, const char *str) {
  int ch;
  unsigned int i;

  ch = parseWhiteSpace(file);

  if(ch != '/') {
    ungetc(ch, file);
    return false;
  }
  for(i=0; i<strlen(str); ++i) {
    ch = getc(file);
    if(ch != str[i])
      return false;
  }
  return true;
}
*/

static bool
isWord(FILE *file, const char *str) {
  int ch;
  unsigned int i;
  for(i=0; i<strlen(str); ++i)
    if((ch = getc(file)) != str[i])
      goto ret;
  return true;
 ret:
  ungetc(ch,file);
  return false;
}

bool
openPDF(FILE *file, EncData *e) {
  bool ret = false;
  int minor_v = 0, major_v = 0;
  if(getc(file) == '%' && getc(file) == 'P' && getc(file) == 'D' 
     && getc(file) == 'F' && getc(file) == '-') {
    major_v = parseInt(file);
    if(getc(file) == '.')
      minor_v = parseInt(file);
    if(major_v >= 0 && minor_v >=0)
      ret = true;
  }

  if(ret) {
    e->version_major = (unsigned)major_v;
    e->version_minor = (unsigned)minor_v;
  } 
  return ret;
}

__attribute__ ((pure)) static uint8_t
hexToInt(const int b) {
  if(b >= '0' && b <= '9')
    return b-'0';
  else if(b >= 'a' && b <= 'f')
    return b-'a'+10;
  else if(b >= 'A' && b <= 'F')
    return b-'A'+10;
  else
    return 0;
}

static p_str*
parseHexString(const uint8_t *buf, const unsigned int len) {
  unsigned int i,j;
  p_str *ret;

  ret = malloc(sizeof(p_str));
  ret->content = malloc(sizeof(uint8_t)*(len/2));
  ret->len = (len/2);

  for(i=0, j=0; i<len; i += 2) {
    ret->content[j] = hexToInt(buf[i]) * 16;
    ret->content[j] += hexToInt(buf[i+1]);
    j++;
  }

  return ret;
}

static p_str*
objStringToByte(const uint8_t* str, const unsigned int len) {
  unsigned int i, j, l;
  uint8_t b, d;
  uint8_t tmp[len];
  p_str *ret;

  for(i=0, l=0; i<len; i++, l++) {
    b = str[i];
    if(b == '\\') {
      /** 
       * We have reached a special character or the beginning of a octal 
       * up to three digit number and should skip the initial backslash
       **/
      i++;
      switch(str[i]) {
      case 'n':
	b = 0x0a;
	break;
      case 'r':
	b = 0x0d;
	break;
      case 't':
	b = 0x09;
	break;
      case 'b':
	b = 0x08;
	break;
      case 'f':
	b = 0x0c;
	break;
      case '(':
	b = '(';
	break;
      case ')':
	b = ')';
	break;
      case '\\':
	b = '\\';
	break;
      default:
	if(str[i] >= '0' && str[i] < '8') {
	  d = 0;
	  for(j=0; i < len && j < 3 &&
		str[i] >= '0' && str[i] < '8' &&
		(d*8)+(str[i]-'0') < 256; j++, i++) {
	    d *= 8;
	    d += (str[i]-'0');
	  }
	  /** 
	   * We need to step back one step if we reached the end of string
	   * or the end of digits (like for example \0000)
	   **/
	  if(i < len || j < 3) {
	    i--;
	  }

	  b = d;
	}
      }
    }
    tmp[l] = b;
  }

  ret = malloc(sizeof(p_str));
  ret->content = malloc(sizeof(uint8_t)*(l));
  ret->len = l-1;

  memcpy(ret->content, tmp, l);

  return ret;
}

static p_str*
parseRegularString(FILE *file) {
  unsigned int len, p;
  int ch;
  p_str *ret = NULL;
  uint8_t buf[BUFFSIZE];
  bool skip = false;

  ch = parseWhiteSpace(file);
  if(ch == '(') {
    p = 1;
    ch = getc(file);
    for(len=0; len < BUFFSIZE && p > 0 && ch != EOF; len++) {
      buf[len] = ch;
      if(skip == false) {
	if(ch == '(')
	  p++;
	else if(ch == ')')
	  p--;
	if(ch == '\\')
	  skip = true;
      }
      else
	skip = false;
      ch = getc(file);
    }
    ungetc(ch, file);
    if(len > 0)
      ret = objStringToByte(buf, len);
  }
  else if(ch == '<') {
    len = 0;
    while(ch != '>' && len < BUFFSIZE && ch != EOF) {
      if((ch >= '0' && ch <= '9') || 
	 (ch >= 'a' && ch <= 'f') || 
	 (ch >= 'A' && ch <= 'F')) {
	buf[len++] = ch;
      }
      ch = getc(file);
    }
    ungetc(ch,file);
    if((len > 1) && ((len%2) == 0))
      ret = parseHexString(buf,len);
  }
  return ret;
}

static int
findTrailerDict(FILE *file, EncData *e) {
  int ch;
  /**  int pos_i; */
  bool encrypt = false;
  bool id = false;
  int e_pos = -1;
  p_str *str = NULL;
  int dict = 0;
  
  ch = getc(file);
  while(ch != EOF) {
    if(isEndOfLine(ch)) {
      ch = parseWhiteSpace(file);
      if(ch == '<' && getc(file) == '<') {
	/** This should be the first dict we stumble upon*/

	/**
	   pos_i = ftell(file);
	   printf("found Trailer at pos %x\n", pos_i);
	*/
	ch = getc(file);
	while(ch != EOF) {
	  if(ch == '<') {
	    ch = getc(file);
	    if(ch == '<') {
	      dict++;
	    }
	  }
	  if(ch == '>') {
	    ch = getc(file);
	    if(ch == '>') {
	      if(dict == 0)
		break;
	    }
	    dict--;
	  }
	  while(ch != '/' && ch != EOF) {
	    ch = getc(file);
	  }
	  ch = getc(file);
	  /**printf("found a name: %c\n", ch);*/
	  if(e_pos < 0 && ch == 'E' && isWord(file, "ncrypt")) {
	    ch = getc(file);
	    if(!isWhiteSpace(ch))
	      continue;
	    e_pos = parseIntWithC(file,parseWhiteSpace(file));
	    if(e_pos >= 0) {
	      /**
		 pos_i = ftell(file);
		 printf("found Encrypt at pos %x, ", pos_i);
		 printf("%d\n", e_pos);
	      */
	      encrypt = true;
	    }
	  }
	  else if(ch == 'I' && getc(file) == 'D') {
	    ch = parseWhiteSpace(file);
	    while(ch != '[' && ch != EOF)
	      ch = getc(file);

	    if(str) {
	      free(str->content);
	      free(str);
	    }
	      
	    str = parseRegularString(file);
	    /**
	       pos_i = ftell(file);
	       printf("found ID at pos %x\n", pos_i);
	    */
	    if(str)
	      id = true;
	    else
	      id = false;

	    ch = getc(file);
	  }
	  else
	    ch = getc(file);
	  if(encrypt && id) {
	    /**printf("found all, returning: epos: %d\n",e_pos);*/
	    e->fileID = str->content;
	    e->fileIDLen = str->len;
	    free(str);
	    return e_pos;
	  }
	}
      }  
      else {
	ch = getc(file);
      }     
    }
    else
      ch = getc(file);
  }
  /**  printf("finished searching\n");*/
  
  if(str) {
    free(str->content);
    free(str);
  }
  
  if(!encrypt && id)
    return ETRENF;
  else if(!id && encrypt) {
    /** We found a encrypt object, but not an ID. Let us try parsing the 
	object as some security handlers seems to encrypt/skip the ID.
	Logic changed to show these handles correctly instead of printing out
	a error that the document is not encrypted.
    **/
    return e_pos;
  }
  else 
    return ETRANF;
}


static int
findTrailer(FILE *file, EncData *e) {
  int ch;
  /**  int pos_i; */
  bool encrypt = false;
  bool id = false;
  int e_pos = -1;
  p_str *str = NULL;
  
  ch = getc(file);
  while(ch != EOF) {
    if(isEndOfLine(ch)) {
      if(isWord(file, "trailer")) {
	/**	printf("found trailer\n");*/
	ch = parseWhiteSpace(file);
	if(ch == '<' && getc(file) == '<') {
	  /** we can be pretty sure to have found the trailer.
	      start looking for the Encrypt-entry */

	  /**
	  pos_i = ftell(file);
	  printf("found Trailer at pos %x\n", pos_i);
	  */
	  ch = getc(file);
	  while(ch != EOF) {
	    if(ch == '>') {
	      ch = getc(file);
	      if(ch == '>')
		break;
	    }
	    while(ch != '/' && ch != EOF) {
	      ch = getc(file);
	    }
	    ch = getc(file);
	    /**printf("found a name: %c\n", ch);*/
	    if(e_pos < 0 && ch == 'E' && isWord(file, "ncrypt")) {
	      ch = getc(file);
	      if(!isWhiteSpace(ch))
		continue;
	      e_pos = parseIntWithC(file,parseWhiteSpace(file));
	      if(e_pos >= 0) {
		/**
		   pos_i = ftell(file);
		   printf("found Encrypt at pos %x, ", pos_i);
		   printf("%d\n", e_pos);
		*/
		encrypt = true;
	      }
	    }
	    else if(!id && ch == 'I' && getc(file) == 'D') {
	      ch = parseWhiteSpace(file);
	      while(ch != '[' && ch != EOF)
		ch = getc(file);

	      if(str) {
		if(str->content)
		  free(str->content);
		free(str);
	      }
	      
	      str = parseRegularString(file);
	      /**
		 pos_i = ftell(file);
		 printf("found ID at pos %x\n", pos_i);
	      */
	      if(str) 
		id = true;
	      ch = getc(file);
	    }
	    else
	      ch = getc(file);
	    if(encrypt && id) {
	      /**printf("found all, returning: epos: %d\n",e_pos);*/
	      e->fileID = str->content;
	      e->fileIDLen = str->len;
	      free(str);
	      return e_pos;
	    }
	  }
	}  
      }
      else {
	ch = getc(file);
      }
    }
    else
      ch = getc(file);
  }
  /**  printf("finished searching\n");*/

  if(str) {
    free(str->content);
    free(str);
  }

  if(!encrypt && id)
      return ETRENF;
  else if(!id && encrypt) {
    /** We found a encrypt object, but not an ID. Let us try parsing the 
	object as some security handlers seems to encrypt/skip the ID.
	Logic changed to show these handles correctly instead of printing out
	a error that the document is not encrypted.
    **/
   return e_pos;
  }
  else 
    return ETRANF;
}

static bool
parseEncrypObject(FILE *file, EncData *e) {
  int ch, dict = 1;
  bool fe = false;
  bool ff = false;
  bool fl = false;
  bool fo = false;
  bool fp = false;
  bool fr = false;
  bool fu = false;
  bool fv = false;
  bool cf = false;
  bool aesv2 = false;
  p_str *str = NULL;

  ch = getc(file);
  while(ch != EOF) {
    if(ch == '>') {
      ch = getc(file);
      if(ch == '>') {
	dict--;
	if(dict <= 0)
	   break;
      }
    }
    else if(ch == '<') {
      ch = getc(file);
      if(ch == '<') {
	dict++;
      }
    }
    if(dict > 1) {
      ch = getc(file);
      if(ch == '/') {
	ch = getc(file);
	switch(ch) {
	case 'A':
	  if(isWord(file, "ESV2")) {
	    aesv2 = true;
	  }
	  break;

	default:
	  break;
	}
      }
      continue;
    }

    if(ch == '/') {
      ch = getc(file);
      switch(ch) {
      case 'C':
	if(isWord(file, "F")) {
	  cf = true;
	}
	break;	
      case 'E':
	if(isWord(file, "ncryptMetadata")) {
	  ungetc(parseWhiteSpace(file), file);
	  if(isWord(file, "false"))
	    fe = true;
	}
	break;
      case 'F':
	if(isWord(file, "ilter")) {
	  char *s_handler = parseName(file);
	  if(s_handler != NULL) {
	    e->s_handler = s_handler;
	    ff = true;
	  }
	}
	break;
      case 'L':
	if(isWord(file, "ength")) {
	  int tmp_l = parseIntWithC(file,parseWhiteSpace(file));
	  if(!fl) { 
	    /* BZZZT!!  This is sooo wrong but will work for most cases.
	       only use the first length we stumble upon */
	    if(tmp_l > 256 || tmp_l < 40 || ((tmp_l % 8) != 0)) {
	      fprintf(stderr, "WARNING: Length = %d\n", tmp_l);
	    }
	    e->length = (unsigned)tmp_l;
	  }
	  fl = true;
	}
	break;
      case 'O':
	/** Check if it is OE string and ignore it then */
	ch = getc(file);
	if(ch == 'E') {
	  str = parseRegularString(file);
	  if(str) {
	    free(str->content);
	    free(str);
	  }
	  break;
	}
	else
	  ungetc(ch,file);
	/** Parse O-String */
	str = parseRegularString(file);
	if(!str)
	  break;
	e->o_string = str->content;
	o_len = str->len;
	free(str);
	fo = true;
	break;
      case 'P':
	ch = getc(file);
	if(isWhiteSpace(ch)) {
	  ch = parseWhiteSpace(file);
	  e->permissions = parseIntWithC(file,ch);
	  fp = true;
	}
	break;
      case 'R':
	ch = getc(file);
	if(isWhiteSpace(ch)) {
	  ch = parseWhiteSpace(file);
	  e->revision = parseIntWithC(file,ch);
	  fr = true;
	}
	break;
      case 'U':
	/** Check if it is UE string and ignore it then */
	ch = getc(file);
	if(ch == 'E') {
	  str = parseRegularString(file);
	  if(str) {
	    free(str->content);
	    free(str);
	  }
	  break;
	}
	else
	  ungetc(ch,file);
	/** Parse U-string */
	str = parseRegularString(file);
	if(!str)
	  break;
	e->u_string = str->content;
	u_len = str->len;
	free(str);
	fu = true;
	break;
      case 'V':
	ch = getc(file);
	if(isWhiteSpace(ch)) {
	  e->version = parseIntWithC(file, parseWhiteSpace(file));
	  fv = true;
	}
	break;
      default:
	break;
      }
    }
    ch = parseWhiteSpace(file);
  }

  if(!fe)
    e->encryptMetaData = true;
  if(!fl || e->length == 0)
    e->length = 40;
  if(!fv)
    e->version = 0;

  if(fr) {
    if (e->revision >= 5 && fu && fo) {
      if(o_len < 48) {
	fprintf(stderr, "WARNING: O-String < 48 Bytes: %d\n", o_len);
	fo = false;
      }
      if(u_len < 48) {
	fprintf(stderr, "WARNING: U-String < 48 Bytes: %d\n", u_len);
	fu = false;
      }
    }
    else {
      if(fu && fo) {
	if(o_len != 32) {
	  fprintf(stderr, "WARNING: O-String != 32 Bytes: %d\n", o_len);
	  fo = false;
	}
	if(u_len != 32) {
	  fprintf(stderr, "WARNING: U-String != 32 Bytes: %d\n", u_len);
	  fu = false;
	}
      }
      if (cf && aesv2) {
	e->revision = 3;
	e->version = 2;
      }
    }
  }

  if(ff && strcmp(e->s_handler,"Standard") != 0)
    return true;
  
  /**
  if(ff)
    printf("Found Filter\n");
  if(fo)
    printf("Found Owner string\n");
  if(fp)
    printf("Found Permission\n");
  if(fr)
    printf("Found Revision\n");
  if(fu)
    printf("Found User String\n");
  */
 
  return ff & fo && fp && fr && fu;
}

/** 
    This is not a really definitive search.
    Should be replaced with something better
*/

static bool
findEncryptObject(FILE *file, const int e_pos, EncData *e) {
  int ch;

  /** only find the encrypt object if e_pos > -1 */
  
  if(e_pos < 0)
    return false;

  ch = getc(file);
  while(ch != EOF) {
    if(isEndOfLine(ch)) {
      if(parseInt(file) == e_pos) {
	ch = parseWhiteSpace(file);
	if(ch >= '0' && ch <= '9') {
	  ch = parseWhiteSpace(file);
	  if(ch == 'o' && getc(file) == 'b' && getc(file) == 'j' &&
	     parseWhiteSpaceOrComment(file) == '<' && getc(file) == '<') {
	    return parseEncrypObject(file, e);
	  }
	}
      }
    }
    ch = getc(file);
  }

  /** If this fails, do relaxed check to see if we might find it without
      a EndOfLine character before the object */
  rewind(file);
  ch = getc(file);
  while(ch != EOF) {
    if(ch >= '0' && ch <= '9') {
      ungetc(ch,file);
      if(parseInt(file) == e_pos) {
	ch = parseWhiteSpace(file);
	if(ch >= '0' && ch <= '9') {
	  ch = parseWhiteSpace(file);
	  if(ch == 'o' && getc(file) == 'b' && getc(file) == 'j' &&
	     parseWhiteSpaceOrComment(file) == '<' && getc(file) == '<') {
	    return parseEncrypObject(file, e);
	  }
	}
      }
    }
    ch = getc(file);
  }

  /** give up */
  return false;
}


int
getEncryptedInfo(FILE *file, EncData *e) {
  int e_pos = -1;
  bool ret;

  /* Start checking if the block is in the end of the file */
  if(fseek(file, -1024, SEEK_END))
    e_pos = findTrailer(file, e);
  if(e_pos < 0) {
    rewind(file);
    e_pos = findTrailer(file, e);
  }
  if(e_pos < 0) {
    rewind(file);
    e_pos = findTrailerDict(file, e);	
  }

  if(e_pos < 0) {
    return e_pos;
  }
  rewind(file);
  ret = findEncryptObject(file, e_pos, e);
  if(!ret)
    return EENCNF;

  return 0;
}
