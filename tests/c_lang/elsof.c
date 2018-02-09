/*
 * optimized lsof to search default max. 2 process ID for opened filename
 * usage:   elsof /path/to/file.search [ x_times ]
 * build:   gcc -O3 -o elsof elsof.c
 * install: chown root:root elsof
 *          chmod 4755 elsof [ sets suid-root flag ]
 *          executable uses /proc FS, and LetoDBf expect it in: /usr/bin
 * (c) 2018 Rolf 'elch' Beckmann
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

int main( const int argc, const char * argv[] )
{
   char            szLink[ 256 ], szProc[ 64 ] = { '/', 'p', 'r', 'o', 'c', '/' };
   DIR *           procfd, * proc = opendir( "/proc" );
   struct dirent * ent, * entfd;
   int             iLenLink, iFound = 2;
   char            * ptr, * ptr2, * ptr3;

   if( proc == NULL || argc < 2 )
      exit( EXIT_FAILURE );
   if( argc > 2 )
   {
      iFound = atoi( argv[ 2 ] );
      if( iFound < 1 )
         iFound = 2;
   }

   while( ( ent = readdir( proc ) ) != NULL )
   {
      ptr3 = ent->d_name;
      if( ! isdigit( *ptr3 ) )
         continue;
      else
      {
         ptr = szProc + 6;
         do
         {
            *ptr++ = *ptr3++;
         }
         while( *ptr3 != '\0' );
         *ptr++ = '/';
         *ptr++ = 'f';
         *ptr++ = 'd';
         *ptr = '\0';

         if( ( procfd = opendir( szProc ) ) != NULL )
         {
            *ptr++ = '/';
            while( ( entfd = readdir( procfd ) ) != NULL )
            {
               ptr3 = entfd->d_name;
               if( *ptr3 == '.' )
                  continue;
               ptr2 = ptr;
               do
               {
                  *ptr2++ = *ptr3++;
               }
               while( *ptr3 != '\0' );
               *ptr2 = '\0';
               iLenLink = readlink( szProc, szLink, 255 );
               if( *szLink != '/' )
                  continue;
               szLink[ iLenLink ] = '\0';
               if( strstr( szLink, argv[ 1 ] ) )
               {
                  printf( "%s:%s\n", ent->d_name, szLink );
                  if( --iFound == 0 )
                  {
                     closedir( procfd );
                     closedir( proc );
                     exit( EXIT_SUCCESS );
                  }
               }
            }

            closedir( procfd );
         }
         else  /* missing root access ? */
         {
            closedir( proc );
            exit( EXIT_FAILURE );
         }
      }
   }

   closedir( proc );
   exit( EXIT_SUCCESS );
}

