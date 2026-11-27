#include "util.h"

#include <cctype>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <string>

namespace {

std::string pangkas(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

// Mengubah teks menjadi long long. '+' di depan diterima.
StatusBaca parseLL(const std::string& teks, long long& hasil) {
    const std::string t = pangkas(teks);
    if (t.empty()) return StatusBaca::BUKAN_ANGKA;
    const char* awal = t.data();
    const char* akhir = t.data() + t.size();
    if (*awal == '+') ++awal;
    if (awal == akhir) return StatusBaca::BUKAN_ANGKA;
    long long nilai = 0;
    const std::from_chars_result r = std::from_chars(awal, akhir, nilai);
    if (r.ec == std::errc::result_out_of_range) {
        // Pastikan seluruh teks memang angka sebelum menyebutnya "terlalu besar".
        const char* p = awal;
        if (*p == '-') ++p;
        if (p == akhir) return StatusBaca::BUKAN_ANGKA;
        for (; p != akhir; ++p) {
            if (!std::isdigit(static_cast<unsigned char>(*p))) return StatusBaca::BUKAN_ANGKA;
        }
        return StatusBaca::DI_LUAR_RENTANG;
    }
    if (r.ec != std::errc() || r.ptr != akhir) return StatusBaca::BUKAN_ANGKA;
    hasil = nilai;
    return StatusBaca::OK;
}

}  // namespace

// ---- Input -----------------------------------------------------------------

StatusBaca bacaBaris(std::istream& in, std::string& baris) {
    if (!std::getline(in, baris)) return StatusBaca::SELESAI;
    if (!baris.empty() && baris.back() == '\r') baris.pop_back();
    return StatusBaca::OK;
}

StatusBaca bacaBaris(std::string& baris) { return bacaBaris(std::cin, baris); }

StatusBaca parseInt(const std::string& teks, int& hasil, int min, int max) {
    long long nilai = 0;
    const StatusBaca s = parseLL(teks, nilai);
    if (s != StatusBaca::OK) return s;
    if (nilai < min || nilai > max) return StatusBaca::DI_LUAR_RENTANG;
    hasil = static_cast<int>(nilai);
    return StatusBaca::OK;
}

StatusBaca parseUnsigned(const std::string& teks, unsigned& hasil, unsigned min, unsigned max) {
    long long nilai = 0;
    const StatusBaca s = parseLL(teks, nilai);
    if (s != StatusBaca::OK) return s;
    if (nilai < 0 || nilai < static_cast<long long>(min) || nilai > static_cast<long long>(max))
        return StatusBaca::DI_LUAR_RENTANG;
    hasil = static_cast<unsigned>(nilai);
    return StatusBaca::OK;
}

StatusBaca parseDouble(const std::string& teks, double& hasil, double min, double max) {
    std::string t = pangkas(teks);
    if (t.empty()) return StatusBaca::BUKAN_ANGKA;
    for (char& c : t) {
        if (c == ',') c = '.';
        const bool boleh = std::isdigit(static_cast<unsigned char>(c)) || c == '+' || c == '-' ||
                           c == '.' || c == 'e' || c == 'E';
        if (!boleh) return StatusBaca::BUKAN_ANGKA;
    }
    char* ujung = nullptr;
    const double nilai = std::strtod(t.c_str(), &ujung);
    if (ujung != t.c_str() + t.size()) return StatusBaca::BUKAN_ANGKA;
    if (!std::isfinite(nilai)) return StatusBaca::DI_LUAR_RENTANG;
    if (nilai < min || nilai > max) return StatusBaca::DI_LUAR_RENTANG;
    hasil = nilai;
    return StatusBaca::OK;
}

StatusBaca bacaInt(std::istream& in, int& hasil, int min, int max) {
    std::string baris;
    if (bacaBaris(in, baris) == StatusBaca::SELESAI) return StatusBaca::SELESAI;
    return parseInt(baris, hasil, min, max);
}

StatusBaca bacaUnsigned(std::istream& in, unsigned& hasil, unsigned min, unsigned max) {
    std::string baris;
    if (bacaBaris(in, baris) == StatusBaca::SELESAI) return StatusBaca::SELESAI;
    return parseUnsigned(baris, hasil, min, max);
}

StatusBaca bacaDouble(std::istream& in, double& hasil, double min, double max) {
    std::string baris;
    if (bacaBaris(in, baris) == StatusBaca::SELESAI) return StatusBaca::SELESAI;
    return parseDouble(baris, hasil, min, max);
}

StatusBaca bacaInt(int& hasil, int min, int max) { return bacaInt(std::cin, hasil, min, max); }
StatusBaca bacaUnsigned(unsigned& hasil, unsigned min, unsigned max) { return bacaUnsigned(std::cin, hasil, min, max); }
StatusBaca bacaDouble(double& hasil, double min, double max) { return bacaDouble(std::cin, hasil, min, max); }

// ---- Acak ------------------------------------------------------------------

unsigned seedAcak() {
    std::random_device rd;
    unsigned s = rd();
    // Di sebagian lingkungan random_device bisa deterministik, jadi campur dengan waktu.
    s ^= static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count());
    return s == 0 ? 1u : s;
}

Acak::Acak(unsigned seed) : mesin_(seed) {}

std::uint64_t Acak::berikutnya() { return static_cast<std::uint64_t>(mesin_()); }

long long Acak::antaraLL(long long bawah, long long atas) {
    if (bawah >= atas) return bawah;
    const std::uint64_t lebar = static_cast<std::uint64_t>(atas) - static_cast<std::uint64_t>(bawah) + 1u;
    if (lebar == 0) return static_cast<long long>(berikutnya());   // seluruh rentang 64 bit
    // Tolak nilai di ekor supaya setiap hasil punya peluang yang sama persis.
    const std::uint64_t maks = ~std::uint64_t{0};
    const std::uint64_t batas = maks - (maks % lebar + 1u) % lebar;
    std::uint64_t r = berikutnya();
    while (r > batas) r = berikutnya();
    return static_cast<long long>(static_cast<std::uint64_t>(bawah) + r % lebar);
}

int Acak::antara(int bawah, int atas) {
    return static_cast<int>(antaraLL(bawah, atas));
}

double Acak::pecahan() {
    return static_cast<double>(berikutnya() >> 11) * (1.0 / 9007199254740992.0);   // 2^53
}

bool Acak::peluang(double p) {
    if (p <= 0.0) return false;
    if (p >= 1.0) return true;
    return pecahan() < p;
}

// ---- Persentase ------------------------------------------------------------

double persen(long long bagian, long long total) {
    if (total <= 0) return 0.0;
    return static_cast<double>(bagian) * 100.0 / static_cast<double>(total);
}

std::string rentangJam(int jam) {
    auto dua = [](int x) { return (x < 10 ? "0" : "") + std::to_string(x); };
    return dua(jam) + "-" + dua((jam + 1) % 24);
}