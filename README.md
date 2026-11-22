# SPEC: Suspicious Account Detector

## 1. Tujuan dan batasan
- Apa yang dibuat: simulasi data transaksi sintetis + detektor rekening tidak wajar + evaluasi.
- Untuk siapa / konteks: tugas kuliah, portofolio, dan eksperimen.
- Non-goals (yang TIDAK dikerjakan): tidak memakai data nyata, tidak ada GUI, tidak ada database.

## 2. Teknologi
- Bahasa/standar: C++17, tanpa library eksternal.
- Compiler dan OS target: g++, Linux/Windows.
- Build: `make`.

## 3. Struktur folder dan aturan dependensi
### a. Struktur folder
```text
suspicious-account-detector/
├── Makefile                  # build (atau CMakeLists.txt)
├── README.md                 # tujuan, cara build, cara pakai
├── main.cpp                  # titik masuk, hanya memanggil menu utama
│
├── src/
│   ├── config.h              # konstanta: batas jumlah, daftar peran, batas nominal, label
│   ├── types.h               # definisi struct transaksi dan rekening
│   │
│   ├── data.h / data.cpp     # penyimpanan data (transaksi, rekening, penanda siap)
│   ├── util.h / util.cpp     # input angka aman, angka acak, persentase
│   ├── sorting.h / .cpp      # pengurutan
│   │
│   ├── generator.h / .cpp    # pembuatan data sintetis
│   ├── detector.h / .cpp     # aturan deteksi dan ambang batas
│   │
│   ├── evaluation.h / .cpp     # TP/FP/FN/TN, precision, recall, F1, pencarian ambang
│   │
│   ├── statistics.h / .cpp    # ringkasan, peringkat, distribusi, statistik per jam
│   ├── appreance.h / .cpp     # format tabel dan pencetakan hasil
│   │
│   └── menu.h / .cpp         # alur menu dan interaksi pengguna
│
└── tests/
    └── test_invarian.cpp     # pengecekan konsistensi data (total masuk = total keluar)
```
### b. Lapisan dependensi
| Lapisan | File | Boleh bergantung pada |
|---|---|---|
| 0 | config, types | tidak ada |
| 1 | data, util, sorting | lapisan 0 |
| 2 | generator, detector | lapisan 0-1 |
| 3 | evaluation | lapisan 0-2 |
| 4 | statistics, appreance | lapisan 0-3 |
| 5 | menu | semua lapisan di bawahnya |
| 6 | main | hanya menu |
### c. Aturan
- File di lapisan bawah tidak boleh meng-include file di lapisan atas.
- Logika (hitung) dan tampilan (cetak) dipisah: evaluasi, statistik, dan detector tidak mencetak apa pun.
- Variabel global didefinisikan sekali di `.cpp` dan dideklarasikan `extern` di `.h`.
- Header memakai `#pragma once` dan tidak berisi `using namespace std;`.

## 4. Model data

## 5. Spesifikasi generator

## 6. Spesifikasi detektor

## 7. Spesifikasi evaluasi

## 8. Antarmuka menu
### 8.1 Pohon menu

```text
Menu
0. Keluar
1. Buat Data
2. Daftar Terindikasi
   2.1 Deteksi dan Peringkat rekening
   2.2 Aliran dana
3. Statistik
   3.1 Ringkasan umum
   3.2 Peringkat pengirim
   3.3 Transaksi terbesar
   3.4 Statistik per jam
   3.5 Distribusi nominal
4. Evaluasi
   4.1 Precision dan Recall
   4.2 Lolos terdeteksi (False Negative)
   4.3 Salah tandai (False Positive)
   4.4 Ubah ambang batas
5. Test
```
### 8.2 Tiga jenis page

| Page | Isi | Contoh | Keluar dengan |
|---|---|---|---|
| Menu utama | Pilihan 0-5 | layar awal | `0` = keluar program |
| Submenu | Pilihan turunan dari menu utama | 2.x, 3.1-3.5, 4.x | `0` = kembali ke menu utama |
| Hasil | Output sebuah submenu (tabel atau ringkasan) | hasil 2.1, 3.3, 4.2 | `-1` = kembali ke submenu asal |

Menu 1 (Buat Data) dan 5 (Test) tidak punya submenu. Keduanya langsung menampilkan page hasil.
### 8.3 Aturan umum

- **Setiap pindah page, layar di-clear.** Tidak ada output yang menumpuk di atas page sebelumnya.
- Pindah page berarti: menu utama ↔ submenu ↔ hasil. Berpindah halaman (paging) dan pencarian di dalam page hasil juga me-render ulang layar (clear lalu cetak), jadi hanya ada satu tampilan aktif.
- Fungsi clear hanya ada satu (di `tampilan`), dipanggil dari satu tempat. Dilarang memanggil `system("cls")` atau `system("clear")` tersebar di banyak file.
- **Pesan error atau info ditampilkan satu kali di bagian atas page berikutnya**, bukan langsung dicetak lalu di-clear. Contoh: "Data belum ada. Pilih menu 1 dulu." muncul di atas menu utama setelah layar dirender ulang. Tanpa aturan ini, pesan akan terhapus sebelum sempat terbaca.
- Navigasi memakai stack page: `kembali` selalu ke page asal, bukan ke menu utama.
- Menu 2, 3, 4 diblokir jika data belum dibuat (menu 1).
### 8.4 Page 1: Menu utama

