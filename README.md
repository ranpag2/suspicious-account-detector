# Suspicious Account Detector

Program konsol C++ yang membuat data transaksi rekening sintetis, mendeteksi
rekening yang tidak wajar, lalu mengevaluasi seberapa baik deteksinya.

## Gambaran besar

Proyek dikerjakan 3 orang dengan pembagian ala tim keamanan siber:

| Tim | Tugas | Memiliki | Tidak boleh |
|---|---|---|---|
| Red | Generator | pembuatan dataset dan kunci jawaban | melihat cara kerja detektor |
| Blue | Detektor | aturan deteksi dan ambangnya | melihat cara kerja generator dan kunci jawaban |
| Purple | Evaluasi dan menu | pengukuran hasil, antarmuka, integrasi | mengubah isi generator atau detektor |

Red dan Blue sengaja tidak saling tahu rancangan dan rumus pihak lain, sehingga
hasil evaluasi Purple mencerminkan kemampuan deteksi, bukan kecocokan dua
bagian yang ditulis oleh orang yang sama.

Yang dirahasiakan adalah **rumus, ambang, dan parameter**. Nama, struktur data,
dan kontrak antarmuka terbuka untuk semua dan ada di dokumen bersama.

## Siklus program

```text
        ┌──────────┐   seed    ┌────────────┐
 User ─▶│ 1. Buat  │──────────▶│  Dataset   │  status: BELUM
        │   Data   │   (Red)   │  + kunci   │
        └──────────┘           └─────┬──────┘
                                     │ user memilih dataset aktif
                                     ▼
                              ┌────────────┐
                              │ 2. Deteksi │  (Blue) menimpa hasil di dataset
                              └─────┬──────┘  status: SUDAH
                                    ▼
                  ┌─────────────────┴─────────────────┐
                  ▼                                   ▼
          ┌──────────────┐                    ┌──────────────┐
          │ 3. Statistik │                    │ 4. Evaluasi  │ (Purple)
          └──────────────┘                    └──────────────┘
                  └────── kembali ke menu utama ──────┘
```

Langkah per langkah:

1. **Buat data.** User memasukkan seed. Generator membuat satu dataset beserta
   kunci jawabannya, dan menambahkannya ke daftar. Jumlah dataset tidak dibatasi.
2. **Pilih dataset aktif.** Semua menu bekerja pada satu dataset yang sedang dipilih.
3. **Deteksi.** Detektor membaca transaksi dataset aktif dan menimpa hasil
   deteksi langsung di dataset itu. Transaksi tidak pernah diubah.
4. **Lihat hasil.** Daftar terindikasi, statistik, dan evaluasi membaca hasil
   yang sudah tersimpan.
5. **Ulangi.** User bisa membuat dataset baru atau mengganti ambang dan
   mendeteksi ulang.

Aturan urutan:
- Menu 2, 3, dan 4 tidak bisa dibuka sebelum ada dataset.
- Menu 4 (Evaluasi) tidak bisa dibuka pada dataset yang statusnya BELUM.

## Tampilan program

Layar selalu di-clear saat berpindah page. Ada tiga jenis page:

| Page | Isi | Kembali dengan |
|---|---|---|
| Menu utama | pilihan 1-5 | `0` keluar |
| Submenu | pilihan turunan | `0` kembali |
| Hasil | tabel (maks. 10 baris, dengan paging dan search) atau ringkasan | `-1` kembali |

Detail lengkap, contoh layar, dan aturan input ada di [docs/menu.md](docs/menu.md).

## Alur pengerjaan

1. Semua anggota menyepakati kontrak bersama (bagian 1-4, 8-10) lebih dulu.
2. Red, Blue, dan Purple mengerjakan bagiannya masing-masing tanpa membuka
   folder atau dokumen privat pihak lain.
3. Purple mengintegrasikan semuanya lewat antarmuka yang sudah disepakati.
4. Purple menjalankan evaluasi, lalu membagikan umpan balik terbatas ke tiap
   pihak (contoh: "recall rendah pada dataset tertentu"), tanpa membuka rumus
   pihak lain.
5. Red dan Blue memperbaiki bagiannya, lalu putaran berikutnya dimulai.

## Peta dokumen

| Bagian | Dokumen | Dibaca oleh |
|---|---|---|
| 1-2, 9 | README ini | semua |
| 3 | [docs/structure.md](docs/structure.md) | semua |
| 4 | [docs/model.md](docs/model.md) | semua |
| 5 Generator | docs/red/ | Red (dan Purple) |
| 6 Detektor | docs/blue/ | Blue (dan Purple) |
| 7 Evaluasi | docs/purple/ | Purple |
| 8 | [docs/menu.md](docs/menu.md) | semua |

## 5. Spesifikasi generator
Antarmuka saja: menerima seed, menambahkan satu dataset. Detail di docs/red/.

## 6. Spesifikasi detektor
Antarmuka saja: menerima dataset, mengisi hasil deteksi. Detail di docs/blue/.

## 7. Spesifikasi evaluasi
Antarmuka saja: membandingkan hasil deteksi dengan kunci jawaban. Detail di docs/purple/.

## 9. Konvensi kode
