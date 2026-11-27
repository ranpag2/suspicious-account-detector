#include "dataset.h"

#include <utility>

#include "config.h"

// Dua penyimpanan paralel. `label` dideklarasikan di label.h; sengaja tidak
// di-include di sini supaya aturan "hanya Red, Purple, dan menu" tetap sederhana.
std::vector<Dataset>  dataset;
std::vector<LabelSet> label;

int jumlahDataset() { return static_cast<int>(dataset.size()); }

bool indeksValid(int k) { return k >= 0 && k < jumlahDataset(); }

int cariSeed(unsigned seed) {
    for (int k = 0; k < jumlahDataset(); ++k) {
        if (dataset[k].seed == seed) return k;
    }
    return -1;
}

std::string formatNoRek(int indeks) {
    std::string angka = std::to_string(indeks + 1);
    const int panjang = static_cast<int>(angka.size());
    if (panjang < NO_REK_LEBAR) angka = std::string(NO_REK_LEBAR - panjang, '0') + angka;
    return std::string(NO_REK_AWAL) + angka;
}

void hitungAgregat(Dataset& d) {
    for (Rek& r : d.rek) {
        r.masuk = 0;
        r.keluar = 0;
        r.in = 0;
        r.out = 0;
    }
    const int nRek = static_cast<int>(d.rek.size());
    for (const Trx& t : d.trx) {
        if (t.dari < 0 || t.dari >= nRek || t.ke < 0 || t.ke >= nRek) continue;
        d.rek[t.dari].keluar += 1;
        d.rek[t.dari].out += t.nom;
        d.rek[t.ke].masuk += 1;
        d.rek[t.ke].in += t.nom;
    }
}

bool validasiDataset(const Dataset& d, std::string& alasan) {
    const int nRek = static_cast<int>(d.rek.size());
    const int nTrx = static_cast<int>(d.trx.size());

    if (nRek <= 0 || nRek > MAX_REK) {
        alasan = "Jumlah rekening di luar batas: " + std::to_string(nRek);
        return false;
    }
    if (nTrx <= 0 || nTrx > MAX_TRX) {
        alasan = "Jumlah transaksi di luar batas: " + std::to_string(nTrx);
        return false;
    }

    std::vector<int>       masuk(nRek, 0), keluar(nRek, 0);
    std::vector<long long> in(nRek, 0), out(nRek, 0);
    for (int i = 0; i < nTrx; ++i) {
        const Trx& t = d.trx[i];
        const std::string no = "Transaksi " + std::to_string(i);
        if (t.dari < 0 || t.dari >= nRek || t.ke < 0 || t.ke >= nRek) {
            alasan = no + ": indeks rekening di luar jangkauan";
            return false;
        }
        if (t.dari == t.ke) {
            alasan = no + ": pengirim sama dengan penerima";
            return false;
        }
        if (t.nom < NOM_MIN || t.nom > NOM_MAX) {
            alasan = no + ": nominal di luar batas";
            return false;
        }
        if (t.jam < JAM_MIN || t.jam > JAM_MAX) {
            alasan = no + ": jam di luar 0-23";
            return false;
        }
        keluar[t.dari] += 1;
        out[t.dari] += t.nom;
        masuk[t.ke] += 1;
        in[t.ke] += t.nom;
    }

    for (int i = 0; i < nRek; ++i) {
        const Rek& r = d.rek[i];
        if (r.no != formatNoRek(i)) {
            alasan = "Rekening " + std::to_string(i) + ": nomor seharusnya " + formatNoRek(i);
            return false;
        }
        if (r.masuk != masuk[i] || r.keluar != keluar[i] || r.in != in[i] || r.out != out[i]) {
            alasan = "Rekening " + r.no + ": agregat tidak cocok dengan transaksi";
            return false;
        }
    }

    if (d.status == Status::BELUM) {
        for (const Rek& r : d.rek) {
            if (r.flag || r.rasio != 0) {
                alasan = "Dataset BELUM dideteksi tetapi rekening " + r.no + " sudah punya hasil";
                return false;
            }
        }
    } else {
        if (d.catatan.empty()) {
            alasan = "Dataset SUDAH dideteksi tetapi catatan kosong";
            return false;
        }
    }
    return true;
}

int tambahDataset(Dataset d, LabelSet l, std::string& alasan) {
    if (!validasiDataset(d, alasan)) return -1;

    if (d.status != Status::BELUM) {
        alasan = "Dataset baru harus berstatus BELUM";
        return -1;
    }
    if (l.size() != d.rek.size()) {
        alasan = "Jumlah label tidak sama dengan jumlah rekening";
        return -1;
    }

    bool adaJahat = false, adaSah = false;
    for (const Label& x : l) {
        if (x.jahat) adaJahat = true;
        else         adaSah = true;
    }
    if (!adaJahat || !adaSah) {
        alasan = "Dataset harus memuat minimal satu rekening jahat dan satu rekening sah";
        return -1;
    }

    const int duplikat = cariSeed(d.seed);
    if (duplikat >= 0) {
        alasan = "Seed sudah ada (dataset " + std::to_string(duplikat + 1) + ")";
        return -1;
    }

    dataset.push_back(std::move(d));
    label.push_back(std::move(l));
    return jumlahDataset() - 1;
}