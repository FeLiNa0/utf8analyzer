#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

const size_t MAX_STRING_SIZE = 1 << 16;
const size_t INVALID_UTF8 = (size_t)-1;

size_t unicode_find_char(char* s) {
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

size_t utf8len(char* s, size_t max) {
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

char** utf8chars(char* s, size_t max, size_t* _utf8length, size_t* _max_codepoint_length) {
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

void byte_analysis(size_t i, char* s) {
  size_t len = strnlen(s, MAX_STRING_SIZE);
  char* quote = "";
  printf("%-4d bytes in #%-4d %*s%s\n", len, i, 5, quote, s, quote);

  int j = 0;
  while (s[j]) {
    char c = s[j];
    uint8_t v = (uint8_t)c;
    printf("byte %5u:  %3" PRIu8 " (0x%02" PRIx8 ")", j + 1, v, v);
    if (v > 0 && v < 128) {
      printf("  %c\n", c);
    } else {
      printf("  (not ascii)\n");
    }
    j++;
  }
}

bool utf8analysis(size_t i, char* s, bool skip_ascii, bool add_python_quotes) {
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

  printf("%-4u utf8 codepoints in #%-4u %s%*s%s%s\n", _utf8length, i, _max_codepoint_length == 1 ? "(all ascii)" : "", (_max_codepoint_length) * 11 - 8, quote, s, quote);

  if (skip_ascii && _max_codepoint_length == 1) {
    return false;
  }

  for (int k = 0; k < _utf8length; k++) {
    printf("utf8 %5d:", k + 1);
    int l = 0;
    while (chars[k][l]) {
      uint8_t v = (uint8_t)chars[k][l];
      printf("  %3" PRIu8 " (0x%02" PRIx8 ")", v, v);
      l++;
    }
    printf("%*s\n", (_max_codepoint_length - l + 1) * 11, chars[k]);
  }
  free(chars);

  return true;
}

int main(int argc, char** argv) {
  if (argc > 1) {
    for (size_t i = 1; i < argc; i++) {
      byte_analysis(i, argv[i]);
      printf("\n");

      utf8analysis(i, argv[i], true, false);
      printf("\n");
    }
  } else {
    printf("USAGE: %s [strings to analyze]\n", argv[0]);
    printf("    prints out each utf-8 codepoint, related byte sequences, and catches certain malformatted utf-8 strings\n");
  }
  return 0;
}
