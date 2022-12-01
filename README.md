# utf8analyzer

Run this on some command line arguments to print out each UTF-8 codepoint, as well as the byte sequences related to the UTF-8 string.

This program can detect some types of malformatted UTF-8 strings.

## Example usage

```
$ ./utf8analyzer
USAGE: ./utf8analyzer [strings to analyze]
    prints out each utf-8 codepoint, related byte sequences, and catches certain malformatted utf-8 strings

$ ./utf8analyzer test strings 🐠аaсдñ²不 012345
4    bytes in #1         test
byte     1:  116 (0x74)  t
byte     2:  101 (0x65)  e
byte     3:  115 (0x73)  s
byte     4:  116 (0x74)  t

4    utf8 codepoints in #1    (all ascii)   test

7    bytes in #2         strings
byte     1:  115 (0x73)  s
byte     2:  116 (0x74)  t
byte     3:  114 (0x72)  r
byte     4:  105 (0x69)  i
byte     5:  110 (0x6e)  n
byte     6:  103 (0x67)  g
byte     7:  115 (0x73)  s

7    utf8 codepoints in #2    (all ascii)   strings

18   bytes in #3         🐠аaсдñ²不
byte     1:  240 (0xf0)  (not ascii)
byte     2:  159 (0x9f)  (not ascii)
byte     3:  144 (0x90)  (not ascii)
byte     4:  160 (0xa0)  (not ascii)
byte     5:  208 (0xd0)  (not ascii)
byte     6:  176 (0xb0)  (not ascii)
byte     7:   97 (0x61)  a
byte     8:  209 (0xd1)  (not ascii)
byte     9:  129 (0x81)  (not ascii)
byte    10:  208 (0xd0)  (not ascii)
byte    11:  180 (0xb4)  (not ascii)
byte    12:  195 (0xc3)  (not ascii)
byte    13:  177 (0xb1)  (not ascii)
byte    14:  194 (0xc2)  (not ascii)
byte    15:  178 (0xb2)  (not ascii)
byte    16:  228 (0xe4)  (not ascii)
byte    17:  184 (0xb8)  (not ascii)
byte    18:  141 (0x8d)  (not ascii)

8    utf8 codepoints in #3                                        🐠аaсдñ²不
utf8     1:  240 (0xf0)  159 (0x9f)  144 (0x90)  160 (0xa0)       🐠
utf8     2:  208 (0xd0)  176 (0xb0)                               а
utf8     3:   97 (0x61)                                           a
utf8     4:  209 (0xd1)  129 (0x81)                               с
utf8     5:  208 (0xd0)  180 (0xb4)                               д
utf8     6:  195 (0xc3)  177 (0xb1)                               ñ
utf8     7:  194 (0xc2)  178 (0xb2)                               ²
utf8     8:  228 (0xe4)  184 (0xb8)  141 (0x8d)                   不

6    bytes in #4         012345
byte     1:   48 (0x30)  0
byte     2:   49 (0x31)  1
byte     3:   50 (0x32)  2
byte     4:   51 (0x33)  3
byte     5:   52 (0x34)  4
byte     6:   53 (0x35)  5

6    utf8 codepoints in #4    (all ascii)   012345
```

## Compiling

```
cc utf8analyzer.c -o utf8analyzer
```

## Compiling and testing

```
cc utf8analyzer.c -o utf8analyzer && ./utf8analyzer ...
```
