# Suspicious Account Detector

Program konsol C++ yang membuat data transaksi rekening sintetis, mendeteksi
rekening yang tidak wajar, lalu mengevaluasi seberapa baik deteksinya.

## Overview

Suspicious Account Detector adalah program konsol C++ yang membantu analis menyaring rekening terindikasi sebagai penampung dana, dengan latar skenario dana judi online. Program tidak hanya pendeteksi tetapi juga dilengkapi pembuat data transaksi sintetis lengkap dengan kunci jawabannya, mendeteksi rekening yang tidak wajar, lalu mengevaluasi seberapa baik deteksinya.

Detektor memakai dua tahap. Tahap pertama adalah aturan sederhana per rekening. Tahap kedua menelusuri aliran dana berlapis supaya rekening perantara ikut terjaring, padahal transaksi masuknya sedikit. Hasilnya adalah petunjuk bagi analis, bukan keputusan akhir: setiap rekening terindikasi masih perlu diperiksa manusia.

Proyek dikerjakan dengan pembagian seperti tim keamanan siber. Red membuat generator, Blue membuat detektor, dan Purple membuat evaluasi dan antarmuka. Red dan Blue sengaja tidak saling mengetahui rumus pihak lain, sehingga hasil evaluasi mencerminkan kemampuan deteksi, bukan kecocokan dua bagian yang ditulis orang yang sama.

## Fitur dan keterbatasan

### Fitur
Generator dataset sintetis :
- Satu seed menghasilkan satu dataset beserta kunci jawabannya. Seed yang sama selalu menghasilkan data yang identik.
- Pola bervariasi: topologi langsung, berlapis 1-4 tingkat, bercabang, dan berputar. Jumlah serta nominal setoran, jam transaksi, dan pembagian dana diteruskan juga bervariasi, termasuk setoran sedikit di bawah batas bulat.

Deteksi dua tahap :
- Tahap 1 menandai rekening dengan trx masuk minimal sekian dan rasio dana diteruskan minimal sekian (default 5 trx dan 0.50).
- Tahap 2 menandai rekening yang meneruskan dana dan menerima minimal 50% dananya dari rekening terindikasi, ditelusuri lapis demi lapis sampai 8 putaran.
- Ambang batas bisa diubah lewat menu.

Statistik :
- Ringkasan umum, peringkat terindikasi, peringkat pengirim, transaksi terbesar, statistik per jam, distribusi nominal, dan aliran dana.

Evaluasi :
- Precision, recall, dan F1.
- Daftar rekening jahat yang lolos (disertai alasan lolosnya) dan daftar rekening sah yang salah ditandai.

### Keterbatasan

Keterbatasan validitas :
karena data transaksi asli tidak tersedia, proyek ini dievaluasi pada data sintetis yang dibuat berdasarkan pola penampung dana judi online, sehingga hasilnya hanya berlaku untuk skenario tersebut dan belum membuktikan kinerja pada data nyata. Meski begitu, data sintetis memberi kunci jawaban yang lengkap sehingga deteksi dapat diukur secara objektif.

Generator dataset sintetis :
- Data sintetis, jadi hasil berlaku untuk skenario yang dibangun dan bukan bukti kinerja di dunia nyata.
- Hanya satu tipologi (penampung dana judi online dan rantai berlapis). Modus lain tidak dicakup.
- Skala kecil: maksimal 2.000 rekening dan 10.000 transaksi per dataset.
- Waktu hanya berupa jam 0-23, tanpa tanggal atau menit. Urutan transaksi dan kecepatan dana diteruskan tidak bisa diukur.
- Tidak ada atribut nasabah seperti usia rekening, profil, perangkat, atau lokasi.

Detektor :
- Berbasis aturan dan ambang tetap, tidak belajar dari data.
- Hasil hanya biner (terindikasi atau normal), tanpa tingkat risiko.

Program :
- Data hanya ada di memori. Tidak ada penyimpanan ke file maupun impor atau ekspor data.

