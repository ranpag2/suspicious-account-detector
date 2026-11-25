#include "evaluation.h"

#include <algorithm>
#include <cstdio>

#include "config.h"
#include "detector.h"

namespace {

double bagi(double a, double b) { return b > 0.0 ? a / b : 0.0; }

std::string dua(double v) {
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.2f", v);
    return buf;
}

const char* const TANPA_POLA = "(tanpa pola)";

}  // namespace

bool bisaDievaluasi(const Dataset& d, const LabelSet& l, std::string& alasan) {
    if (d.status != Status::SUDAH) {
        alasan = "Dataset belum dideteksi. Jalankan deteksi dahulu.";
        return false;
    }
    if (l.size() != d.rek.size()) {
        alasan = "Jumlah label tidak sama dengan jumlah rekening";
        return false;
    }
    return true;
}

Konfusi hitungKonfusi(const Dataset& d, const LabelSet& l) {
    Konfusi k;
    const std::size_t n = std::min(d.rek.size(), l.size());
    for (std::size_t i = 0; i < n; ++i) {
        const bool jahat = l[i].jahat, tanda = d.rek[i].flag;
        if (jahat && tanda)        ++k.tp;
        else if (!jahat && tanda)  ++k.fp;
        else if (jahat && !tanda)  ++k.fn;
        else                       ++k.tn;
    }
    return k;
}

Metrik hitungMetrik(const Konfusi& k) {
    Metrik m;
    m.k = k;
    m.precision = bagi(k.tp, static_cast<double>(k.tp) + k.fp);
    m.recall    = bagi(k.tp, static_cast<double>(k.tp) + k.fn);
    m.f1        = bagi(2.0 * m.precision * m.recall, m.precision + m.recall);
    return m;
}

Metrik evaluasi(const Dataset& d, const LabelSet& l) { return hitungMetrik(hitungKonfusi(d, l)); }

std::vector<int> daftarFN(const Dataset& d, const LabelSet& l) {
    std::vector<int> hasil;
    const int n = static_cast<int>(std::min(d.rek.size(), l.size()));
    for (int i = 0; i < n; ++i) {
        if (l[i].jahat && !d.rek[i].flag) hasil.push_back(i);
    }
    return hasil;
}

std::vector<int> daftarFP(const Dataset& d, const LabelSet& l) {
    std::vector<int> hasil;
    const int n = static_cast<int>(std::min(d.rek.size(), l.size()));
    for (int i = 0; i < n; ++i) {
        if (!l[i].jahat && d.rek[i].flag) hasil.push_back(i);
    }
    return hasil;
}

std::string alasanLolos(const Rek& r, const Ambang& a) {
    if (r.in <= 0) return "tidak ada dana masuk";

    const double rasio = static_cast<double>(r.out) / static_cast<double>(r.in);
    std::vector<std::string> bagian;
    if (r.masuk < a.minTrxMasuk)
        bagian.push_back("trx masuk " + std::to_string(r.masuk) + " < " + std::to_string(a.minTrxMasuk));
    if (rasio < a.minRasio)
        bagian.push_back("rasio " + dua(rasio) + " < " + dua(a.minRasio));

    if (bagian.empty()) return "memenuhi ambang, tetapi tidak ditandai oleh deteksi";

    // Jalur kedua (aliran dana) hanya terbuka untuk rekening yang meneruskan dana.
    if (rasio >= a.minRasio) bagian.push_back("dana masuknya tidak cukup berasal dari rekening terindikasi");

    std::string teks;
    for (std::size_t i = 0; i < bagian.size(); ++i) {
        if (i) teks += "; ";
        teks += bagian[i];
    }
    return teks;
}

std::vector<RecallPola> recallPerPola(const Dataset& d, const LabelSet& l) {
    std::vector<RecallPola> hasil;
    const std::size_t n = std::min(d.rek.size(), l.size());
    for (std::size_t i = 0; i < n; ++i) {
        if (!l[i].jahat) continue;
        const std::string nama = l[i].pola.empty() ? TANPA_POLA : l[i].pola;
        auto it = std::find_if(hasil.begin(), hasil.end(),
                               [&](const RecallPola& p) { return p.pola == nama; });
        if (it == hasil.end()) {
            hasil.push_back(RecallPola{nama, 0, 0});
            it = hasil.end() - 1;
        }
        ++it->jahat;
        if (d.rek[i].flag) ++it->terdeteksi;
    }
    return hasil;
}

Ambang ambangTerbaik(const Dataset& d, const LabelSet& l, double& f1Terbaik) {
    static const int TRX[] = {1, 2, 3, 5, 8, 10, 15, 20, 25, 30, 40, 50, 75, 100};

    Dataset salinan = d;   // deteksi menimpa hasil, jadi dikerjakan pada salinan
    Ambang terbaik;
    f1Terbaik = -1.0;
    for (int trx : TRX) {
        for (int persenRasio = 30; persenRasio <= 100; persenRasio += 5) {
            Ambang a;
            a.minTrxMasuk = trx;
            a.minRasio    = persenRasio / 100.0;
            deteksi(salinan, a);
            const double f1 = evaluasi(salinan, l).f1;
            if (f1 > f1Terbaik) {
                f1Terbaik = f1;
                terbaik = a;
            }
        }
    }
    if (f1Terbaik < 0.0) f1Terbaik = 0.0;
    return terbaik;
}
