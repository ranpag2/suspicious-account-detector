## 8. Antarmuka menu
### 8.1 Pohon menu

```text
Menu
0. Keluar
1. Buat Data
2. Deteksi
3. Statistik
   3.1 Ringkasan umum
   3.2 Peringkat terindikasi
   3.3 Peringkat pengirim
   3.4 Transaksi terbesar
   3.5 Statistik per jam
   3.6 Distribusi nominal
   3.7 Aliran dana
4. Evaluasi
   4.1 Precision dan Recall
   4.2 Lolos terdeteksi (False Negative)
   4.3 Salah tandai (False Positive)
   4.4 Ubah ambang batas
5. Test
```

### 8.2 Empat jenis page

| Page | Isi | Contoh | Keluar dengan |
|---|---|---|---|
| Menu utama | Pilihan 0-5 | layar awal | `0` = keluar program |
| Submenu | Pilihan turunan dari menu utama | 3.1-3.7, 4.x | `0` = kembali ke menu utama |
| Pilih dataset | List dari dataset yang ada pada file `dataset.cpp` | ketika memilih 2, 3.3, 4.2 | `0` = kembali ke submenu asal |
| Hasil | Output sebuah dataset yang dipilih (tabel atau ringkasan) | hasil memilih dataset | `0` = kembali ke submenu asal |
Menu 1 (Buat Data), 2 (Deteksi), dan 5 (Test) tidak punya submenu.
Menu 1 dan 4.4 tidak memilih dataset, Menu lainnya pasti memilih dataset.

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
2. Deteksi
3. Statistik
4. Evaluasi
5. Test
0. Keluar

Pilih: _
```

### 8.5 Page 2: Submenu (contoh menu 3)

```text
--- 3. STATISTIK ---

0. Kembali
1. Ringkasan umum
2. Peringkat terindikasi
3. Peringkat pengirim
4. Transaksi terbesar
5. Statistik per jam
6. Distribusi nominal
7. Aliran dana

Pilih: _
```
Submenu 2 dan 4 memakai format yang sama (daftar bernomor, `0. Kembali`).

### 8.6 Page 3: Pilih dataset (contoh menu 3.2)

```text
--- 3.1 PERINGKAT TERINDIKASI ---
Pilih Dataset

0. Kembali
1. Dataset: Seed(43), 1.555 Rekening dan 8.673 Transaksi [SUDAH DETEKSI]
2. Dataset: Seed(7), 1.925 Rekening dan 9.161 Transaksi [BELUM DETEKSI]

Pilih: _
```
Submenu 2 dan 4 memakai format yang sama (daftar bernomor, `0. Kembali`).

### 8.7 Page 4: Hasil (contoh dataset 1)

Ada dua bentuk:

**a. Hasil berupa tabel (dengan paging dan search).** Maksimal 10 baris per halaman. Contoh menu 2.1, halaman 1 dari 3 (angka hanya ilustrasi):

```text
--- 3.1 PERINGKAT TERINDIKASI ---
Ambang: min 20 trx masuk, rasio >= 0.70
Dataset: Seed(43), 1.555 Rekening dan 8.673 Transaksi
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

Type 00 for search | Type page 1-3 | Type 0 back: _
```

Arti input pada baris terakhir:

| Input | Aksi |
|---|---|
| `00` | Masuk mode search |
| `0` | Kembali ke submenu asal |
| `1` sampai jumlah halaman | Lompat ke halaman itu |
| selain itu | Pesan "Halaman tidak ada" (satu kali), page dirender ulang |

**Alur search**, jika user mengetik `00`:

```text
Type 00 for search | Type page 1-3 | Type 0 back: 00
Search: R14_
```

Setelah Enter, page dirender ulang dengan filter aktif:
```text
--- 2.1 DETEKSI DAN PERINGKAT REKENING ---
Ambang: min 20 trx masuk, rasio >= 0.70
Dataset: Seed(43), 1.555 Rekening dan 8.673 Transaksi
Filter: "R14" | Data 1-5 dari 5 | Halaman 1/1

No  Rek   Trx   Dana Masuk   Dana Keluar   Rasio   Status
1   R145  34    4120000      3708000       0.90    TERINDIKASI
2   R144  33    3980000      3582000       0.90    TERINDIKASI
3   R148  31    3650000      3102000       0.85    TERINDIKASI
4   R146  30    3410000      2900000       0.85    TERINDIKASI
5   R147  29    3300000      2640000       0.80    TERINDIKASI

Type 00 for search | Type page 1-1 | Type 0 back: _
```

Aturan search:
- Mencocokkan teks pada baris (minimal kolom nomor rekening), tidak peka huruf besar/kecil, sebagian kata sudah cocok ("r14" cocok dengan R144).
- Search diterapkan pada seluruh data, bukan hanya halaman yang sedang tampil.
- Numerasi baris dan jumlah halaman dihitung ulang dari hasil filter.
- Search kosong (langsung Enter) menghapus filter dan kembali ke semua data.
- Tidak ada hasil: tampilkan `Tidak ada hasil untuk "xyz"` dan tetap di page, filter tidak diubah menjadi kosong.
- Filter hilang saat user keluar dari page hasil.

**b. Hasil berupa ringkasan (tanpa paging dan search).** Contoh 3.1 dan 4.1:

```text
--- 4.1 PRECISION DAN RECALL ---
Ambang: min 20 trx masuk, rasio >= 0.70
TP 5 | FP 3 | FN 3 | TN 140
Precision: 62.50%
Recall   : 62.50%
F1-score : 62.50%

