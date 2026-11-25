#pragma once
// Lapisan 1 - utilitas: input angka aman, angka acak, persentase.
// Boleh bergantung pada lapisan 0 saja. Tidak mencetak apa pun: pesan kesalahan
// dan prompt dicetak oleh pemanggil (menu/tampilan), lihat structure.md 3.3.
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <string>

// ---- Input angka aman (menu.md 8.8) ----------------------------------------
// Semua pembacaan memakai satu baris penuh (getline), jadi tidak ada sisa newline
// yang mengganggu pembacaan berikutnya.
enum class StatusBaca {
    OK,               // hasil terisi
    BUKAN_ANGKA,      // kosong atau bukan angka utuh ("12abc", "")
    DI_LUAR_RENTANG,  // angka valid tetapi di luar [min, max] atau terlalu besar
    SELESAI           // input habis (EOF); pemanggil harus keluar dengan bersih
};

// Membaca satu baris. Karakter '\r' di ujung dibuang. SELESAI jika EOF.
StatusBaca bacaBaris(std::istream& in, std::string& baris);
StatusBaca bacaBaris(std::string& baris);                       // dari std::cin

// Pengubah teks ke angka (tanpa I/O). Spasi di ujung diabaikan; seluruh teks
// harus berupa angka. Tidak pernah mengembalikan SELESAI. `hasil` hanya berubah
// jika OK.
StatusBaca parseInt(const std::string& teks, int& hasil,
                    int min = std::numeric_limits<int>::min(),
                    int max = std::numeric_limits<int>::max());
StatusBaca parseUnsigned(const std::string& teks, unsigned& hasil,
                         unsigned min = 0,
                         unsigned max = std::numeric_limits<unsigned>::max());
// Desimal boleh memakai '.' atau ',' ("0,7" sama dengan "0.7"). Notasi ilmiah
// ("1e-2") diterima; "nan", "inf", dan heksadesimal ditolak.
StatusBaca parseDouble(const std::string& teks, double& hasil, double min, double max);

// Membaca satu baris lalu mengubahnya. SELESAI jika input habis.
StatusBaca bacaInt(std::istream& in, int& hasil,
                   int min = std::numeric_limits<int>::min(),
                   int max = std::numeric_limits<int>::max());
StatusBaca bacaUnsigned(std::istream& in, unsigned& hasil,
                        unsigned min = 0,
                        unsigned max = std::numeric_limits<unsigned>::max());
StatusBaca bacaDouble(std::istream& in, double& hasil, double min, double max);

// Versi untuk std::cin.
StatusBaca bacaInt(int& hasil,
                   int min = std::numeric_limits<int>::min(),
                   int max = std::numeric_limits<int>::max());
StatusBaca bacaUnsigned(unsigned& hasil,
                        unsigned min = 0,
                        unsigned max = std::numeric_limits<unsigned>::max());
StatusBaca bacaDouble(double& hasil, double min, double max);

// ---- Angka acak ------------------------------------------------------------
// Seed acak untuk "seed 0 = pilih acak" (model.md 4.4). Tidak pernah 0.
unsigned seedAcak();

// Pembangkit acak yang hasilnya SAMA di semua kompiler dan sistem untuk seed yang
// sama (std::uniform_int_distribution tidak menjamin itu, jadi pemetaan ke rentang
// ditulis sendiri). Dipakai generator supaya satu seed selalu menghasilkan data
// yang sama.
class Acak {
public:
    explicit Acak(unsigned seed);

    std::uint64_t berikutnya();                        // 64 bit mentah
    int           antara(int bawah, int atas);         // inklusif; jika bawah >= atas, mengembalikan bawah
    long long     antaraLL(long long bawah, long long atas);
    double        pecahan();                           // [0, 1)
    bool          peluang(double p);                   // true dengan peluang p (p <= 0 selalu false, p >= 1 selalu true)

private:
    std::mt19937_64 mesin_;
};

// ---- Persentase ------------------------------------------------------------
// bagian / total * 100. Mengembalikan 0 jika total <= 0. Tidak dibulatkan.
double persen(long long bagian, long long total);