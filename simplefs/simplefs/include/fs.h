#ifndef FS_H
#define FS_H

#define DISK_FILE "disk.sim"
#define DISK_SIZE (1024 * 1024)
#define METADATA_SIZE 4096
#define MAX_FILES 64
#define FILENAME_MAX_LEN 32

typedef struct {
    char name[FILENAME_MAX_LEN];
    int size;
    int start;
    char created[20];
    int used;
} FileEntry;

int fs_format();
int fs_create(char* filename);
void fs_list();
int fs_delete(char* filename);
int fs_write(char* filename, char* data, int size);
int fs_read(char* filename, int offset, int size, char* buffer);
int fs_rename(char* old_name, char* new_name);
int fs_exists(char* filename);
int fs_size(char* filename);
int fs_append(char* filename, char* data, int size);
int fs_truncate(char* filename, int new_size);
int fs_copy(char* src_filename, char* dest_filename);
int fs_mv(char* old_path, char* new_path);
void fs_defragment();
void fs_check_integrity();
int fs_backup(char* backup_filename);
int fs_restore(char* backup_filename);
void fs_cat(char* filename);
void fs_diff(char* file1, char* file2);
void log_operation(const char* message);
void fs_log();

#endif