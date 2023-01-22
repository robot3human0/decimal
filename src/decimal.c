#include "decimal.h"

////  ==Преобразование==  ////
int from_decimal_to_float(Decimal src, float *dst) {
  int res_code = 9;
  if (dst) {
    *dst = 0;
    res_code = 0;
    double temp = 0;
    int sign = get_sign(src);
    int scale = get_scale(src);
    for (size_t i = 0; i < 96; i++) {
      if (get_bit(src, i)) {
        temp += pow(2, i);
      }
    }
    if (scale < 29) {
      for (int i = 0; i < scale; i++) {
        temp /= 10;
      }
      *dst = (float)temp;
      if (sign) {
        *dst = -*dst;
      }
    } else {
      res_code = 1;
    }
  } else {
    res_code = 1;
  }
  return res_code;
}

int from_float_to_decimal(float src, Decimal *dst) {
  int res_code = 9;
  init_decimal(dst);
  if (src && dst) {
    res_code = 0;
    char sign = 0;
    if (src < 0) {
      sign = 1;
      src = -src;
    }
    char buff[100];
    sprintf(buff, "%7f", src);
    size_t limit = strlen(buff);
    char buff2[limit - 1];
    int x_ten = 1;
    int zero = 0;
    int not_zero = 0;
    char dot = 0;
    char post_dot_counter = 0;
    for (size_t i = 0; i < limit; i++) {
      if (buff[i] == '.') {
        dot = 1;
        continue;
      }
      buff2[i - dot] = buff[i];
      if (dot) {
        if (buff[i] == '0') {
          zero++;
          x_ten *= 10;
          if (not_zero) {
            post_dot_counter++;
            x_ten = 10;
            not_zero = 0;
          }
        } else {
          post_dot_counter++;
          if (zero) {
            not_zero++;
            zero = 0;
          }
        }
      }
    }
    int result = atoi(buff2);
    result /= x_ten;
    dst->bits[0] = result;
    set_scale(dst, post_dot_counter);
    if (sign) {
      set_bit(dst, 127, 1);
    }
  } else {
    res_code = 1;
  }
  return res_code;
}

int from_int_to_decimal(int src, Decimal *dst) {
  int res_code = 1;
  init_decimal(dst);
  if (dst) {
    res_code = 0;
    char src_bit = 0;
    char sign = 0;
    if (src < 0) {
      sign = 1;
      src = -src;
    }
    dst->bits[0] = src;
    set_bit(dst, 127, sign);
  }
  return res_code;
}

int from_decimal_to_int(Decimal src, int *dst) {
  int res_code = 9;
  int sign = get_sign(src);
  int scale = get_scale(src);
  if (scale) {
    while (scale) {
      to_ten_div(&src);
      scale--;
    }
  }

  *dst = src.bits[0];
  if (sign) {
    *dst = -*dst;
  }
  res_code = 0;

  return res_code;
}

////  ==Сравнение==  ////
int is_greater(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code == 2) {
    res_code = 0;
  }
  return res_code;
}

int is_greater_or_equal(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code == 2) {
    res_code = 1;
  }
  return res_code;
}

int is_less(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code) {
    res_code = 0;
  } else {
    res_code = 1;
  }
  return res_code;
}

int is_less_or_equal(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code == 1) {
    res_code = 0;
  } else {
    res_code = 1;
  }
  return res_code;
}

int is_equal(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code == 2) {
    res_code = 1;
  } else {
    res_code = 0;
  }
  return res_code;
}

int is_not_equal(Decimal dec_a, Decimal dec_b) {
  int res_code = engine_comp(dec_a, dec_b);
  if (res_code == 2) {
    res_code = 0;
  } else {
    res_code = 1;
  }
  return res_code;
}

////  ==Прочие функции==  ////
int u_floor(Decimal value, Decimal *result) {
  int res_code = 0;
  int scale = get_scale(value);
  int sign = get_sign(value);
  if (scale) {
    res_code = truncate(value, result);
    if (sign) {
      Decimal the_one;
      init_decimal(&the_one);
      the_one.bits[3] = 0x80000000;
      the_one.bits[0] = 1;
      res_code = add(*result, the_one, result);
    }
  } else {
    *result = value;
  }
  return res_code;
}

