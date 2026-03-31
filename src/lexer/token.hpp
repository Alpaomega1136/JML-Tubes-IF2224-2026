#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>

enum ListToken {
    // =========================================================
    // === Literal / Konstanta
    // =========================================================
    intcon,       // Konstanta integer          contoh: 1, 3, 48
    realcon,      // Konstanta bilangan riil    contoh: 3.14, 26.7
    charcon,      // Konstanta karakter tunggal contoh: 'j', 'k', 't'
    stringcon,    // Sekuens karakter           contoh: 'IRK', 'TBFO'

    // =========================================================
    // === Operator Aritmatika
    // =========================================================
    plus,         // Pertambahan                contoh: +
    minus,        // Pengurangan                contoh: -
    times,        // Perkalian                  contoh: *
    idiv,         // Pembagian integer          contoh: div
    rdiv,         // Pembagian riil             contoh: /
    imod,         // Modulo                     contoh: MOD

    // =========================================================
    // === Operator Logika
    // =========================================================
    notsy,        // Negasi (NOT)               contoh: NOT
    andsy,        // Konjungsi (AND)            contoh: AND
    orsy,         // Disjungsi (OR)             contoh: OR

    // =========================================================
    // === Operator Perbandingan
    // =========================================================
    eql,          // Sama dengan                contoh: ==
    neq,          // Tidak sama dengan          contoh: <>
    gtr,          // Lebih besar                contoh: >
    geq,          // Lebih besar atau sama      contoh: >=
    lss,          // Lebih kecil                contoh: <
    leq,          // Lebih kecil atau sama      contoh: <=

    // =========================================================
    // === Tanda Baca / Delimiter
    // =========================================================
    lparent,      // Kurung kiri                contoh: (
    rparent,      // Kurung kanan               contoh: )
    lbrack,       // Kurung siku kiri           contoh: [
    rbrack,       // Kurung siku kanan          contoh: ]
    comma,        // Koma                       contoh: ,
    semicolon,    // Titik koma                 contoh: ;
    period,       // Titik                      contoh: .
    colon,        // Titik dua                  contoh: :
    becomes,      // Assignment                 contoh: :=

    // =========================================================
    // === Keyword Deklarasi
    // =========================================================
    constsy,      // Deklarasi konstanta        contoh: const
    typesy,       // Deklarasi tipe data        contoh: type
    varsy,        // Deklarasi variabel         contoh: var
    functionsy,   // Deklarasi fungsi           contoh: function
    proceduresy,  // Deklarasi prosedur         contoh: procedure
    arraysy,      // Deklarasi array            contoh: array
    recordsy,     // Deklarasi record           contoh: record
    programsy,    // Deklarasi program          contoh: program

    // =========================================================
    // === Keyword Kontrol Alur
    // =========================================================
    beginsy,      // Awal blok                  contoh: begin
    endsy,        // Akhir blok                 contoh: end
    ifsy,         // Percabangan                contoh: if
    thensy,       // Cabang kondisi benar       contoh: then
    elsesy,       // Cabang kondisi salah       contoh: else
    casesy,       // Percabangan kasus          contoh: case
    ofsy,         // Pilihan case               contoh: of
    whilesy,      // Perulangan while           contoh: while
    dosy,         // Badan while/for            contoh: do
    repeatsy,     // Perulangan repeat          contoh: repeat
    untilsy,      // Kondisi akhir repeat       contoh: until
    forsy,        // Perulangan for             contoh: for
    tosy,         // Arah naik for              contoh: to
    downtosy,     // Arah turun for             contoh: downto

    // =========================================================
    // === Identifier & Komentar
    // =========================================================
    ident,        // Nama variabel/fungsi/dll (case-insensitive, diawali huruf)
    comment       // Komentar: { ... } atau (* ... *)
};
class Token {
    private:

    public:
        ListToken type;
        std::string value;  
        Token(ListToken type, std::string val) : type(type), value(val) {}
        const std::string tokenTypeToString() const;
};

#endif
