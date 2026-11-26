#include "menu.h"

#include <string>
#include <vector>

#include "appreance.h"
#include "config.h"
#include "dataset.h"
#include "detector.h"
#include "evaluation.h"
#include "generator.h"    // Red: int buatDataset(unsigned seed, std::string& alasan)
#include "searching.h"
#include "statistics.h"
#include "util.h"

namespace {

// ---- Dasar: pesan satu kali dan pembacaan input ----------------------------

// Dilempar saat input habis (EOF); ditangkap di menuUtama supaya keluar bersih.
struct InputHabis {};

// Pesan error/info tampil satu kali di page berikutnya (menu.md 8.3).
std::string gPesan;

void pesan(const std::string& teks) { gPesan = teks; }

std::string ambilPesan() {
    std::string p;
    p.swap(gPesan);
    return p;
}

std::string pangkas(const std::string& s) {
    std::size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) --b;
    return s.substr(a, b - a);
}

std::string bacaTeks() {
    std::string s;
    if (bacaBaris(s) == StatusBaca::SELESAI) throw InputHabis{};
    return s;
}

// Membaca satu pilihan angka. Jika tidak valid, menyiapkan pesan dan mengembalikan false
// (pemanggil cukup merender ulang page).
bool bacaPilihan(int& v, int min, int max) {
    const StatusBaca st = parseInt(bacaTeks(), v, min, max);
    if (st == StatusBaca::OK) return true;
    pesan(st == StatusBaca::BUKAN_ANGKA ? "Masukkan angka." : "Pilihan tidak ada.");
    return false;
}

// ---- Ambang yang dipakai saat deteksi tiap dataset -------------------------
// Dicatat di sini (bukan di Dataset, yang kontraknya dipegang Red/Blue) supaya
// menu 4.x menjelaskan hasil dengan ambang yang benar walau ambang aktif sudah diubah.
std::vector<Ambang> gAmbangDeteksi;
std::vector<char>   gAdaAmbang;

void catatAmbang(int k, const Ambang& a) {
    const std::size_t n = static_cast<std::size_t>(k) + 1;
    if (gAmbangDeteksi.size() < n) {
        gAmbangDeteksi.resize(n);
        gAdaAmbang.resize(n, 0);
    }
    gAmbangDeteksi[static_cast<std::size_t>(k)] = a;
    gAdaAmbang[static_cast<std::size_t>(k)] = 1;
}

Ambang ambangUntuk(int k) {
    const std::size_t i = static_cast<std::size_t>(k);
    if (i < gAdaAmbang.size() && gAdaAmbang[i]) return gAmbangDeteksi[i];
    return ambangSaatIni();
}

// ---- Page generik -----------------------------------------------------------

// Page "Pilih dataset". Mengembalikan indeks 0-based, atau -1 jika user memilih 0.
int pilihDataset(const std::string& judul) {
    for (;;) {
        tampilPilihDataset(judul, ambilPesan());
        int v = 0;
        if (!bacaPilihan(v, 0, jumlahDataset())) continue;
        return v - 1;
    }
}

void halamanRingkasan(const std::string& judul, const std::vector<std::string>& baris) {
    tampilRingkasan(judul, baris, ambilPesan());
    bacaTeks();
}

// Page hasil berbentuk tabel: paging, search, dan navigasi ditangani di sini sekali,
// bukan per menu (menu.md 8.8). Kembali dengan 0.
void halamanTabel(const SpecTabel& s) {
    std::vector<int> semua(s.baris.size());
    for (std::size_t i = 0; i < semua.size(); ++i) semua[i] = static_cast<int>(i);

    std::vector<int> terlihat = semua;
    std::string filter;
    int halaman = 1;

    for (;;) {
        tampilTabel(s, terlihat, halaman, filter, ambilPesan());
        const std::string masukan = pangkas(bacaTeks());

        int v = 0;
        if (parseInt(masukan, v) == StatusBaca::OK) {
            if (v == 0) return;
            if (v == -1) {
                cetakPrompt("Search: ");
                const std::string kata = pangkas(bacaTeks());
                if (kata.empty()) {                       // kosong = hapus filter
                    filter.clear();
                    terlihat = semua;
                    halaman = 1;
                } else {
                    const std::vector<int> hasil = saringBaris(s.baris, s.kolomCari, kata);
                    if (hasil.empty()) {                  // filter lama tetap
                        pesan("Tidak ada hasil untuk \"" + kata + "\"");
                    } else {
                        filter = kata;
                        terlihat = hasil;
                        halaman = 1;
                    }
                }
                continue;
            }
            if (v >= 1 && v <= jumlahHalaman(terlihat.size())) {
                halaman = v;
                continue;
            }
        }
        pesan("Halaman tidak ada");
    }
}

