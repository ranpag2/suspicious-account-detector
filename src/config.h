#pragma once
// Lapisan 0 - konstanta bersama. Tidak boleh meng-include file proyek lain.
//
// Hanya berisi hal yang boleh diketahui semua peran. Rumus, ambang, dan
// parameter milik Red (generator) atau Blue (detektor) TIDAK ditaruh di sini.

// ---- Batas jumlah (model.md 4.4) -------------------------------------------
// Setiap dataset wajib: 0 < rek.size() <= MAX_REK dan 0 < trx.size() <= MAX_TRX.
inline constexpr int MAX_REK = 2000;
inline constexpr int MAX_TRX = 10000;

// ---- Batas nilai transaksi (model.md 4.6) ----------------------------------
inline constexpr int       JAM_MIN = 0;
inline constexpr int       JAM_MAX = 23;
inline constexpr long long NOM_MIN = 1;                      // nominal > 0 (rupiah)
inline constexpr long long NOM_MAX = 1'000'000'000'000LL;    // batas kewajaran, bukan parameter generator

// ---- Seed (model.md 4.4) ---------------------------------------------------
inline constexpr unsigned SEED_ACAK = 0;                     // seed 0 = pilih acak

// ---- Nomor rekening --------------------------------------------------------
// Bentuk: awalan + angka selebar NO_REK_LEBAR, mis. "R0001" .. "R2000".
inline constexpr const char* NO_REK_AWAL  = "R";
inline constexpr int         NO_REK_LEBAR = 4;

// ---- Tampilan (menu.md 8.8) ------------------------------------------------
inline constexpr int BARIS_PER_HALAMAN = 10;                 // jumlah halaman = ceil(data / 10)
inline constexpr const char* JUDUL_APLIKASI = "SUSPICIOUS ACCOUNT DETECTOR";

// ---- Label teks ------------------------------------------------------------
// Status dataset pada daftar "Pilih dataset".
inline constexpr const char* LABEL_BELUM = "BELUM DETEKSI";
inline constexpr const char* LABEL_SUDAH = "SUDAH DETEKSI";
// Status rekening pada tabel hasil.
inline constexpr const char* LABEL_TERINDIKASI = "TERINDIKASI";
inline constexpr const char* LABEL_NORMAL      = "NORMAL";

// ---- Peran (Label.peran) ---------------------------------------------------
// Peran disimpan sebagai teks bebas supaya Red boleh menambah jenis baru tanpa
// mengubah kontrak bersama. Daftar ini hanya nama yang sudah disepakati.
inline constexpr const char* PERAN_BIASA     = "Biasa";
inline constexpr const char* PERAN_PEDAGANG  = "Pedagang";
inline constexpr const char* PERAN_PENAMPUNG = "Penampung";