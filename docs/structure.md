## 3. Struktur folder dan aturan dependensi

### 3.1 Struktur folder
```text
suspicious-account-detector/
├── Makefile                  # build (atau CMakeLists.txt)
├── README.md                 # tujuan, cara build, cara pakai
├── main.cpp                  # titik masuk, hanya memanggil menu utama
│
├── src/
│   ├── config.h              # konstanta: batas jumlah, daftar peran, batas nominal, label
│   ├── types.h               # definisi struct transaksi dan rekening
│   │
│   ├── data.h / data.cpp     # penyimpanan data (transaksi, rekening, penanda siap)
│   ├── util.h / util.cpp     # input angka aman, angka acak, persentase
│   ├── sorting.h / .cpp      # pengurutan
│   ├── searching.h / .cpp    # pencarian
│   │
│   ├── generator.h / .cpp    # pembuatan data sintetis
│   ├── detector.h / .cpp     # aturan deteksi dan ambang batas
│   │
│   ├── evaluation.h / .cpp     # TP/FP/FN/TN, precision, recall, F1, pencarian ambang
│   │
│   ├── statistics.h / .cpp    # ringkasan, peringkat, distribusi, statistik per jam
│   ├── appreance.h / .cpp     # format tabel dan pencetakan hasil
│   │
│   └── menu.h / .cpp         # alur menu dan interaksi pengguna
│
└── tests/
    └── test_invarian.cpp     # pengecekan konsistensi data (total masuk = total keluar)
```

### 3.2 Lapisan dependensi
| Lapisan | File | Boleh bergantung pada |
|---|---|---|
| 0 | config, types | tidak ada |
| 1 | data, util, sorting, pencarian | lapisan 0 |
| 2 | generator, detector | lapisan 0-1 |
| 3 | evaluation | lapisan 0-2 |
| 4 | statistics, appreance | lapisan 0-3 |
| 5 | menu | semua lapisan di bawahnya |
| 6 | main | hanya menu |

### 3.3 Aturan
- File di lapisan bawah tidak boleh meng-include file di lapisan atas.
- Logika (hitung) dan tampilan (cetak) dipisah: evaluasi, statistik, dan detector tidak mencetak apa pun.
- Variabel global didefinisikan sekali di `.cpp` dan dideklarasikan `extern` di `.h`.
- Header memakai `#pragma once` dan tidak berisi `using namespace std;`.
