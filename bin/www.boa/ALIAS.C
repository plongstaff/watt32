/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Some changes Copyright (C) 1996 Larry Doolittle <ldoolitt@cebaf.gov>
 *  Some changes Copyright (C) 1996 Jon Nelson <nels0988@tc.umn.edu>
 *  Some changes Copyright (C) 1996 Russ Nelson <nelson@crynwr.com>
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

#define FOUR_LETTER_SUM(x) (x[0] + x[1] + x[2] + x[3])

static alias * alias_hashtable[ALIAS_HASHTABLE_SIZE];

/*
 * Name: add_alias
 *
 * Description: add an Alias, Redirect, or ScriptAlias to the 
 * alias hash table.
 */
void add_alias (char *fakename, char *realname, int type)
{
  int    hash;
  alias *current;

  if (strlen(fakename) >= 4)
  {
    hash = fakename[0] + fakename[1] + fakename[2] + fakename[3];
    hash %= ALIAS_HASHTABLE_SIZE;
  }
  else
    hash = 0;                               /* should be very rare */

  current = alias_hashtable[hash];

  if (!current)
  {
    alias_hashtable[hash] = malloc (sizeof(alias));
    if (!alias_hashtable[hash])
       die (OUT_OF_MEMORY);
    alias_hashtable [hash]->fakename = strdup (fakename);
    alias_hashtable [hash]->fake_len = strlen (fakename);
    alias_hashtable [hash]->realname = strdup (realname);
    alias_hashtable [hash]->real_len = strlen (realname);
    alias_hashtable [hash]->type     = type;
    alias_hashtable [hash]->next     = NULL;
  }
  else
  {
    while (current)
    {
      if (!strcmp(fakename, current->fakename))    /* don't add twice */
         return;
      if (current->next)
           current = current->next;
      else break;
    }

    current->next = malloc(sizeof(alias));
    if (!current->next)
        die (OUT_OF_MEMORY);
    current = current->next;

    current->fakename = strdup (fakename);
    current->fake_len = strlen (fakename);
    current->realname = strdup (realname);
    current->real_len = strlen (realname);
    current->type     = type;
    current->next     = NULL;
  }
}


/* 
 * Name: translate_uri
 * 
 * Description: Parse a request's virtual path.  Sets path_info,
 * query_string, path_translated, and script_name data if it's a 
 * ScriptAlias or a CGI.  Note -- this should be broken up.
 * 
 * Return values: 
 *   0: failure, close it down
 *   1: success, continue 
 */
