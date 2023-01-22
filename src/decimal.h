#ifndef SRC_DECIMAL_H_
#define SRC_DECIMAL_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  unsigned bits[4];
} Decimal;

typedef struct {
  unsigned bits[7];
} Big_decimal;

//  ==Преобразование==  //

int from_int_to_decimal(int src, Decimal *dst);
int from_float_to_decimal(float src, Decimal *dst);
int from_decimal_to_int(Decimal src, int *dst);
int from_decimal_to_float(Decimal src, float *dst);

//  ==Сравнение==  //

int is_less(Decimal dec_a, Decimal dec_b);
//  dec_a < dec_b
int is_less_or_equal(Decimal dec_a, Decimal dec_b);
//  dec_a <= dec_b
int is_greater(Decimal dec_a, Decimal dec_b);
//  dec_a > dec_b
int is_greater_or_equal(Decimal dec_a, Decimal dec_b);
//  dec_a >= dec_b
int is_equal(Decimal dec_a, Decimal dec_b);
//  dec_a == dec_b
int is_not_equal(Decimal dec_a, Decimal dec_b);
//  dec_a != dec_b

//  ==Прочие функции==  //

int u_floor(Decimal value, Decimal *result);
int u_round(Decimal value, Decimal *result);
int truncate(Decimal value, Decimal *result);
int negate(Decimal value, Decimal *result);  // done

//  ==Арифметика==  //
int add(Decimal value_1, Decimal value_2,
            Decimal *result);  // myeshask
int sub(Decimal value_1, Decimal value_2,
            Decimal *result);  // minellaa
int mul(Decimal value_1, Decimal value_2,
            Decimal *result);  // windhelg
int u_div(Decimal value_1, Decimal value_2,
            Decimal *result);  // minellaa
int mod(Decimal value_1, Decimal value_2, Decimal *result);

//////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ====  //////
//  ====REGULAR Decimal====  //
void print_decimal(Decimal for_print);
void init_decimal(Decimal *set_me_free);
int get_bit(Decimal src, int position);
void set_bit(Decimal *src, const int n, char bit);
int get_scale(Decimal src);
void set_scale(Decimal *dst, unsigned char mask);
int get_sign(Decimal src);
void from_decimal_to_big_decimal(Decimal src, Big_decimal *dst);

//  ====BIG Decimal====  //
void print_big_decimal(Big_decimal for_print);
void init_big_decimal(Big_decimal *set_me_free);
int get_big_bit(Big_decimal src, int position);
void set_big_bit(Big_decimal *src, const int n, char bit);
int get_big_scale(Big_decimal src);
void set_big_scale(Big_decimal *dst, unsigned char mask);
int get_big_sign(Big_decimal src);
int from_big_decimal_to_decimal(Big_decimal src, Decimal *dst);
int check_highest_bits(Big_decimal value);
int normalizer(Big_decimal *value);

////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ СЛОЖЕНИЯ====  ////
//  ====REGULAR Decimal====  //
int to_ten(Decimal *value);
int equal_scales(Decimal *value_1, Decimal *value_2);

//  ====BIG Decimal====  //
void big_add_to_self(Big_decimal value, Big_decimal *result);
void big_left_shift(Big_decimal *b_value);
void big_add(Big_decimal value_1, Big_decimal value_2,
                 Big_decimal *result);
int big_to_ten(Big_decimal *value);
int equal_big_scales(Big_decimal *value_1, Big_decimal *value_2);

//  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ TRUNCATE====  //
void to_ten_div(Decimal *value);
void big_to_ten_div(Big_decimal *value);
void char_shift_set_bit(char *dst, char bit);

//  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ ВЫЧИТАНИЯ====  //
char who_has_highest_bit(Decimal value_1, Decimal value_2);
char who_big_has_highest_bit(Big_decimal value_1,
                                 Big_decimal value_2);

//  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ ROUND====  //
char absolute_zero(Decimal value);
char big_absolute_zero(Big_decimal value);

////  ====ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ СРАВНЕНИЙ====  ////
int engine_comp(Decimal dec_a, Decimal dec_b);

////  ======ВСПОМОГАТЕЛЬНАЯ ДЛЯ ДЕЛЕНИЯ======  ////
void big_div(Big_decimal v1, Big_decimal v2,
                 Big_decimal *result, Big_decimal *res);
void mul_to_ten(Big_decimal *value);
size_t dot_part(Big_decimal v1, Big_decimal v2,
                    Big_decimal *result);
int lessly(Big_decimal v1, Big_decimal v2);

#endif  // SRC_DECIMAL_H_