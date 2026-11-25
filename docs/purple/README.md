# Bagian Purple: evaluasi, statistik, tampilan, menu

## File

| File | Lapisan | Isi |
|---|---|---|
| `src/label.h` | 0-1 | deklarasi `label` (kunci jawaban), hanya untuk Red dan Purple |
| `src/evaluation.h/.cpp` | 3 | TP/FP/FN/TN, precision, recall, F1, daftar FN dan FP, alasan lolos, recall per pola, pencarian ambang terbaik |
| `src/invarian.h/.cpp` | 3 | pengecekan invarian model.md 4.6; dipakai menu 5 dan `tests/` |
| `src/statistics.h/.cpp` | 4 | ringkasan, peringkat, per jam, distribusi nominal, aliran dana |
| `src/appreance.h/.cpp` | 4 | pemformatan, satu-satunya tempat `clear` dan `cout` |
| `src/menu.h/.cpp` | 5 | alur page, paging, search, input aman |
| `tests/test_invarian.cpp` | - | `make test`, memakai dataset buatan sendiri |

`evaluation` dan `invarian` menerima `Dataset` dan `LabelSet` lewat argumen, jadi tidak
meng-include `label.h`. Hanya `menu.cpp` yang membaca `label` global.

## Yang dibutuhkan dari Red

`menu.cpp` meng-include `generator.h` dan memanggil:

```cpp
// Membuat satu dataset dari seed (seed sudah bukan 0), menambahkannya lewat
// tambahDataset(), mengembalikan indeks 0-based atau -1 dan mengisi `alasan`.
int buatDataset(unsigned seed, std::string& alasan);
```

Jika nama atau bentuknya berbeda, ubah satu pemanggilan di `menuBuatData()`.

## Keputusan yang tidak ditentukan docs

- Kelompok distribusi nominal (menu 3.6): `< 50.000`, `50.000-249.999`, `250.000-999.999`,
  `1.000.000-4.999.999`, `>= 5.000.000`. Konstanta ada di `statistics.cpp`.
- Menu 3.4 menampilkan `TOP_TRX_TERBESAR = 100` transaksi (selection sort pada 10.000 trx terlalu boros).
- Menu 3.3 mengurutkan menurut total dana keluar; rekening yang tidak pernah mengirim tidak ikut.
- Ambang yang dipakai saat deteksi dicatat per dataset di `menu.cpp`, supaya menu 4.x tetap
  benar walau ambang aktif diubah lewat 4.4 setelahnya.
- Menu 5 menjalankan `jalankanTest` pada dataset yang dipilih.
- Isian teks (seed, ambang, nomor rekening) bisa dibatalkan dengan Enter kosong.
