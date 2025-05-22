#include "fs.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

int fs_format() {
    int fd = open(DISK_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        perror("Disk dosyasi olusturulamadi.");
        return -1;
    }

    char* empty = calloc(1, DISK_SIZE);
    if (!empty) {
        perror("Bellek ayirma hatasi.");
        close(fd);
        return -1;
    }

    if (write(fd, empty, DISK_SIZE) != DISK_SIZE) {
        perror("Disk yazma hatasi.");
        free(empty);
        close(fd);
        return -1;
    }

    free(empty);
    log_operation("Disk formatlandi.");
    close(fd);
    return 0;
}

int fs_create(char* filename) {
    if (strlen(filename) >= FILENAME_MAX_LEN) {
        printf("HATA: Dosya adi çok uzun.\n");
        return -1;
    }

    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    lseek(fd, 0, SEEK_SET);
    read(fd, entries, sizeof(entries));

    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            printf("HATA: Bu isimde bir dosya zaten var.\n");
            close(fd);
            return -1;
        }
    }

    int empty_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) {
            empty_index = i;
            break;
        }
    }

    if (empty_index == -1) {
        printf("HATA: Maksimum dosya sayisina ulasildi.\n");
        close(fd);
        return -1;
    }

    FileEntry* e = &entries[empty_index];
    strcpy(e->name, filename);
    e->size = 0;
    e->start = METADATA_SIZE + empty_index * 1024; 
    e->used = 1;

    time_t now = time(NULL);
    strftime(e->created, sizeof(e->created), "%F %T", localtime(&now));

    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));

    char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya olusturuldu: %s", filename);
log_operation(logbuf);

    close(fd);
    return 0;
}

void fs_list() {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilamadi");
        return;
    }

    FileEntry entries[MAX_FILES];
    lseek(fd, 0, SEEK_SET);
    read(fd, entries, sizeof(entries));

    printf("\n%-32s  %-6s  %-8s  %-20s\n", "Dosya Adi", "Boyut", "Adres", "Olusturulma");
    printf("-------------------------------------------------------------------------------\n");

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used) {
            printf("%-32s  %-6d  %-8d  %-20s\n",
                   entries[i].name,
                   entries[i].size,
                   entries[i].start,
                   entries[i].created);
            found = 1;
        }
    }

    if (!found)
        printf("Kayitli dosya bulunamadi.\n");


    log_operation("Dosyalar listelendi.");

    close(fd);
}

int fs_delete(char* filename) {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk dosyasi acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    lseek(fd, 0, SEEK_SET);
    read(fd, entries, sizeof(entries));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            entries[i].used = 0;
            found = 1;
            break;
        }
    }

    if (!found) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
        close(fd);
        return -1;
    }

    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
    char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya silindi: %s", filename);
log_operation(logbuf);

    close(fd);

    return 0;
}

int fs_write(char* filename, char* data, int size) {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            found = 1;

           
            lseek(fd, entries[i].start, SEEK_SET);
            if (write(fd, data, size) != size) {
                perror("Veri yazilamadi");
                close(fd);
                return -1;
            }

           
            entries[i].size = size;

            lseek(fd, 0, SEEK_SET);
            write(fd, entries, sizeof(entries));
            char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Veri yazildi: %s (%d byte)", filename, size);
log_operation(logbuf);


            break;
        }
    }

    if (!found) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
    }

    close(fd);
    return found ? 0 : -1;
}
int fs_read(char* filename, int offset, int size, char* buffer) {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilmadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            found = 1;

            if (offset >= entries[i].size) {
                printf("HATA: Offset dosya boyutunun disinda.\n");
                close(fd);
                return -1;
            }

            if (offset + size > entries[i].size) {
                size = entries[i].size - offset;
            }

            lseek(fd, entries[i].start + offset, SEEK_SET);
            int okunan = read(fd, buffer, size);
            buffer[okunan] = '\0';

            break;
        }
    }

    if (!found) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya okundu: %s (offset: %d, size: %d)", filename, offset, size);
log_operation(logbuf);

    close(fd);
    return found ? 0 : -1;
}

int fs_rename(char* old_name, char* new_name) {
    if (strlen(new_name) >= FILENAME_MAX_LEN) {
        printf("HATA: Yeni dosya adi çok uzun.\n");
        return -1;
    }

    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int old_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used) {
            if (strcmp(entries[i].name, new_name) == 0) {
                printf("HATA: '%s' adli dosya zaten var.\n", new_name);
                close(fd);
                return -1;
            }

            if (strcmp(entries[i].name, old_name) == 0) {
                old_index = i;
            }
        }
    }

    if (old_index == -1) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", old_name);
        close(fd);
        return -1;
    }

    strcpy(entries[old_index].name, new_name);

    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
    char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya adi degistirildi: %s -> %s", old_name, new_name);
