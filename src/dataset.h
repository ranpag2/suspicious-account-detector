#pragma once
// Lapisan 1 - penyimpanan dataset. Boleh bergantung pada lapisan 0 (config, types).
// Tidak mencetak apa pun (lihat structure.md 3.3).
//
// Dua penyimpanan paralel (model.md 4.1), keduanya didefinisikan di dataset.cpp:
//   dataset[k] dan label[k] adalah pasangan; rek[i] di dataset[k] berpasangan
//   dengan label[k][i].
// Header ini hanya mendeklarasikan `dataset` (terbuka untuk semua peran).
// Deklarasi `label` ada di label.h, yang TIDAK boleh di-include detektor.
#include <string>
#include <vector>
#include "types.h"

extern std::vector<Dataset> dataset;
extern std::vector<LabelSet> label;

// ---- Akses ringan ----------------------------------------------------------
int  jumlahDataset();                  // dataset.size()
bool indeksValid(int k);               // 0 <= k < jumlahDataset()
int  cariSeed(unsigned seed);          // indeks dataset dengan seed itu, atau -1

// ---- Bantuan untuk pembuat data --------------------------------------------
// Nomor rekening dari indeks 0-based: 0 -> "R0001". Melebar jika lebih dari
// NO_REK_LEBAR digit.
std::string formatNoRek(int indeks);

// Hitung ulang masuk, keluar, in, out semua rekening dari d.trx.
// Transaksi dengan indeks rekening di luar jangkauan dilewati di sini dan
// dilaporkan oleh validasiDataset, bukan dibuang diam-diam.
void hitungAgregat(Dataset& d);

// ---- Validasi dan penambahan -----------------------------------------------
// Memeriksa invarian model.md 4.6 yang bisa diperiksa dari dataset saja:
// batas jumlah, nilai tiap trx, nomor rekening, agregat konsisten dengan trx,
// dan aturan status (BELUM: rasio 0 dan flag false; SUDAH: catatan
// terisi). Mengembalikan false dan mengisi `alasan` pada pelanggaran
// pertama yang ditemukan.
bool validasiDataset(const Dataset& d, std::string& alasan);

// Menambahkan satu pasangan dataset + label sekaligus, sehingga
// dataset.size() == label.size() selalu terjaga. Semua pemeriksaan dilakukan
// sebelum apa pun disimpan; jika gagal, tidak ada yang berubah.
// Menolak: dataset tidak valid, status bukan BELUM, ukuran label tidak sama
// dengan jumlah rekening, label tanpa rekening jahat atau tanpa rekening sah,
// dan seed duplikat ("Seed sudah ada (dataset N)", N mulai dari 1 seperti di menu).
// Mengembalikan indeks 0-based dataset baru, atau -1 dan mengisi `alasan`.
int tambahDataset(Dataset d, LabelSet l, std::string& alasan);