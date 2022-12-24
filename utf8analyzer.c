#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

const size_t MAX_UTF8_BYTES = 4;
const size_t MAX_UNICODE_CODEPOINT_BYTES = 3;
const size_t MAX_UNICODE_CODEPOINTS = 1114112;
const size_t MAX_UNICODE_CODEPOINTS_DECIMALS = 7;
const size_t MAX_STRING_SIZE = 1 << 16;
const size_t INVALID_UTF8 = (size_t)-1;

size_t unicode_find_char(const char* s) {
  // Check that the byte starts with "0b10"
  // If not, then it must be invalid UTF8
  #define CHECK_REMAINING_BYTE(remaining_byte) if (((uint8_t)remaining_byte) >> 6 != 2) return INVALID_UTF8;

  uint8_t c = s[0];
  if (c <= 0x7f) {
    // ASCII, first 128 code points
    return 1;
  } else if (c >= 0xc0 && c < 0xe0) {
    // 1920 code points
    CHECK_REMAINING_BYTE(s[1]);
    return 2;
  } else if (c >= 0xe0 && c < 0xf0) {
    // 61440 code points
    CHECK_REMAINING_BYTE(s[1]);
    CHECK_REMAINING_BYTE(s[2]);
    return 3;
  } else if (c >= 0xf0) {
    // 1048576 code points
    CHECK_REMAINING_BYTE(s[1]);
    CHECK_REMAINING_BYTE(s[2]);
    CHECK_REMAINING_BYTE(s[3]);
    return 4;
  } else {
    return -1;
  }
}

size_t utf8len(const char* s, size_t max) {
  size_t i = 0;
  size_t length = 0;
  while (s[i] && i < max) {
    //printf("len=%u, %u %s %" PRIx8 " %u\n", length, i, s + i, (uint8_t)*(s + i));
    size_t codepoint_width = unicode_find_char(s + i);
    if (codepoint_width == INVALID_UTF8) return 0;
    i += codepoint_width;
    length++;
  }
  return length;
}

char** utf8chars(const char* s, size_t max, size_t* _utf8length, size_t* _max_codepoint_length) {
  size_t i = 0;
  char** chars = (char**) calloc(max, sizeof((char*)0));
  size_t codepoints = 0;
  *_max_codepoint_length = 0;
  while (s[i] && i < max) {
    size_t codepoint_width = unicode_find_char(s + i);

    if (codepoint_width > *_max_codepoint_length) {
      *_max_codepoint_length = codepoint_width;
    }

    char* c;
    if (codepoint_width == INVALID_UTF8) {
      chars[codepoints] = "?";
      break;
    } else {
      c = (char*)calloc(codepoint_width + 1, 1);
      for (int j = 0; j < codepoint_width; j++) {
        c[j] = *(s + i + j);
      }
      c[codepoint_width] = '\0';
      // DEBUG
      // printf("%u %u %u %u %s\n", i, codepoints, codepoint_width, max, c);
    }
    chars[codepoints] = c;
    codepoints++;
    // DEBUG
    // printf("len=%u byte #%u byte=0x%" PRIx8 " str=%s\n", codepoints, i, (uint8_t)*(s + i), s + i);
    i += codepoint_width;
  }
  chars[codepoints] = (char*)0;
  *_utf8length = codepoints;
  return chars;
}

uint32_t utf8_to_unicode_codepoint(const char* utf8) {
  uint8_t length = strnlen(utf8, MAX_UTF8_BYTES);
  uint8_t parts[4] = {0, 0, 0, 0};

  switch (length) {
    case 1:
      // TODO if this byte is > 127, it's a UTF8 error
      parts[0] = utf8[0] & 127;
      break;
    case 2:
      parts[1] = utf8[0] & 31;
      parts[0] = utf8[1] & 63;
      break;
    case 3:
      parts[2] = utf8[0] & 15;
      parts[1] = utf8[1] & 63;
      parts[0] = utf8[2] & 63;
      break;
    case 4:
      parts[3] = utf8[0] & 7;
      parts[2] = utf8[1] & 63;
      parts[1] = utf8[2] & 63;
      parts[0] = utf8[3] & 63;
      break;
  }
  // DEBUG
  // for (int i = 0; i < 4; i++) {
  //   printf("%s %d = %d %x\n", utf8, i, parts[i], parts[i]);
  // }
  return parts[0] | (parts[1] << 6) | (parts[2] << 12) | (parts[3] << 18);
}

// TODO write unicode_codepoint_to_utf8

uint32_t utf8_as_int(const char* c) {
  // Unfortunately, all I can think of for this is iterating backwards
  // which requires knowing the length
  // OR reversing bits of an int, which seems to take as many operations
  size_t length = strnlen(c, MAX_UTF8_BYTES);
  int i = length - 1;
  uint32_t sum = 0;
  while (i >= 0) {
    sum |= (uint32_t)((uint8_t)c[i]) << (8 * (length - i - 1));
    i--;
  }
  return sum;
}

