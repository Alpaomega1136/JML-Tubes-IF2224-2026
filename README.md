# Arion Compiler — Milestone 1: Lexical Analyzer

**IF2224 Teori Bahasa Formal dan Otomata**  
Laboratorium Ilmu Rekayasa dan Komputasi, STEI ITB

---

## Identitas Kelompok

**Nama Kelompok:** JML

| Nama Lengkap | NIM |
|---|---|
| Mahatma Brahmana | 13524015 |
| Benedict Darrel Setiawan | 13524057 |
| Raymond Jonathan Dwi Putra J | 13524059 |
| Reynard Anderson Wijaya | 13524111 |

---

## Deskripsi Program

Program ini merupakan implementasi **lexical analyzer (lexer)** untuk bahasa pemrograman **Arion** sebagai bagian dari Milestone 1 Tugas Besar IF2224. Lexer bertugas membaca kode sumber Arion dan mengonversinya menjadi deretan token menggunakan **Deterministic Finite Automaton (DFA)**.

Program mampu mengenali dan mengklasifikasikan 52 jenis token, meliputi:
- **Literal:** `intcon`, `realcon`, `charcon`, `string`
- **Identifier:** `ident`
- **Keyword:** `program`, `var`, `const`, `type`, `function`, `procedure`, `array`, `record`, `begin`, `end`, `if`, `then`, `else`, `case`, `of`, `while`, `do`, `repeat`, `until`, `for`, `to`, `downto`
- **Operator aritmatika & logika:** `plus`, `minus`, `times`, `idiv`, `rdiv`, `imod`, `andsy`, `orsy`, `notsy`
- **Operator relasional:** `eql`, `neq`, `gtr`, `geq`, `lss`, `leq`
- **Delimiter:** `lparent`, `rparent`, `lbrack`, `rbrack`, `comma`, `semicolon`, `period`, `colon`, `becomes`
- **Komentar:** `comment`

---

## Requirements

- **Compiler:** `g++` (GNU C++ Compiler)
- **Standar C++:** C++17
- **Build tool:** `make`
- **Sistem Operasi:** Linux / macOS / Windows (dengan MinGW atau WSL)

---

## Cara Instalasi dan Penggunaan Program

### 1. Clone Repository

```bash
git clone https://github.com/Alpaomega1136/JML-Tubes-IF2224-2026.git
cd JML-Tubes-IF2224-2026
```

### 2. Build Program

```bash
make
```

Perintah ini akan mengompilasi seluruh source code dan menghasilkan executable bernama `lexer`.

### 3. Menjalankan Program

```bash
./lexer <input_file> <output_file>
```

**Contoh:**

```bash
./lexer input.txt output.txt
```

- `<input_file>` : file teks berisi kode sumber Arion
- `<output_file>` : file teks hasil tokenisasi (satu token per baris)

### 4. Format Output

Setiap baris pada file output merepresentasikan satu token dengan format:

```
tokenType (value)
```

Untuk token tanpa nilai literal (misalnya keyword dan operator), formatnya:

```
tokenType
```

**Contoh output:**
```
program
ident (myProgram)
semicolon
intcon (42)
```

### 5. Membersihkan Build

```bash
make clean
```

---

## Struktur Repository

```
JML-Tubes-IF2224-2026/
├── src/
│   ├── main.cpp
│   └── lexer/
│       ├── lexer.hpp
│       ├── lexer.cpp
│       ├── token.hpp
│       └── token.cpp
├── doc/
│   └── milestone-1/
│       ├── Laporan-1-JML-1.pdf
│       ├── Diagram DFA Lexer.drawio.xml
│       └── Spesifikasi Milestone 1 - Tubes IF2224 TBFO.pdf
├── test/
│   └── milestone-1/
│       ├── input/
│       │   ├── input-1.txt
│       │   ├── input-2.txt
│       │   ├── input-3.txt
│       │   ├── input-4.txt
│       │   └── input-5.txt
│       └── output/
│           ├── output-1.txt
│           ├── output-2.txt
│           ├── output-3.txt
│           ├── output-4.txt
│           └── output-5.txt
├── Makefile
└── README.md
```

---

## Pembagian Tugas

| Nama Lengkap | NIM | Tugas |
|---|---|---|
| Mahatma Brahmana | 13524015 |Membuat diagram DFA & membuat kode lexer |
| Benedict Darrel Setiawan | 13524057 |Membuat diagram DFA & membuat kode lexer |
| Raymond Jonathan Dwi Putra J | 13524059 | Membuat diagram DFA & membuat laporan|
| Reynard Anderson Wijaya | 13524111 | Membuat diagram DFA & membuat laporan |