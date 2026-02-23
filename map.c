#include "./include/map.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "./include/table.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: map <outfile> <infiles...>\n");
        return 1;
    }
    const char *outfile = argv[1];
    table_t *table = table_init();
    if (!table) {
        return 1;
    }
    for (int i=2;i<argc;i++) {
        if (map_log(table,argv[i])!=0) {
            table_free(table);
            return 1;
        }
    }
    if (table_to_file(table,outfile) !=0) {
        table_free(table);
        return 1;
    }
    table_free(table);
    return 0;
}

int map_log(table_t *table, const char file_path[MAX_PATH]) {
    FILE *fp = fopen(file_path, "r");
    if (!fp){
        return 1;
    }
    char line[256];
    while (fgets(line,sizeof(line),fp)) {
        char timestamp[20];
        char ip[16];
        char method[8];
        char route[37];
        char status[4];
        if (sscanf(line,"%19[^,],%15[^,],%7[^,],%36[^,],%3s", timestamp,ip,method,route,status)==5) {
            bucket_t *bucket = table_get(table,ip);
            if (bucket) {
                bucket->requests++;
            }
            else {
                bucket = bucket_init(ip);
                bucket->requests = 1;
                table_add(table,bucket);
            }
        }
    }
    fclose(fp);
    return 0;
}
