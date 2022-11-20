#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h> 

#define PERMS_SIZE 10
#define TBUF_SIZE 256
#define DNAME_SIZE 256
#define MEM_SAFE_ALIGN 4

char* perms(struct stat *st){

    char *modeval = malloc(sizeof(char) * PERMS_SIZE);

    mode_t perm = st->st_mode;
    modeval[0] = (perm & S_IRUSR) ? 'r' : '-';
    modeval[1] = (perm & S_IWUSR) ? 'w' : '-';
    modeval[2] = (perm & S_IXUSR) ? 'x' : '-';
    modeval[3] = (perm & S_IRGRP) ? 'r' : '-';
    modeval[4] = (perm & S_IWGRP) ? 'w' : '-';
    modeval[5] = (perm & S_IXGRP) ? 'x' : '-';
    modeval[6] = (perm & S_IROTH) ? 'r' : '-';
    modeval[7] = (perm & S_IWOTH) ? 'w' : '-';
    modeval[8] = (perm & S_IXOTH) ? 'x' : '-';
    modeval[9] = '\0';

    return modeval;
}

bool check_last_symbol(const char *str)
{
    static char last[2];
    sprintf(last,"%c",str[strlen(str)-1]);
    if(strcmp(last,"/") == 0 )
        return true;
    return false;
}

int list_single_file(const char *filename)
{
    struct stat my_stat;
    struct tm lt;  
    struct passwd *pwd;

    if(check_last_symbol(filename))
    {
        printf("list_single_file : %s : %s \n", filename, strerror(errno));
        return errno;
    }

    if ( (stat( filename, &my_stat) ) != 0 ){
        printf("list_single_file : %s : %s \n", filename, strerror(errno));
        return errno;
    }
    pwd = getpwuid(my_stat.st_uid);

    time_t t = my_stat.st_mtime;
    localtime_r(&t, &lt);
    char timebuf[TBUF_SIZE];
    strftime(timebuf, sizeof(timebuf), "%c", &lt);

    printf("%s \t %s \t %ld \t %s \t %s", perms(&my_stat), pwd->pw_name, (long)my_stat.st_size, timebuf, filename);
    printf("\n");

    return 0;
}

int list_dir(const char *dirname, DIR* directory)
{
    struct dirent* current_directory;
    struct stat my_stat;
    struct tm lt;  
    struct passwd *pwd;

    printf("Directory : %s\n", dirname);
    printf("\n");
    static char* dirname_str;
    static char* fullpath;
    static char cur_dir_str[DNAME_SIZE];

    while( current_directory = readdir(directory) )
    {

        dirname_str=malloc(strlen(dirname)*MEM_SAFE_ALIGN);
        sprintf(dirname_str,"%s",dirname);

        if(!check_last_symbol(dirname_str))
            strcat(dirname_str,"/");

        sprintf(cur_dir_str,"%s",current_directory->d_name);
        fullpath=malloc((strlen(cur_dir_str) + strlen(dirname_str))*MEM_SAFE_ALIGN);

        sprintf(fullpath,"%s",strcat(dirname_str,cur_dir_str));
        
        if ( (stat( realpath(fullpath,NULL), &my_stat) ) != 0 ){
            printf("list_dir : %s : %s \n", fullpath, strerror(errno));
            return errno;
        }
        pwd = getpwuid(my_stat.st_uid); // Get User-ID

        // Last Modified 
        time_t t = my_stat.st_mtime;
        localtime_r(&t, &lt);
        char timebuf[TBUF_SIZE];
        strftime(timebuf, sizeof(timebuf), "%c", &lt);

        printf("%s \t %s \t %ld \t %s \t %s", perms(&my_stat), pwd->pw_name, (long)my_stat.st_size, timebuf, cur_dir_str);
        printf("\n");

        free(dirname_str);
        free(fullpath);
    }
    return 0;
}

int ls_get_list(const char *dirname)
{

    DIR* directory = opendir(dirname);

    if(directory == NULL) {
        if(errno == ENOENT)
        {
            printf("list_dir : %s : %s \n", dirname, strerror(errno));
            return 0;
        }
        else if(errno ==  ENOTDIR){
            return list_single_file(dirname);
        }
    }

    list_dir(dirname, directory);

    closedir(directory);

    return 0;
}

int main(int argc, char* argv[])
{
    int ret = 0;

    if ( argc == 1 ) {
        return ls_get_list ( "." );
    }
    else
    {
        for (int i = 1; i < argc; i += 1 ) {
            if ( ls_get_list ( argv[i] ) != 0 ) {
                ret = 1; 
            }
        }
    }

    return ret;
}