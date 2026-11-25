#pragma once
// Lapisan 0 - semua definisi tipe data proyek. Hanya struct dan enum, tanpa
// logika, tanpa cetak, dan tanpa variabel global.
//
// Yang ada di sini adalah BENTUK datanya (terbuka untuk semua peran).
// Penyimpanannya (vector dataset dan label) dideklarasikan di tempat lain:
//   - dataset.h : extern std::vector<Dataset> dataset   (semua peran)
//   - label.h   : extern std::vector<LabelSet> label    (hanya Red dan Purple;
//                 detector.cpp dilarang meng-include label.h, lihat model.md 4.5)
#include <string>
#include <vector>

// ---- Kontrak bersama: boleh dibaca semua peran -----------------------------

// Satu transaksi. Hanya hal yang bisa diamati dari luar.
struct Trx {
    int       dari = 0;   // indeks rekening pengirim
    int       ke   = 0;   // indeks rekening penerima
    long long nom  = 0;   // nominal (rupiah, > 0)
    int       jam  = 0;   // 0-23
};

// Satu rekening.
struct Rek {
    std::string no;           // "R0001", "R0002", ... (lihat NO_REK_* di config.h)

    // Agregat: diisi sekali oleh generator (Red), tidak pernah diubah detektor.
    int       masuk  = 0;     // jumlah trx masuk
    int       keluar = 0;     // jumlah trx keluar
    long long in     = 0;     // total dana masuk
    long long out    = 0;     // total dana keluar

    // Hasil deteksi: diisi detektor (Blue), awalnya kosong.
    double    rasio = 0;      // rasio dana keluar terhadap masuk
    double    skor  = 0;      // 0-1, makin tinggi makin mencurigakan
    bool      flag  = false;  // true = terindikasi
};

// Sudah dideteksi atau belum.
enum class Status { BELUM, SUDAH };

// Satu dataset hasil satu kali "Buat Data".
struct Dataset {
    unsigned          seed = 0;              // seed yang benar-benar dipakai
    std::vector<Trx>  trx;
    std::vector<Rek>  rek;
    Status            status = Status::BELUM;
    std::string       catatan;               // catatan mengenai deteksi
};

// Rentang indeks setengah terbuka [awal, akhir) hasil pencarian. Kosong jika awal == akhir.
struct Rentang {
    int awal  = 0;
    int akhir = 0;
};

// ---- Kunci jawaban: bentuknya terbuka, isinya hanya Red (tulis) dan Purple (baca)

// Kebenaran untuk satu rekening. rek[i] di dataset[k] berpasangan dengan label[k][i].
struct Label {
    std::string peran;        // "Biasa", "Pedagang", "Penampung", dst. (teks bebas)
    std::string pola;         // pola yang membuat rekening ini jahat (model.md 4.7)
    bool        jahat = false;
};
using LabelSet = std::vector<Label>;

// ---- Detektor (Blue) -------------------------------------------------------
 
// Ambang yang bisa diubah user lewat menu 4.4 dan dipakai menu 2 berikutnya.
// Nilai awal sama dengan contoh tampilan di menu.md ("min 20 trx masuk, rasio >= 0.70").
// Konstanta lain milik detektor (bobot skor, batas aliran dana) ada di detector.cpp.
struct Ambang {
    int    minTrxMasuk = 20;     // minimal jumlah trx masuk untuk dianggap terindikasi langsung
    double minRasio    = 0.70;   // minimal rasio dana keluar / dana masuk (0 < nilai <= 1)
};
 
// Ringkasan satu kali deteksi, dipakai menu 2 untuk menampilkan hasil.
struct HasilDeteksi {
    int total       = 0;         // jumlah rekening yang diperiksa
    int terindikasi = 0;         // langsung + lewatAliran
    int langsung    = 0;         // memenuhi ambang sendiri (trx masuk dan rasio)
    int lewatAliran = 0;         // terindikasi karena dananya datang dari rekening terindikasi
};