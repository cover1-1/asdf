#include "./include/reduce.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "./include/table.h"

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: reduce <read dir> <out file> <start ip> <end ip>\n");
        return 1;
    }
    const char *dir = argv[1];
    const char *outf = argv[2];
    const char *start = argv[3];
    const char *end = argv[4];
    char *endptr;
    long start_ip = strtol(start,&endptr,10);
    long end_ip = strtol(end,&endptr,10);
    if (*endptr != '\0' || start_ip < 0 || start_ip>255 || end_ip<0 || end_ip >256 || start_ip >= end_ip) {
        fprintf(stderr, "reduce: invalid IP range\n");
        return 1;
    }


    table_t *master = table_init();
    DIR *d = opendir(dir);

    table_t *range_table = table_init();
    if (!master || !range_table) {
        return 1;
    }
    if (!d) {
        table_free(master);
        table_free(range_table);
        return 1;
    }
    struct dirent *entry;
    char path[MAX_PATH];
    while ((entry = readdir(d)) != NULL) { 
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        if (reduce_file(master,path, start_ip, end_ip) !=0) {
            closedir(d);
            table_free(master);
            table_free(range_table);
            return 1;
            
        }
    }
    closedir(d);


    for (int i =0; i<TABLE_LEN; i++) {
        bucket_t *curr = master->buckets[i];
        while (curr) {
            int f;
            if (sscanf(curr->ip, "%d",&f)==1) {
                if (f>= start_ip && f < end_ip) {
                    bucket_t *new = bucket_init(curr->ip);
                    new->requests = curr->requests;
                    table_add(range_table, new);
        
                }
            }
            curr = curr->next;
        }
    }
    if (table_to_file(range_table, outf) != 0) {
        table_free(master);
        table_free(range_table);
        return 1;
    }
    table_free(master);
    table_free(range_table);
    
    return 0;
}

int reduce_file(table_t *table, const char file_path[MAX_PATH], const int start_ip,
                const int end_ip) {
    
    
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        return 1;
    }
    bucket_t tmp;
    while (fread(&tmp, sizeof(bucket_t), 1, fp) ==1) {
        bucket_t *e = table_get(table,tmp.ip);
            if (e) {
                e->requests += tmp.requests;
            }
            else {
                bucket_t *new = bucket_init(tmp.ip);
                new->requests = tmp.requests;
                table_add(table,new);
            }
        }
    



    fclose(fp);
    return 0;
}
