# Arion Compiler and Interpreter

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

Program ini merupakan implementasi pipeline bahasa pemrograman **Arion** untuk Tugas Besar IF2224. Sampai Milestone 4, program sudah mencakup lexer, parser, AST builder, semantic analyzer, intermediate code generator, dan interpreter berbasis stack machine.

Alur utama program:

```text
Source code Arion
-> Lexer
-> Parser
-> Parse Tree
-> AST Builder
-> Semantic Analyzer
-> Decorated AST + Symbol Table
-> Intermediate Code Generator
-> Instruksi Stack Machine
-> Interpreter / Virtual Machine
-> Program Output
```

Pada Milestone 1, program mengimplementasikan **lexical analyzer**. Lexer membaca source code Arion dan menghasilkan daftar token menggunakan pendekatan Deterministic Finite Automaton (DFA).

Pada Milestone 2, program menambahkan **syntax analyzer**. Parser membaca daftar token, memeriksa grammar Arion menggunakan Recursive Descent Parser, lalu menghasilkan parse tree.

Pada Milestone 3, program menambahkan **semantic analyzer**. Tahap ini membangun AST dari parse tree, melakukan pengecekan semantic seperti deklarasi identifier, scope, type checking, validasi structured type, dan menghasilkan Decorated AST beserta Symbol Table (`TAB`, `ATAB`, dan `BTAB`).

Pada Milestone 4, program menambahkan **intermediate code generator** dan **interpreter**. Decorated AST dan symbol table diterjemahkan menjadi instruksi stack machine seperti `INT`, `LIT`, `LOD`, `STO`, `PLO`, `PST`, `JMP`, `JPC`, `OPR`, dan `RET`. Instruksi tersebut kemudian dijalankan oleh interpreter untuk menghasilkan output program.

Fitur utama program:

- Tokenisasi source code Arion.
- Parsing program Arion sesuai grammar.
- Pembangunan AST dari parse tree.
- Semantic analysis untuk deklarasi, scope, type checking, assignment, procedure/function call, array, record, subrange, dan enumerated type.
- Pembuatan Decorated AST dan Symbol Table terformat.
- Intermediate code generator berbasis instruksi stack machine.
- Runtime interpreter dengan program counter, stack, memory model, dan fetch-decode-execute cycle.
- Eksekusi assignment, ekspresi aritmatika, comparison, `write`, `writeln`, `if`, `if-else`, `while`, array access, dan record access dasar.
- Runtime validation untuk stack underflow/overflow, memory out-of-bounds, invalid jump target, invalid OPR, division/modulo by zero, dan arithmetic overflow/underflow.

Catatan implementasi:

- Format intermediate code akhir yang dibaca interpreter adalah instruksi stack machine, bukan TAC eksplisit dengan temporary variable.
- `CAL` untuk procedure/function call user-defined sudah dikenali, tetapi runtime call frame penuh belum menjadi fitur final.
- Runtime array bounds saat ini memvalidasi alamat memory, belum validasi eksplisit terhadap batas deklarasi array pada setiap akses.

---

## Requirements

- **Compiler:** `g++`
- **Build tool:** `make`
- **Standar C++:** C++17
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

Perintah ini akan mengompilasi seluruh source code, menyimpan file object di folder `bin/`, dan menghasilkan executable `bin/arion`.

### Menjalankan Program

```bash
./bin/arion <input_file> <output_file>
```

Contoh:

```bash
./bin/arion test/milestone-4/input/input-8.txt output.txt
```

Keterangan:

- `<input_file>`: file teks berisi source code Arion.
- `<output_file>`: file teks hasil analisis, intermediate code, program output, atau pesan error.

---

## Format Output Milestone 4

Jika input valid dan tidak ada runtime error, output berisi Decorated AST, Symbol Tables, Intermediate Code, dan Program Output.

```text
=== Decorated AST Tree ===
ProgramNode(name: 'CoordinateVertex') [type=program, tab=38, lev=0]
|-- Declarations
...

=== Symbol Tables ===
=== TAB ===
identifier | link | obj | type | ref | nrm | lev | adr
...

=== Intermediate Code ===
0 INT 0 6
1 LIT 0 1
2 LIT 0 0
3 LIT 0 3
4 OPR 0 2
5 PST 0 0
...

=== Program Output ===
(1,2,2)
```