int u_round(Decimal value, Decimal *result) {
  int res_code = 9;
  init_decimal(result);
  Decimal truncate_result = {0};
  Decimal standart;
  standart.bits[3] = 0x00010000;
  standart.bits[2] = 0;
  standart.bits[1] = 0;
  standart.bits[0] = 5;
  char sign = get_sign(value);
  char scale = get_scale(value);
  char hi_bit = 0;
  if (sign) {
    set_bit(&value, 127, 0);
  }
  char till_trunc = absolute_zero(value);
  res_code = truncate(value, &truncate_result);
  if (!res_code && !absolute_zero(truncate_result)) {
    res_code = sub(value, truncate_result, result);
    if (!res_code) {
      for (char i = 0; i < scale - 1; i++) {
        to_ten(&standart);
      }
      hi_bit = who_has_highest_bit(*result, standart);
      standart.bits[3] = 0;
      standart.bits[0] = 1;
      if (!hi_bit) {
        if (sign) {
          *result = truncate_result;
        } else {
          res_code = add(standart, truncate_result, result);
        }
      } else if (hi_bit) {
        if (sign) {
          res_code = add(standart, truncate_result, result);
        } else {
          *result = truncate_result;
        }
      }
      result->bits[3] = 0;
      if (sign) {
        set_bit(result, 127, 1);
      }
    }
  } else if (sign && !absolute_zero(value)) {
    result->bits[0] = 1;
    set_bit(result, 127, sign);
  }
  return res_code;
}

int truncate(Decimal value, Decimal *result) {
  int res_code = 1;
  if (result) {
    init_decimal(result);
    int sign = get_sign(value);
    int scale = get_scale(value);
    value.bits[3] = 0;
    if (scale) {
      for (int i = 0; i < scale; i++) {
        to_ten_div(&value);
      }
      *result = value;
      if (sign) {
        set_bit(result, 127, 1);
      }
      res_code = 0;
    } else {
      *result = value;
      set_bit(result, 127, sign);
      res_code = 0;
    }
  }
  return res_code;
}

int negate(Decimal value, Decimal *result) {
  int res_code = 9;
  if (result) {
    res_code = 0;
    result->bits[3] = value.bits[3];
    result->bits[2] = value.bits[2];
    result->bits[1] = value.bits[1];
    result->bits[0] = value.bits[0];
    if (get_sign(value)) {
      set_bit(result, 127, 0);
    } else {
      set_bit(result, 127, 1);
    }
  } else {
    res_code = 1;
  }
  return res_code;
}

////  ==Арифметика==  ////
int add(Decimal value_1, Decimal value_2, Decimal *result) {
  int res_code = 9;
  init_decimal(result);
  int sign_1 = get_sign(value_1);
  int sign_2 = get_sign(value_2);
  if (sign_1 != sign_2) {
    res_code = sub(value_1, value_2, result);
  } else {
    int scale_1 = get_scale(value_1);
    int scale_2 = get_scale(value_2);
    Big_decimal b_value_1;
    Big_decimal b_value_2;
    Big_decimal b_result;
    from_decimal_to_big_decimal(value_1, &b_value_1);
    from_decimal_to_big_decimal(value_2, &b_value_2);
    if (scale_1 != scale_2) {
      res_code = equal_big_scales(&b_value_1, &b_value_2);
      scale_1 = get_big_scale(b_value_1);
    }
    if (res_code == 0 || res_code == 9) {
      big_add(b_value_1, b_value_2, &b_result);
      res_code = from_big_decimal_to_decimal(b_result, result);
    }
  }
  return res_code;
}

