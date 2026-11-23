## 4. Model data

### 4.1 Gambaran

Ada dua penyimpanan paralel di `dataset.cpp`, keduanya `std::vector` sehingga tidak ada batas jumlah:

```text
dataset = [ Dataset 0 , Dataset 1 , Dataset 2 , ... ]   <- bisa dibaca semua peran
label   = [ LabelSet 0, LabelSet 1, LabelSet 2, ... ]   <- hanya red (tulis) dan purple (baca)

dataset[k] dan label[k] adalah pasangan.
rekening ke-i di dataset[k] berpasangan dengan label[k][i].
```

Generator mengisi `dataset[k]` dan `label[k]` bersamaan. Detektor hanya boleh menyentuh `dataset`, tidak pernah `label`. Detektor mengisi field hasil deteksi langsung di dataset yang dipilih (ditimpa, bukan dibuat salinan baru).

### 4.2 Struktur data

```cpp
// Kontrak bersama (dataset.h): hanya hal yang bisa diamati dari luar
struct Trx {
    int       dari;   // indeks rekening pengirim
    int       ke;     // indeks rekening penerima
    long long nom;    // nominal (rupiah, > 0)
    int       jam;    // 0-23
};

struct Rek {
    std::string no;           // "R001", "R002", ... (lebar 3 digit, melebar jika > 999)
    // agregat: diisi sekali oleh generator saat membuat data
    int       masuk = 0;      // jumlah trx masuk
    int       keluar = 0;     // jumlah trx keluar
    long long in = 0;         // total dana masuk
    long long out = 0;        // total dana keluar
    // hasil deteksi: diisi detektor, awalnya kosong
    double    skor = 0;       // 0-1, makin tinggi makin mencurigakan
    bool      flag = false;   // true = terindikasi
};

enum class Status { BELUM, SUDAH };   // sudah dideteksi atau belum

struct Dataset {
    unsigned          seed;           // seed asal data ini
    std::vector<Trx>  trx;
    std::vector<Rek>  rek;
    Status            status = Status::BELUM;
    std::string       catatan;        // catatan mengenai deteksi
};

// Khusus red dan purple (label.h): kunci jawaban
struct Label {
    std::string peran;   // "Biasa", "Pedagang", "Penampung", dst. (teks bebas)
    bool        jahat;   // label kebenaran
};
using LabelSet = std::vector<Label>;

// dataset.cpp
std::vector<Dataset>  dataset;
std::vector<LabelSet> label;
```

Peran disimpan sebagai teks, bukan `enum`, supaya red bisa menambah jenis peran baru tanpa mengubah kontrak bersama.

### 4.3 Siklus hidup sebuah dataset

```text
1. Buat      Red menerima seed dari user, lalu menambahkan satu Dataset dan satu LabelSet.
             status = BELUM, flag/skor kosong.
2. Dipilih   User memilih dataset mana yang akan dideteksi (indeks k).
3. Deteksi   Blue membaca dataset[k].trx dan agregatnya, lalu MENIMPA field skor, flag,
             catatan, dan status = SUDAH. trx dan agregat tidak pernah diubah.
```

Setiap dataset cukup dideteksi satu kali untuk mendapat label indikasi. Contoh kondisi setelah dataset 0 dideteksi:

```text
dataset = [ Dataset0 (SUDAH), Dataset1 (BELUM) ]
label   = [ LabelSet0       , LabelSet1        ]   <- tidak berubah
```

### 4.4 Aturan seed dan jumlah dataset

```cpp
// config.h
const int MAX_REK = 2000;
const int MAX_TRX = 10000;
```

- Satu kali "Buat Data" dengan satu seed menghasilkan satu dataset. Jumlah dataset tidak dibatasi, bertambah setiap user memasukkan seed baru.
- Setiap dataset wajib memenuhi `rek.size() <= MAX_REK` dan `trx.size() <= MAX_TRX`.
- **Red menjamin SEMUA seed menghasilkan dataset yang sah.** Tidak ada seed yang boleh gagal karena melebihi batas.
- **Dilarang membuang transaksi diam-diam.** `kirim()` yang hanya `return` saat penuh di kode lama diganti: anggaran dihitung di depan (lihat 4.8), dan pelanggaran batas dianggap bug (assert / pesan error), bukan hal biasa.
- Nomor rekening memakai 4 digit tetap: "R0001" sampai "R2000".
- Seed `0` berarti acak. Seed yang benar-benar dipakai disimpan di `Dataset.seed`.
- Seed sama menghasilkan dataset dan label yang identik.
- Seed duplikat ditolak: "Seed sudah ada (dataset k)".

