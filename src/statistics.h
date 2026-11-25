#pragma once
// Lapisan 4 - statistik (Purple). Hanya menghitung, tidak mencetak (structure.md 3.3).
// Boleh bergantung pada lapisan 0-3.
//
// Fungsi peringkat mengembalikan DAFTAR INDEKS (rek atau trx tetap tidak diurutkan
// ulang), memakai selection sort dari sorting.h.
#include <string>
#include <vector>

#include "types.h"

// Jumlah transaksi terbesar yang ditampilkan di menu 3.4.
inline constexpr int TOP_TRX_TERBESAR = 100;

struct RingkasanUmum {
    int       rekening    = 0;
    int       transaksi   = 0;
    long long totalNominal = 0;
    long long rataNominal  = 0;   // pembagian bulat
    long long nominalTerbesar = 0;
    int       jamTersibuk  = 0;   // jam dengan trx terbanyak (seri: jam paling awal)
    int       trxJamTersibuk = 0;
    bool      sudahDeteksi = false;
    int       terindikasi  = 0;   // hanya bermakna jika sudahDeteksi
};
RingkasanUmum ringkasanUmum(const Dataset& d);

struct BarisJam {
    int       jam       = 0;
    int       trx       = 0;
    long long total     = 0;
    long long rata      = 0;   // pembagian bulat, 0 jika tidak ada trx
};
std::vector<BarisJam> statistikPerJam(const Dataset& d);   // selalu 24 baris, jam 0-23

struct KelompokNominal {
    std::string label;
    int         jumlah = 0;
    long long   total  = 0;
};
std::vector<KelompokNominal> distribusiNominal(const Dataset& d);   // selalu 5 kelompok

// Rekening terindikasi menurut skor (kosong jika dataset BELUM).
std::vector<int> peringkatTerindikasi(const Dataset& d);
// Rekening yang pernah mengirim, menurut total dana keluar.
std::vector<int> peringkatPengirim(const Dataset& d);
// TOP_TRX_TERBESAR transaksi dengan nominal terbesar.
std::vector<int> transaksiTerbesar(const Dataset& d);
// Transaksi keluar dari satu rekening, nominal terbesar lebih dulu.
std::vector<int> aliranKeluar(const Dataset& d, int rekening);