int sub(Decimal value_1, Decimal value_2, Decimal *result) {
  int res_code = 9;
  int sign_1 = get_sign(value_1);
  int sign_2 = get_sign(value_2);
  char greater_flag = 0;
  init_decimal(result);
  if (sign_1 && sign_2) {
    res_code = add(value_1, value_2, result);
  } else {
    Big_decimal b_value_1;
    Big_decimal b_value_2;
    Big_decimal b_result;
    init_big_decimal(&b_result);
    from_decimal_to_big_decimal(value_1, &b_value_1);
    from_decimal_to_big_decimal(value_2, &b_value_2);
    int scale_1 = get_big_scale(b_value_1);
    int scale_2 = get_big_scale(b_value_2);
    if (scale_1 != scale_2) {
      res_code = equal_big_scales(&b_value_1, &b_value_2);
      scale_1 = get_big_scale(b_value_1);
    }
    if (res_code == 0 || res_code == 9) {
      Big_decimal swap;
      init_big_decimal(&swap);
      greater_flag = who_big_has_highest_bit(b_value_1, b_value_2);
      if (greater_flag == 0) {
        res_code = 0;
      } else if (greater_flag == 1) {
        res_code = 0;
        swap = b_value_1;
        b_value_1 = b_value_2;
        b_value_2 = swap;
      }
      if (res_code == 0) {
        char a = 0;
        char b = 0;
        char count_mem = 0;
        for (size_t i = 0; i < 192; i++) {
          a = get_big_bit(b_value_1, i);
          b = get_big_bit(b_value_2, i);
          if (a && !b) {
            set_big_bit(&b_result, i, 1);
          } else if (b && !a) {
            while (!a) {
              a = get_big_bit(b_value_1, ++i);
              count_mem++;
            }
            set_big_bit(&b_value_1, i, 0);
            while (count_mem) {
              i--;
              if (count_mem != 1) {
                set_big_bit(&b_value_1, i, 1);
              }
              count_mem--;
            }
            set_big_bit(&b_result, i, 1);
          }
        }
      }
    }
    if (scale_1) {
      set_big_scale(&b_result, scale_1);
    }
    if (!sign_1 && greater_flag == 1) {
      set_big_bit(&b_result, 223, 1);
    } else if (greater_flag) {
      set_big_bit(&b_result, 223, sign_2);
    } else {
      set_big_bit(&b_result, 223, sign_1);
    }
    res_code = from_big_decimal_to_decimal(b_result, result);
  }
  return res_code;
}

int mul(Decimal value_1, Decimal value_2, Decimal *result) {
  int res_code = 9;
  int scale1 = get_scale(value_1);
  int scale2 = get_scale(value_2);
  if (absolute_zero(value_1) || absolute_zero(value_2)) {
    init_decimal(result);
    res_code = 0;
  } else {
    Big_decimal b_value_2 = {0};
    Big_decimal b_value_1 = {0};
    Big_decimal b_result = {0};
    from_decimal_to_big_decimal(value_1, &b_value_1);
    from_decimal_to_big_decimal(value_2, &b_value_2);
    if (scale1 < scale2) {
      scale1 = scale2;
    }
    while (b_value_2.bits[0] || b_value_2.bits[1] || b_value_2.bits[2]) {
      if (b_value_2.bits[0] & 1) {
        big_add(b_value_1, b_result, &b_result);
      }
      for (int i = 0; i < 6; ++i) {
        b_value_1.bits[5 - i] = b_value_1.bits[5 - i] << 1;
        if ((b_value_2.bits[i] & (1 << 31))) {
          b_value_2.bits[i] &= ~(1 << 31);
          b_value_2.bits[i] = (b_value_2.bits[i] >> 1);
          b_value_2.bits[i] |= (1 << 30);
          b_value_2.bits[i] |= (b_value_2.bits[i + 1] & 1) << 31;
        } else {
          b_value_2.bits[i] = (b_value_2.bits[i] >> 1);
        }
        if (i != 5) {
          if ((b_value_1.bits[4 - i] & (1 << 31))) {
            b_value_1.bits[5 - i] |= 1;
          }
          b_value_2.bits[i] |= (b_value_2.bits[i + 1] & 1) << 31;
        }
      }
    }
    res_code = from_big_decimal_to_decimal(b_result, result);
    scale1 += scale2;
    set_scale(result, scale1);
    if (get_sign(value_1) != get_sign(value_2)) {
      set_bit(result, 127, 1);
    }
    result->bits[3] |= scale1 << 16;
  }
  return res_code;
}

