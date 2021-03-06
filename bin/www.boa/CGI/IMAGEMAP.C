/*
** mapper 1.2
** 7/26/93 Kevin Hughes, kevinh@pulua.hcc.hawaii.edu
** "macmartinized" polygon code copyright 1992 by Eric Haines, erich@eye.com
** All suggestions, help, etc. gratefully accepted!
**
** 1.1 : Better formatting, added better polygon code.
** 1.2 : Changed isname(), added config file specification.
**
** 11/13/93: Rob McCool, robm@ncsa.uiuc.edu
**
** Rewrote configuration stuff for NCSA /htbin script
**
** 12/05/93: Rob McCool, robm@ncsa.uiuc.edu
** 
** Made CGI/1.0 compliant.
** 
** 7/18/96:  Harry H. Cheng, hhcheng@ucdavis.edu
** Ported to run in both ANSI C and CH language environment.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define LF 10
#define CR 13

#define CONF_FILE "/usr/local/etc/httpd/conf/imagemap.conf"

#define MAXLINE 500
#define MAXVERTS 100
#define X 0
#define Y 1

int isname(char);

int main(int argc, char **argv)
{
    char input[MAXLINE], *mapname, def[MAXLINE], conf[80];
    double testpoint[2], pointarray[MAXVERTS][2];
    int i, j, k;
    FILE *fp;
    char *t;
    char confname[MAXLINE];
    char type[MAXLINE];
    char url[MAXLINE];
    char num[10];
    
    if (argc != 2)
        servererr ("Wrong number of arguments, client may not support ISMAP.");
    mapname=getenv ("PATH_INFO");

    if (!mapname || !mapname[0])
       servererr ("No map name given. Please read the "
                  "<A HREF=\"http://hoohoo.ncsa.uiuc.edu/docs/setup/admin/Imagemap.html\">instructions</A>.<P>");

    mapname++;
    t = strchr (argv[1],',');
    if (!t)
       servererr ("Your client doesn't support image mapping properly.");

    *t++ = '\0';
    testpoint[X] = (double) atoi(argv[1]);
    testpoint[Y] = (double) atoi(t);
    
    fp = fopen (CONF_FILE, "r");
    if (fp == NULL)
        servererr ("Couldn't open configuration file.");

    while (!getline(input,MAXLINE,fp))
    {
      if (input[0] == '#' || !input[0])
         continue;

      for (i = 0; isname(input[i]) && input[i] != ':'; i++)
          confname[i] = input[i];
      confname[i] = '\0';
      if (!strcmp(confname,mapname))
         break;
    }
    if (feof(fp))
       servererr ("Map not found in configuration file.");

    fclose (fp);
    while (isspace(input[i]) || input[i] == ':')
          ++i;

    for (j = 0; input[i] && isname(input[i]); ++i,++j)
        conf[j] = input[i];
    conf[j] = '\0';

    fp = fopen (conf,"r");
    if (!fp)
       servererr ("Couldn't open map file.");
    
    while (!getline(input,MAXLINE,fp))
    {
      if((input[0] == '#') || (!input[0]))
          continue;

      type[0] = '\0';url[0] = '\0';

      for(i=0;isname(input[i]) && (input[i]);i++)
          type[i] = input[i];
      type[i] = '\0';

      while(isspace(input[i])) ++i;
      for(j=0;input[i] && isname(input[i]);++i,++j)
          url[j] = input[i];
      url[j] = '\0';

      if(!strcmp(type,"default")) {
          strcpy(def,url);
          continue;
      }

      k=0;
      while (input[i]) {
          while (isspace(input[i]) || input[i] == ',')
              i++;
          j = 0;
          while (isdigit(input[i]))
              num[j++] = input[i++];
          num[j] = '\0';
          if (num[0] != '\0')
              pointarray[k][X] = (double) atoi(num);
          else
              break;
          while (isspace(input[i]) || input[i] == ',')
              i++;
          j = 0;
          while (isdigit(input[i]))
              num[j++] = input[i++];
          num[j] = '\0';
          if (num[0] != '\0')
              pointarray[k++][Y] = (double) atoi(num);
          else {
              fclose(fp);
              servererr("Missing y value.");
          }
      }
      pointarray[k][X] = -1;
      if(!strcmp(type,"poly"))
          if(pointinpoly(testpoint,pointarray))
              sendmesg(url);
      if(!strcmp(type,"circle"))
          if(pointincircle(testpoint,pointarray))
              sendmesg(url);
      if(!strcmp(type,"rect"))
          if(pointinrect(testpoint,pointarray))
              sendmesg(url);
    }
    if(def[0])
        sendmesg(def);
    servererr("No default specified.");
}

int sendmesg (char *url)
{
  printf ("Location: %s%c%c",url,10,10);
  printf ("This document has moved <A HREF=\"%s\">here</A>%c",url,10);
  exit (1);
}

int pointinrect(double point[2], double coords[MAXVERTS][2])
{
  return ((point[X] >= coords[0][X] && point[X] <= coords[1][X]) &&
          (point[Y] >= coords[0][Y] && point[Y] <= coords[1][Y]));
}

int pointincircle (double point[2], double coords[MAXVERTS][2])
{
  int radius1, radius2;

  radius1 = ((coords[0][Y] - coords[1][Y]) * (coords[0][Y] -
            coords[1][Y])) + ((coords[0][X] - coords[1][X]) * (coords[0][X] -
            coords[1][X]));
  radius2 = ((coords[0][Y] - point[Y]) * (coords[0][Y] - point[Y])) +
            ((coords[0][X] - point[X]) * (coords[0][X] - point[X]));
  return (radius2 <= radius1);
}

int pointinpoly (double point[2], double pgon[MAXVERTS][2])
{
  int     i, numverts, inside_flag, xflag0;
  int     crossings;
  double *p, *stopit;
  double  tx, ty, y;

  for (i = 0; pgon[i][X] != -1 && i < MAXVERTS; i++)
      ;
  numverts = i;
  crossings = 0;

  tx = point[X];
  ty = point[Y];
  y = pgon[numverts - 1][Y];

  p = (double *) pgon + 1;
  if ((y >= ty) != (*p >= ty))
  {
    xflag0 = pgon[numverts - 1][X] >= tx;
    if (xflag0 == (*(double*)pgon >= tx))
    {
      if (xflag0)
         crossings++;
    }
    else
    {
      crossings += (pgon[numverts - 1][X] - (y - ty) *
                   (*(double *) pgon - pgon[numverts - 1][X]) /
                   (*p - y)) >= tx;
    }
  }

  stopit = pgon[numverts];

  for (y = *p, p += 2; p < stopit; y = *p, p += 2)
  {
    if (y >= ty)
    {
      while ((p < stopit) && (*p >= ty))
             p += 2;
      if (p >= stopit)
         break;
      xflag0 = *(p - 3) >= tx;
      if (xflag0 == (*(p - 1) >= tx))
      {
        if (xflag0)
           crossings++;
      }
      else
      {
        crossings += (*(p - 3) - (*(p - 2) - ty) *
                     (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
      }
    }
    else
    {
      while ((p < stopit) && (*p < ty))
             p += 2;
      if (p >= stopit)
         break;
      xflag0 = (*(p - 3) >= tx);
      if (xflag0 == (*(p - 1) >= tx))
      {
        if (xflag0)
           crossings++;
      }
      else
      {
        crossings += (*(p - 3) - (*(p - 2) - ty) *
                     (*(p - 1) - *(p - 3)) / (*p - *(p - 2))) >= tx;
      }
    }
  }
  inside_flag = crossings & 0x01;
  return (inside_flag);
}

int servererr(char *msg)
{
  printf ("Content-type: text/html%c%c",10,10);
  printf ("<title>Mapping Server Error</title>");
  printf ("<h1>Mapping Server Error</h1>");
  printf ("This server encountered an error:<p>");
  printf ("%s", msg);
  exit (-1);
}

int isname (char c)
{
  return (!isspace(c));
}

int getline (char *s, int n, FILE *f)
{
  int i=0;

  while(1)
  {
    s[i] = (char)fgetc(f);

    if(s[i] == CR)
       s[i] = fgetc(f);

    if((s[i] == 0x4) || (s[i] == LF) || (i == (n-1)))
    {
      s[i] = '\0';
      return (feof(f) ? 1 : 0);
    }
    ++i;
  }
}

