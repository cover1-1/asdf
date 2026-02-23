#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"
#include "./include/map.h"

bucket_t *bucket_init(const char ip[IP_LEN]) {
    bucket_t *bucket = malloc(sizeof(bucket_t));
    strncpy(bucket->ip,ip,IP_LEN);
    bucket->ip[IP_LEN-1]='\0';
    bucket->next=NULL;
    bucket->requests=0;
    return bucket;
}

table_t *table_init() {
    table_t *table = calloc(1,sizeof(table_t));
    
    return table;
}

void table_print(const table_t *table) {
    for (int i =0; i<TABLE_LEN;i++) {
        bucket_t *curr = table->buckets[i];
        while(curr) {
            printf("%s - %d\n", curr->ip, curr->requests);
            curr = curr->next;
        }
    }
    return;
}

void table_free(table_t *table) {
    for (int i = 0; i<TABLE_LEN; i++) {
        bucket_t *curr = table->buckets[i];
        while (curr) {
            bucket_t *next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(table);
}

int table_add(table_t *table, bucket_t *bucket) {
    int idx = hash_ip(bucket->ip);
    bucket->next = table->buckets[idx];
    table->buckets[idx] = bucket;
    return 0;
}

bucket_t *table_get(table_t *table, const char ip[IP_LEN]) {
    int idx = hash_ip(ip);
    bucket_t *curr = table->buckets[idx];
    while (curr) {
        if (strcmp(curr->ip, ip)==0) {
            return curr;
        } 
        curr = curr->next;
    }
    return NULL;
}

int hash_ip(const char ip[IP_LEN]) {
    int sum = 0;
    for (int i =0;ip[i] != '\0';i++) {
        sum+= ip[i];
    }
    return sum % TABLE_LEN;
}

int table_to_file(table_t *table, const char out_file[MAX_PATH]) {
    FILE *fp = fopen(out_file, "wb");
    for (int i=0;i<TABLE_LEN; i++) {
        bucket_t *curr = table->buckets[i];
        while (curr) {
            if (fwrite(curr, sizeof(bucket_t), 1, fp) !=1) {
                fclose(fp);
                return -1;
            }
            curr = curr->next;
        }
    }
    fclose(fp);
    return 0;
}

table_t *table_from_file(const char in_file[MAX_PATH]) {
    FILE *fp = fopen(in_file, "rb");
    table_t *table = table_init();
    bucket_t tmp;
    while (fread(&tmp, sizeof(bucket_t),1,fp)==1) {
        bucket_t *new = bucket_init(tmp.ip);
        new->requests = tmp.requests;
        table_add(table,new);
    }
    fclose(fp);
    return table;
}