int u_div(Decimal v1, Decimal v2, Decimal *result) {
  int res_code = 9;
  Decimal the_one = {1, 0};
  Decimal minor_one = {0};
  negate(the_one, &minor_one);
  init_decimal(result);
  int sign1 = get_sign(v1);
  int sign2 = get_sign(v2);
  if (absolute_zero(v2)) {
    res_code = 0;
  } else if (absolute_zero(v1)) {
    res_code = 3;
  } else if (is_equal(v2, the_one) || is_equal(v2, minor_one)) {
    res_code = 0;
    *result = v1;
    if (sign1 && sign2) {
      set_bit(result, 127, 0);
    } else if (sign1 || sign2) {
      set_bit(result, 127, 1);
    }
  } else {
    res_code = 0;
    Big_decimal bv_1 = {0};
    Big_decimal bv_2 = {0};
    Big_decimal b_result = {0};
    Big_decimal b_res = {0};
    from_decimal_to_big_decimal(v1, &bv_1);
    from_decimal_to_big_decimal(v2, &bv_2);
    int scale1 = get_scale(v1);
    int scale2 = get_scale(v2);
    if (scale1 != scale2) {
      equal_big_scales(&bv_1, &bv_2);
      scale1 = get_big_scale(bv_1);
    }
    bv_1.bits[6] = 0;
    bv_2.bits[6] = 0;
    big_div(bv_1, bv_2, &b_result, &b_res);
    if (!big_absolute_zero(b_res)) {
      scale1 = dot_part(b_res, bv_2, &b_result);
    } else {
      scale1 = 0;
    }
    from_big_decimal_to_decimal(b_result, result);
    set_scale(result, scale1);
    if (sign1 && sign2) {
      set_bit(result, 127, 0);
    } else if (sign1 || sign2) {
      set_bit(result, 127, 1);
    }
  }
  return res_code;
}

int mod(Decimal value_1, Decimal value_2, Decimal *result) {
  int res_code = 9;
  init_decimal(result);
  int sign1 = get_sign(value_1);
  int sign2 = get_sign(value_2);
  if (absolute_zero(value_1)) {
    res_code = 0;
  } else if (absolute_zero(value_2)) {
    res_code = 3;
  } else {
    res_code = 0;
    int scale1 = get_scale(value_1);
    int scale2 = get_scale(value_2);
    Decimal another_result = {0};
    Big_decimal bv_1 = {0};
    Big_decimal bv_2 = {0};
    Big_decimal b_result = {0};
    Big_decimal b_another_result = {0};
    from_decimal_to_big_decimal(value_1, &bv_1);
    from_decimal_to_big_decimal(value_2, &bv_2);
    if (scale1 != scale2) {
      equal_big_scales(&bv_1, &bv_2);
    }
    scale1 = get_big_scale(bv_1);
    bv_1.bits[6] = 0;
    bv_2.bits[6] = 0;
    big_div(bv_1, bv_2, &b_result, &b_another_result);
    from_big_decimal_to_decimal(b_another_result, result);
    set_scale(result, scale1);
    if (sign1) set_bit(result, 127, 1);
  }
  return res_code;
}

//////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ====  //////
//  ====REGULAR Decimal====  //
void print_decimal(Decimal for_print) {
  int c = 0;
  char buff[4][33];
  for (int i = 3, z = 0; i > -1; i--, z++) {
    for (int k = 31; k > -1; k--) {
      c = (for_print.bits[z] & 1);
      buff[i][k] = c + 48;
      for_print.bits[z] = (for_print.bits[z] >> 1);
    }
    buff[i][32] = '\0';
  }

  printf("             /-scale\\\n");
  printf("\033[46m [3] %s \033[m \n", buff[0]);
  printf("\033[43m [2] %s \033[m \n", buff[1]);
  printf("\033[41m [1] %s \033[m \n", buff[2]);
  printf("\033[45m [0] %s \033[m \n", buff[3]);
  printf("\n");
}

void init_decimal(Decimal *set_me_free) {
  for (size_t i = 0; i < 4; i++) set_me_free->bits[i] = 0x0;
}

void set_bit(Decimal *src, const int n, char bit) {
  if (src) {
    unsigned mask = 1u << (n % 32);
    if (bit == 0) {
      src->bits[n / 32] &= ~mask;
    } else {
      src->bits[n / 32] |= mask;
    }
  }
}

void set_scale(Decimal *dst, unsigned char mask) {
  unsigned sign = get_sign(*dst);
  dst->bits[3] = mask;
  dst->bits[3] = (dst->bits[3] << 16);
  set_bit(dst, 127, sign);
}

void from_decimal_to_big_decimal(Decimal src, Big_decimal *dst) {
  init_big_decimal(dst);
  dst->bits[0] = src.bits[0];
  dst->bits[1] = src.bits[1];
  dst->bits[2] = src.bits[2];
  dst->bits[6] = src.bits[3];
}

int get_sign(Decimal src) { return ((src.bits[3] >> 31) & 1); }

int get_scale(Decimal src) { return ((src.bits[3] & 0xFF0000) >> 16); }

