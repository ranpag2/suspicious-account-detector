#include "searching.h"

#include <cctype>

namespace {

char kecil(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

// Membandingkan dua teks tanpa membedakan huruf besar/kecil: <0, 0, >0.
int bandingTeks(const std::string& a, const std::string& b) {
    const std::size_t n = a.size() < b.size() ? a.size() : b.size();
    for (std::size_t i = 0; i < n; ++i) {
        const char x = kecil(a[i]), y = kecil(b[i]);
        if (x != y) return x < y ? -1 : 1;
    }
    if (a.size() == b.size()) return 0;
    return a.size() < b.size() ? -1 : 1;
}

// Membandingkan awal teks `a` (sepanjang awalan) dengan `awalan`.
// 0 berarti `a` diawali `awalan`.
int bandingAwalan(const std::string& a, const std::string& awalan) {
    for (std::size_t i = 0; i < awalan.size(); ++i) {
        if (i >= a.size()) return -1;   // a lebih pendek, jadi berada sebelum awalan
        const char x = kecil(a[i]), y = kecil(awalan[i]);
        if (x != y) return x < y ? -1 : 1;
    }
    return 0;
}

}  // namespace

int cariRekening(const std::vector<Rek>& rek, const std::string& no) {
    return cariBiner(rek, [&](const Rek& r) { return bandingTeks(r.no, no); });
}

Rentang cariAwalanRekening(const std::vector<Rek>& rek, const std::string& awalan) {
    auto banding = [&](const Rek& r) { return bandingAwalan(r.no, awalan); };
    Rentang hasil;
    hasil.awal = batasBawah(rek, banding);
    hasil.akhir = batasAtas(rek, banding);
    return hasil;
}

bool nomorTerurut(const std::vector<Rek>& rek) {
    for (std::size_t i = 1; i < rek.size(); ++i) {
        if (bandingTeks(rek[i - 1].no, rek[i].no) >= 0) return false;
    }
    return true;
}