log_operation(logbuf);

    close(fd);

    return 0;
}

int fs_exists(char* filename) {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilamadi");
        return 0;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

   for (int i = 0; i < MAX_FILES; i++) {
    if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
        char logbuf[64];
        snprintf(logbuf, sizeof(logbuf), "Dosya var mi kontrol edildi: %s -> EVET", filename);
        log_operation(logbuf);

        close(fd);
        return 1;
    }
}

char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya var mi kontrol edildi: %s -> HAYIR", filename);
log_operation(logbuf);
close(fd);
return 0;

    
}

int fs_size(char* filename) {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
                char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya boyutu sorgulandi: %s (boyut: %d)", filename, entries[i].size);
log_operation(logbuf);
            close(fd);
            return entries[i].size;
        }
    }




    close(fd);
    return -1;
}

int fs_append(char* filename, char* data, int size) {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            found = 1;

            // Gerçek veri sonuna gider ve eski verinin en son dolu byte'ını bulur.
            int veri_basi = entries[i].start;
            int max_okunabilir = entries[i].size;
            char* tmp = calloc(1, max_okunabilir + 1);
            lseek(fd, veri_basi, SEEK_SET);
            read(fd, tmp, max_okunabilir);

            // \0 olmadan veri uzunluğunu bulur.
            int gercek_boyut = 0;
            while (gercek_boyut < max_okunabilir && tmp[gercek_boyut] != '\0')
                gercek_boyut++;

            free(tmp);

            // Append işlemini gerçek verinin sonuna yapar.
            int write_pos = veri_basi + gercek_boyut;
            lseek(fd, write_pos, SEEK_SET);
            write(fd, data, size);

            
            entries[i].size = gercek_boyut + size;
            lseek(fd, 0, SEEK_SET);
            write(fd, entries, sizeof(entries));
            break;
        }
    }

    if (!found) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Veri eklendi: %s (%d byte)", filename, size);
log_operation(logbuf);

    close(fd);
    return found ? 0 : -1;
}

int fs_truncate(char* filename, int new_size) {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            found = 1;
            int old_size = entries[i].size;

            if (new_size < old_size) {
                // küçült işleminde sadece "size" güncellenir
                entries[i].size = new_size;
            } else if (new_size > old_size) {
                // büyüt işleminde sonuna \0'lar eklenir
                int fark = new_size - old_size;
                char* zeros = calloc(1, fark);
                lseek(fd, entries[i].start + old_size, SEEK_SET);
                write(fd, zeros, fark);
                free(zeros);
                entries[i].size = new_size;
            }

           
            lseek(fd, 0, SEEK_SET);
            write(fd, entries, sizeof(entries));
            break;
        }
    }

    if (!found) {
        printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya kesildi: %s (yeni boyut: %d)", filename, new_size);
log_operation(logbuf);

    close(fd);
    return found ? 0 : -1;
}

int fs_copy(char* src_filename, char* dest_filename) {

    if (fs_exists(dest_filename)) {
        printf("HATA: '%s' adli hedef dosya zaten var.\n", dest_filename);
        return -1;
    }

   
    if (!fs_exists(src_filename)) {
        printf("HATA: '%s' adli kaynak dosya bulunamadi.\n", src_filename);
        return -1;
    }

    int size = fs_size(src_filename);
    if (size <= 0) {
        printf("Uyari: Kaynak dosya bos.\n");
    }

    char* buffer = malloc(size + 1);
    if (!buffer) {
        perror("Bellek ayrilirken hata");
        return -1;
    }

    if (fs_read(src_filename, 0, size, buffer) != 0) {
        free(buffer);
        return -1;
    }

    if (fs_create(dest_filename) != 0) {
        free(buffer);
        return -1;
    }

    if (fs_write(dest_filename, buffer, size) != 0) {
        free(buffer);
        return -1;
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya kopyalandi: %s -> %s", src_filename, dest_filename);
log_operation(logbuf);

    free(buffer);
    return 0;
}

int fs_mv(char* old_path, char* new_path) {
    if (strlen(new_path) >= FILENAME_MAX_LEN) {
        printf("HATA: Yeni yol cok uzun.\n");
        return -1;
    }

    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return -1;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    int old_index = -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, new_path) == 0) {
            printf("HATA: '%s' zaten var.\n", new_path);
            close(fd);
            return -1;
        }

        if (entries[i].used && strcmp(entries[i].name, old_path) == 0) {
            old_index = i;
        }
    }

    if (old_index == -1) {
        printf("HATA: '%s' bulunamadi.\n", old_path);
        close(fd);
        return -1;
    }

    strcpy(entries[old_index].name, new_path);

    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
    char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya tasindi: %s -> %s", old_path, new_path);
