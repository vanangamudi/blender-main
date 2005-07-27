/*
 * blenlib/fileops.h 
 *
 * cleaned up (a bit) mar-01 nzc
 *
 * More low-level file things.
 *
 * $Id$
 *
 * ***** BEGIN GPL/BL DUAL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. The Blender
 * Foundation also sells licenses for use in proprietary software under
 * the Blender License.  See http://www.blender.org/BL/ for information
 * about this.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL/BL DUAL LICENSE BLOCK *****
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "zlib.h"

#ifdef WIN32
#include "BLI_winstuff.h"
#include <io.h>
#else
#include <sys/param.h>
#endif

#include "BLI_blenlib.h"
#include "BLI_storage.h"
#include "BLI_fileops.h"
#include "BLI_callbacks.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "BKE_utildefines.h"
#include <errno.h>

/* implementations: */
char *first_slash(char *string) {
	char *ffslash, *fbslash;
	
	ffslash= strchr(string, '/');	
	fbslash= strchr(string, '\\');
	
	if (!ffslash) return fbslash;
	else if (!fbslash) return ffslash;
	
	if ((int)ffslash < (int)fbslash) return ffslash;
	else return fbslash;
}

char *BLI_last_slash(char *string) {
	char *lfslash, *lbslash;
	
	lfslash= strrchr(string, '/');	
	lbslash= strrchr(string, '\\');

	if (!lfslash) return lbslash; 
	else if (!lbslash) return lfslash;
	
	if ((int)lfslash < (int)lbslash) return lbslash;
	else return lfslash;
}

/* gzip the file in from and write it to "to". 
 return -1 if zlib fails, -2 if the originating file does not exist
 note: will remove the "from" file
  */
int BLI_gzip(char *from, char *to) {
	char buffer[10240];
	int file;
	int readsize = 0;
	
	gzFile gzfile = gzopen(to,"wb"); 
	if (NULL == gzfile) return -1;
	
	file = open(from,O_BINARY|O_RDONLY);
	
	if ( -1 == file ) 	return -2;

	while ( 1 )
	{
		readsize = read(file, buffer, 10240);
		
		if (readsize <= 0) break;
		
		gzwrite(gzfile,buffer,readsize);
	}
	
	gzclose(gzfile);
	close(file);
	
	remove(from);

	return 0;
}

#ifdef WIN32

static char str[MAXPATHLEN+12];

int BLI_delete(char *file, int dir, int recursive) {
	int err;

	if (recursive) {
		callLocalErrorCallBack("Recursive delete is unsupported on Windows");
		err= 1;
	} else if (dir) {
		err= !RemoveDirectory(file);
		if (err) printf ("Unable to remove directory");
	} else {
		err= !DeleteFile(file);
		if (err) callLocalErrorCallBack("Unable to delete file");
	}

	return err;
}

int BLI_touch(char *file) {
	callLocalErrorCallBack("Touching files is unsupported on Windows");
	
	return 1;
}

int BLI_move(char *file, char *to) {
	int err;

	// windows doesn't support moveing to a directory
	// it has to be 'mv filename filename' and not
	// 'mv filename destdir'

	strcpy(str, to);
	// points 'to' to a directory ?
	if (BLI_last_slash(str) == (str + strlen(str) - 1)) {
		if (BLI_last_slash(file) != NULL) {
			strcat(str, BLI_last_slash(file) + 1);
		}
	}

	err= !MoveFile(file, str);
	if (err) {
		callLocalErrorCallBack("Unable to move file");
		printf(" Move from '%s' to '%s' failed\n", file, str);
	}

	return err;
}


int BLI_copy_fileops(char *file, char *to) {
	int err;

	// windows doesn't support copying to a directory
	// it has to be 'cp filename filename' and not
	// 'cp filename destdir'

	strcpy(str, to);
	// points 'to' to a directory ?
	if (BLI_last_slash(str) == (str + strlen(str) - 1)) {
		if (BLI_last_slash(file) != NULL) {
			strcat(str, BLI_last_slash(file) + 1);
		}
	}

	err= !CopyFile(file,str,FALSE);
	
	if (err) {
		callLocalErrorCallBack("Unable to copy file!");
		printf(" Copy from '%s' to '%s' failed\n", file, str);
	}

	return err;
}

