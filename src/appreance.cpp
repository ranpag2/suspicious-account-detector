#include "appreance.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "config.h"
#include "dataset.h"

namespace {

std::string kecil(const std::string& s) {
    std::string r = s;
    for (char& c : r) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return r;
}

std::string isi(const std::string& s, std::size_t lebar) {
    return s.size() >= lebar ? s : s + std::string(lebar - s.size(), ' ');
}

int digit(std::size_t n) {
    int d = 1;
    while (n >= 10) {
        n /= 10;
        ++d;
    }
    return d;
}

void cetakJudul(const std::string& judul, const std::string& pesan) {
    std::cout << "--- " << judul << " ---\n";
    if (!pesan.empty()) std::cout << "[!] " << pesan << "\n";
}

// Satu baris tabel: kolom dipisah dua spasi, kolom terakhir tidak diisi spasi.
void cetakBaris(const Baris& sel, const std::vector<std::size_t>& lebar) {
    std::string baris;
    for (std::size_t c = 0; c < sel.size(); ++c) {
        if (c) baris += "  ";
        baris += (c + 1 < sel.size()) ? isi(sel[c], lebar[c]) : sel[c];
    }
    std::cout << baris << "\n";
}

}  // namespace

// ---- Pemformatan murni -----------------------------------------------------

std::string titik(long long n) {
    const bool negatif = n < 0;
    const unsigned long long u = negatif ? 0ULL - static_cast<unsigned long long>(n)
                                         : static_cast<unsigned long long>(n);
    const std::string s = std::to_string(u);
    std::string hasil;
    int hitung = 0;
    for (int i = static_cast<int>(s.size()) - 1; i >= 0; --i) {
        hasil.push_back(s[static_cast<std::size_t>(i)]);
        if (++hitung % 3 == 0 && i > 0) hasil.push_back('.');
    }
    if (negatif) hasil.push_back('-');
    std::reverse(hasil.begin(), hasil.end());
    return hasil;
}

std::string desimal(double v, int d) {
    char buf[64];
    std::snprintf(buf, sizeof buf, "%.*f", d, v);
    return buf;
}

std::string persenTeks(double persen, int d) { return desimal(persen, d) + "%"; }

std::string teksDataset(const Dataset& d) {
    return "Seed(" + std::to_string(d.seed) + "), " + titik(static_cast<long long>(d.rek.size())) +
           " Rekening dan " + titik(static_cast<long long>(d.trx.size())) + " Transaksi";
}

std::string statusDataset(const Dataset& d) {
    return d.status == Status::SUDAH ? LABEL_SUDAH : LABEL_BELUM;
}

std::string statusRekening(const Dataset& d, const Rek& r) {
    if (d.status != Status::SUDAH) return "-";
    return r.flag ? LABEL_TERINDIKASI : LABEL_NORMAL;
}

int jumlahHalaman(std::size_t jumlahData) {
    const std::size_t per = static_cast<std::size_t>(BARIS_PER_HALAMAN);
    const std::size_t h = (jumlahData + per - 1) / per;
    return h == 0 ? 1 : static_cast<int>(h);
}

bool memuatTeks(const std::string& teks, const std::string& kata) {
    return kecil(teks).find(kecil(kata)) != std::string::npos;
}

std::vector<int> saringBaris(const std::vector<Baris>& baris, const std::vector<int>& kolomCari,
                             const std::string& kata) {
    std::vector<int> hasil;
    for (int i = 0; i < static_cast<int>(baris.size()); ++i) {
        const Baris& b = baris[static_cast<std::size_t>(i)];
        for (int c : kolomCari) {
            if (c >= 0 && c < static_cast<int>(b.size()) && memuatTeks(b[static_cast<std::size_t>(c)], kata)) {
                hasil.push_back(i);
                break;
            }
        }
    }
    return hasil;
}

// ---- Pencetakan ------------------------------------------------------------

void bersihkanLayar() {
    // Satu-satunya tempat layar dibersihkan (menu.md 8.3).
#ifdef _WIN32
    std::system("cls");
#else
    std::cout << "\033[2J\033[3J\033[H" << std::flush;
#endif
}

void tampilMenuUtama(const std::string& pesan) {
    bersihkanLayar();
    std::cout << "=== " << JUDUL_APLIKASI << " ===\n";
    if (!pesan.empty()) std::cout << "[!] " << pesan << "\n";
    std::cout << "\n"
                 "1. Buat Data\n"
                 "2. Deteksi\n"
                 "3. Statistik\n"
                 "4. Evaluasi\n"
                 "5. Test\n"
                 "0. Keluar\n"
                 "\n"
                 "Pilih: "
              << std::flush;
}

