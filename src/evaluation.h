#pragma once
// Lapisan 3 - evaluasi (Purple). Boleh bergantung pada lapisan 0-2.
// Hanya menghitung, tidak mencetak apa pun (structure.md 3.3).
//
// Semua fungsi menerima Dataset dan LabelSet lewat argumen. Kunci jawaban tidak
// pernah dibaca dari variabel global di sini, jadi file ini tidak perlu label.h.
#include <string>
#include <vector>

#include "types.h"

// Matriks konfusi. Positif = "jahat" menurut label; prediksi positif = flag.
struct Konfusi {
    int tp = 0;   // jahat dan terindikasi
    int fp = 0;   // sah tetapi terindikasi
    int fn = 0;   // jahat tetapi lolos
    int tn = 0;   // sah dan tidak terindikasi
};

// Semua nilai berupa pecahan 0-1. Pembagi nol menghasilkan 0 (bukan NaN).
struct Metrik {
    Konfusi k;
    double  precision = 0;
    double  recall    = 0;
    double  f1        = 0;
};

// Recall untuk satu pola (Label.pola). Pola kosong dilaporkan sebagai "(tanpa pola)".
struct RecallPola {
    std::string pola;
    int jahat      = 0;   // jumlah rekening jahat dengan pola ini
    int terdeteksi = 0;   // dari jumlah itu, yang terindikasi
};

// Evaluasi hanya bermakna jika dataset sudah dideteksi dan label berpasangan
// dengan rekening. Mengisi `alasan` jika tidak.
bool bisaDievaluasi(const Dataset& d, const LabelSet& l, std::string& alasan);

Konfusi hitungKonfusi(const Dataset& d, const LabelSet& l);
Metrik  hitungMetrik(const Konfusi& k);
Metrik  evaluasi(const Dataset& d, const LabelSet& l);

// Indeks rekening, naik. FN = jahat yang lolos, FP = sah yang salah ditandai.
std::vector<int> daftarFN(const Dataset& d, const LabelSet& l);
std::vector<int> daftarFP(const Dataset& d, const LabelSet& l);

// Alasan singkat sebuah rekening tidak ditandai, berdasarkan ambang yang dipakai
// saat deteksi, mis. "trx masuk 12 < 20; rasio 0.42 < 0.70".
std::string alasanLolos(const Rek& r, const Ambang& a);

// Recall per pola, urutan sesuai kemunculan pertama. Hanya rekening jahat.
std::vector<RecallPola> recallPerPola(const Dataset& d, const LabelSet& l);

// Mencari ambang dengan F1 tertinggi pada dataset ini (grid tetap). Seri dimenangkan
// ambang yang ditemukan lebih dulu. `f1Terbaik` diisi pecahan 0-1. d tidak diubah.
Ambang ambangTerbaik(const Dataset& d, const LabelSet& l, double& f1Terbaik);
