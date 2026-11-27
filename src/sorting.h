#pragma once
// Lapisan 1 - pengurutan dengan selection sort. Boleh bergantung pada lapisan 0
// saja. Tidak mencetak apa pun.
//
// Selection sort: pada tiap langkah i, cari elemen terbaik di sisa data lalu taruh
// di posisi i. Waktu O(n * k) untuk k posisi teratas (O(n^2) untuk semua), sehingga
// cocok untuk "peringkat teratas" tanpa mengurutkan seluruh data.
//
// Versi di sini STABIL: elemen yang setara tetap dalam urutan awalnya (elemen
// terpilih digeser ke depan, bukan ditukar). Jadi peringkat dengan nilai sama
// selalu berurutan sama di setiap tampilan.
#include <cstddef>
#include <utility>
#include <vector>

#include "types.h"

// ---- Selection sort umum ---------------------------------------------------
// `lebihDulu(a, b)` bernilai true jika a harus berada SEBELUM b. Harus ketat
// (false untuk dua elemen yang setara), mis. [](x, y) { return x > y; } untuk
// urutan menurun.

// Mengurutkan seluruh vektor.
template <class T, class LebihDulu>
void selectionSort(std::vector<T>& v, LebihDulu lebihDulu);

// Hanya k posisi pertama yang dijamin terurut dan berisi k elemen terbaik.
// Sisa elemen tetap ada di belakangnya dalam urutan semula.
template <class T, class LebihDulu>
void selectionSortTop(std::vector<T>& v, std::size_t k, LebihDulu lebihDulu);

// ---- Urutan siap pakai untuk rekening dan transaksi ------------------------
// Fungsi-fungsi ini mengembalikan DAFTAR INDEKS, bukan mengurutkan ulang vektor
// aslinya. Penting: rek harus tetap terurut menurut nomor agar binary search di
// searching.h benar.
enum class KunciRek { RASIO, DANA_MASUK, DANA_KELUAR, TRX_MASUK, TRX_KELUAR };

// Indeks rekening, nilai terbesar lebih dulu. Seri diurutkan menurut indeks naik.
// hanyaTerindikasi = true memuat hanya rekening dengan flag. top > 0 membatasi
// hasil pada top teratas; top = 0 berarti semua.
std::vector<int> urutRekening(const std::vector<Rek>& rek, KunciRek kunci,
                              bool hanyaTerindikasi = false, std::size_t top = 0);

// Indeks transaksi, nominal terbesar lebih dulu. Seri menurut indeks naik.
std::vector<int> urutTransaksiNominal(const std::vector<Trx>& trx, std::size_t top = 0);

// ---- Implementasi template -------------------------------------------------

template <class T, class LebihDulu>
void selectionSortTop(std::vector<T>& v, std::size_t k, LebihDulu lebihDulu) {
    const std::size_t n = v.size();
    if (k > n) k = n;
    for (std::size_t i = 0; i < k && i + 1 < n; ++i) {
        std::size_t terbaik = i;
        for (std::size_t j = i + 1; j < n; ++j) {
            if (lebihDulu(v[j], v[terbaik])) terbaik = j;   // ketat: yang pertama menang saat seri
        }
        if (terbaik != i) {
            T sementara = std::move(v[terbaik]);
            for (std::size_t j = terbaik; j > i; --j) v[j] = std::move(v[j - 1]);
            v[i] = std::move(sementara);
        }
    }
}

template <class T, class LebihDulu>
void selectionSort(std::vector<T>& v, LebihDulu lebihDulu) {
    selectionSortTop(v, v.size(), lebihDulu);
}