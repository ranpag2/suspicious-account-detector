#pragma once
// Lapisan 2 - detektor (Blue). Boleh bergantung pada lapisan 0-1.
// Tidak mencetak apa pun (lihat structure.md 3.3).
//
// Aturan isolasi (model.md 4.5): detektor hanya menerima Dataset lewat argumen
// dan TIDAK boleh meng-include dataset.h, generator.h, ataupun menyentuh `label`.
// Karena itu detector.h/.cpp hanya bergantung pada types.h dan config.h.
#include <string>
#include "types.h"

// ---- Pengaturan ambang (menu 4.4) ------------------------------------------
// Ambang aktif disimpan di detector.cpp dan dipakai oleh deteksi(d) berikutnya.
// Mengubah ambang tidak mengubah dataset yang sudah dideteksi.
Ambang ambangSaatIni();

// Memeriksa ambang: 1 <= minTrxMasuk <= MAX_TRX dan 0 < minRasio <= 1.
// Mengisi `alasan` jika tidak valid.
bool ambangValid(const Ambang& a, std::string& alasan);

// Mengganti ambang aktif. Mengembalikan false (ambang lama tetap) jika tidak valid.
bool aturAmbang(const Ambang& a, std::string& alasan);

// Teks ambang untuk tampilan, mis. "min 20 trx masuk, rasio >= 0.70".
// Pemanggil menambahkan awalan "Ambang: " sendiri.
std::string teksAmbang(const Ambang& a);

// ---- Deteksi (menu 2) ------------------------------------------------------
// Membaca d.trx dan agregat rekening, lalu MENIMPA rasio dan flag pada tiap
// rekening serta catatan dan status (= SUDAH) pada dataset. trx dan agregat tidak
// pernah diubah. Hasil tidak bergantung pada hasil deteksi sebelumnya, jadi
// mendeteksi ulang dengan ambang yang sama menghasilkan keluaran yang sama.
// `a` harus lolos ambangValid.
//
// Aturan:
//   1. Langsung  : trx masuk >= a.minTrxMasuk dan rasio (keluar/masuk) >= a.minRasio.
//   2. Lewat aliran: rekening yang belum terindikasi, meneruskan dana (rasio >=
//      a.minRasio), dan sebagian besar dana masuknya berasal dari rekening yang
//      sudah terindikasi. Diulang per lapis sampai tidak ada perubahan (maks. 8 putaran),
//      sehingga rantai berlapis ikut tertangkap.
// flag ditentukan sepenuhnya oleh aturan di atas.
HasilDeteksi deteksi(Dataset& d, const Ambang& a);
HasilDeteksi deteksi(Dataset& d);   // memakai ambangSaatIni()