int get_bit(Decimal src, int position) {
  int a = 0;
  int shift_num = 0;
  if (position >= 0 && position <= 31) {
    a = src.bits[0];
    shift_num = position;
  } else if (position >= 32 && position <= 63) {
    a = src.bits[1];
    shift_num = position - 32;
  } else if (position >= 64 && position <= 95) {
    a = src.bits[2];
    shift_num = position - 64;
  } else {
    a = src.bits[3];
    shift_num = position - 96;
  }

  // return !!(src.bits[position / 32] & (1 << (position % 32)));

  int res = (a >> shift_num) & 1;
  return res;
}

//  ====BIG Decimal====  //
void print_big_decimal(Big_decimal for_print) {
  int c = 0;
  char buff[7][33];
  for (int i = 6, z = 0; i > -1; i--, z++) {
    for (int k = 31; k > -1; k--) {
      c = (for_print.bits[z] & 1);
      buff[i][k] = c + 48;
      for_print.bits[z] = (for_print.bits[z] >> 1);
    }
    buff[i][32] = '\0';
  }

  printf("             /-scale\\\n");
  printf("\033[37m [6] %s \033[m \n", buff[0]);
  printf("\033[35m [5] %s \033[m \n", buff[1]);
  printf("\033[34m [4] %s \033[m \n", buff[2]);
  printf("\033[36m [3] %s \033[m \n", buff[3]);
  printf("\033[32m [2] %s \033[m \n", buff[4]);
  printf("\033[33m [1] %s \033[m \n", buff[5]);
  printf("\033[31m [0] %s \033[m \n", buff[6]);
  printf("\n");
}

void init_big_decimal(Big_decimal *set_me_free) {
  for (size_t i = 0; i < 7; i++) set_me_free->bits[i] = 0x0;
}

void set_big_bit(Big_decimal *src, const int n, char bit) {
  if (src) {
    unsigned mask = 1u << (n % 32);
    if (bit == 0) {
      src->bits[n / 32] &= ~mask;
    } else {
      src->bits[n / 32] |= mask;
    }
  }
}

void set_big_scale(Big_decimal *dst, unsigned char mask) {
  unsigned sign = get_big_sign(*dst);
  dst->bits[6] = mask;
  dst->bits[6] = (dst->bits[6] << 16);
  set_big_bit(dst, 223, sign);
}

int from_big_decimal_to_decimal(Big_decimal src, Decimal *dst) {
  int res_code = 9;
  init_decimal(dst);
  if (check_highest_bits(src)) {
    res_code = normalizer(&src);
  } else {
    res_code = 0;
  }
  dst->bits[0] = src.bits[0];
  dst->bits[1] = src.bits[1];
  dst->bits[2] = src.bits[2];
  dst->bits[3] = src.bits[6];
  return res_code;
}

int get_big_sign(Big_decimal src) { return ((src.bits[6] >> 31) & 1); }

int get_big_scale(Big_decimal src) {
  return ((src.bits[6] & 0xFF0000) >> 16);
}

int get_big_bit(Big_decimal src, int position) {
  int a = 0;
  int shift_num = 0;
  if (position >= 0 && position <= 31) {
    a = src.bits[0];
    shift_num = position;
  } else if (position >= 32 && position <= 63) {
    a = src.bits[1];
    shift_num = position - 32;
  } else if (position >= 64 && position <= 95) {
    a = src.bits[2];
    shift_num = position - 64;
  } else if (position >= 96 && position <= 127) {
    a = src.bits[3];
    shift_num = position - 96;
  } else if (position >= 128 && position <= 159) {
    a = src.bits[4];
    shift_num = position - 128;
  } else if (position >= 160 && position <= 191) {
    a = src.bits[5];
    shift_num = position - 160;
  } else {
    a = src.bits[6];
    shift_num = position - 192;
  }

  int res = (a >> shift_num) & 1;
  return res;
}

int check_highest_bits(Big_decimal value) {
  char check = 0;
  if (value.bits[3] != 0) check = 1;
  if (value.bits[4] != 0) check = 1;
  if (value.bits[5] != 0) check = 1;
  return check;
}

int normalizer(Big_decimal *value) {
  char res_code = 0;
  if (check_highest_bits(*value)) {
    int scale = get_big_scale(*value);
    while (check_highest_bits(*value) && scale) {
      big_to_ten_div(value);
      set_big_scale(value, --scale);
    }
    res_code = check_highest_bits(*value);
  }
  return res_code;
}

