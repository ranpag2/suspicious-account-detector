#include "invarian.h"

#include <algorithm>

#include "config.h"
#include "dataset.h"      // formatNoRek
#include "detector.h"
#include "searching.h"
#include "sorting.h"

namespace {

// Setiap pengecekan mengembalikan string kosong jika lulus, atau alasan jika gagal.
using Alasan = std::string;

Alasan cekJumlahParalel(std::size_t nd, std::size_t nl) {
    if (nd == nl) return "";
    return "dataset.size() = " + std::to_string(nd) + ", label.size() = " + std::to_string(nl);
}

Alasan cekLabelSamaRek(const Dataset& d, const LabelSet& l) {
    if (l.size() == d.rek.size()) return "";
    return "label " + std::to_string(l.size()) + " != rekening " + std::to_string(d.rek.size());
}

Alasan cekBatas(const Dataset& d) {
    if (d.rek.empty() || d.rek.size() > static_cast<std::size_t>(MAX_REK))
        return "jumlah rekening " + std::to_string(d.rek.size()) + " di luar 1-" + std::to_string(MAX_REK);
    if (d.trx.empty() || d.trx.size() > static_cast<std::size_t>(MAX_TRX))
        return "jumlah transaksi " + std::to_string(d.trx.size()) + " di luar 1-" + std::to_string(MAX_TRX);
    return "";
}

Alasan cekTransaksi(const Dataset& d) {
    const int n = static_cast<int>(d.rek.size());
    for (std::size_t i = 0; i < d.trx.size(); ++i) {
        const Trx& t = d.trx[i];
        const std::string no = "transaksi " + std::to_string(i) + ": ";
        if (t.dari < 0 || t.dari >= n || t.ke < 0 || t.ke >= n) return no + "indeks rekening di luar jangkauan";
        if (t.dari == t.ke) return no + "pengirim sama dengan penerima";
        if (t.nom < NOM_MIN || t.nom > NOM_MAX) return no + "nominal di luar batas";
        if (t.jam < JAM_MIN || t.jam > JAM_MAX) return no + "jam di luar 0-23";
    }
    return "";
}

Alasan cekNomorRekening(const Dataset& d) {
    for (std::size_t i = 0; i < d.rek.size(); ++i) {
        if (d.rek[i].no != formatNoRek(static_cast<int>(i)))
            return "rekening ke-" + std::to_string(i) + " bernomor " + d.rek[i].no + ", seharusnya " +
                   formatNoRek(static_cast<int>(i));
    }
    if (!nomorTerurut(d.rek)) return "nomor rekening tidak terurut menaik";
    return "";
}

Alasan cekAgregat(const Dataset& d) {
    const std::size_t n = d.rek.size();
    std::vector<int>       masuk(n, 0), keluar(n, 0);
    std::vector<long long> in(n, 0), out(n, 0);
    for (const Trx& t : d.trx) {
        keluar[t.dari] += 1;
        out[t.dari]    += t.nom;
        masuk[t.ke]    += 1;
        in[t.ke]       += t.nom;
    }
    for (std::size_t i = 0; i < n; ++i) {
        const Rek& r = d.rek[i];
        if (r.masuk != masuk[i] || r.keluar != keluar[i] || r.in != in[i] || r.out != out[i])
            return "rekening " + r.no + ": agregat tidak cocok dengan transaksi";
    }
    return "";
}

Alasan cekTotalDana(const Dataset& d) {
    long long nom = 0, in = 0, out = 0;
    for (const Trx& t : d.trx) nom += t.nom;
    for (const Rek& r : d.rek) {
        in += r.in;
        out += r.out;
    }
    if (in == nom && out == nom) return "";
    return "total masuk " + std::to_string(in) + ", keluar " + std::to_string(out) +
           ", nominal transaksi " + std::to_string(nom);
}

Alasan cekStatus(const Dataset& d) {
    if (d.status == Status::BELUM) {
        for (const Rek& r : d.rek) {
            if (r.flag || r.skor != 0 || r.rasio != 0)
                return "dataset BELUM tetapi rekening " + r.no + " sudah punya hasil";
        }
        return "";
    }
    if (d.catatan.empty()) return "dataset SUDAH tetapi catatan kosong";
    for (const Rek& r : d.rek) {
        if (r.skor < 0 || r.skor > 1) return "rekening " + r.no + ": skor di luar 0-1";
    }
    return "";
}

Alasan cekAdaJahatDanSah(const LabelSet& l) {
    bool jahat = false, sah = false;
    for (const Label& x : l) (x.jahat ? jahat : sah) = true;
    if (jahat && sah) return "";
    return jahat ? "tidak ada rekening sah" : "tidak ada rekening jahat";
}

Alasan cekPolaTercatat(const LabelSet& l) {
    for (std::size_t i = 0; i < l.size(); ++i) {
        if (l[i].jahat && l[i].pola.empty())
            return "rekening ke-" + std::to_string(i) + " jahat tetapi Label.pola kosong";
    }
    return "";
}

bool sama(const std::vector<Trx>& a, const std::vector<Trx>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].dari != b[i].dari || a[i].ke != b[i].ke || a[i].nom != b[i].nom || a[i].jam != b[i].jam)
            return false;
    }
    return true;
}

