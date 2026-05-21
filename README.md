# Arion Compiler

**IF2224 Teori Bahasa Formal dan Otomata**  
Laboratorium Ilmu Rekayasa dan Komputasi, STEI ITB

---

## Identitas Kelompok

**Nama Kelompok:** JML

| NIM | Nama Lengkap |
|---|---|
| 13524015 | Mahatma Brahmana |
| 13524057 | Benedict Darrel Setiawan |
| 13524059 | Raymond Jonathan Dwi Putra J |
| 13524111 | Reynard Anderson Wijaya |

---

## Deskripsi Program

Program ini merupakan implementasi compiler tahap awal untuk bahasa pemrograman **Arion** pada Tugas Besar IF2224.

Pada **Milestone 1**, program mengimplementasikan **lexical analyzer (lexer)**. Lexer membaca kode sumber Arion dan mengubahnya menjadi deretan token menggunakan pendekatan **Deterministic Finite Automaton (DFA)**. Token yang dikenali mencakup literal, identifier, keyword, operator, delimiter, komentar, dan unknown token.

Pada **Milestone 2**, program menambahkan **syntax analyzer (parser)**. Parser menerima token dari lexer, memeriksa kesesuaian urutan token terhadap grammar Arion menggunakan **Recursive Descent Parser**, lalu menghasilkan **Parse Tree**. Parser juga menangani syntax error dengan pesan error yang informatif.

Fitur utama program:
- Tokenisasi source code Arion.
- Pengenalan keyword, identifier, konstanta, operator, delimiter, dan komentar.
- Parsing program Arion sesuai grammar Milestone 2.
- Pembuatan parse tree terformat ke terminal dan file output.
- Error handling untuk input yang tidak sesuai grammar.

---

## Requirements

- **Compiler:** `g++`
- **Standar C++:** C++17
- **Build tool:** `make`
- **Sistem Operasi:** Linux

---

## Cara Instalasi dan Penggunaan Program

### Clone Repository

```bash
git clone https://github.com/Alpaomega1136/JML-Tubes-IF2224-2026.git
cd JML-Tubes-IF2224-2026
```

### Build Program

```bash
make
```

Perintah ini akan mengompilasi seluruh source code, menyimpan file object (`.o`) di folder `bin/`, dan menghasilkan executable bernama `bin/arion`.

### Menjalankan Program

```bash
./bin/arion <input_file> <output_file>
```

Contoh:

```bash
./bin/arion test/milestone-2/input/input-1.txt test/milestone-2/output/output-1.txt
```

Keterangan:
- `<input_file>`: file teks berisi source code Arion.
- `<output_file>`: file teks hasil parse tree atau pesan syntax error.

### Format Output Milestone 2

Jika input valid, output berupa parse tree:

```text
<program>
├── <program-header>
│   ├── programsy
│   ├── ident(Kalkulator)
│   └── semicolon
...
```

Jika input tidak valid, output berupa pesan error:

```text
Syntax error in ProgramHeader: expected semicolon (;), found varsy (var)
```

### Membersihkan Build

```bash
make clean
```

---

## Struktur Repository

```text
JML-Tubes-IF2224-2026/
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── lexer.hpp
│   │   ├── lexer.cpp
│   │   ├── token.hpp
│   │   └── token.cpp
│   └── parser/
│       ├── parser.hpp
│       └── parser.cpp
├── doc/
│   ├── milestone-1/
│   └── milestone-2/
├── test/
│   ├── milestone-1/
│   │   ├── input/
│   │   └── output/
│   └── milestone-2/
│       ├── input/
│       └── output/
├── Makefile
└── README.md
```

---

## Pembagian Tugas

| NIM | Nama Lengkap | Milestone 1 | Milestone 2 |
|---|---|---|---|
| 13524015 | Mahatma Brahmana | Membuat diagram DFA & membuat kode lexer | Membuat laporan milestone 2 & merevisi kode lexer |
| 13524057 | Benedict Darrel Setiawan | Membuat diagram DFA & membuat kode lexer | Membuat laporan milestone 2 & merevisi kode lexer |
| 13524059 | Raymond Jonathan Dwi Putra J | Membuat diagram DFA & membuat laporan | Membuat laporan milestone 2 & membuat kode parser |
| 13524111 | Reynard Anderson Wijaya | Membuat diagram DFA & membuat laporan | Membuat laporan milestone 2 & membuat kode parser |