////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ СЛОЖЕНИЯ====  ////
//  ====REGULAR Decimal====  //
int to_ten(Decimal *value) {
  int res_code = 9;
  int sign = get_sign(*value);
  int scale = get_scale(*value);
  Big_decimal b_value;
  Big_decimal result;
  init_big_decimal(&result);
  init_big_decimal(&b_value);
  from_decimal_to_big_decimal(*value, &b_value);
  int b = 10;
  while (b != 0) {
    if (b & 0x1 == 0x1) {
      big_add_to_self(b_value, &result);
    }
    b >>= 1;
    big_left_shift(&b_value);
  }
  res_code = from_big_decimal_to_decimal(result, value);
  if (res_code == 0) {
    set_bit(value, 127, sign);
    set_scale(value, ++scale);
  }
  return res_code;
}

int equal_scales(Decimal *value_1, Decimal *value_2) {
  int res_code = 9;
  int scale_1 = get_scale(*value_1);
  int scale_2 = get_scale(*value_2);
  if (scale_1 < scale_2) {
    while (scale_1 != scale_2) {
      res_code = to_ten(value_1);
      scale_1 = get_scale(*value_1);
      if (res_code != 0) {
        break;
      }
    }
  } else if (scale_2 < scale_1) {
    while (scale_1 != scale_2) {
      res_code = to_ten(value_2);
      scale_2 = get_scale(*value_2);
      if (res_code != 0) {
        break;
      }
    }
  }
  return res_code;
}

//  ====BIG Decimal====  //
void big_add_to_self(Big_decimal value, Big_decimal *result) {
  Big_decimal secunda;
  secunda.bits[6] = result->bits[6];
  secunda.bits[5] = result->bits[5];
  secunda.bits[4] = result->bits[4];
  secunda.bits[3] = result->bits[3];
  secunda.bits[2] = result->bits[2];
  secunda.bits[1] = result->bits[1];
  secunda.bits[0] = result->bits[0];
  init_big_decimal(result);
  result->bits[6] = value.bits[6];
  int x = 0;
  int y = 0;
  char mark = 0;
  for (size_t i = 0; i < 192; i++) {
    x = get_big_bit(value, i);
    y = get_big_bit(secunda, i);
    if (x && y) {
      if (mark) {
        set_big_bit(result, i, 1);
      } else {
        mark = 1;
      }
    } else if (x || y) {
      if (!mark) {
        set_big_bit(result, i, 1);
      }
    } else {
      if (mark) {
        set_big_bit(result, i, 1);
        mark = 0;
      }
    }
  }
}

void big_left_shift(Big_decimal *b_value) {
  int thirty_one = get_big_bit(*b_value, 31);
  int sixty_three = get_big_bit(*b_value, 63);
  int ninety_five = get_big_bit(*b_value, 95);
  int one_two_seven = get_big_bit(*b_value, 127);
  int one_five_nine = get_big_bit(*b_value, 159);
  b_value->bits[5] <<= 1;
  b_value->bits[4] <<= 1;
  b_value->bits[3] <<= 1;
  b_value->bits[2] <<= 1;
  b_value->bits[1] <<= 1;
  b_value->bits[0] <<= 1;
  if (thirty_one) set_big_bit(b_value, 32, 1);
  if (sixty_three) set_big_bit(b_value, 64, 1);
  if (ninety_five) set_big_bit(b_value, 96, 1);
  if (one_two_seven) set_big_bit(b_value, 128, 1);
  if (one_five_nine) set_big_bit(b_value, 160, 1);
}

int big_to_ten(Big_decimal *value) {
  int sign = get_big_sign(*value);
  int scale = get_big_scale(*value);
  Big_decimal result;
  init_big_decimal(&result);
  int b = 10;
  while (b != 0) {
    if (b & 0x1 == 0x1) {
      big_add_to_self(*value, &result);
    }
    b >>= 1;
    big_left_shift(value);
  }
  *value = result;
  set_big_bit(value, 223, sign);
  set_big_scale(value, ++scale);
  return 0;
}