## Siklus program

```text
        ┌──────────┐   seed    ┌────────────┐
User ─▶│ 1. Buat  │ ────────▶│  Dataset   │  status: BELUM
        │   Data   │   (Red)   │  + kunci   │
        └──────────┘           └─────┬──────┘
                                     │ user memilih dataset aktif
                                     ▼
                              ┌────────────┐
                              │ 2. Deteksi │  (Blue) menimpa hasil di dataset
                              └─────┬──────┘  status: SUDAH
                                    ▼
                  ┌─────────────────┴─────────────────┐
                  ▼                                   ▼
          ┌──────────────┐                    ┌──────────────┐
          │ 3. Statistik │                    │ 4. Evaluasi  │ (Purple)
          └──────────────┘                    └──────────────┘
                  └────── kembali ke menu utama ──────┘
```

Langkah per langkah:

1. **Buat data.** User memasukkan seed. Generator membuat satu dataset beserta
   kunci jawabannya, dan menambahkannya ke daftar. Jumlah dataset tidak dibatasi.
2. **Pilih dataset aktif.** Semua menu bekerja pada satu dataset yang sedang dipilih.
3. **Deteksi.** Detektor membaca transaksi dataset aktif dan menimpa hasil
   deteksi langsung di dataset itu. Transaksi tidak pernah diubah.
4. **Lihat hasil.** Daftar terindikasi, statistik, dan evaluasi membaca hasil
   yang sudah tersimpan.
5. **Ulangi.** User bisa membuat dataset baru atau mengganti ambang dan
   mendeteksi ulang.

Aturan urutan:
- Menu 2, 3, dan 4 tidak bisa dibuka sebelum ada dataset.
- Menu 4 (Evaluasi) tidak bisa dibuka pada dataset yang statusnya BELUM.

## Tampilan program

Layar selalu di-clear saat berpindah page. Ada tiga jenis page:

| Page | Isi |
|---|---|
| Menu utama | pilihan 1-5 |
| Submenu | pilihan turunan |
| Pemilihan dataset | pilihan dataset untuk tampilan hasil |
| Hasil | tabel dengan paging dan search atau ringkasan |

Detail lengkap, contoh layar, dan aturan input ada di [docs/menu.md](docs/menu.md).

## Alur pengerjaan

1. Semua anggota menyepakati kontrak bersama (bagian 1-4, 8-10) lebih dulu.
2. Red, Blue, dan Purple mengerjakan bagiannya masing-masing tanpa membuka
   folder atau dokumen privat pihak lain.
3. Purple mengintegrasikan semuanya lewat antarmuka yang sudah disepakati.
4. Purple menjalankan evaluasi, lalu membagikan umpan balik terbatas ke tiap
   pihak (contoh: "recall rendah pada dataset tertentu").

## Peta dokumen

| Bagian | Dokumen | Dibaca oleh |
|---|---|---|
| Overview | README ini | semua |
| Project structure | [docs/structure.md](docs/structure.md) | semua |
| Model data | [docs/model.md](docs/model.md) | semua |
| Generator spec | docs/red/ | Red (dan Purple) |
| Detektor spec | docs/blue/ | Blue (dan Purple) |
| Evaluasi spec | docs/purple/ | Purple |
| Menu program | [docs/menu.md](docs/menu.md) | semua |


## Contributors

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/ficaficul">
        <img src="https://github.com/ficaficul.png?size=100" width="100px;" alt="Firsya"/><br />
        <sub><b>Firsya</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/ShenDoll">
        <img src="https://github.com/ShenDoll.png?size=100" width="100px;" alt="Rangga"/><br />
        <sub><b>Rangga</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/ranpag2">
        <img src="https://github.com/ranpag2.png?size=100" width="100px;" alt="Raphael"/><br />
        <sub><b>Raphael</b></sub>
      </a>
    </td>
  </tr>
</table>


