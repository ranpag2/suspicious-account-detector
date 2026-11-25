# 5. Spesifikasi generator (Red)

Dibaca oleh Red dan Purple. **Tidak dibuka ke Blue** (model.md 4.7).

## 5.1 Antarmuka

| Fungsi | Fungsi |
|---|---|
| `int buatDataset(unsigned seed, std::string& alasan)` | Dipakai menu 1. Membuat satu dataset + label, lalu `tambahDataset`. Mengembalikan indeks 0-based, atau -1 dan `alasan`. |
| `bool rakitDataset(unsigned seed, Dataset&, LabelSet&, std::string& alasan)` | Versi murni tanpa menyentuh penyimpanan global. Untuk uji sapuan. |

- `seed == 0` dipilih acak antara 1 dan 999999 yang belum dipakai.
- Seed sama menghasilkan dataset dan label identik. Pembangkit acak ditulis sendiri (splitmix64) karena `std::uniform_int_distribution` boleh beda hasil antar compiler.
- Seed duplikat ditolak oleh `tambahDataset`: "Seed sudah ada (dataset N)".

## 5.2 Alur satu seed

1. **Rencana.** Seed menentukan semua pilihan di katalog (5.3).
2. **Anggaran.** Batas atas jumlah transaksi dihitung dari rencana. Jika melebihi `MAX_TRX`, kuantitas dikecilkan (noise 25% per langkah, lalu pemain, lalu pedagang). Transaksi tidak pernah dipotong.
3. **Peran.** Peran ditempatkan pada nomor rekening yang diacak, jadi nomor rekening tidak membocorkan peran.
4. **Transaksi**, berurutan karena aliran judol memakai dana yang sudah masuk: noise orang biasa, pedagang sah, setoran pemain, aliran rantai, dana kembali.
5. **Agregat dan label.** `hitungAgregat`, isi `Label`, lalu `validasiDataset`.

Pelanggaran batas atau transaksi ke diri sendiri dianggap bug: dilaporkan lewat `alasan`, bukan diabaikan.

## 5.3 Katalog pola

| Dimensi | Pilihan |
|---|---|
| Topologi | langsung (0 perantara), berlapis 1-4 tingkat, bercabang 2-3 tingkat (tiap node ke banyak tujuan), putaran (rantai pendek + dana kembali ke pemain) |
| Jumlah setoran per pemain | banyak (3-6), sedikit (1-2), sedang (2-4) |
| Nominal setoran | kecil (10-200rb), seragam (satu nilai), bawah batas (90-99% dari batas bulat), lebar (campuran), wajar (20-300rb, saat menyamar) |
| Jam setoran | malam, merata, burst 3 jam, siang (saat menyamar) |
| Jam aliran | dini hari, merata, siang (saat menyamar) |
| Pembagian dana diteruskan | seragam atau acak |
| Penyamaran | 30% dataset: setoran siang dengan nominal wajar, mirip pedagang |
| Pengecoh sah | 0 (20%), 1-3 (50%), 4-8 (30%) pedagang |
| Dana diteruskan per node | 45-98%; node di bawah 65% ditandai `menahan-dana` |
| Kuantitas | biasa 150-1400, pemain 15-160, penampung 2-8, akhir 1-2, noise 1-6 transaksi per rekening biasa |

Transaksi orang biasa sengaja tidak seragam: 15% jatuh di luar jam 06-22, supaya "malam" bukan penanda pasti.

## 5.4 Anggaran

```text
batas = noise
      + pemain   x setoran maksimum per pemain
      + pedagang x (masuk maksimum + bayar pemasok)
      + node rantai x transfer per node
      + akhir    x transfer dana kembali (jika putaran)
```

Setiap suku adalah batas atas, sehingga hasil nyata selalu <= batas <= `MAX_TRX`. Lebar tiap lapis rantai dibatasi (`batasLebar`) agar setiap rekening jahat pasti menerima dana. Tidak ada rekening jahat yang tanpa transaksi.

## 5.5 Label

| Peran | `jahat` | `pola` |
|---|---|---|
| Biasa | tidak | kosong |
| Pemain | tidak | kosong (pengirim setoran, bukan sasaran deteksi) |
| Pedagang | tidak | `pengecoh-sah` |
| Penampung | ya | `penampung topologi=... setor=... nominal=... jam=... samar=... teruskan=NN% [menahan-dana]` |
| Perantara | ya | `perantara-tingkat-N ...` |
| Akhir | ya | `struktural:akhir-ujung-rantai ...` |

- Awalan `struktural:` menandai batas yang tidak bisa dilewati detektor berbasis rasio (rekening akhir hampir tidak meneruskan dana). Itu bukan kegagalan Blue (model.md 4.7).
- `menahan-dana` menandai node yang sengaja menahan dana. Purple bisa melaporkan recall per kata kunci pola.
- Peran `Pemain`, `Perantara`, dan `Akhir` tambahan dari Red. Peran adalah teks bebas, jadi kontrak tidak berubah.

## 5.6 Kriteria selesai

- Seed 1..2000: semua invarian lulus, tidak ada yang melewati batas, seed sama identik (`uji_generator.cpp`).
- Jalur pengecilan anggaran diuji dengan `MAX_TRX` diturunkan: tetap valid.
- Bersih dari `-Wall -Wextra -Wpedantic`, ASan, dan UBSan.
