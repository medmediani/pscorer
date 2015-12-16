#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>


#ifndef _PROCESSDIR_H
#define _PROCESSDIR_H

#define PATH_SEP '/'

using namespace std;

bool isdir(string );
bool remove_tree(string ) ;
#endif
