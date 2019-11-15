/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_SHELL_BISON_HH_INCLUDED
# define YY_YY_SHELL_BISON_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    WORD = 258,
    BACKTIK = 259,
    NOTOKEN = 260,
    GREAT = 261,
    NEWLINE = 262,
    GTFO = 263,
    LESS = 264,
    TOOGREAT = 265,
    TOOGREATAND = 266,
    PIPE = 267,
    AMPERSAND = 268,
    GREATAND = 269,
    TAB = 270,
    SRC = 271,
    ANDAND = 272,
    ALIAS = 273,
    GETS = 274,
    PUSHD = 275,
    POPD = 276,
    FG = 277,
    BG = 278
  };
#endif
/* Tokens.  */
#define WORD 258
#define BACKTIK 259
#define NOTOKEN 260
#define GREAT 261
#define NEWLINE 262
#define GTFO 263
#define LESS 264
#define TOOGREAT 265
#define TOOGREATAND 266
#define PIPE 267
#define AMPERSAND 268
#define GREATAND 269
#define TAB 270
#define SRC 271
#define ANDAND 272
#define ALIAS 273
#define GETS 274
#define PUSHD 275
#define POPD 276
#define FG 277
#define BG 278

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 8 "shell_bison.yy" /* yacc.c:1909  */

    char * string_val;

#line 104 "shell_bison.hh" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_SHELL_BISON_HH_INCLUDED  */
