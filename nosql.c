#include "nosql.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

int get_key(db_table *pl, const char *key, db_entry *entry) {
    const void *matches[2];
    int result;

    assert(pl && key && entry);
    result = cb_find_prefix(&pl->trie, key, strlen(key)+1, matches, 2, 0);
    printf("found %d matches for %s\n", result, key);
    if (result==1) {
        cb_get_kv(matches[0], entry, sizeof(db_entry));
        return 200;
    } else {
        return 404;
    }
}

static void insert_key(critbit_tree *trie, const char *key, size_t keylen, db_entry *entry) {
    char data[MAXENTRY+MAXKEY];
    size_t len;

    len = cb_new_kv(key, keylen, entry, sizeof(db_entry), data);
    cb_insert(trie, data, len);    
}

void set_key(db_table *pl, const char *key, db_entry *entry) {
    size_t len = strlen(key);

    fwrite(&len, sizeof(len), 1, pl->binlog);
    fwrite(key, len, 1, pl->binlog);
    fwrite(&entry->size, sizeof(entry->size), 1, pl->binlog);
    fwrite(entry->data, entry->size, 1, pl->binlog);
    fflush(pl->binlog);
    
    insert_key(&pl->trie, key, len, entry);
}

void read_log(db_table *pl, const char *logfile) {
    int fd = open(logfile, O_RDONLY);
    if (fd>0) {
        off_t fsize;
        void *logdata;
        fsize = lseek(fd, 0, SEEK_END);
        logdata = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
        if (logdata) {
            const char *data = (const char *)logdata;
            printf("reading %u bytes from binlogs\n", (unsigned)fsize);
            while(data-fsize<(const char *)logdata) {
                db_entry entry;
                const char *key;
                size_t len;
                
                len = *(size_t *)data;
                data += sizeof(size_t);
                key = (const char *)data;
                data += len;
                entry.size = *(size_t *)data;
                data += sizeof(size_t);
                entry.data = memcpy(malloc(entry.size), (void *)data, entry.size);
                data += entry.size;
                insert_key(&pl->trie, key, len, &entry);
            }
            munmap(logdata, fsize);
        }
    }
}
