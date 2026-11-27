#include "statistics.h"

#include <algorithm>

#include "config.h"
#include "sorting.h"

namespace {

struct BatasKelompok {
    const char* label;
    long long   bawah;   // nominal >= bawah masuk kelompok ini (sampai batas kelompok berikutnya)
};

// Lima kelompok tetap, supaya hasil antar dataset bisa dibandingkan.
const BatasKelompok KELOMPOK[] = {
    {"< 50.000", 0},
    {"50.000 - 249.999", 50'000},
    {"250.000 - 999.999", 250'000},
    {"1.000.000 - 4.999.999", 1'000'000},
    {">= 5.000.000", 5'000'000},
};
constexpr int JUMLAH_KELOMPOK = 5;

int kelompokDari(long long nom) {
    int k = 0;
    for (int i = 0; i < JUMLAH_KELOMPOK; ++i) {
        if (nom >= KELOMPOK[i].bawah) k = i;
    }
    return k;
}

bool jamSah(int jam) { return jam >= JAM_MIN && jam <= JAM_MAX; }

}  // namespace

RingkasanUmum ringkasanUmum(const Dataset& d) {
    RingkasanUmum r;
    r.rekening  = static_cast<int>(d.rek.size());
    r.transaksi = static_cast<int>(d.trx.size());

    std::vector<int> perJam(JAM_MAX + 1, 0);
    for (const Trx& t : d.trx) {
        r.totalNominal += t.nom;
        r.nominalTerbesar = std::max(r.nominalTerbesar, t.nom);
        if (jamSah(t.jam)) ++perJam[t.jam];
    }
    if (r.transaksi > 0) r.rataNominal = r.totalNominal / r.transaksi;
    for (int j = JAM_MIN; j <= JAM_MAX; ++j) {
        if (perJam[j] > r.trxJamTersibuk) {
            r.trxJamTersibuk = perJam[j];
            r.jamTersibuk = j;
        }
    }

    r.sudahDeteksi = d.status == Status::SUDAH;
    if (r.sudahDeteksi) {
        for (const Rek& rek : d.rek) {
            if (rek.flag) ++r.terindikasi;
        }
    }
    return r;
}

std::vector<BarisJam> statistikPerJam(const Dataset& d) {
    std::vector<BarisJam> baris(JAM_MAX + 1);
    for (int j = JAM_MIN; j <= JAM_MAX; ++j) baris[j].jam = j;
    for (const Trx& t : d.trx) {
        if (!jamSah(t.jam)) continue;
        baris[t.jam].trx += 1;
        baris[t.jam].total += t.nom;
    }
    for (BarisJam& b : baris) {
        if (b.trx > 0) b.rata = b.total / b.trx;
    }
    return baris;
}

std::vector<KelompokNominal> distribusiNominal(const Dataset& d) {
    std::vector<KelompokNominal> hasil(JUMLAH_KELOMPOK);
    for (int i = 0; i < JUMLAH_KELOMPOK; ++i) hasil[i].label = KELOMPOK[i].label;
    for (const Trx& t : d.trx) {
        KelompokNominal& k = hasil[kelompokDari(t.nom)];
        k.jumlah += 1;
        k.total += t.nom;
    }
    return hasil;
}

std::vector<int> peringkatTerindikasi(const Dataset& d) {
    if (d.status != Status::SUDAH) return {};
    return urutRekening(d.rek, KunciRek::RASIO, /*hanyaTerindikasi=*/true);
}

std::vector<int> peringkatPengirim(const Dataset& d) {
    std::vector<int> urut = urutRekening(d.rek, KunciRek::DANA_KELUAR);
    urut.erase(std::remove_if(urut.begin(), urut.end(), [&](int i) { return d.rek[i].keluar <= 0; }),
               urut.end());
    return urut;
}

std::vector<int> transaksiTerbesar(const Dataset& d) {
    return urutTransaksiNominal(d.trx, static_cast<std::size_t>(TOP_TRX_TERBESAR));
}

std::vector<int> aliranKeluar(const Dataset& d, int rekening) {
    std::vector<int> idx;
    for (int i = 0; i < static_cast<int>(d.trx.size()); ++i) {
        if (d.trx[i].dari == rekening) idx.push_back(i);
    }
    selectionSort(idx, [&](int a, int b) { return d.trx[a].nom > d.trx[b].nom; });
    return idx;
}
