/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996 Larry Doolittle <ldoolitt@cebaf.gov>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "boa.h"

/* 
 * There are two hash tables used, each with a key/value pair
 * stored in a hash_struct.  They are:
 * 
 * mime_hashtable:
 *     key = file extension
 *   value = mime type
 * 
 * passwd_hashtable: 
 *     key = username
 *   value = home directory
 * 
 */

struct hash {
       char        *key;
       char        *value;
       struct hash *next;
     };

static struct hash *mime_hashtable   [MIME_HASHTABLE_SIZE];
//static struct hash *passwd_hashtable [PASSWD_HASHTABLE_SIZE];

/* 
 * Name: add_mime_type
 * Description: Adds a key/value pair to the mime_hashtable
 */
void add_mime_type (char *extension, char *type)
{
  int    hash;
  struct hash *current;

  if (!extension)
     return;

  hash = get_mime_hash_value (extension);

  current = mime_hashtable[hash];

  if (!current)
  {
    mime_hashtable[hash] = malloc (sizeof(struct hash));
    mime_hashtable[hash]->key   = strdup (extension);
    mime_hashtable[hash]->value = strdup (type);
    mime_hashtable[hash]->next  = NULL;
  }
  else
  {
    while (current)
    {
      if (!strcmp(current->key, extension))
         return;         /* don't add extension twice */
      if (current->next)
           current = current->next;
      else break;
    }

    current->next = malloc (sizeof(struct hash));
    current = current->next;

    current->key   = strdup (extension);
    current->value = strdup (type);
    current->next  = NULL;
  }
}

/*
 * Name: get_mime_hash_value
 * 
 * Description: adds the ASCII values of the file extension letters
 * and mods by the hashtable size to get the hash value
 */
int get_mime_hash_value (char *extension)
{
  int  hash  = 0;
  int  index = 0;
  char c;

  while ((c = extension[index++]) != 0)
        hash += (int)c;

  hash %= MIME_HASHTABLE_SIZE;

  return (hash);
}

/*
 * Name: get_mime_type
 *
 * Description: Returns the mime type for a supplied filename.
 * Returns default type if not found.
 */
char * get_mime_type (char *filename)
{
  char        *extension;
  struct hash *current;
  int    hash;

  extension = strrchr (filename, '.');

  if (!extension || *extension++ == '\0')
     return (default_type);

  hash    = get_mime_hash_value (extension);
  current = mime_hashtable [hash];

  while (current)
  {
    if (!strcmp(current->key, extension))
       return (current->value);
    current = current->next;
  }
  return (default_type);
}

#if 0
/*
 * Name: get_home_dir
 * 
 * Description: Returns a point to the supplied user's home directory.  
 * Adds to the hashtable if it's not already present.
 * 
 */
char *get_home_dir (char *name)
{
  struct passwd *passwdbuf;
  struct hash   *current, *trailer;
  int    hash;

  /*
   * first check hash table -- if username is less than four characters,
   * just hash to zero (this should be very rare)
   */

  hash = ((strlen(name) < 4) ? 0 :
         ((name[0] + name[1] + name[2] + name[3]) % PASSWD_HASHTABLE_SIZE));

  current = passwd_hashtable[hash];

  if (!current)                              /* definite miss */
  {
    passwdbuf = getpwnam (name);
    if (!passwdbuf)                          /* does not exist */
       return (NULL);

    passwd_hashtable[hash] = malloc (sizeof(struct hash));
    passwd_hashtable[hash]->key   = strdup (name);
    passwd_hashtable[hash]->value = strdup (passwdbuf->pw_dir);
    passwd_hashtable[hash]->next  = NULL;
    return (passwd_hashtable[hash]->value);
  }

  while (current)
  {
    if (!strcmp(current->key, name))
       return (current->value);

    trailer = current;
    current = current->next;
  }

  /* not in hash table -- let's look it up */

  passwdbuf = getpwnam (name);

  if (!passwdbuf)                             /* does not exist */
     return (NULL);

  /* exists -- have to add to hashtable */

  trailer->next = malloc (sizeof(struct hash));
  current = trailer->next;

  current->key   = strdup (name);
  current->value = strdup (passwdbuf->pw_dir);
  current->next  = NULL;

  return (current->value);
}

#endif
