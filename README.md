# utf8analyzer

Run this on some command line arguments to print out each UTF-8 codepoint, as well as the byte sequences related to the UTF-8 string.

This program can detect some types of malformatted UTF-8 strings.

## Compiling

```
utf8analyzer test strings 🐠аaсдñ²不 012345
```

## Compiling

```
cc utf8analyzer.c -o utf8analyzer
```

## Compiling and testing

```
cc utf8analyzer.c -o utf8analyzer && ./utf8analyzer ...
```
