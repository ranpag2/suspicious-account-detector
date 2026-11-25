#pragma once
// Lapisan 4 - format tabel dan pencetakan (nama file mengikuti structure.md 3.1;
// di menu.md lapisan ini disebut "tampilan"). Boleh bergantung pada lapisan 0-3.
//
// Satu-satunya tempat yang mencetak ke layar dan membersihkan layar. Alur page
// dan pembacaan input ada di menu.cpp (menu.md 8.8).
#include <cstddef>
#include <string>
#include <vector>

#include "types.h"

using Baris = std::vector<std::string>;

// Data satu page hasil berbentuk tabel. Kolom "No" ditambahkan otomatis dan
// dihitung ulang dari hasil filter, jadi header dan baris tidak memuatnya.
struct SpecTabel {
    std::string              judul;       // tanpa "---", mis. "3.2 PERINGKAT TERINDIKASI"
    std::vector<std::string> info;        // baris di bawah judul (Ambang, Dataset, ...)
    Baris                    header;
    std::vector<Baris>       baris;       // seluruh data, belum difilter
    std::vector<int>         kolomCari;   // indeks kolom (tanpa "No") yang dicocokkan oleh search
};

// ---- Pemformatan murni (tanpa I/O) -----------------------------------------
std::string titik(long long n);                         // 1555 -> "1.555"
std::string desimal(double v, int digit = 2);           // 0.7 -> "0.70"
std::string persenTeks(double persen, int digit = 2);   // 62.5 -> "62.50%"
std::string teksDataset(const Dataset& d);              // "Seed(43), 1.555 Rekening dan 8.673 Transaksi"
std::string statusDataset(const Dataset& d);            // LABEL_SUDAH / LABEL_BELUM
std::string statusRekening(const Dataset& d, const Rek& r);   // TERINDIKASI / NORMAL / "-" (BELUM)

// ceil(jumlahData / BARIS_PER_HALAMAN), minimal 1.
int jumlahHalaman(std::size_t jumlahData);

// Apakah `kata` ada di dalam `teks` (tidak peka huruf besar/kecil, sebagian kata cocok).
bool memuatTeks(const std::string& teks, const std::string& kata);

// Indeks baris yang salah satu kolom di `kolomCari`-nya memuat `kata`.
// Dicari pada seluruh data, bukan satu halaman.
std::vector<int> saringBaris(const std::vector<Baris>& baris, const std::vector<int>& kolomCari,
                             const std::string& kata);

// ---- Pencetakan ------------------------------------------------------------
// Setiap fungsi tampil* membersihkan layar lebih dulu, lalu mencetak satu page utuh.
// `pesan` (boleh kosong) dicetak satu kali tepat di bawah judul sebagai "[!] ...".
void bersihkanLayar();

void tampilMenuUtama(const std::string& pesan);
void tampilSubmenu(const std::string& judul, const std::vector<std::string>& opsi, const std::string& pesan);
void tampilPilihDataset(const std::string& judul, const std::string& pesan);

// `terlihat` = indeks baris yang lolos filter (urut tampil), `halaman` mulai dari 1.
void tampilTabel(const SpecTabel& s, const std::vector<int>& terlihat, int halaman,
                 const std::string& filter, const std::string& pesan);

// Ringkasan tanpa paging; berakhir dengan "Tekan Enter untuk kembali...".
void tampilRingkasan(const std::string& judul, const std::vector<std::string>& baris, const std::string& pesan);

// Page isian: judul, baris info, lalu `prompt` tanpa newline.
void tampilInput(const std::string& judul, const std::vector<std::string>& info,
                 const std::string& pesan, const std::string& prompt);

void cetakPrompt(const std::string& teks);   // teks tanpa newline, langsung di-flush
void tampilSelesai();
