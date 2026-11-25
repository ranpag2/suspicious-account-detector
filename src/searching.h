#pragma once
// Lapisan 1 - pencarian dengan binary search. Boleh bergantung pada lapisan 0
// saja. Tidak mencetak apa pun.
//
// SYARAT: data harus terurut menurut kunci yang dicari. rek di dataset memenuhi
// syarat ini karena nomornya dibuat berurutan (R0001, R0002, ...) dan tidak
// pernah diurutkan ulang (urutan peringkat memakai daftar indeks, lihat sorting.h).
#include <cstddef>
#include <string>
#include <vector>

#include "types.h"

// ---- Binary search umum ----------------------------------------------------
// `banding(elemen)` membandingkan elemen dengan sasaran dan mengembalikan:
//   < 0  elemen berada SEBELUM sasaran
//   = 0  elemen cocok
//   > 0  elemen berada SESUDAH sasaran
// Hasilnya harus monoton mengikuti urutan data (data terurut menaik).

// Indeks pertama yang banding(...) >= 0 (v.size() jika tidak ada).
template <class T, class Banding>
int batasBawah(const std::vector<T>& v, Banding banding);

// Indeks pertama yang banding(...) > 0 (v.size() jika tidak ada).
template <class T, class Banding>
int batasAtas(const std::vector<T>& v, Banding banding);

// Indeks salah satu elemen yang cocok, atau -1.
template <class T, class Banding>
int cariBiner(const std::vector<T>& v, Banding banding);

// ---- Pencarian rekening menurut nomor --------------------------------------
// Tidak peka huruf besar/kecil: "r145" sama dengan "R145".

// Nomor lengkap, mis. "R0145". Mengembalikan indeks rekening atau -1.
int cariRekening(const std::vector<Rek>& rek, const std::string& no);

// Semua rekening yang nomornya diawali teks tersebut, mis. "R01" mencakup
// R0100 sampai R0199. Hasil [awal, akhir) kosong jika tidak ada. Awalan kosong
// mencakup semua rekening.
Rentang cariAwalanRekening(const std::vector<Rek>& rek, const std::string& awalan);

// Pemeriksaan syarat: apakah nomor rekening terurut menaik tanpa duplikat.
bool nomorTerurut(const std::vector<Rek>& rek);

// ---- Implementasi template -------------------------------------------------

template <class T, class Banding>
int batasBawah(const std::vector<T>& v, Banding banding) {
    int kiri = 0, kanan = static_cast<int>(v.size());   // jawaban ada di [kiri, kanan]
    while (kiri < kanan) {
        const int tengah = kiri + (kanan - kiri) / 2;
        if (banding(v[tengah]) < 0) kiri = tengah + 1;
        else                        kanan = tengah;
    }
    return kiri;
}

template <class T, class Banding>
int batasAtas(const std::vector<T>& v, Banding banding) {
    int kiri = 0, kanan = static_cast<int>(v.size());
    while (kiri < kanan) {
        const int tengah = kiri + (kanan - kiri) / 2;
        if (banding(v[tengah]) <= 0) kiri = tengah + 1;
        else                         kanan = tengah;
    }
    return kiri;
}

template <class T, class Banding>
int cariBiner(const std::vector<T>& v, Banding banding) {
    int kiri = 0, kanan = static_cast<int>(v.size()) - 1;
    while (kiri <= kanan) {
        const int tengah = kiri + (kanan - kiri) / 2;
        const int h = banding(v[tengah]);
        if (h == 0) return tengah;
        if (h < 0) kiri = tengah + 1;
        else       kanan = tengah - 1;
    }
    return -1;
}