bool sudahDeteksi(int k) {
    if (dataset[static_cast<std::size_t>(k)].status == Status::SUDAH) return true;
    pesan("Dataset belum dideteksi. Jalankan deteksi dahulu.");
    return false;
}

bool adaLabel(int k) {
    if (static_cast<std::size_t>(k) < label.size()) return true;
    pesan("Kunci jawaban dataset ini tidak ditemukan.");
    return false;
}

std::string infoDataset(const Dataset& d) { return "Dataset: " + teksDataset(d); }
std::string infoAmbang(const Ambang& a) { return "Ambang: " + teksAmbang(a); }

// ---- Menu 1: Buat Data ------------------------------------------------------

void menuBuatData() {
    const std::string judul = "1. BUAT DATA";
    for (;;) {
        tampilInput(judul, {"Seed 0 = acak. Seed yang sama menghasilkan data yang sama."}, ambilPesan(),
                    "Seed (Enter = batal): ");
        const std::string masukan = pangkas(bacaTeks());
        if (masukan.empty()) return;

        unsigned seed = 0;
        const StatusBaca st = parseUnsigned(masukan, seed);
        if (st != StatusBaca::OK) {
            pesan(st == StatusBaca::BUKAN_ANGKA ? "Seed harus berupa angka bulat >= 0." : "Seed harus antara 0 dan 4294967295.");
            continue;
        }
        if (seed == SEED_ACAK) seed = seedAcak();

        std::string alasan;
        const int k = buatDataset(seed, alasan);
        if (k < 0) {
            pesan("Gagal membuat data: " + alasan);
            continue;
        }
        const Dataset& d = dataset[static_cast<std::size_t>(k)];
        halamanRingkasan(judul, {
            "Seed       : " + std::to_string(d.seed),
            "Dataset ke : " + std::to_string(k + 1),
            "Rekening   : " + titik(static_cast<long long>(d.rek.size())),
            "Transaksi  : " + titik(static_cast<long long>(d.trx.size())),
            "Status     : " + statusDataset(d),
        });
        return;
    }
}

// ---- Menu 2: Deteksi --------------------------------------------------------

void menuDeteksi() {
    const std::string judul = "2. DETEKSI";
    const int k = pilihDataset(judul);
    if (k < 0) return;

    Dataset& d = dataset[static_cast<std::size_t>(k)];
    const Ambang a = ambangSaatIni();
    const HasilDeteksi h = deteksi(d, a);
    catatAmbang(k, a);

    std::vector<std::string> baris = {
        infoAmbang(a),
        infoDataset(d),
        "Terindikasi: " + titik(h.terindikasi) + " dari " + titik(h.total) + " rekening (" +
            titik(h.langsung) + " langsung, " + titik(h.lewatAliran) + " lewat aliran dana)",
        "Status     : " + statusDataset(d),
    };
    std::string alasan;
    if (!validasiDataset(d, alasan)) baris.push_back("Peringatan: hasil deteksi melanggar invarian: " + alasan);
    halamanRingkasan(judul, baris);
}

// ---- Menu 3: Statistik ------------------------------------------------------

void statRingkasan() {
    const std::string judul = "3.1 RINGKASAN UMUM";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];
    const RingkasanUmum r = ringkasanUmum(d);

    std::vector<std::string> baris = {
        infoDataset(d),
        "Status         : " + statusDataset(d),
        "Rekening       : " + titik(r.rekening),
        "Transaksi      : " + titik(r.transaksi),
        "Total nominal  : " + titik(r.totalNominal),
        "Rata-rata trx  : " + titik(r.rataNominal),
        "Trx terbesar   : " + titik(r.nominalTerbesar),
        "Jam tersibuk   : " + std::to_string(r.jamTersibuk) + ":00 (" + titik(r.trxJamTersibuk) + " trx)",
    };
    if (r.sudahDeteksi)
        baris.push_back("Terindikasi    : " + titik(r.terindikasi) + " dari " + titik(r.rekening) + " rekening");
    halamanRingkasan(judul, baris);
}

