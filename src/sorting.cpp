#include "sorting.h"

namespace {

double nilaiKunci(const Rek& r, KunciRek kunci) {
    switch (kunci) {
        case KunciRek::SKOR:       return r.skor;
        case KunciRek::RASIO:      return r.rasio;
        case KunciRek::DANA_MASUK: return static_cast<double>(r.in);
        case KunciRek::DANA_KELUAR:return static_cast<double>(r.out);
        case KunciRek::TRX_MASUK:  return static_cast<double>(r.masuk);
        case KunciRek::TRX_KELUAR: return static_cast<double>(r.keluar);
    }
    return 0.0;
}

}  // namespace

std::vector<int> urutRekening(const std::vector<Rek>& rek, KunciRek kunci,
                              bool hanyaTerindikasi, std::size_t top) {
    std::vector<int> idx;
    idx.reserve(rek.size());
    for (int i = 0; i < static_cast<int>(rek.size()); ++i) {
        if (!hanyaTerindikasi || rek[i].flag) idx.push_back(i);
    }
    // Sort stabil dari indeks naik, jadi seri otomatis berurutan menurut indeks.
    auto lebihDulu = [&](int a, int b) { return nilaiKunci(rek[a], kunci) > nilaiKunci(rek[b], kunci); };
    const std::size_t k = (top == 0 || top > idx.size()) ? idx.size() : top;
    selectionSortTop(idx, k, lebihDulu);
    idx.resize(k);
    return idx;
}

std::vector<int> urutTransaksiNominal(const std::vector<Trx>& trx, std::size_t top) {
    std::vector<int> idx(trx.size());
    for (int i = 0; i < static_cast<int>(trx.size()); ++i) idx[i] = i;
    auto lebihDulu = [&](int a, int b) { return trx[a].nom > trx[b].nom; };
    const std::size_t k = (top == 0 || top > idx.size()) ? idx.size() : top;
    selectionSortTop(idx, k, lebihDulu);
    idx.resize(k);
    return idx;
}