Jika input memiliki semantic error, output diawali bagian `=== Semantic Errors ===`, lalu tetap menampilkan Decorated AST dan Symbol Table yang berhasil dibentuk.

Jika runtime error terjadi, output menambahkan bagian `=== Runtime Error ===` setelah `=== Program Output ===`.

---

## Menjalankan Test

### Menjalankan Semua Test Milestone 4

```bash
for input in test/milestone-4/input/input-*.txt; do
  base=$(basename "$input" .txt)
  ./bin/arion "$input" "test/milestone-4/output/${base/input/output}.txt"
done
```

### Smoke Test Milestone Sebelumnya

Program masih dapat dijalankan menggunakan input milestone sebelumnya.

```bash
./bin/arion test/milestone-3/input/input-1.txt /tmp/output-m3.txt
```

---

## Membersihkan Build

```bash
make clean
```

---

## Struktur Repository

```text
JML-Tubes-IF2224-2026/
|-- src/
|   |-- main.cpp
|   |-- lexer/
|   |   |-- lexer.hpp
|   |   |-- lexer.cpp
|   |   |-- token.hpp
|   |   `-- token.cpp
|   |-- parser/
|   |   |-- parser.hpp
|   |   `-- parser.cpp
|   |-- semantic_analysis/
|   |   |-- ast.hpp
|   |   |-- ast.cpp
|   |   |-- ast_printer.hpp
|   |   |-- ast_printer.cpp
|   |   |-- semantic_analyzer.hpp
|   |   |-- semantic_analyzer.cpp
|   |   |-- symbol_table.hpp
|   |   |-- symbol_table.cpp
|   |   `-- type.hpp
|   |-- codegen/
|   |   |-- codegen.hpp
|   |   `-- codegen.cpp
|   `-- interpreter/
|       |-- interpreter.hpp
|       `-- interpreter.cpp
|-- doc/
|   |-- milestone-1/
|   |-- milestone-2/
|   |-- milestone-3/
|   `-- milestone-4/
|-- test/
|   |-- milestone-1/
|   |-- milestone-2/
|   |-- milestone-3/
|   `-- milestone-4/
|       |-- input/
|       `-- output/
|-- Makefile
`-- README.md
```

---

## Pembagian Tugas

### Pembagian Tugas Milestone 1-3

| NIM | Nama Lengkap | Milestone 1 | Milestone 2 | Milestone 3 |
|---|---|---|---|---|
| 13524015 | Mahatma Brahmana | Membuat diagram DFA dan membuat kode lexer | Membuat laporan milestone 2 dan merevisi kode lexer | Semantic analyzer, type checking, scope, dan validasi symbol |
| 13524057 | Benedict Darrel Setiawan | Membuat diagram DFA dan membuat kode lexer | Membuat laporan milestone 2 dan merevisi kode lexer | Foundation struktur data AST dan symbol table |
| 13524059 | Raymond Jonathan Dwi Putra J | Membuat diagram DFA dan membuat laporan | Membuat laporan milestone 2 dan membuat kode parser | Output integration, AST printer, test case, dokumentasi, dan beberapa revisi |
| 13524111 | Reynard Anderson Wijaya | Membuat diagram DFA dan membuat laporan | Membuat laporan milestone 2 dan membuat kode parser | AST builder dari parse tree ke AST |

### Pembagian Tugas Milestone 4

| NIM | Nama | Pengerjaan pada milestone ini |
|---|---|---|
| 13524015 | Mahatma Brahmana | Implementasi control flow (`IF-ELSE`, `WHILE`, `JMP`, `JPC`), runtime execution, dokumentasi control flow, laporan |
| 13524057 | Benedict Darrel Setiawan | Runtime validation dan error handling, overflow/underflow detection, wrapper untuk type checking, array dan record access, dokumentasi pengujian dan validasi runtime, laporan |
| 13524059 | Raymond Jonathan D P J | Interpreter core, stack machine, manajemen memori runtime, dokumentasi interpreter, laporan |
| 13524111 | Reynard Anderson W | Intermediate code generator, translasi expression dan assignment, dokumentasi code generation, laporan |