void statTerindikasi() {
    const std::string judul = "3.2 PERINGKAT TERINDIKASI";
    const int k = pilihDataset(judul);
    if (k < 0 || !sudahDeteksi(k)) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoAmbang(ambangUntuk(k)), infoDataset(d)};
    s.header = {"Rek", "Trx", "Dana Masuk", "Dana Keluar", "Rasio", "Skor", "Status"};
    s.kolomCari = {0};
    for (int i : peringkatTerindikasi(d)) {
        const Rek& r = d.rek[static_cast<std::size_t>(i)];
        s.baris.push_back({r.no, std::to_string(r.masuk), std::to_string(r.in), std::to_string(r.out),
                           desimal(r.rasio), desimal(r.skor), statusRekening(d, r)});
    }
    halamanTabel(s);
}

void statPengirim() {
    const std::string judul = "3.3 PERINGKAT PENGIRIM";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoDataset(d), "Urut menurut total dana keluar"};
    s.header = {"Rek", "Trx Keluar", "Dana Keluar", "Status"};
    s.kolomCari = {0};
    for (int i : peringkatPengirim(d)) {
        const Rek& r = d.rek[static_cast<std::size_t>(i)];
        s.baris.push_back({r.no, std::to_string(r.keluar), std::to_string(r.out), statusRekening(d, r)});
    }
    halamanTabel(s);
}

void statTransaksiTerbesar() {
    const std::string judul = "3.4 TRANSAKSI TERBESAR";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoDataset(d), std::to_string(TOP_TRX_TERBESAR) + " transaksi dengan nominal terbesar"};
    s.header = {"Asal", "Tujuan", "Nominal", "Jam"};
    s.kolomCari = {0, 1};
    for (int i : transaksiTerbesar(d)) {
        const Trx& t = d.trx[static_cast<std::size_t>(i)];
        s.baris.push_back({d.rek[static_cast<std::size_t>(t.dari)].no, d.rek[static_cast<std::size_t>(t.ke)].no,
                           std::to_string(t.nom), std::to_string(t.jam)});
    }
    halamanTabel(s);
}

void statPerJam() {
    const std::string judul = "3.5 STATISTIK PER JAM";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoDataset(d)};
    s.header = {"Jam", "Trx", "Total Nominal", "Rata-rata"};
    s.kolomCari = {0};
    for (const BarisJam& b : statistikPerJam(d)) {
        s.baris.push_back({(b.jam < 10 ? "0" : "") + std::to_string(b.jam), std::to_string(b.trx),
                           std::to_string(b.total), std::to_string(b.rata)});
    }
    halamanTabel(s);
}

void statDistribusi() {
    const std::string judul = "3.6 DISTRIBUSI NOMINAL";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoDataset(d)};
    s.header = {"Kelompok", "Trx", "Total Nominal", "Porsi"};
    s.kolomCari = {0};
    for (const KelompokNominal& g : distribusiNominal(d)) {
        s.baris.push_back({g.label, std::to_string(g.jumlah), std::to_string(g.total),
                           persenTeks(persen(g.jumlah, static_cast<long long>(d.trx.size())))});
    }
    halamanTabel(s);
}

// Menerima nomor lengkap ("R0145", tidak peka huruf besar/kecil) atau angka saja ("145").
int cariRekeningMasukan(const Dataset& d, const std::string& masukan) {
    int i = cariRekening(d.rek, masukan);
    if (i >= 0) return i;
    int n = 0;
    if (parseInt(masukan, n, 1, MAX_REK) == StatusBaca::OK) return cariRekening(d.rek, formatNoRek(n - 1));
    return -1;
}

void statAliranDana() {
    const std::string judul = "3.7 ALIRAN DANA";
    const int k = pilihDataset(judul);
    if (k < 0) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];

    int idx = -1;
    for (;;) {
        tampilInput(judul, {infoDataset(d)}, ambilPesan(), "Nomor rekening (Enter = batal): ");
        const std::string masukan = pangkas(bacaTeks());
        if (masukan.empty()) return;
        idx = cariRekeningMasukan(d, masukan);
        if (idx >= 0) break;
        pesan("Rekening tidak ditemukan: " + masukan);
    }
    const Rek& asal = d.rek[static_cast<std::size_t>(idx)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoDataset(d), "Rekening: " + asal.no + " | Keluar " + titik(asal.keluar) + " trx, total " + titik(asal.out)};
    s.header = {"Tujuan", "Nominal", "Jam", "Status Tujuan"};
    s.kolomCari = {0};
    for (int i : aliranKeluar(d, idx)) {
        const Trx& t = d.trx[static_cast<std::size_t>(i)];
        const Rek& tujuan = d.rek[static_cast<std::size_t>(t.ke)];
        s.baris.push_back({tujuan.no, std::to_string(t.nom), std::to_string(t.jam), statusRekening(d, tujuan)});
    }
    halamanTabel(s);
}