int BLI_link(char *file, char *to) {
	callLocalErrorCallBack("Linking files is unsupported on Windows");
	
	return 1;
}

int BLI_backup(char *file, char *from, char *to) {
	callLocalErrorCallBack("Backing up files is unsupported on Windows");
	
	return 1;
}

int BLI_exists(char *file) {
	return (GetFileAttributes(file) != 0xFFFFFFFF);
}

void BLI_recurdir_fileops(char *dirname) {
	char *lslash;
	char tmp[MAXPATHLEN];
	
	// First remove possible slash at the end of the dirname.
	// This routine otherwise tries to create
	// blah1/blah2/ (with slash) after creating
	// blah1/blah2 (without slash)

	strcpy(tmp, dirname);
	lslash= BLI_last_slash(tmp);

	if (lslash == tmp + strlen(tmp) - 1) {
		*lslash = 0;
	}
	
	if (BLI_exists(tmp)) return;
		
	lslash= BLI_last_slash(tmp);
	if (lslash) {
			/* Split about the last slash and recurse */	
		*lslash = 0;
		BLI_recurdir_fileops(tmp);
	}
	
	if(dirname[0]) /* patch, this recursive loop tries to create a nameless directory */
		if (!CreateDirectory(dirname, NULL))
			callLocalErrorCallBack("Unable to create directory\n");
}

int BLI_rename(char *from, char *to) {
	if (!BLI_exists(from)) return 0;

	if (BLI_exists(to))
		if(BLI_delete(to, 0, 0)) return 1;
		
	return rename(from, to);
}

#else /* The sane UNIX world */

/*
 * but the sane UNIX world is tied to the interface, and the system
 * timer, and... We implement a callback mechanism. The system will
 * have to initialise the callback before the functions will work!
 * */
static char str[MAXPATHLEN+12];

int BLI_delete(char *file, int dir, int recursive) 
{
	if(strchr(file, '"')) {
		printf("Error: not deleted file %s because of quote!\n", file);
	}
	else {
		if (recursive) sprintf(str, "/bin/rm -rf \"%s\"", file);
		else if (dir) sprintf(str, "/bin/rmdir \"%s\"", file);
		else sprintf(str, "/bin/rm -f \"%s\"", file);

		return system(str);
	}
	return -1;
}

int BLI_touch(char *file) 
{
	
	if( BLI_exists("/bin/touch") )
		sprintf(str, "/bin/touch %s", file);
	else
		sprintf(str, "/usr/bin/touch %s", file);
	
	return system(str);
}

int BLI_move(char *file, char *to) {
	sprintf(str, "/bin/mv -f \"%s\" \"%s\"", file, to);

	return system(str);
}

int BLI_copy_fileops(char *file, char *to) {
	sprintf(str, "/bin/cp -rf \"%s\" \"%s\"", file, to);

	return system(str);
}

int BLI_link(char *file, char *to) {
	sprintf(str, "/bin/ln -f \"%s\" \"%s\"", file, to);
	
	return system(str);
}

int BLI_backup(char *file, char *from, char *to) {
	sprintf(str, "/bin/su root -c 'cd %s; /bin/tar cf - \"%s\" | (/bin/cd %s; /bin/tar xf -)'", from, file, to);
	
	return system(str);
}

int BLI_exists(char *file) {
	return BLI_exist(file);
}

void BLI_recurdir_fileops(char *dirname) {
	char *lslash;
	char tmp[MAXPATHLEN];
		
	if (BLI_exists(dirname)) return;

	strcpy(tmp, dirname);
		
	lslash= BLI_last_slash(tmp);
	if (lslash) {
			/* Split about the last slash and recurse */	
		*lslash = 0;
		BLI_recurdir_fileops(tmp);
	}

	mkdir(dirname, 0777);
}

int BLI_rename(char *from, char *to) {
	if (!BLI_exists(from)) return 0;
	
	if (BLI_exists(to))	if(BLI_delete(to, 0, 0)) return 1;
		
	return rename(from, to);
}

#endif
