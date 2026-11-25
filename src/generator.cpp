#include "generator.h"

#include <algorithm>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "config.h"
#include "dataset.h"
#include "types.h"

namespace {

// ===========================================================================
// 1. Pembangkit acak
// Ditulis sendiri (splitmix64), bukan std::uniform_int_distribution, karena
// distribusi standar boleh memberi hasil beda antar compiler. Seed sama harus
// selalu menghasilkan dataset yang sama.
// ATURAN: satu panggilan antara()/peluang() per pernyataan. Urutan evaluasi
// argumen fungsi tidak dijamin C++, jadi nilai acak selalu ditampung di variabel.
// ===========================================================================
struct Acak { unsigned long long s = 0; };

unsigned long long langkah(Acak& r) {
    r.s += 0x9E3779B97F4A7C15ULL;
    unsigned long long z = r.s;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

Acak mulaiAcak(unsigned seed) {
    Acak awal;
    awal.s = seed;
    Acak r;
    r.s = langkah(awal);   // diaduk dulu, supaya seed 1 dan 2 tidak berupa aliran yang sama bergeser satu langkah
    return r;
}

int  antara(Acak& r, int a, int b) { return a + static_cast<int>(langkah(r) % static_cast<unsigned long long>(b - a + 1)); }
bool peluang(Acak& r, int persen)  { return antara(r, 1, 100) <= persen; }

// ===========================================================================
// 2. Katalog pola (model.md 4.7). Tambah variasi di sini.
// ===========================================================================
enum Topologi { LANGSUNG, BERLAPIS, BERCABANG, PUTARAN };
enum JmlSetor { S_BANYAK, S_SEDIKIT, S_SEDANG };
enum GayaNominal { N_KECIL, N_SERAGAM, N_BAWAH_BATAS, N_LEBAR, N_WAJAR };
enum GayaJam { J_MALAM, J_MERATA, J_BURST, J_SIANG, J_DINI };
enum GayaPecah { P_SERAGAM, P_ACAK };

const char* const NAMA_TOPOLOGI[] = {"langsung", "berlapis", "bercabang", "putaran"};
const char* const NAMA_SETOR[]    = {"banyak", "sedikit", "sedang"};
const char* const NAMA_NOMINAL[]  = {"kecil", "seragam", "bawah-batas", "lebar", "wajar"};
const char* const NAMA_JAM[]      = {"malam", "merata", "burst", "siang", "dini"};

const char* const PERAN_PEMAIN    = "Pemain";
const char* const PERAN_PERANTARA = "Perantara";
const char* const PERAN_AKHIR     = "Akhir";

// Setoran per pemain: jumlah transaksi (min, maks) menurut JmlSetor.
const int SETOR_MIN[] = {3, 1, 2};
const int SETOR_MAX[] = {6, 2, 4};
// Pedagang sah (pengecoh): transaksi masuk (min, maks) dan transaksi bayar pemasok.
const int PED_MASUK_MIN = 30, PED_MASUK_MAX = 80, PED_KELUAR = 3;
const int PERSEN_MENAHAN = 65;   // node yang meneruskan < 65% diberi tag "menahan-dana"

// Semua pilihan untuk SATU dataset, ditentukan dari seed.
struct Rencana {
    // kuantitas
    int nBiasa = 0, nPedagang = 0, nPemain = 0, nPenampung = 0, nAkhir = 0, nNoise = 0;
    // topologi rantai
    int  topologi = LANGSUNG;
    int  tingkat = 0;       // jumlah lapis perantara (di antara penampung dan akhir)
    int  lebar = 1;         // lebar tiap lapis perantara
    int  kali = 1;          // jumlah transfer keluar tiap node
    bool cabang = false;    // true: tiap transfer ke tujuan berbeda, false: satu tujuan
    bool putaran = false;   // akhir mengembalikan sebagian dana ke pemain
    int  kembaliPersen = 0, kembaliKali = 0;
    // setoran
    int       setor = S_BANYAK, nominal = N_KECIL, jamSetor = J_MALAM, jamAlir = J_DINI, pecah = P_SERAGAM;
    bool      menyamar = false;   // setoran meniru pola pedagang (siang, nominal wajar)
    long long nilaiSeragam = 0, batasLapor = 0;
    int       burstMulai = 0;
    // seberapa banyak dana yang diteruskan tiap node
    int teruskanMin = 0, teruskanMax = 0;
};

Rencana susunRencana(Acak& r) {
    Rencana p;
    p.nBiasa = antara(r, 150, 1400);
    const int pengecoh = antara(r, 1, 100);                       // 20% tanpa, 50% sedikit, 30% banyak
    if (pengecoh <= 20)      p.nPedagang = 0;
    else if (pengecoh <= 70) p.nPedagang = antara(r, 1, 3);
    else                     p.nPedagang = antara(r, 4, 8);
    p.nPemain    = antara(r, 15, 160);
    p.nPenampung = antara(r, 2, 8);
    p.nAkhir     = antara(r, 1, 2);
    const int noiseFaktor = antara(r, 1, 6);
    p.nNoise = p.nBiasa * noiseFaktor;   // bisa melewati MAX_TRX; dikecilkan di rakitDataset

    p.topologi = antara(r, LANGSUNG, PUTARAN);
    switch (p.topologi) {
        case LANGSUNG:
            p.tingkat = 0;
            p.kali = antara(r, 1, 3);
            break;
        case BERLAPIS:
            p.tingkat = antara(r, 1, 4);
            p.lebar = antara(r, 1, 3);
            p.kali = antara(r, 1, 2);
            break;
        case BERCABANG:
            p.tingkat = antara(r, 2, 3);
            p.lebar = antara(r, 2, 4);
            p.kali = antara(r, 2, 3);
            p.cabang = true;
            break;
        default:  // PUTARAN
            p.tingkat = antara(r, 1, 2);
            p.lebar = antara(r, 1, 3);
            p.kali = antara(r, 1, 3);
            p.cabang = peluang(r, 50);
            p.putaran = true;
            break;
    }
    p.kembaliPersen = antara(r, 10, 25);
    p.kembaliKali = antara(r, 2, 3);

    p.setor = antara(r, S_BANYAK, S_SEDANG);
    p.menyamar = peluang(r, 30);
    const int gayaNominal = antara(r, N_KECIL, N_LEBAR);
    p.nominal = p.menyamar ? static_cast<int>(N_WAJAR) : gayaNominal;
    const int gayaJam = antara(r, J_MALAM, J_BURST);
    p.jamSetor = p.menyamar ? static_cast<int>(J_SIANG) : gayaJam;
    const int alirDini = antara(r, 0, 1);
    p.jamAlir = p.menyamar ? static_cast<int>(J_SIANG) : (alirDini == 0 ? static_cast<int>(J_DINI) : static_cast<int>(J_MERATA));
    p.pecah = antara(r, P_SERAGAM, P_ACAK);

    const long long nilaiSeragam[] = {50000, 100000, 200000, 500000};
    const long long batasLapor[]   = {500000, 1000000, 5000000, 10000000};
    p.nilaiSeragam = nilaiSeragam[antara(r, 0, 3)];
    p.batasLapor   = batasLapor[antara(r, 0, 3)];
    p.burstMulai   = antara(r, 0, 21);                 // jendela burst 3 jam: tidak melewati jam 23

    p.teruskanMin = antara(r, 45, 88);                 // sebagian dataset sengaja sulit: banyak dana ditahan
    p.teruskanMax = p.teruskanMin + antara(r, 5, 10);
    return p;
}

// ===========================================================================
// 3. Anggaran (model.md 4.4, 4.8): hitung batas atas transaksi SEBELUM membuat,
// supaya batas MAX_TRX tidak pernah dilanggar dan tidak ada yang dibuang.
// ===========================================================================

// Lebar sebuah lapis dibatasi agar tiap node di lapis itu pasti menerima dana.
int batasLebar(const Rencana& p, int sumber, int diminta) {
    const int maks = p.cabang ? sumber * p.kali : sumber;
    return std::min(diminta, maks);
}

// Ukuran tiap lapis: [penampung, perantara tingkat 1..T, akhir]
std::vector<int> ukuranLapis(const Rencana& p) {
    std::vector<int> u;
    u.push_back(p.nPenampung);
    for (int t = 0; t < p.tingkat; ++t) u.push_back(batasLebar(p, u.back(), p.lebar));
    u.push_back(batasLebar(p, u.back(), p.nAkhir));
    return u;
}

int batasTrx(const Rencana& p) {
    const std::vector<int> u = ukuranLapis(p);
    int node = 0;
    for (std::size_t l = 0; l + 1 < u.size(); ++l) node += u[l];   // semua node selain akhir meneruskan
    int total = p.nNoise;
    total += p.nPemain * SETOR_MAX[p.setor];
    total += p.nPedagang * (PED_MASUK_MAX + PED_KELUAR);
    total += node * p.kali;
    if (p.putaran) total += u.back() * p.kembaliKali;
    return total;
}

// ===========================================================================
// 4. Pembangun dataset
// ===========================================================================
struct Bangun {
    Acak                   rng;
    Dataset                d;
    LabelSet               lab;
    std::vector<long long> dana;         // dana masuk sementara tiap rekening (untuk menentukan jumlah yang diteruskan)
    std::vector<int>       persenNode;   // persen dana yang diteruskan tiap node (0 = bukan node rantai)
    std::string            galat;        // tidak kosong = bug generator, dilaporkan di akhir
};

void kirim(Bangun& b, int dari, int ke, long long nom, int jam) {
    if (dari == ke) { b.galat = "Generator menghasilkan transaksi ke diri sendiri"; return; }
    if (static_cast<int>(b.d.trx.size()) >= MAX_TRX) { b.galat = "Anggaran transaksi terlampaui"; return; }
    nom = std::max(nom, NOM_MIN);
    nom = std::min(nom, NOM_MAX);
    Trx t;
    t.dari = dari; t.ke = ke; t.nom = nom; t.jam = jam;
    b.d.trx.push_back(t);
    b.dana[ke] += nom;
}

long long ribuan(Bangun& b, int dariRibu, int sampaiRibu) {
    const int v = antara(b.rng, dariRibu, sampaiRibu);
    return 1000LL * v;
}

int ambilJam(Bangun& b, const Rencana& p, int gaya) {
    switch (gaya) {
        case J_MALAM: { const int v = antara(b.rng, 19, 27); return v % 24; }
        case J_BURST: { const int v = antara(b.rng, 0, 2);   return p.burstMulai + v; }
        case J_SIANG: return antara(b.rng, 8, 17);
        case J_DINI:  return antara(b.rng, 0, 6);
        default:      return antara(b.rng, JAM_MIN, JAM_MAX);   // J_MERATA
    }
}

long long nominalSetor(Bangun& b, const Rencana& p) {
    switch (p.nominal) {
        case N_SERAGAM: return p.nilaiSeragam;
        case N_BAWAH_BATAS: {
            const int persen = antara(b.rng, 90, 99);
            return p.batasLapor * persen / 100 / 1000 * 1000;
        }
        case N_LEBAR: {
            const int t = antara(b.rng, 1, 100);
            if (t <= 60) return ribuan(b, 10, 200);
            if (t <= 90) return ribuan(b, 200, 2000);
            return ribuan(b, 2000, 10000);
        }
        case N_WAJAR: return ribuan(b, 20, 300);
        default:      return ribuan(b, 10, 200);   // N_KECIL
    }
}

// Transaksi orang biasa: siang hari dominan, tapi ada yang malam (supaya malam bukan penanda pasti).
int jamNormal(Bangun& b) {
    const int t = antara(b.rng, 1, 100);
    if (t <= 85) return antara(b.rng, 6, 22);
    return antara(b.rng, JAM_MIN, JAM_MAX);
}

long long nominalNormal(Bangun& b) {
    const int t = antara(b.rng, 1, 100);
    if (t <= 70) return ribuan(b, 20, 500);
    if (t <= 95) return ribuan(b, 500, 3000);
    return ribuan(b, 3000, 20000);
}

// Teruskan `persen` dari dana yang sudah masuk ke `dari`, dalam `kali` transfer.
// `rr` = penghitung bergilir: memastikan setiap tujuan di satu lapis kebagian dana.
void alirkan(Bangun& b, const Rencana& p, int dari, const std::vector<int>& tujuan,
             int persen, int kali, bool cabang, int pecah, int gayaJam, int& rr) {
    const long long total = b.dana[dari] * persen / 100;
    std::vector<int> bobot(kali, 1);
    if (pecah == P_ACAK) {
        for (int& w : bobot) w = antara(b.rng, 1, 10);
    }
    int jumlahBobot = 0;
    for (int w : bobot) jumlahBobot += w;

    const int ukuran = static_cast<int>(tujuan.size());
    const int tetap = cabang ? -1 : tujuan[rr++ % ukuran];
    for (int k = 0; k < kali; ++k) {
        long long nom = total * bobot[k] / jumlahBobot;
        if (nom >= 1000) nom = nom / 1000 * 1000;
        const int ke = cabang ? tujuan[rr++ % ukuran] : tetap;
        const int jam = ambilJam(b, p, gayaJam);
        kirim(b, dari, ke, nom, jam);
    }
}

void buatNoise(Bangun& b, const Rencana& p, const std::vector<int>& biasa) {
    const int n = static_cast<int>(biasa.size());
    for (int k = 0; k < p.nNoise; ++k) {
        const int a = antara(b.rng, 0, n - 1);
        int c = antara(b.rng, 0, n - 2);
        if (c >= a) ++c;                          // pengirim dan penerima pasti berbeda
        const long long nom = nominalNormal(b);
        const int jam = jamNormal(b);
        kirim(b, biasa[a], biasa[c], nom, jam);
    }
}

// Pedagang sah: banyak transaksi masuk di siang hari, lalu membayar pemasok.
// Sengaja mirip penampung (pengecoh).
void buatPedagang(Bangun& b, const Rencana& p, const std::vector<int>& pedagang, const std::vector<int>& biasa) {
    int rr = antara(b.rng, 0, 999);
    const int n = static_cast<int>(biasa.size());
    for (int ped : pedagang) {
        const int masuk = antara(b.rng, PED_MASUK_MIN, PED_MASUK_MAX);
        for (int k = 0; k < masuk; ++k) {
            const int dari = antara(b.rng, 0, n - 1);
            const long long nom = ribuan(b, 20, 300);
            const int jam = antara(b.rng, 8, 21);
            kirim(b, biasa[dari], ped, nom, jam);
        }
        const int persen = antara(b.rng, 60, 90);
        alirkan(b, p, ped, biasa, persen, PED_KELUAR, true, P_ACAK, J_SIANG, rr);
    }
}

// Pemain menyetor ke penampung, lalu dana mengalir lapis demi lapis sampai rekening akhir.
void buatAliranJudol(Bangun& b, const Rencana& p, const std::vector<std::vector<int>>& lapis, const std::vector<int>& pemain) {
    const std::vector<int>& penampung = lapis.front();
    const int nTampung = static_cast<int>(penampung.size());
    int urut = 0;
    for (int m : pemain) {
        const int jml = antara(b.rng, SETOR_MIN[p.setor], SETOR_MAX[p.setor]);
        for (int k = 0; k < jml; ++k) {
            // setoran pertama dibagi bergilir agar semua penampung pasti menerima dana
            const int pilih = antara(b.rng, 0, nTampung - 1);
            const int ke = penampung[urut < nTampung ? urut : pilih];
            ++urut;
            const long long nom = nominalSetor(b, p);
            const int jam = ambilJam(b, p, p.jamSetor);
            kirim(b, m, ke, nom, jam);
        }
    }

    int rr = antara(b.rng, 0, 999);
    for (std::size_t l = 0; l + 1 < lapis.size(); ++l) {
        for (int node : lapis[l]) {
            const int persen = antara(b.rng, p.teruskanMin, p.teruskanMax);
            b.persenNode[node] = persen;
            alirkan(b, p, node, lapis[l + 1], persen, p.kali, p.cabang, p.pecah, p.jamAlir, rr);
        }
    }

    if (p.putaran && !pemain.empty()) {   // dana kembali ke pemain (kemenangan)
        for (int akhir : lapis.back()) {
            alirkan(b, p, akhir, pemain, p.kembaliPersen, p.kembaliKali, true, P_ACAK, J_MERATA, rr);
        }
    }
}

// ===========================================================================
// 5. Kunci jawaban
// ===========================================================================
std::string ringkasPola(const Rencana& p) {
    return std::string("topologi=") + NAMA_TOPOLOGI[p.topologi] + std::to_string(p.tingkat) +
           " setor=" + NAMA_SETOR[p.setor] + " nominal=" + NAMA_NOMINAL[p.nominal] +
           " jam=" + NAMA_JAM[p.jamSetor] + (p.menyamar ? " samar=ya" : " samar=tidak");
}

void isiLabel(Bangun& b, const Rencana& p, const std::vector<int>& pedagang, const std::vector<int>& pemain,
              const std::vector<std::vector<int>>& lapis) {
    for (int i : pedagang) {
        b.lab[i].peran = PERAN_PEDAGANG;
        b.lab[i].pola = "pengecoh-sah";            // sah; dicatat agar Purple bisa melaporkan salah tandai per pengecoh
    }
    for (int i : pemain) b.lab[i].peran = PERAN_PEMAIN;   // pengirim setoran, dianggap sah

    const std::string dasar = ringkasPola(p);
    for (std::size_t l = 0; l < lapis.size(); ++l) {
        const bool terakhir = (l + 1 == lapis.size());
        for (int i : lapis[l]) {
            Label& x = b.lab[i];
            x.jahat = true;
            const std::string tahan = (b.persenNode[i] > 0 && b.persenNode[i] < PERSEN_MENAHAN) ? " menahan-dana" : "";
            const std::string teruskan = " teruskan=" + std::to_string(b.persenNode[i]) + "%";
            if (terakhir) {
                x.peran = PERAN_AKHIR;
                x.pola = "struktural:akhir-ujung-rantai " + dasar + (p.putaran ? " kembali=ya" : " kembali=tidak");
            } else if (l == 0) {
                x.peran = PERAN_PENAMPUNG;
                x.pola = "penampung " + dasar + teruskan + tahan;
            } else {
                x.peran = PERAN_PERANTARA;
                x.pola = "perantara-tingkat-" + std::to_string(l) + " " + dasar + teruskan + tahan;
            }
        }
    }
}

std::vector<int> ambilKelompok(const std::vector<int>& urutan, std::size_t& pos, int n) {
    std::vector<int> hasil(urutan.begin() + pos, urutan.begin() + pos + n);
    pos += n;
    return hasil;
}

}  // namespace

// ===========================================================================
// 6. Antarmuka publik
// ===========================================================================
bool rakitDataset(unsigned seed, Dataset& d, LabelSet& l, std::string& alasan) {
    Bangun b;
    b.rng = mulaiAcak(seed);
    Rencana p = susunRencana(b.rng);

    // Anggaran: kecilkan kuantitas (bukan memotong transaksi) sampai pasti muat.
    while (batasTrx(p) > MAX_TRX) {
        if (p.nNoise > 200)        p.nNoise = p.nNoise * 3 / 4;
        else if (p.nPemain > 10)   p.nPemain -= p.nPemain / 4 + 1;
        else if (p.nPedagang > 0)  --p.nPedagang;
        else break;
    }

    const std::vector<int> ukuran = ukuranLapis(p);
    int nRek = p.nBiasa + p.nPedagang + p.nPemain;
    for (int u : ukuran) nRek += u;

    // Peran ditempatkan pada nomor rekening acak, jadi nomor tidak membocorkan peran.
    std::vector<int> urutan(nRek);
    for (int i = 0; i < nRek; ++i) urutan[i] = i;
    for (int i = nRek - 1; i > 0; --i) {
        const int j = antara(b.rng, 0, i);
        std::swap(urutan[i], urutan[j]);
    }
    std::size_t pos = 0;
    const std::vector<int> biasa    = ambilKelompok(urutan, pos, p.nBiasa);
    const std::vector<int> pedagang = ambilKelompok(urutan, pos, p.nPedagang);
    const std::vector<int> pemain   = ambilKelompok(urutan, pos, p.nPemain);
    std::vector<std::vector<int>> lapis;
    for (int u : ukuran) lapis.push_back(ambilKelompok(urutan, pos, u));

    b.d.seed = seed;
    b.d.rek.resize(nRek);
    for (int i = 0; i < nRek; ++i) b.d.rek[i].no = formatNoRek(i);
    b.dana.assign(nRek, 0);
    b.persenNode.assign(nRek, 0);
    Label biasaLabel;
    biasaLabel.peran = PERAN_BIASA;
    b.lab.assign(nRek, biasaLabel);

    // Urutan penting: aliran judol memakai dana yang sudah masuk.
    buatNoise(b, p, biasa);
    buatPedagang(b, p, pedagang, biasa);
    buatAliranJudol(b, p, lapis, pemain);
    if (!b.galat.empty()) { alasan = b.galat; return false; }

    hitungAgregat(b.d);
    isiLabel(b, p, pedagang, pemain, lapis);
    if (!validasiDataset(b.d, alasan)) return false;

    d = std::move(b.d);
    l = std::move(b.lab);
    return true;
}

int buatDataset(unsigned seed, std::string& alasan) {
    if (seed == SEED_ACAK) {
        std::random_device rd;
        do {
            seed = rd() % 999999u + 1u;
        } while (cariSeed(seed) >= 0);
    }
    Dataset d;
    LabelSet l;
    if (!rakitDataset(seed, d, l, alasan)) return -1;
    return tambahDataset(std::move(d), std::move(l), alasan);
}