Alasan cekDeteksiTidakMengubahData(const Dataset& d) {
    Dataset salinan = d;
    deteksi(salinan);
    if (!sama(d.trx, salinan.trx)) return "transaksi berubah setelah deteksi";
    if (salinan.trx.size() != d.trx.size()) return "jumlah transaksi berubah setelah deteksi";
    for (std::size_t i = 0; i < d.rek.size(); ++i) {
        const Rek &a = d.rek[i], &b = salinan.rek[i];
        if (a.no != b.no || a.masuk != b.masuk || a.keluar != b.keluar || a.in != b.in || a.out != b.out)
            return "agregat rekening " + a.no + " berubah setelah deteksi";
    }
    return "";
}

Alasan cekDeteksiDeterministik(const Dataset& d) {
    Dataset a = d, b = d;
    deteksi(a);
    deteksi(b);
    deteksi(b);   // deteksi ulang tidak boleh bergantung pada hasil sebelumnya
    if (a.catatan != b.catatan) return "catatan berbeda pada deteksi ulang";
    for (std::size_t i = 0; i < a.rek.size(); ++i) {
        if (a.rek[i].flag != b.rek[i].flag || a.rek[i].skor != b.rek[i].skor || a.rek[i].rasio != b.rek[i].rasio)
            return "rekening " + a.rek[i].no + " berbeda pada deteksi ulang";
    }
    return "";
}

Alasan cekPencarianNomor(const Dataset& d) {
    for (std::size_t i = 0; i < d.rek.size(); ++i) {
        if (cariRekening(d.rek, d.rek[i].no) != static_cast<int>(i))
            return "pencarian " + d.rek[i].no + " tidak menemukan indeks " + std::to_string(i);
    }
    return "";
}

Alasan cekPeringkatMenurun(const Dataset& d) {
    Dataset salinan = d;
    deteksi(salinan);
    const std::vector<int> urut = urutRekening(salinan.rek, KunciRek::SKOR);
    if (urut.size() != salinan.rek.size()) return "peringkat tidak memuat semua rekening";
    for (std::size_t i = 1; i < urut.size(); ++i) {
        if (salinan.rek[urut[i - 1]].skor < salinan.rek[urut[i]].skor) return "peringkat skor tidak menurun";
    }
    return "";
}

void tambah(std::vector<HasilTest>& hasil, const char* nama, const Alasan& alasan) {
    HasilTest t;
    t.nama   = nama;
    t.lulus  = alasan.empty();
    t.detail = alasan;
    hasil.push_back(t);
}

}  // namespace

std::vector<HasilTest> jalankanTest(const Dataset& d, const LabelSet& l,
                                    std::size_t jumlahDataset, std::size_t jumlahLabel) {
    std::vector<HasilTest> hasil;

    tambah(hasil, "dataset.size() == label.size()", cekJumlahParalel(jumlahDataset, jumlahLabel));
    tambah(hasil, "Jumlah label sama dengan jumlah rekening", cekLabelSamaRek(d, l));

    const Alasan batas = cekBatas(d);
    tambah(hasil, "Batas jumlah rekening dan transaksi", batas);

    // Pengecekan di bawah mengindeks rek lewat trx, jadi hanya aman jika trx sah.
    Alasan trx = batas.empty() ? cekTransaksi(d) : "dilewati: batas jumlah dilanggar";
    tambah(hasil, "Nilai tiap transaksi sah", trx);
    const bool dasarSah = batas.empty() && trx.empty();
    const Alasan lewati = "dilewati: transaksi tidak sah";

    tambah(hasil, "Nomor rekening berurutan", dasarSah ? cekNomorRekening(d) : lewati);
    tambah(hasil, "Agregat cocok dengan transaksi", dasarSah ? cekAgregat(d) : lewati);
    tambah(hasil, "Total masuk = total keluar = total nominal", dasarSah ? cekTotalDana(d) : lewati);
    tambah(hasil, "Aturan status dataset", cekStatus(d));
    tambah(hasil, "Memuat rekening jahat dan sah", cekAdaJahatDanSah(l));
    tambah(hasil, "Rekening jahat punya catatan pola", cekPolaTercatat(l));

    tambah(hasil, "Deteksi tidak mengubah transaksi dan agregat", dasarSah ? cekDeteksiTidakMengubahData(d) : lewati);
    tambah(hasil, "Deteksi ulang menghasilkan keluaran sama", dasarSah ? cekDeteksiDeterministik(d) : lewati);
    tambah(hasil, "Pencarian nomor rekening konsisten", dasarSah ? cekPencarianNomor(d) : lewati);
    tambah(hasil, "Peringkat skor terurut menurun", dasarSah ? cekPeringkatMenurun(d) : lewati);
    return hasil;
}

int jumlahLulus(const std::vector<HasilTest>& hasil) {
    return static_cast<int>(std::count_if(hasil.begin(), hasil.end(), [](const HasilTest& t) { return t.lulus; }));
}
