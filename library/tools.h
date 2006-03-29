/*
 *  USEMARCON Software - Command Line version
 *
 *  File:  tools.h
 *
 *
 *  NOTE:  IN NO WAY WHATSOEVER SHOULD THIS FILE BE USED IN THE EARLIER
 *         VERSIONS OF USEMARCON SOFTWARE.
 *

OVERVIEW TEXT
    USEMARCON Software - Command Line version
    Copyright The British Library, The USEMarcon Consortium, 1995-2000
    Adapted by Crossnet Systems Limited - British Library Contract No. BSDS 851
    Adapted by ATP Library Systems Ltd, Finland, 2002-2003
    Adapted by The National Library of Finland, 2004-2006

AUTHOR
    Crossnet Systems Limited
    ATP Library Systems Limited
    Helsinki University Library, the National Library of Finland

*/

#ifndef _TOOLS_H
#define _TOOLS_H

#include <stdio.h>
#include <string.h>
#include "tfile.h"

#define APRES  0
#define DESSUS 1

#define AT_BEGINING	0x01
#define INSIDE		0x02
#define AT_END		0x04
#define EVERYWHERE	AT_BEGINING|INSIDE|AT_END

int RemoveSpace		(char *String,int where=EVERYWHERE) ;
int InsertChar		(char *entree,int *lg_entree,int position,char *caracteres,int code);
int	ToUpperCase		(char *String);
int CodeHexaToChar	(char *String);

// trim whitespace from the beginning and the end of the string
void trim_string(char *str);

// trim single and double quotes from the beginning and the end of the string
void trim_string_quotes(char *str);

// read a string from the specified ini file
void get_ini_string(const char *section_name,
                    const char *key_name,
                    const char *default_value,
                    char *return_buffer,
                    int return_buffer_size,
                    const char *ini_name);

///////////////////////////////////////////////////////////////////////////////
//
// get_ini_filename()
//
// Calls get_ini_string() to fetch a filename from the .ini
// files. If the filename does not contain path (no slashes/backslashes), the path
// to the ini file itself (including slash/backslash) is added to the beginning
// of the path.
//
///////////////////////////////////////////////////////////////////////////////
void get_ini_filename(const char *section_name,
                      const char *key_name,
                      const char *default_value,
                      char *return_buffer,
                      int return_buffer_size,
                      const char *ini_name);

// write a string to the specified ini file (the file must exist already)
bool put_ini_string(const char *section_name,
                    const char *key_name,
                    const char *value,
                    const char *ini_name);

// copy the path of filename
void copy_path_from_filename(char *path, const char *filename);

// append a filename to the path
void append_filename(char *path, const char *filename);

// get a file attribute
bool file_exists(FILE_SPEC *fspec);

// show a warning to the user
void show_warning(const char *message);

// show a message to the user
void show_message(const char *message);

// UTF-8 aware functions (somewhat crude implementations)
unsigned int utf8_strlen(const char *str);
unsigned int utf8_charindex(const char *str, unsigned long idx);
unsigned int utf8_charlen(const char *p);


#endif // _TOOLS_H
