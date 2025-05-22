
# 📁 Basit Dosya Sistemi Simülatörü

Bu proje, bir sanal disk üzerinde temel dosya işlemlerini gerçekleştirebilen basit bir dosya sistemi simülasyonudur. Disk verisi `disk.sim` adlı bir dosyada saklanır. Tüm işlemler bu sanal diskte gerçekleştirilir.

## 🛠 Derleme

Makefile sayesinde basitce mingw32-make yaparak programı derleyebilirsiniz.

## 📜 Kullanım Menüsü

Program çalıştırıldığında aşağıdaki gibi bir menü ile karşılaşırsınız:

| No | İşlem             | Açıklama |
|----|-------------------|----------|
| 1  | Format            | Sanal diski sıfırlar ve temizler. |
| 2  | Olustur           | Yeni bir dosya oluşturur. |
| 3  | Listele           | Tüm dosyaları listeler. |
| 4  | Sil               | Bir dosyayı siler. |
| 5  | Yaz               | Belirtilen dosyaya veri yazar. |
| 6  | Oku               | Dosyadan belirli konumdan veri okur. |
| 7  | Yeniden Adlandir  | Dosya adını değiştirir. |
| 8  | Var mi?           | Dosyanın var olup olmadığını kontrol eder. |
| 9  | Boyut             | Dosya boyutunu gösterir. |
| 10 | Ekle              | Dosya sonuna veri ekler (append). |
| 11 | Kes               | Dosyayı istenen byte sayısına indirir veya genişletir. |
| 12 | Kopyala           | Dosyayı başka bir dosya adıyla kopyalar. |
| 13 | Tasi              | Dosya adını veya yolunu değiştirerek taşır. |
| 14 | Defragment        | Disk üzerindeki boşlukları düzenler. |
| 15 | Butunluk Kontrolu | Disk bütünlüğünü kontrol eder ve sorunları `integrity.log` dosyasına yazar. |
| 16 | Yedekle           | Diskin tamamını yedekler (`.backup` dosyası). |
| 17 | Geri Yukle        | Daha önce alınmış bir yedeği geri yükler. |
| 18 | Goster (cat)      | Dosya içeriğini ekrana yazdırır. |
| 19 | Karsilastir (diff)| İki dosyayı byte düzeyinde karşılaştırır. |
| 20 | Loglari Goruntule | Yapılan tüm işlemlerin `fs.log` içeriğini gösterir. |
| 0  | Cikis             | Programdan çıkar. |

---

## 📁 Dosya Yapısı

--bin
    └── Simplefs.exe         → Derlenmiş çalıştırılabilir dosya.
--include
    └── fs.h                 → Başlık (header) dosyası, tüm fonksiyon prototiplerini içerir.
--lib
    └── fs.o                 → `fs.c` dosyasının derlenmiş hali (nesne dosyası).
    └── main.o               → `main.c` dosyasının derlenmiş hali.
--src
    └── fs.c                 → Dosya sistemi işlemlerini içeren kaynak dosya.
    └── main.c               → Kullanıcı menüsünü ve arayüzü sağlayan ana kaynak dosya.
--disk.sim                   → Sanal disk dosyası (formatlanmış disk içeriği burada tutulur).
--fs.log                     → Yapılan işlemlerin kaydedildiği günlük (log) dosyası.
--integrity.log              → Disk bütünlük kontrolü sırasında tespit edilen sorunlar burada listelenir.
--makefile                   → Derleme işlemlerini otomatikleştirmek için kullanılan yapılandırma dosyası.

## 📝 Loglama

Tüm işlemler `fs.log` dosyasına tarih-saat damgası ile birlikte kaydedilir. Disk bütünlük sorunları ise `integrity.log` dosyasına yazılır.