Tekan Enter untuk kembali...
```

### 8.7 Pemetaan menu ke jenis page hasil

**Alur page per menu**

| Menu | Alur |
|---|---|
| 1 Buat Data | Menu utama → Hasil |
| 2 Deteksi | Menu utama → Pilih dataset → Hasil |
| 3.1 - 3.6, 4.1 - 4.3 | Menu utama → Submenu → Pilih dataset → Hasil |
| 3.7 Aliran dana | Menu utama → Submenu → Pilih dataset → Input nomor rekening → Hasil |
| 4.4 Ubah ambang | Menu utama → Submenu → Input ambang → Hasil |
| 5 Test | Menu utama → Pilih dataset → Hasil |

**Pemetaan hasil**

| Menu | Pilih dataset | Jenis hasil | Search mencari pada | `0` kembali ke |
|---|---|---|---|---|
| 1 Buat Data | tidak | ringkasan (seed, indeks dataset baru, jumlah rekening, jumlah transaksi) | - | Menu utama |
| 2 Deteksi | ya | ringkasan (ambang yang dipakai, jumlah terindikasi dari total rekening, status menjadi SUDAH) | - | Menu utama |
| 3.1 Ringkasan umum | ya | ringkasan | - | Submenu 3 |
| 3.2 Peringkat terindikasi | ya | tabel (rekening terindikasi, urut dana masuk terbesar) | nomor rekening | Submenu 3 |
| 3.3 Peringkat pengirim | ya | tabel | nomor rekening | Submenu 3 |
| 3.4 Transaksi terbesar | ya | tabel | rekening asal atau tujuan | Submenu 3 |
| 3.5 Statistik per jam | ya | tabel (24 baris, 1 halaman) | jam | Submenu 3 |
| 3.6 Distribusi nominal | ya | tabel (5 baris) | label kelompok | Submenu 3 |
| 3.7 Aliran dana | ya | input nomor rekening, lalu tabel transaksi keluar | rekening tujuan | Submenu 3 |
| 4.1 Precision dan Recall | ya | ringkasan | - | Submenu 4 |
| 4.2 False Negative | ya | tabel (kolom Peran dan alasan) | nomor rekening, peran | Submenu 4 |
| 4.3 False Positive | ya | tabel (kolom Peran) | nomor rekening, peran | Submenu 4 |
| 4.4 Ubah ambang | tidak | input ambang, lalu ringkasan ambang baru (tanpa deteksi ulang) | - | Submenu 4 |
| 5 Test | ya | tabel daftar test (nama, LULUS/GAGAL) | nama test | Menu utama |

**Aturan tambahan**
- Page Pilih dataset menampilkan daftar dataset (indeks, seed, status BELUM/SUDAH), dan `0` kembali ke page asal.
- Menu 3.2, 4.1, 4.2, dan 4.3 membutuhkan dataset berstatus SUDAH. Jika masih BELUM, tampil pesan satu kali: "Dataset belum dideteksi. Jalankan deteksi dahulu." lalu kembali ke submenu.
- Menu 3.1, 3.3 - 3.7 boleh untuk dataset BELUM. Pada 3.1, baris "Terindikasi" hanya tampil jika status SUDAH.
- Menu 2 pada dataset yang sudah SUDAH menimpa hasil deteksi lama (hanya `rasio`, `flag`, `catatan`, `status`), pengubahan mengikuti spec deteksi.
- Ambang di 4.4 disimpan sebagai pengaturan global dan dipakai oleh menu 2 berikutnya. Dataset yang sudah dideteksi tidak berubah sampai dideteksi ulang lewat menu 2.
- Tabel dengan 10 baris atau kurang tetap memakai page hasil yang sama (halaman 1/1).

### 8.8 Aturan input dan implementasi

- Semua input angka memakai fungsi baca angka yang aman (non-angka diulang, EOF keluar bersih).
- Search membaca satu baris penuh (`getline`). Setelah membaca angka dengan `cin >>`, sisa newline harus dibuang dulu agar `getline` tidak membaca baris kosong.
- Jumlah baris per halaman adalah konstanta `BARIS_PER_HALAMAN = 10` di `config.h`. Jumlah halaman = `ceil(jumlah data / 10)`.
- Page hasil bersifat generik: menerima judul, header, daftar baris (sudah berupa teks), dan menangani paging, search, dan navigasi sendiri. Menu 3.x, dan 4.x hanya menyiapkan datanya. Tidak ada kode paging yang ditulis ulang per menu.
- Logika (hitung) tidak mencetak apa pun. Pencetakan dan clear hanya dilakukan di `tampilan`, dan alur page di `menu`.

### 8.9 Kriteria selesai untuk antarmuka

- Setiap pindah page layar bersih, tidak ada sisa output page sebelumnya.
- Tidak ada page hasil yang menampilkan lebih dari 10 baris data.
- Lompat ke halaman di luar jangkauan tidak crash dan tidak mengubah halaman.
- Search yang tidak menemukan apa pun tidak menghapus data dan bisa di-reset.
- Input non-angka dan EOF tidak membuat program berputar terus atau crash.