void submenuStatistik() {
    const std::vector<std::string> opsi = {
        "Ringkasan umum", "Peringkat terindikasi", "Peringkat pengirim", "Transaksi terbesar",
        "Statistik per jam", "Distribusi nominal", "Aliran dana"};
    for (;;) {
        tampilSubmenu("3. STATISTIK", opsi, ambilPesan());
        int v = 0;
        if (!bacaPilihan(v, 0, static_cast<int>(opsi.size()))) continue;
        switch (v) {
            case 0: return;
            case 1: statRingkasan(); break;
            case 2: statTerindikasi(); break;
            case 3: statPengirim(); break;
            case 4: statTransaksiTerbesar(); break;
            case 5: statPerJam(); break;
            case 6: statDistribusi(); break;
            case 7: statAliranDana(); break;
        }
    }
}

// ---- Menu 4: Evaluasi -------------------------------------------------------

void evalPrecisionRecall() {
    const std::string judul = "4.1 PRECISION DAN RECALL";
    const int k = pilihDataset(judul);
    if (k < 0 || !sudahDeteksi(k) || !adaLabel(k)) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];
    const LabelSet& l = label[static_cast<std::size_t>(k)];

    std::string alasan;
    if (!bisaDievaluasi(d, l, alasan)) {
        pesan(alasan);
        return;
    }
    const Metrik m = evaluasi(d, l);

    std::vector<std::string> baris = {
        infoAmbang(ambangUntuk(k)),
        infoDataset(d),
        "TP " + std::to_string(m.k.tp) + " | FP " + std::to_string(m.k.fp) + " | FN " + std::to_string(m.k.fn) +
            " | TN " + std::to_string(m.k.tn),
        "Precision: " + persenTeks(m.precision * 100.0),
        "Recall   : " + persenTeks(m.recall * 100.0),
        "F1-score : " + persenTeks(m.f1 * 100.0),
    };
    const std::vector<RecallPola> pola = recallPerPola(d, l);
    if (!pola.empty()) {
        baris.push_back("");
        baris.push_back("Recall per pola:");
        for (const RecallPola& p : pola) {
            baris.push_back("  " + p.pola + ": " + std::to_string(p.terdeteksi) + "/" + std::to_string(p.jahat) +
                            " (" + persenTeks(persen(p.terdeteksi, p.jahat)) + ")");
        }
    }
    halamanRingkasan(judul, baris);
}

void evalLolos() {
    const std::string judul = "4.2 LOLOS TERDETEKSI (FALSE NEGATIVE)";
    const int k = pilihDataset(judul);
    if (k < 0 || !sudahDeteksi(k) || !adaLabel(k)) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];
    const LabelSet& l = label[static_cast<std::size_t>(k)];
    const Ambang a = ambangUntuk(k);

    SpecTabel s;
    s.judul = judul;
    s.info = {infoAmbang(a), infoDataset(d)};
    s.header = {"Rek", "Peran", "Pola", "Alasan lolos"};
    s.kolomCari = {0, 1};
    for (int i : daftarFN(d, l)) {
        const std::size_t u = static_cast<std::size_t>(i);
        s.baris.push_back({d.rek[u].no, l[u].peran, l[u].pola.empty() ? "-" : l[u].pola, alasanLolos(d.rek[u], a)});
    }
    halamanTabel(s);
}

