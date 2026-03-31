#ifndef LEXER_HPP
#define LEXER_HPP

#include <vector>
#include "token.hpp"
#include <string>
#include <fstream>
#include <cctype>
using namespace std;

enum State {
    START_STATE,
    IDENT_STATE,      // membaca identifier/keyword
    COMMENT1_STATE,   // membaca '{' → komentar
    COMMENT2_STATE,   // membaca '(*' → komentar
    COMMENT2_END_STATE,

    INT_STATE,        // membaca angka integer
    INT_PERIOD_STATE,  // membaca angka integer lalu .
    REAL_STATE,       // membaca angka riil (setelah titik)

    START_QUOTE_STATE,
    STRING_ONGOING_STATE,
    STRING_END_STATE,
    EMPTY_STRING_STATE,
    CHAR_STATE,
    CHAR_END_STATE,

    MINUS_STATE,
    PLUS_STATE,
    LPARENT_STATE,
    RPARENT_STATE,
    TIMES_STATE,
    COMMA_STATE,
    PERIOD_STATE,
    RDIV_STATE,
    COLON_STATE,
    BECOMES_STATE,
    LBRACK_STATE,
    RBRACK_STATE,
    SEMICOLON_STATE,
    LSS_STATE,
    LEQ_STATE,
    NEQ_STATE,
    SEMI_EQL_STATE,
    EQL_STATE,
    GTR_STATE,
    GEQ_STATE,


    A_STATE,
    AN_STATE,
    AND_STATE,
    AR_STATE,
    ARR_STATE,
    ARRA_STATE,
    ARRAY_STATE,
    B_STATE,
    BE_STATE,
    BEG_STATE,
    BEGI_STATE,
    BEGIN_STATE,
    C_STATE,
    CA_STATE,
    CAS_STATE,
    CASE_STATE,
    CO_STATE,
    CON_STATE,
    CONS_STATE,
    CONST_STATE,
    D_STATE,
    DI_STATE,
    DIV_STATE,
    DO_STATE,
    DOW_STATE,
    DOWN_STATE,
    DOWNT_STATE,
    DOWNTO_STATE,
    E_STATE,
    EL_STATE,
    ELS_STATE,
    ELSE_STATE,
    EN_STATE,
    END_STATE,
    F_STATE,
    FO_STATE,
    FOR_STATE,
    FU_STATE,
    FUN_STATE,
    FUNC_STATE,
    FUNCT_STATE,
    FUNCTI_STATE,
    FUNCTIO_STATE,
    FUNCTION_STATE,
    I_STATE,
    IF_STATE,
    M_STATE,
    MO_STATE,
    MOD_STATE,
    N_STATE,
    NO_STATE,
    NOT_STATE,
    O_STATE,
    OF_STATE,
    OR_STATE,
    P_STATE,
    PR_STATE,
    PRO_STATE,
    PROC_STATE,
    PROCE_STATE,
    PROCED_STATE,
    PROCEDU_STATE,
    PROCEDUR_STATE,
    PROCEDURE_STATE,
    PROG_STATE,
    PROGR_STATE,
    PROGRA_STATE,
    PROGRAM_STATE,
    R_STATE,
    RE_STATE,
    REC_STATE,
    RECO_STATE,
    RECOR_STATE,
    RECORD_STATE,
    REP_STATE,
    REPE_STATE,
    REPEA_STATE,
    REPEAT_STATE,
    T_STATE,
    TH_STATE,
    THE_STATE,
    THEN_STATE,
    TO_STATE,
    TY_STATE,
    TYP_STATE,
    TYPE_STATE,
    U_STATE,
    UN_STATE,
    UNT_STATE,
    UNTI_STATE,
    UNTIL_STATE,
    V_STATE,
    VA_STATE,
    VAR_STATE,
    W_STATE,
    WH_STATE,
    WHI_STATE,
    WHIL_STATE,
    WHILE_STATE,

    UNKNOWN_STATE,
    UNKNOWN2_STATE
};

// Fungsi utama yanh berfungsi untuk mengubah isi file.txt menjadi list of token
vector<Token> tokenize(const std::string& filename);

#endif