int equal_big_scales(Big_decimal *value_1, Big_decimal *value_2) {
  int res_code = 9;
  int scale_1 = get_big_scale(*value_1);
  int scale_2 = get_big_scale(*value_2);
  if (scale_1 < scale_2) {
    while (scale_1 != scale_2) {
      res_code = big_to_ten(value_1);
      scale_1 = get_big_scale(*value_1);
      if (res_code != 0) {
        break;
      }
    }
  } else if (scale_2 < scale_1) {
    while (scale_1 != scale_2) {
      res_code = big_to_ten(value_2);
      scale_2 = get_big_scale(*value_2);
      if (res_code != 0) {
        break;
      }
    }
  }
  return res_code;
}

void big_add(Big_decimal value_1, Big_decimal value_2,
                 Big_decimal *result) {
  init_big_decimal(result);
  int sign = get_big_sign(value_1);
  int scale = get_big_scale(value_1);
  char x = 0;
  char y = 0;
  char mark = 0;
  for (size_t i = 0; i < 192; i++) {
    x = get_big_bit(value_1, i);
    y = get_big_bit(value_2, i);
    if (x && y) {
      if (mark) {
        set_big_bit(result, i, 1);
      } else {
        mark = 1;
      }
    } else if (x || y) {
      if (!mark) {
        set_big_bit(result, i, 1);
      }
    } else {
      if (mark) {
        set_big_bit(result, i, 1);
        mark = 0;
      }
    }
  }
  set_big_bit(result, 223, sign);
  set_big_scale(result, scale);
}

////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ TRUNCATE====  ////

void char_shift_set_bit(char *dst, char bit) {
  *dst <<= 1;
  char mask = 1;
  if (bit) {
    *dst |= mask;
  } else {
    *dst &= ~mask;
  }
}

void to_ten_div(Decimal *value) {
  Decimal result;
  init_decimal(&result);
  char ten = 10;
  char temp = 0;
  char buff = 0;
  int count = 95;
  while (count != -1) {
    temp = get_bit(*value, count);
    if (temp || buff) {
      while (buff < ten && count > -1) {
        char_shift_set_bit(&buff, temp);
        temp = get_bit(*value, --count);
      }
      ++count;
      if (buff >= ten) set_bit(&result, count, 1);
      buff = buff - ten;
    }
    count--;
  }
  value->bits[2] = result.bits[2];
  value->bits[1] = result.bits[1];
  value->bits[0] = result.bits[0];
}

void big_to_ten_div(Big_decimal *value) {
  Big_decimal result;
  init_big_decimal(&result);
  char ten = 10;
  char temp = 0;
  char buff = 0;
  int count = 191;
  while (count != -1) {
    temp = get_big_bit(*value, count);
    if (temp || buff) {
      while (buff < ten && count > -1) {
        char_shift_set_bit(&buff, temp);
        temp = get_big_bit(*value, --count);
      }
      ++count;
      if (buff >= ten) set_big_bit(&result, count, 1);
      buff = buff - ten;
    }
    count--;
  }
  value->bits[5] = result.bits[5];
  value->bits[4] = result.bits[4];
  value->bits[3] = result.bits[3];
  value->bits[2] = result.bits[2];
  value->bits[1] = result.bits[1];
  value->bits[0] = result.bits[0];
}

////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ ВЫЧИТАНИЯ====  ////
char who_has_highest_bit(Decimal value_1, Decimal value_2) {
  char answear = 0;
  int a = 0;
  int b = 0;
  for (int i = 95; i > -1; i--) {
    a = get_bit(value_1, i);
    b = get_bit(value_2, i);
    if (a == b) {
      continue;
    } else if (a) {
      break;
    } else if (b) {
      answear = 1;
      break;
    }
  }
  return answear;
}

char who_big_has_highest_bit(Big_decimal value_1,
                                 Big_decimal value_2) {
  char answear = 0;
  int a = 0;
  int b = 0;
  for (int i = 191; i > -1; i--) {
    a = get_big_bit(value_1, i);
    b = get_big_bit(value_2, i);
    if (a == b) {
      continue;
    } else if (a) {
      break;
    } else if (b) {
      answear = 1;
      break;
    }
  }
  return answear;
}

////  ====ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ ФУНКЦИИ ROUND====  ////
char absolute_zero(Decimal value) {
  char res = 1;
  if (value.bits[2] > 0) res = 0;
  if (value.bits[1] > 0) res = 0;
  if (value.bits[0] > 0) res = 0;
  return res;
}

char big_absolute_zero(Big_decimal value) {
  char res = 1;
  if (value.bits[5] > 0) res = 0;
  if (value.bits[4] > 0) res = 0;
  if (value.bits[3] > 0) res = 0;
  if (value.bits[2] > 0) res = 0;
  if (value.bits[1] > 0) res = 0;
  if (value.bits[0] > 0) res = 0;
  return res;
}