int translate_uri (request * req)
{
  char   buffer[MAX_HEADER_LENGTH + 1];
  char  *req_urip, * user_homedir;
  struct stat statbuf;
  alias *current;
  int    hash = 0, index = 0, is_nph = 0;
  char   c;

  /* Move anything after ? into req->query_string */

  req_urip = req->request_uri;
  if (*req_urip != '/')
  {
    send_r_not_found (req);
    return (0);
  }

  while ((c = *req_urip) != 0 && (c != '?'))
  {
    req_urip++;
    if (c == '/')
    {
      if (!strncmp("nph-",req_urip,4))
           is_nph = 1;
      else is_nph = 0;
    }
  }

  if (c == '?')
  {
    *req_urip++ = '\0';
    req->query_string = strdup (req_urip);
  }

  /* Percent-decode request */

  if (unescape_uri(req->request_uri) == 0)
  {
    send_r_bad_request (req);
    log_error_doc (req);
    log_printf ("Problem unescaping uri\n");
    return (0);
  }

  /* Find ScriptAlias, Alias, or Redirect */

  if (strlen(req->request_uri) >= 4)
  {
    hash = FOUR_LETTER_SUM(req->request_uri);
    hash %= ALIAS_HASHTABLE_SIZE;
  }

  current = alias_hashtable[hash];
  while (current)
  {
    if (!strncmp(req->request_uri,current->fakename,current->fake_len))
    {
      if (current->type == REDIRECT)                      /* Redirect */
      {
        sprintf (buffer, "%s%s", current->realname,
                 &req->request_uri[current->fake_len]);
        req->ret_content_type = strdup ("text/html");
        send_redirect (req, escape_uri(buffer));
        return (0);
      }
      else if (current->type == SCRIPTALIAS)               /* Script */
      {
        if (is_nph)
             req->is_cgi = NPH;
        else req->is_cgi = CGI;
        return init_script_alias (req, current);
      }

      /* Alias */

      sprintf (buffer, "%s%s", current->realname,
               &req->request_uri[current->fake_len]);

      dosify_path (buffer);
      if (stat(buffer,&statbuf) == -1)        /* file does not exist */
      {
        send_r_not_found (req);
        return (0);
      }
      else
      {
        req->path_translated = strdup (buffer);
        return (1);
      }
    }
    current = current->next;
  }

  /* Find UserDir */

  if (user_dir && req->request_uri[1] == '~')
  {
    req_urip = (req->request_uri + 2);
    index    = 0;
    while ((c = *req_urip) != 0)
    {
      if (c == '/')
         break;
      buffer[index++] = c;
      req_urip++;
    }

    buffer[index] = '\0';
    user_homedir = get_home_dir (buffer);
    if (!user_homedir)                          /* no such user */
    {
      send_r_not_found (req);
      return (0);
    }

    sprintf (buffer, "%s%c%s", user_homedir,SLASH,user_dir);
    strcat (buffer, req_urip);
  }
  else
  {
#ifdef EXPERIMENTAL_VIRTUAL_HOST
    struct   sockaddr_in salocal;
    unsigned char iplocal[4];
    int      dummy = sizeof(salocal);

    if (getsockname(req->fd, (struct sockaddr*)&salocal, &dummy) == -1)
       die (SERVER_ERROR);

    memcpy (&iplocal, &salocal.sin_addr, 4);
    sprintf (buffer, "%s/%d.%d.%d.%d%s", document_root,
             iplocal[0], iplocal[1], iplocal[2], iplocal[3], req->request_uri);
#else
    sprintf (buffer, "%s%s", document_root, req->request_uri);
#endif
  }

  req->path_translated = strdup (buffer);
  dosify_path (req->path_translated);

  if (stat(req->path_translated,&statbuf) == -1)  /* file does not exist */
  {
    send_r_not_found (req);
    return (0);
  }

  if (strcmp(CGI_MIME_TYPE,get_mime_type(buffer)) == 0)  /* cgi */
  {
    if (is_nph)
         req->is_cgi = NPH;
    else req->is_cgi = CGI;

    req->script_name = strdup (req->request_uri);
    return (1);
  }
  else if (req->method == M_POST)            /* POST to non-script */
  {
    send_r_not_implemented (req);
    return (0);
  }
  return (1);
}

/*
 * Name: init_script_alias
 * 
 * Description: Performs full parsing on a ScriptAlias request
 * 
 * Return values:
 *
 * 0: failure, shut down
 * 1: success, continue          
 */
int init_script_alias (request *req, alias *current)
{
  char   path_translated [MAX_HEADER_LENGTH+1];
  char   path_info       [MAX_HEADER_LENGTH+1];
  char   script_name     [MAX_HEADER_LENGTH+1];
  struct stat statbuf;
  int    index = 0;
  int    index_trailer = 0;
  char   c;

  sprintf (path_translated, "%s%s", current->realname,
           &req->request_uri[current->fake_len]);
  strcpy (script_name, req->request_uri);

  index = current->real_len;
  index_trailer = current->fake_len;

  while ((c = path_translated[index]) != 0)
  {
    if (c == '/')
       break;
    index_trailer++;
    index++;
  }

  path_translated[index++] = '\0';
  dosify_path (path_translated);

  if (stat(path_translated,&statbuf) == -1)
  {
    send_r_not_found (req);
    return (0);
  }
  else if (!(S_ISREG(statbuf.st_mode)))
  {
    send_r_forbidden (req);
    return (0);
  }

  req->path_translated = strdup (path_translated);
  script_name[index_trailer] = '\0';
  req->script_name = strdup (script_name);
  index_trailer = 0;

  if (c == '/')
  {
    path_info[index_trailer++] = c;
    while ((c = path_translated[index]) != 0)
    {
      index++;
      if (c == '?')
         break;
      path_info[index_trailer++] = c;
    }
    path_info[index_trailer] = '\0';
    req->path_info = strdup (path_info);
  }
  return (1);
}
