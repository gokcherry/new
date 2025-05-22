#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fs.h"

#define BUFFER_SIZE 1024
#define FILE_NAME_SIZE 32
#define INLINE static inline

static char dosya_adi[FILE_NAME_SIZE];
static char yeni_ad[FILE_NAME_SIZE];
static char veri[BUFFER_SIZE];
static char okunanVeri[BUFFER_SIZE];
static int offset, boyut;

INLINE void show_menu(void) {
    static const char *menu = "\n1. Format\n2. Olustur\n3. Listele\n"
                             "4. Sil\n5. Yaz\n6. Oku\n7. Yeniden Adlandir\n"
                             "8. Var mi?\n9. Boyut\n10. Ekle\n11. Kes\n"
                             "12. Kopyala\n13. Tasi\n14. Defragment\n"
                             "15. Butunluk Kontrolu\n16. Yedekle\n"
                             "17. Geri Yukle\n18. Goster (cat)\n"
                             "19. Karsilastir (diff)\n20. Loglari Goruntule\n"
                             "0. Cikis\nSecim: ";
    printf("%s", menu);
}


int main() {
    int secim;

    while (1) {
        show_menu();
        if (scanf("%d", &secim) != 1) {
            while (getchar() != '\n'); // Buffer temizleme
            continue;
        }
        getchar(); 

        switch (secim) {
            case 1:
                if (fs_format() == 0)
                    printf("Disk formatlandi.\n");
                break;
            case 2:
                printf("Dosya adi gir: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                if (fs_create(dosya_adi) == 0)
                    printf("Dosya olusturuldu.\n");
                break;
            case 3:
                fs_list();
                break;
            case 4:
                printf("Silinecek dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                if (fs_delete(dosya_adi) == 0)
                    printf("Dosya silindi.\n");
                break;
            
                 case 5:
                printf("Yazilacak dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Yazilacak veri: ");
                fgets(veri, sizeof(veri), stdin);
                 boyut = strlen(veri);
                if (veri[boyut - 1] == '\n') veri[--boyut] = 0;

                if (fs_write(dosya_adi, veri, boyut) == 0)
                    printf("Veri yazildi.\n");
                break;

                            case 6:
                printf("Okunacak dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Offset (baslangic): ");
                scanf("%d", &offset);
                printf("Okunacak byte sayisi: ");
                scanf("%d", &boyut);
                getchar(); 

                if (fs_read(dosya_adi, offset, boyut, okunanVeri) == 0) {
                    printf("Okunan veri: %s\n", okunanVeri);
                }
                break;

                case 7:
                
                printf("Eski adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Yeni adi: ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                if (fs_rename(dosya_adi, yeni_ad) == 0)
                    printf("Dosya adi degistirildi.\n");
                break;

                            case 8:
                printf("Kontrol edilecek dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                if (fs_exists(dosya_adi))
                    printf("'%s' dosyasi mevcut.\n", dosya_adi);
                else
                    printf("'%s' dosyasi bulunamadi.\n", dosya_adi);
                break;

                            case 9:
                printf("Boyutu alinacak dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                int boyut = fs_size(dosya_adi);
                if (boyut >= 0)
                    printf("'%s' dosyasinin boyutu: %d byte\n", dosya_adi, boyut);
                else
                    printf("Dosya bulunamadi.\n");
                break;
            case 10:
                printf("Veri eklenecek dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Eklenecek veri: ");
                fgets(veri, sizeof(veri), stdin);
                boyut = strlen(veri);
                if (veri[boyut - 1] == '\n') veri[--boyut] = 0;

                if (fs_append(dosya_adi, veri, boyut) == 0)
                    printf("Veri eklendi.\n");
                break;

                            case 11:
                printf("Kesilecek dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Yeni boyut (byte): ");
                scanf("%d", &boyut);
                getchar(); 

                if (fs_truncate(dosya_adi, boyut) == 0)
                    printf("Dosya basariyla kesildi.\n");
                break;

                            case 12:
                printf("Kaynak dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Hedef dosya adi: ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                if (fs_copy(dosya_adi, yeni_ad) == 0)
                    printf("Dosya basariyla kopyalandi.\n");
                break;

                            case 13:
                printf("Tasinacak dosya (eski yol): ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("Yeni yol: ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                if (fs_mv(dosya_adi, yeni_ad) == 0)
                    printf("Dosya basariyla tasindi/yeniden adlandirildi.\n");
                break;

                            case 14:
                fs_defragment();
                break;

                            case 15:
                fs_check_integrity();
                break;

                            case 16:
                printf("Yedek dosya adi (ornek: disk.sim.backup): ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                if (fs_backup(yeni_ad) == 0)
                    printf("Disk yedegi olusturuldu.\n");
                break;
            case 17:
                printf("Yuklenecek yedek dosya adi: ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                if (fs_restore(yeni_ad) == 0)
                    printf("Disk geri yuklendi.\n");
                break;

                           case 18:
                printf("icerigi gosterilecek dosya: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;
                fs_cat(dosya_adi);
                break;

                             case 19:
                printf("1. dosya adi: ");
                fgets(dosya_adi, sizeof(dosya_adi), stdin);
                dosya_adi[strcspn(dosya_adi, "\n")] = 0;

                printf("2. dosya adi: ");
                fgets(yeni_ad, sizeof(yeni_ad), stdin);
                yeni_ad[strcspn(yeni_ad, "\n")] = 0;

                fs_diff(dosya_adi, yeni_ad);
                break;

                            case 20:
                fs_log();
                break;

            case 0:
                return 0;
        }
    }
}