void tampilSubmenu(const std::string& judul, const std::vector<std::string>& opsi, const std::string& pesan) {
    bersihkanLayar();
    cetakJudul(judul, pesan);
    std::cout << "\n0. Kembali\n";
    for (std::size_t i = 0; i < opsi.size(); ++i) std::cout << (i + 1) << ". " << opsi[i] << "\n";
    std::cout << "\nPilih: " << std::flush;
}

void tampilPilihDataset(const std::string& judul, const std::string& pesan) {
    bersihkanLayar();
    cetakJudul(judul, pesan);
    std::cout << "Pilih Dataset\n\n0. Kembali\n";
    for (int k = 0; k < jumlahDataset(); ++k) {
        const Dataset& d = dataset[static_cast<std::size_t>(k)];
        std::cout << (k + 1) << ". Dataset: " << teksDataset(d) << " [" << statusDataset(d) << "]\n";
    }
    std::cout << "\nPilih: " << std::flush;
}

void tampilTabel(const SpecTabel& s, const std::vector<int>& terlihat, int halaman,
                 const std::string& filter, const std::string& pesan) {
    const int n = static_cast<int>(terlihat.size());
    const int semuaHalaman = jumlahHalaman(terlihat.size());
    halaman = std::max(1, std::min(halaman, semuaHalaman));
    const int mulai = (halaman - 1) * BARIS_PER_HALAMAN;
    const int akhir = std::min(n, mulai + BARIS_PER_HALAMAN);

    // Lebar kolom dihitung dari seluruh data supaya tidak berubah antar halaman.
    std::vector<std::size_t> lebar(s.header.size() + 1, 0);
    lebar[0] = std::max<std::size_t>(2, static_cast<std::size_t>(digit(terlihat.size())));
    for (std::size_t c = 0; c < s.header.size(); ++c) {
        lebar[c + 1] = s.header[c].size();
        for (const Baris& b : s.baris) {
            if (c < b.size()) lebar[c + 1] = std::max(lebar[c + 1], b[c].size());
        }
    }

    bersihkanLayar();
    cetakJudul(s.judul, pesan);
    for (const std::string& baris : s.info) std::cout << baris << "\n";

    std::string ringkas;
    if (!filter.empty()) ringkas += "Filter: \"" + filter + "\" | ";
    if (n == 0) ringkas += "Data 0 dari 0";
    else        ringkas += "Data " + std::to_string(mulai + 1) + "-" + std::to_string(akhir) + " dari " + std::to_string(n);
    ringkas += " | Halaman " + std::to_string(halaman) + "/" + std::to_string(semuaHalaman);
    std::cout << ringkas << "\n\n";

    Baris kepala{"No"};
    kepala.insert(kepala.end(), s.header.begin(), s.header.end());
    cetakBaris(kepala, lebar);
    if (n == 0) {
        std::cout << "(tidak ada data)\n";
    } else {
        for (int i = mulai; i < akhir; ++i) {
            Baris sel{std::to_string(i + 1)};
            const Baris& b = s.baris[static_cast<std::size_t>(terlihat[static_cast<std::size_t>(i)])];
            sel.insert(sel.end(), b.begin(), b.end());
            cetakBaris(sel, lebar);
        }
    }
    std::cout << "\nType -1 for search | Type page 1-" << semuaHalaman << " | Type 0 back: " << std::flush;
}

void tampilRingkasan(const std::string& judul, const std::vector<std::string>& baris, const std::string& pesan) {
    bersihkanLayar();
    cetakJudul(judul, pesan);
    for (const std::string& b : baris) std::cout << b << "\n";
    std::cout << "\nTekan Enter untuk kembali..." << std::flush;
}

void tampilInput(const std::string& judul, const std::vector<std::string>& info,
                 const std::string& pesan, const std::string& prompt) {
    bersihkanLayar();
    cetakJudul(judul, pesan);
    for (const std::string& b : info) std::cout << b << "\n";
    std::cout << "\n" << prompt << std::flush;
}

void cetakPrompt(const std::string& teks) { std::cout << teks << std::flush; }

void tampilSelesai() {
    bersihkanLayar();
    std::cout << "Sampai jumpa.\n";
}