void byte_analysis(size_t i, const char* s) {
  size_t len = strnlen(s, MAX_STRING_SIZE);
  char* quote = "";
  printf("%-4d bytes %s%s\n", len, quote, s, quote);

  int j = 0;
  while (s[j]) {
    char c = s[j];
    uint8_t v = (uint8_t)c;
    printf("byte %5u:  0x%02" PRIx8 "  %3" PRIu8, j + 1, v, v);
    if (v > 0 && v < 128) {
      printf("  %c\n", c);
    } else {
      printf("  (not ascii)\n");
    }
    j++;
  }
}

bool utf8analysis(size_t i, const char* s, bool skip_ascii, bool add_python_quotes, bool also_print_decimal) {
  size_t len = strnlen(s, MAX_STRING_SIZE);
  size_t _utf8length = -1;
  size_t _max_codepoint_length = -1;
  char** chars = utf8chars(s, len, &_utf8length, &_max_codepoint_length);
  char* quote = "";

  if (add_python_quotes) {
    bool has_dquote = false;
    bool has_squote = false;
    bool has_sneaky = false;
    for (int k = 0; k < _utf8length; k++) {
      if (chars[k][0] == '"') {
        has_dquote = true;
      } else if (chars[k][0] == '\'') {
        has_squote = true;
      } else if (chars[k][0] == ' ') {
        has_sneaky = true;
      }
    }
    if (has_squote && !has_dquote) {
      quote = "\"";
    } else if (!has_squote && has_dquote) {
      quote = "'";
    } else if (has_sneaky) {
      if (has_squote || has_dquote) {
        quote = "\"\"\"";
      } else {
        quote = "'";
      }
    }
  }

  printf("%-4u unicode codepoints (%u bytes) in %s%s%s%s\n", _utf8length, len, _max_codepoint_length == 1 ? "(all ascii) " : "", quote, s, quote);

  if (skip_ascii && _max_codepoint_length == 1) {
    return false;
  }

  for (int k = 0; k < _utf8length; k++) {
    uint8_t length = strnlen(chars[k], MAX_UTF8_BYTES);
    uint32_t codepoint = utf8_to_unicode_codepoint(chars[k]);

    printf("codepoint %-3d", k + 1);

    printf(" U+%- *X", MAX_UNICODE_CODEPOINT_BYTES * 2, codepoint);

    if (also_print_decimal) {
      printf(" % *u ", MAX_UNICODE_CODEPOINTS_DECIMALS, codepoint);
    }

    printf("  utf8: ");

    printf(" 0x%- *X", MAX_UTF8_BYTES * 2, utf8_as_int(chars[k]));

    if (also_print_decimal) {
      for (uint8_t l = 0; l < length; l++) {
        printf(" % 3" PRIu8, (uint8_t)chars[k][l]);
      }

      printf("%*s", 4 * (MAX_UTF8_BYTES - length), "");

      // Max number of decimals needed for 2**32
      printf(" % *u", 1 + 10, utf8_as_int(chars[k]));
    }

    printf("  %s", chars[k]);

    printf("\n");
  }
  free(chars);

  return true;
}

bool bool_env_var(const char* envvar_name) {
  const char* envvar = getenv(envvar_name);
  return !(envvar == NULL || *envvar == (char)0);
}

int main(int argc, const char** argv) {
  if (argc > 1) {
    const bool RUN_BYTE_ANALYSIS = bool_env_var("RUN_BYTE_ANALYSIS");
    const bool RUN_UTF8_ANALYSIS = !bool_env_var("NO_RUN_UTF8_ANALYSIS");
    const bool SKIP_ASCII = !bool_env_var("NO_SKIP_ASCII");
    const bool ADD_PYTHON_QUOTES = bool_env_var("ADD_PYTHON_QUOTES");
    const bool ALSO_PRINT_DECIMAL = bool_env_var("ALSO_PRINT_DECIMAL");

    for (size_t i = 1; i < argc; i++) {
      if (RUN_BYTE_ANALYSIS) {
        byte_analysis(i, argv[i]);
        printf("\n");
      }

      if (RUN_UTF8_ANALYSIS) {
        utf8analysis(i, argv[i], SKIP_ASCII, ADD_PYTHON_QUOTES, ALSO_PRINT_DECIMAL);
        printf("\n");
      }
    }
  } else {
    printf("USAGE: %s [strings to analyze]\n", argv[0]);
    printf("    prints out UTF8 and byte information about input, and catches certain malformatted utf-8 strings\n");
    printf("\n");
    printf("  env vars:\n\n");
    printf("    RUN_BYTE_ANALYSIS - set this to run a byte-by-byte analysis\n");
    printf("    NO_RUN_UTF8_ANALYSIS - set this to not run a utf8 codepoint analysis\n");
    printf("    NO_SKIP_ASCII - set this to print utf8 analysis of ASCII characters\n");
    printf("    ADD_PYTHON_QUOTES - set this to print quotes for human readability\n");
  }
  return 0;
}
