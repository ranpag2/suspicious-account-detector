#pragma once
// Lapisan 2 - generator (Red). Boleh bergantung pada lapisan 0-1.
// Tidak mencetak apa pun. Tidak boleh meng-include detector.h (structure.md 3.3).
#include <string>
#include "types.h"

// Membuat SATU dataset beserta kunci jawabannya dari `seed`, lalu menambahkannya
// lewat tambahDataset (dataset[k] dan label[k] selalu berpasangan).
//   seed == SEED_ACAK (0)  -> dipilih acak (1..999999) yang belum dipakai.
//   seed sama              -> dataset dan label identik.
// Mengembalikan indeks 0-based dataset baru, atau -1 dan mengisi `alasan`
// (mis. "Seed sudah ada (dataset 3)").
// Jaminan: SEMUA seed menghasilkan dataset sah; tidak ada transaksi yang
// dibuang diam-diam (anggaran dihitung di depan, lihat docs/red/generator.md).
int buatDataset(unsigned seed, std::string& alasan);

// Versi murni: merakit pasangan (d, l) tanpa menyentuh penyimpanan global.
// Dipakai uji sapuan seed (menyimpan 1000 dataset sekaligus terlalu boros).
// Sudah memanggil validasiDataset; false berarti bug generator, bukan input user.
bool rakitDataset(unsigned seed, Dataset& d, LabelSet& l, std::string& alasan);
