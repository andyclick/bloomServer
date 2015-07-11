#include   <stdio.h> 
#define SPIDER1PATH "/oracle/spider1/checkpoint"
#define SPIDER1NEWXPATH "/oracle/spider1news/checkpoint"
#define DELETE_DATA_FLAG_FILE "delete_data_flag_file"
void setflag(const char * filename);
main()
{
    char filename[64] ;filename[0] = 0;
    sprintf(filename, "%s/%s", SPIDER1PATH, DELETE_DATA_FLAG_FILE);
    setflag(filename);
    sprintf(filename, "%s/%s", SPIDER1NEWXPATH, DELETE_DATA_FLAG_FILE);
    setflag(filename);
}
void setflag(const char * filename)
{
    FILE  *f  = fopen(filename, "w");
    if (f) {
        char key[32] ;key[0] = 0;
        int keylen = sprintf(key, "%d", (int)1);
        fwrite(key, 1, keylen, f);
        fclose(f);
    } else {
        printf("can not open %s\n",filename);
        fclose(f);
    }
}
