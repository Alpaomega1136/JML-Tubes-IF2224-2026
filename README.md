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

Pada **Milestone 3**, program menambahkan **semantic analyzer**. Tahap ini membangun AST dari parse tree, melakukan pengecekan semantic seperti deklarasi identifier, scope, type checking, validasi structured type, dan menghasilkan **Decorated AST** beserta **Symbol Table** (`TAB`, `ATAB`, dan `BTAB`).

Fitur utama program:
- Tokenisasi source code Arion.
- Pengenalan keyword, identifier, konstanta, operator, delimiter, dan komentar.
- Parsing program Arion sesuai grammar Milestone 2 dan revisi Milestone 3.
- Pembangunan AST yang lebih ringkas dari parse tree.
- Analisis semantic untuk deklarasi, scope, type checking, array, record, subrange, dan enumerated type.
- Pembuatan Decorated AST dan Symbol Table terformat ke terminal dan file output.
- Error handling untuk lexical, syntax, dan semantic error.

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
./bin/arion test/milestone-3/input/input-1.txt test/milestone-3/output/output-1.txt
```

Keterangan:
- `<input_file>`: file teks berisi source code Arion.
- `<output_file>`: file teks hasil semantic analysis atau pesan error.

### Format Output Milestone 3

Jika input valid, output berupa semantic report:

```text
=== Decorated AST Tree ===
ProgramNode(name: 'Kalkulator') [type=program, tab=37, lev=0]
|-- Declarations
...

=== Symbol Tables ===
=== TAB ===
identifier | link | obj | type | ref | nrm | lev | adr
```

Jika input tidak valid, output berupa pesan error:

```text
Semantic error: identifier 'x' is not declared.
```

### Menjalankan Semua Test Milestone 3

```bash
for input in test/milestone-3/input/input-*.txt; do \
  base=$(basename "$input" .txt); \
  ./bin/arion "$input" "test/milestone-3/output/${base/input/output}.txt"; \
done
```

### Membersihkan Build

```bash
make clean
```

---

## Struktur Repository

```text
JML-Tubes-IF2224-2026/
├── bin/
│   ├── arion
│   └── ... file object .o
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── lexer.hpp
│   │   ├── lexer.cpp
│   │   ├── token.hpp
│   │   └── token.cpp
│   ├── parser/
│   │   ├── parser.hpp
│   │   └── parser.cpp
│   └── semantic_analysis/
│       ├── ast.hpp
│       ├── ast.cpp
│       ├── ast_printer.hpp
│       ├── ast_printer.cpp
│       ├── semantic_analyzer.hpp
│       ├── semantic_analyzer.cpp
│       ├── symbol_table.hpp
│       ├── symbol_table.cpp
│       └── type.hpp
├── doc/
│   ├── milestone-1/
│   ├── milestone-2/
│   └── milestone-3/
├── test/
│   ├── milestone-1/
│   │   ├── input/
│   │   └── output/
│   ├── milestone-2/
│   │   ├── input/
│   │   └── output/
│   └── milestone-3/
│       ├── input/
│       └── output/
├── Makefile
└── README.md
```

---

## Pembagian Tugas

| NIM | Nama Lengkap | Milestone 1 | Milestone 2 | Milestone 3 |
|---|---|---|---|---|
| 13524015 | Mahatma Brahmana | Membuat diagram DFA & membuat kode lexer | Membuat laporan milestone 2 & merevisi kode lexer | Semantic analyzer, type checking, scope, dan validasi symbol |
| 13524057 | Benedict Darrel Setiawan | Membuat diagram DFA & membuat kode lexer | Membuat laporan milestone 2 & merevisi kode lexer | Foundation struktur data AST dan symbol table |
| 13524059 | Raymond Jonathan Dwi Putra J | Membuat diagram DFA & membuat laporan | Membuat laporan milestone 2 & membuat kode parser | Output integration, AST printer, test case, dokumentasi, dan beberapa revisi |
| 13524111 | Reynard Anderson Wijaya | Membuat diagram DFA & membuat laporan | Membuat laporan milestone 2 & membuat kode parser |  AST builder dari parse tree ke AST |
