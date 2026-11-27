#include "detector.h"

#include <algorithm>
#include <cstdio>
#include <vector>

#include "config.h"

namespace {

// Parameter internal detektor (bukan bagian ambang yang bisa diubah user).
constexpr double BATAS_ALIRAN  = 0.50;  // porsi dana masuk dari rekening terindikasi agar ikut ditandai
constexpr int    MAKS_PUTARAN  = 8;     // batas lapis penelusuran aliran dana

Ambang ambangAktif;   // nilai awal dari default member Ambang

// Total dana yang diterima tiap rekening dari rekening yang sedang bertanda terindikasi.
void hitungDariTerindikasi(const Dataset& d, const std::vector<char>& terindikasi,
                           std::vector<long long>& dari) {
    const int n = static_cast<int>(d.rek.size());
    std::fill(dari.begin(), dari.end(), 0LL);
    for (const Trx& t : d.trx) {
        if (t.dari < 0 || t.dari >= n || t.ke < 0 || t.ke >= n) continue;
        if (terindikasi[t.dari]) dari[t.ke] += t.nom;
    }
}

}  // namespace

Ambang ambangSaatIni() { return ambangAktif; }

bool ambangValid(const Ambang& a, std::string& alasan) {
    if (a.minTrxMasuk < 1 || a.minTrxMasuk > MAX_TRX) {
        alasan = "Minimal trx masuk harus 1 sampai " + std::to_string(MAX_TRX);
        return false;
    }
    if (!(a.minRasio > 0.0 && a.minRasio <= 1.0)) {
        alasan = "Rasio harus lebih dari 0 dan maksimal 1";
        return false;
    }
    return true;
}

bool aturAmbang(const Ambang& a, std::string& alasan) {
    if (!ambangValid(a, alasan)) return false;
    ambangAktif = a;
    return true;
}

std::string teksAmbang(const Ambang& a) {
    char rasio[32];
    std::snprintf(rasio, sizeof rasio, "%.2f", a.minRasio);
    return "min " + std::to_string(a.minTrxMasuk) + " trx masuk, rasio >= " + rasio;
}

HasilDeteksi deteksi(Dataset& d, const Ambang& a) {
    const int n = static_cast<int>(d.rek.size());
    std::vector<char>      terindikasi(n, 0), lewatAliran(n, 0);
    std::vector<double>    rasio(n, 0.0);
    std::vector<long long> dari(n, 0);

    // Tahap 1: aturan langsung dari agregat rekening.
    for (int i = 0; i < n; ++i) {
        const Rek& r = d.rek[i];
        rasio[i] = r.in > 0 ? static_cast<double>(r.out) / static_cast<double>(r.in) : 0.0;
        if (r.masuk >= a.minTrxMasuk && rasio[i] >= a.minRasio) terindikasi[i] = 1;
    }

    // Tahap 2: telusuri aliran dana per lapis. Pembaruan serentak per putaran,
    // sehingga satu putaran = satu tingkat berlapis.
    for (int putaran = 0; putaran < MAKS_PUTARAN; ++putaran) {
        hitungDariTerindikasi(d, terindikasi, dari);
        bool berubah = false;
        for (int i = 0; i < n; ++i) {
            if (terindikasi[i]) continue;
            const Rek& r = d.rek[i];
            if (r.in <= 0 || rasio[i] < a.minRasio) continue;
            if (static_cast<double>(dari[i]) >= BATAS_ALIRAN * static_cast<double>(r.in)) {
                terindikasi[i] = 1;
                lewatAliran[i] = 1;
                berubah = true;
            }
        }
        if (!berubah) break;
    }

    // Tulis hasil: hanya rasio, flag, catatan, status.
    HasilDeteksi h;
    h.total = n;
    for (int i = 0; i < n; ++i) {
        Rek& r = d.rek[i];
        r.rasio = rasio[i];
        r.flag  = terindikasi[i] != 0;
        if (r.flag) {
            ++h.terindikasi;
            if (lewatAliran[i]) ++h.lewatAliran;
            else                ++h.langsung;
        }
    }

    d.catatan = "Ambang " + teksAmbang(a) + "; " + std::to_string(h.terindikasi) + " dari " +
                std::to_string(h.total) + " rekening terindikasi (" + std::to_string(h.langsung) +
                " langsung, " + std::to_string(h.lewatAliran) + " lewat aliran dana)";
    d.status = Status::SUDAH;
    return h;
}

HasilDeteksi deteksi(Dataset& d) { return deteksi(d, ambangAktif); }