log_operation(logbuf);

    close(fd);

    return 0;
}

void fs_defragment() {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return;
    }

    FileEntry entries[MAX_FILES];
    lseek(fd, 0, SEEK_SET);
    read(fd, entries, sizeof(entries));

    
    int current_offset = METADATA_SIZE;

    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) continue;

        
        if (entries[i].start == current_offset) {
            current_offset += entries[i].size;
            continue;
        }

        // Eski konumdan veri okur
        char* buffer = malloc(entries[i].size);
        lseek(fd, entries[i].start, SEEK_SET);
        read(fd, buffer, entries[i].size);

        // Yeni konuma yazar
        lseek(fd, current_offset, SEEK_SET);
        write(fd, buffer, entries[i].size);

       
        entries[i].start = current_offset;
        current_offset += entries[i].size;

        free(buffer);
    }

    
    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
log_operation("Disk defragment edildi.");

    close(fd);
    printf("Disk basariyla defragment edildi.\n");
}

 void fs_check_integrity() {
    int fd = open(DISK_FILE, O_RDWR);
    if (fd == -1) {
        perror("Disk acilamadi");
        return;
    }

    FileEntry entries[MAX_FILES];
    lseek(fd, 0, SEEK_SET);
    read(fd, entries, sizeof(entries));

    FILE* log = fopen("integrity.log", "w");
    if (!log) {
        perror("integrity.log dosyasi acilamadi");
        close(fd);
        return;
    }

    int sorun_var = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) continue;

        int start = entries[i].start;
        int end = entries[i].start + entries[i].size;

        if (start < METADATA_SIZE || end > DISK_SIZE || entries[i].size < 0) {
            fprintf(log, "UYARI: %s bozuk - gecersiz sinirlar (%d ~ %d)\n",
                    entries[i].name, start, end);
            entries[i].used = 0; 
            sorun_var = 1;
            continue;
        }

        for (int j = 0; j < MAX_FILES; j++) {
            if (i == j || !entries[j].used) continue;

            int other_start = entries[j].start;
            int other_end = entries[j].start + entries[j].size;

            // çakışma varsa
            if ((start < other_end && end > other_start)) {
                fprintf(log, "UYARI: %s ve %s blok cakismasi\n",
                        entries[i].name, entries[j].name);
                entries[i].used = 0;
                sorun_var = 1;
                break;
            }
        }
    }

    
    lseek(fd, 0, SEEK_SET);
    write(fd, entries, sizeof(entries));
    log_operation("Disk butunlugu kontrol edildi.");

    close(fd);
    fclose(log);

    if (sorun_var)
        printf("Tutarsizlik(lar) bulundu. Detaylar integrity.log dosyasina yazildi.\n");
    else
        printf("Disk butunlugu basarili. Her sey normal.\n");
}