### 4.5 Siapa boleh membaca dan menulis apa

| Data | Red (generator) | Blue (detektor) | Purple (evaluasi/menu) |
|---|---|---|---|
| `Trx`, `no` | tulis | baca | baca |
| agregat (`masuk`, `keluar`, `in`, `out`) | tulis | baca | baca |
| `rasio`, `skor`, `flag`, `catatan`, `status` | tidak menyentuh | tulis | baca |
| `label` (seluruhnya) | tulis | **dilarang** | baca |

Karena semuanya berjalan dalam satu proses, isolasi dijaga lewat aturan include:
- `detector.cpp` tidak boleh meng-include `label.h`.
- Hanya `evaluasi.cpp` dan `menu.cpp` yang boleh meng-include `label.h`.
- Detektor tidak boleh meng-include apa pun dari `generator`.

### 4.6 Invarian model data

- `dataset.size() == label.size()`.
- Untuk setiap `k`: `dataset[k].rek.size() == label[k].size()`.
- Untuk setiap trx: `0 <= dari, ke < rek.size()`, `dari != ke`, `nom > 0`, `0 <= jam <= 23`.
- Untuk setiap dataset: Σ`in` = Σ`out` = Σ`nom` seluruh trx.
- Agregat konsisten dengan trx: `rek[i].masuk` sama dengan jumlah trx yang `ke == i`, dan seterusnya.
- Jika `status == BELUM`: semua `flag == false` dan `skor == 0`.
- Jika `status == SUDAH`: `catatan` tidak kosong.
- Deteksi tidak mengubah `trx` maupun agregat (bandingkan sebelum dan sesudah).
- Tidak ada akses ke `label` dari kode detektor.
- `0 < rek.size() <= MAX_REK` dan `0 < trx.size() <= MAX_TRX` untuk setiap dataset.
- Uji sapuan: buat dataset untuk seed 1..1000, semua invarian lulus, tidak ada yang melewati batas.
- Jumlah trx sebelum dan sesudah deteksi sama.

### 4.7 Variasi pola dalam batas

Karena jatah transaksi terbatas, red tidak lagi memilih "berapa banyak", tetapi **memilih bentuk pola dari katalog dan membagi anggaran di antaranya.**

**Alur red untuk satu seed:**

```text
1. Dari seed, tentukan jumlah rekening total (<= MAX_REK).
2. Dari seed, pilih pola mana saja yang aktif dari katalog, beserta parameternya.
3. Hitung anggaran: perkiraan jumlah trx tiap pola. Jumlahnya (plus noise) <= MAX_TRX.
4. Jika melebihi, kecilkan parameter pola (kuantitas), bukan memotong trx.
5. Buat transaksi sesuai urutan yang bergantung pada dana masuk (pola berlapis dibuat setelah pola penyetoran).
```

**Dimensi variasi** (red bebas menambah, ini minimum):

| Dimensi | Contoh variasi yang dipilih seed |
|---|---|
| Topologi | fan-in sederhana, berlapis 1-4 tingkat, bercabang, dana kembali (round-trip) |
| Pemecahan dana | setoran kecil banyak, setoran sedang sedikit, dipecah seragam, dipecah acak |
| Jam | dominan malam, merata sepanjang hari, burst pada jam tertentu |
| Nominal | kecil seragam, di bawah batas pelaporan, bervariasi lebar |
| Penyamaran | penampung meniru pedagang sah (jam siang, nominal wajar), atau tidak menyamar |
| Pengecoh sah | tidak ada, sedikit, atau banyak pedagang yang mirip penampung |
| Noise | proporsi transaksi normal terhadap total anggaran |
| Kuantitas | jumlah rekening tiap peran (hanya salah satu dimensi, bukan satu-satunya) |

**Aturan:**
- Dua seed berbeda sebaiknya berbeda dalam bentuk pola, bukan hanya jumlahnya.
- `Label.peran` tetap teks bebas. Tambahkan `Label.pola` (teks) untuk mencatat pola mana yang membuat rekening itu jahat. Purple bisa melaporkan recall per pola ("lemah pada pola berlapis 3 tingkat").
- Kombinasi yang dihasilkan harus tetap memuat minimal satu rekening jahat dan satu rekening sah, supaya evaluasi tidak pecah (pembagian nol).
- Pola mustahil ditandai oleh detektor (seperti Akhir dengan rasio 0 pada kode lama) boleh ada, tetapi harus tercatat di `Label.pola` supaya purple tahu itu batas struktural, bukan kegagalan blue.
- Anggaran tiap pola dan alasan pemilihannya adalah milik spec red dan tidak dibuka ke blue.