```text
=== SUSPICIOUS ACCOUNT DETECTOR ===
[!] Data belum ada. Pilih menu 1 dulu.      <- pesan satu kali, hanya jika ada

1. Buat Data
2. Daftar Terindikasi
3. Statistik
4. Evaluasi
5. Test
0. Keluar

Pilih: _
```
### 8.5 Page 2: Submenu (contoh menu 3)

```text
--- 3. STATISTIK ---

1. Ringkasan umum
2. Peringkat pengirim
3. Transaksi terbesar
4. Statistik per jam
5. Distribusi nominal
0. Kembali

Pilih: _
```

Submenu 2 dan 4 memakai format yang sama (daftar bernomor, `0. Kembali`).

### 8.6 Page 3: Hasil

Ada dua bentuk:

**a. Hasil berupa tabel (dengan paging dan search).** Maksimal 10 baris per halaman. Contoh menu 2.1, halaman 1 dari 3 (angka hanya ilustrasi):

```text
--- 2.1 DETEKSI DAN PERINGKAT REKENING ---
Ambang: min 20 trx masuk, rasio >= 0.70
Data 1-10 dari 27 | Halaman 1/3

No  Rek   Trx   Dana Masuk   Dana Keluar   Rasio   Status
1   R145  34    4120000      3708000       0.90    TERINDIKASI
2   R144  33    3980000      3582000       0.90    TERINDIKASI
3   R148  31    3650000      3102000       0.85    TERINDIKASI
4   R146  30    3410000      2900000       0.85    TERINDIKASI
5   R147  29    3300000      2640000       0.80    TERINDIKASI
6   R102  60    5880000      4704000       0.80    TERINDIKASI
7   R101  60    5700000      4560000       0.80    TERINDIKASI
8   R103  58    5400000      4320000       0.80    TERINDIKASI
9   R031  22    2900000      2175000       0.75    TERINDIKASI
10  R077  21    2700000      1998000       0.74    TERINDIKASI

Type 0 for search | Type page 1-3 | Type -1 back: _
```

Arti input pada baris terakhir:

| Input | Aksi |
|---|---|
| `0` | Masuk mode search |
| `1` sampai jumlah halaman | Lompat ke halaman itu |
| `-1` | Kembali ke submenu asal |
| selain itu | Pesan "Halaman tidak ada" (satu kali), page dirender ulang |

**Alur search**, jika user mengetik `0`:

```text
Type 0 for search | Type page 1-3 | Type -1 back: 0
Search: R14_
```

Setelah Enter, page dirender ulang dengan filter aktif:
```text
--- 2.1 DETEKSI DAN PERINGKAT REKENING ---
Ambang: min 20 trx masuk, rasio >= 0.70
Filter: "R14" | Data 1-5 dari 5 | Halaman 1/1

No  Rek   Trx   Dana Masuk   Dana Keluar   Rasio   Status
1   R145  34    4120000      3708000       0.90    TERINDIKASI
2   R144  33    3980000      3582000       0.90    TERINDIKASI
3   R148  31    3650000      3102000       0.85    TERINDIKASI
4   R146  30    3410000      2900000       0.85    TERINDIKASI
5   R147  29    3300000      2640000       0.80    TERINDIKASI

Type 0 for search | Type page 1-1 | Type -1 back: _
```

Aturan search:
- Mencocokkan teks pada baris (minimal kolom nomor rekening), tidak peka huruf besar/kecil, sebagian kata sudah cocok ("r14" cocok dengan R144).
- Search diterapkan pada seluruh data, bukan hanya halaman yang sedang tampil.
- Numerasi baris dan jumlah halaman dihitung ulang dari hasil filter.
- Search kosong (langsung Enter) menghapus filter dan kembali ke semua data.
- Tidak ada hasil: tampilkan `Tidak ada hasil untuk "xyz"` dan tetap di page, filter tidak diubah menjadi kosong.
- Filter hilang saat user keluar dari page hasil.

**b. Hasil berupa ringkasan (tanpa paging dan search).** Contoh 3.1 dan 4.1:

## 9. Konvensi kode

## 10. Invarian dan kriteria selesai
- Σ masuk = Σ keluar = Σ nominal.
- tp + fp + fn + tn == jumlah rekening.
- Tidak ada transfer ke diri sendiri.
- Program tidak crash pada input non-angka atau EOF.