int fs_backup(char* backup_filename) {
    int src = open(DISK_FILE, O_RDONLY);
    if (src == -1) {
        perror("Disk (disk.sim) acilamadi");
        return -1;
    }

    int dest = open(backup_filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest == -1) {
        perror("Yedek dosyasi olusturulamadi");
        close(src);
        return -1;
    }

    char* buffer = malloc(DISK_SIZE);
    if (!buffer) {
        perror("Bellek hatasi");
        close(src);
        close(dest);
        return -1;
    }

    if (read(src, buffer, DISK_SIZE) != DISK_SIZE) {
        perror("Disk okuma hatasi");
        free(buffer);
        close(src);
        close(dest);
        return -1;
    }

    if (write(dest, buffer, DISK_SIZE) != DISK_SIZE) {
        perror("Yedek yazma hatasi");
        free(buffer);
        close(src);
        close(dest);
        return -1;
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Yedek alindi: %s", backup_filename);
log_operation(logbuf);

    free(buffer);
    close(src);
    close(dest);

    printf("Yedek basariyla '%s' dosyasina alindi.\n", backup_filename);
    return 0;
}

int fs_restore(char* backup_filename) {
    int backup = open(backup_filename, O_RDONLY);
    if (backup == -1) {
        perror("Yedek dosyasi acilamadi");
        return -1;
    }

    int dest = open(DISK_FILE, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (dest == -1) {
        perror("disk.sim üzerine yazilamadi");
        close(backup);
        return -1;
    }

    char* buffer = malloc(DISK_SIZE);
    if (!buffer) {
        perror("Bellek ayrilirken hata");
        close(backup);
        close(dest);
        return -1;
    }

    if (read(backup, buffer, DISK_SIZE) != DISK_SIZE) {
        perror("Yedek okuma hatasi");
        free(buffer);
        close(backup);
        close(dest);
        return -1;
    }

    if (write(dest, buffer, DISK_SIZE) != DISK_SIZE) {
        perror("Disk geri yukleme hatasi");
        free(buffer);
        close(backup);
        close(dest);
        return -1;
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Yedek geri yuklendi: %s", backup_filename);
log_operation(logbuf);

    free(buffer);
    close(backup);
    close(dest);

    printf("'%s' yedeği disk.sim dosyasina geri yuklendi.\n", backup_filename);
    return 0;
}

void fs_cat(char* filename) {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilamadi");
        return;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    for (int i = 0; i < MAX_FILES; i++) {
        if (entries[i].used && strcmp(entries[i].name, filename) == 0) {
            int size = entries[i].size;
            char* buffer = malloc(size + 1);
            if (!buffer) {
                perror("Bellek ayrilirken hata");
                close(fd);
                return;
            }

            lseek(fd, entries[i].start, SEEK_SET);
            read(fd, buffer, size);
            buffer[size] = '\0'; 

            printf("=== %s ===\n%s\n", filename, buffer);
            free(buffer);
            char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosya goruntulendi: %s", filename);
log_operation(logbuf);

            close(fd);
            return;
        }
    }

    printf("HATA: '%s' adli dosya bulunamadi.\n", filename);
    close(fd);
}
void fs_diff(char* file1, char* file2) {
    int fd = open(DISK_FILE, O_RDONLY);
    if (fd == -1) {
        perror("Disk acilamadi");
        return;
    }

    FileEntry entries[MAX_FILES];
    read(fd, entries, sizeof(entries));

    FileEntry *f1 = NULL, *f2 = NULL;

    for (int i = 0; i < MAX_FILES; i++) {
        if (!entries[i].used) continue;
        if (strcmp(entries[i].name, file1) == 0) f1 = &entries[i];
        if (strcmp(entries[i].name, file2) == 0) f2 = &entries[i];
    }

    if (!f1 || !f2) {
        printf("HATA: Dosyalardan biri veya ikisi bulunamadi.\n");
        close(fd);
        return;
    }

    if (f1->size != f2->size) {
        printf("Dosyalar farkli boyutta (%d vs %d).\n", f1->size, f2->size);
        close(fd);
        return;
    }

    char* buf1 = malloc(f1->size);
    char* buf2 = malloc(f2->size);

    lseek(fd, f1->start, SEEK_SET);
    read(fd, buf1, f1->size);

    lseek(fd, f2->start, SEEK_SET);
    read(fd, buf2, f2->size);

    int fark_yok = 1;
    for (int i = 0; i < f1->size; i++) {
        if (buf1[i] != buf2[i]) {
            printf("Fark: byte %d → '%c' vs '%c'\n", i, buf1[i], buf2[i]);
            fark_yok = 0;
        }
    }

    if (fark_yok) {
        printf("Dosyalar birebir ayni.\n");
    }
char logbuf[64];
snprintf(logbuf, sizeof(logbuf), "Dosyalar karsilasitirildi: %s <-> %s", file1, file2);
log_operation(logbuf);

    free(buf1);
    free(buf2);
    close(fd);
}

void log_operation(const char* message) {
    FILE* log = fopen("fs.log", "a");
    if (!log) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    char zaman[32];
    strftime(zaman, sizeof(zaman), "%F %T", t); 

    fprintf(log, "[%s] %s\n", zaman, message);
    fclose(log);
}

void fs_log() {
    FILE* log = fopen("fs.log", "r");
    if (!log) {
        printf("Log dosyasi bulunamadi.\n");
        return;
    }

    char satir[256];
    printf("=== islem Logu ===\n");
    while (fgets(satir, sizeof(satir), log)) {
        printf("%s", satir);
    }
    fclose(log);
}
