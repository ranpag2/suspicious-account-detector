#pragma once
// Lapisan 3 - pengecekan invarian model data (model.md 4.6), dipakai oleh menu 5
// (Test) dan tests/test_invarian.cpp. Hanya menghitung, tidak mencetak.
//
// Dataset dan LabelSet diterima lewat argumen, jadi file ini tidak perlu label.h.
#include <cstddef>
#include <string>
#include <vector>

#include "types.h"

struct HasilTest {
    std::string nama;
    bool        lulus = false;
    std::string detail;   // alasan jika gagal, kosong jika lulus
};

// Menjalankan semua pengecekan pada satu pasangan dataset + label.
// jumlahDataset dan jumlahLabel adalah ukuran dua penyimpanan paralel
// (dataset.size() dan label.size()).
// Test yang memanggil detektor dilewati (ditandai gagal dengan detail "dilewati")
// jika data dasarnya sudah tidak sah, supaya detektor tidak diberi data rusak.
std::vector<HasilTest> jalankanTest(const Dataset& d, const LabelSet& l,
                                    std::size_t jumlahDataset, std::size_t jumlahLabel);

// Menghitung berapa test yang lulus.
int jumlahLulus(const std::vector<HasilTest>& hasil);