void evalSalahTandai() {
    const std::string judul = "4.3 SALAH TANDAI (FALSE POSITIVE)";
    const int k = pilihDataset(judul);
    if (k < 0 || !sudahDeteksi(k) || !adaLabel(k)) return;
    const Dataset& d = dataset[static_cast<std::size_t>(k)];
    const LabelSet& l = label[static_cast<std::size_t>(k)];

    SpecTabel s;
    s.judul = judul;
    s.info = {infoAmbang(ambangUntuk(k)), infoDataset(d)};
    s.header = {"Rek", "Peran", "Trx Masuk", "Rasio", "Skor"};
    s.kolomCari = {0, 1};
    for (int i : daftarFP(d, l)) {
        const std::size_t u = static_cast<std::size_t>(i);
        s.baris.push_back({d.rek[u].no, l[u].peran, std::to_string(d.rek[u].masuk), desimal(d.rek[u].rasio),
                           desimal(d.rek[u].skor)});
    }
    halamanTabel(s);
}

void evalUbahAmbang() {
    const std::string judul = "4.4 UBAH AMBANG BATAS";
    Ambang a = ambangSaatIni();
    const std::vector<std::string> info = {"Ambang saat ini: " + teksAmbang(a), "Kosongkan isian untuk tidak mengubah."};

    for (;;) {   // minimal trx masuk
        tampilInput(judul, info, ambilPesan(), "Minimal trx masuk [" + std::to_string(a.minTrxMasuk) + "]: ");
        const std::string masukan = pangkas(bacaTeks());
        if (masukan.empty()) break;
        int v = 0;
        const StatusBaca st = parseInt(masukan, v, 1, MAX_TRX);
        if (st == StatusBaca::OK) {
            a.minTrxMasuk = v;
            break;
        }
        pesan(st == StatusBaca::BUKAN_ANGKA ? "Masukkan angka." : "Minimal trx masuk harus 1 sampai " + std::to_string(MAX_TRX));
    }
    for (;;) {   // minimal rasio
        tampilInput(judul, {"Minimal trx masuk: " + std::to_string(a.minTrxMasuk), info[1]}, ambilPesan(),
                    "Minimal rasio [" + desimal(a.minRasio) + "]: ");
        const std::string masukan = pangkas(bacaTeks());
        if (masukan.empty()) break;
        double v = 0;
        const StatusBaca st = parseDouble(masukan, v, 0.0, 1.0);
        if (st == StatusBaca::BUKAN_ANGKA) {
            pesan("Masukkan angka.");
            continue;
        }
        Ambang coba = a;
        coba.minRasio = v;
        std::string alasan;
        if (st != StatusBaca::OK || !ambangValid(coba, alasan)) {
            pesan(st != StatusBaca::OK ? "Rasio harus lebih dari 0 dan maksimal 1" : alasan);
            continue;
        }
        a.minRasio = v;
        break;
    }

    std::string alasan;
    if (!aturAmbang(a, alasan)) {
        pesan(alasan);
        return;
    }
    halamanRingkasan(judul, {
        "Ambang baru: " + teksAmbang(ambangSaatIni()),
        "Dipakai oleh menu 2 (Deteksi) berikutnya.",
        "Dataset yang sudah dideteksi tidak berubah sampai dideteksi ulang lewat menu 2.",
    });
}

void submenuEvaluasi() {
    const std::vector<std::string> opsi = {
        "Precision dan Recall", "Lolos terdeteksi (False Negative)", "Salah tandai (False Positive)",
        "Ubah ambang batas"};
    for (;;) {
        tampilSubmenu("4. EVALUASI", opsi, ambilPesan());
        int v = 0;
        if (!bacaPilihan(v, 0, static_cast<int>(opsi.size()))) continue;
        switch (v) {
            case 0: return;
            case 1: evalPrecisionRecall(); break;
            case 2: evalLolos(); break;
            case 3: evalSalahTandai(); break;
            case 4: evalUbahAmbang(); break;
        }
    }
}

}  // namespace

// ---- Menu utama -------------------------------------------------------------

void menuUtama() {
    try {
        for (;;) {
            tampilMenuUtama(ambilPesan());
            int v = 0;
            if (!bacaPilihan(v, 0, 5)) continue;
            if (v == 0) break;
            if (v == 1) {
                menuBuatData();
                continue;
            }
            if (jumlahDataset() == 0) {   // menu 2-5 diblokir sebelum ada data
                pesan("Data belum ada. Pilih menu 1 dulu.");
                continue;
            }
            switch (v) {
                case 2: menuDeteksi(); break;
                case 3: submenuStatistik(); break;
                case 4: submenuEvaluasi(); break;
            }
        }
    } catch (const InputHabis&) {
        // EOF: keluar dengan bersih.
    }
    tampilSelesai();
}