////  ====ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ СРАВНЕНИЙ====  ////
int engine_comp(Decimal dec_a, Decimal dec_b) {
  int res_code = 9;
  char is_zero_a = absolute_zero(dec_a);
  char is_zero_b = absolute_zero(dec_b);
  char zero_flag = 0;
  if (is_zero_a && is_zero_b) {
    init_decimal(&dec_a);
    init_decimal(&dec_b);
    zero_flag = 1;
  }
  int sign_a = get_sign(dec_a);
  int sign_b = get_sign(dec_b);
  if (sign_a != sign_b) {
    if (sign_a) {
      res_code = 0;
    } else {
      res_code = 1;
    }
  } else if (sign_a == sign_b) {
    if (zero_flag) {
      res_code = 2;
    } else {
      Big_decimal big_dec_a;
      Big_decimal big_dec_b;
      from_decimal_to_big_decimal(dec_a, &big_dec_a);
      from_decimal_to_big_decimal(dec_b, &big_dec_b);
      int scale_a = get_big_scale(big_dec_a);
      int scale_b = get_big_scale(big_dec_b);
      if (scale_a != scale_b) {
        res_code = equal_big_scales(&big_dec_a, &big_dec_b);
      }
      if (res_code == 0 || res_code == 9) {
        res_code = 2;
        int sign = get_big_sign(big_dec_a);
        char a = 0;
        char b = 0;
        for (int i = 191; i > -1; i--) {
          a = get_big_bit(big_dec_a, i);
          b = get_big_bit(big_dec_b, i);
          if (a == b) {
            continue;
          } else if (a) {
            res_code = (sign) ? 0 : 1;
            break;
          } else if (b) {
            res_code = (sign) ? 1 : 0;
            break;
          }
        }
      }
    }
  }
  return res_code;
}

////  ======ВСПОМОГАТЕЛЬНАЯ ДЛЯ ДЕЛЕНИЯ======  ////
void big_div(Big_decimal v1, Big_decimal v2,
                 Big_decimal *result, Big_decimal *res) {
  Decimal mini_v2 = {0};
  Decimal mini_res = {0};
  init_big_decimal(res);
  init_big_decimal(result);
  from_big_decimal_to_decimal(v2, &mini_v2);
  for (int i = 191; i > -1; i--) {
    big_left_shift(res);
    if (get_big_bit(v1, i)) {
      set_big_bit(res, 0, 1);
    }
    if (lessly(v2, *res)) {
      from_big_decimal_to_decimal(*res, &mini_res);
      sub(mini_res, mini_v2, &mini_res);
      from_decimal_to_big_decimal(mini_res, res);
      set_big_bit(result, i, 1);
    }
  }
}

void mul_to_ten(Big_decimal *value) {
  Big_decimal result;
  init_big_decimal(&result);
  int b = 10;
  while (b != 0) {
    if (b & 0x1 == 0x1) {
      big_add_to_self(*value, &result);
    }
    b >>= 1;
    big_left_shift(value);
  }
  *value = result;
}

size_t dot_part(Big_decimal v1, Big_decimal v2,
                    Big_decimal *result) {
  size_t offset = 0;
  Big_decimal t_div = {0};
  Decimal small_result = {0};
  Decimal small_t_div = {0};
  while (!big_absolute_zero(v1) && offset < 29) {
    mul_to_ten(result);
    mul_to_ten(&v1);
    big_div(v1, v2, &t_div, &v1);
    from_big_decimal_to_decimal(*result, &small_result);
    from_big_decimal_to_decimal(t_div, &small_t_div);
    add(small_result, small_t_div, &small_result);
    from_decimal_to_big_decimal(small_result, result);
    from_decimal_to_big_decimal(small_t_div, &t_div);
    offset++;
  }
  return offset;
}

int lessly(Big_decimal v1, Big_decimal v2) {
  int res_code = 0;
  int a = 0;
  int b = 0;
  int i = 191;
  for (; i > -1; i--) {
    a = get_big_bit(v1, i);
    b = get_big_bit(v2, i);
    if (a && !b) {
      break;
    } else if (b && !a) {
      res_code = 1;
      break;
    }
  }
  if (i == -1) {
    res_code = 1;
  }
  return res_code;
}
