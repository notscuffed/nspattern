# nspattern

Header only c++20 constexpr pattern matching library.

### Usage:
```cpp
void test()
{
    // data can be array or any contiguous container that implements .data() & .size()
    const unsigned char data[] = { 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x00 };

    // non static pattern will be inlined as instructions loading the pattern on stack
    // add static keyword or initialize outside method to put it on .rdata section
    constexpr auto pattern = COMPILE_PATTERN("01 02 ?? 04");

    auto offset = ns::find_pattern(pattern, data);
    if (offset != ns::no_match)
        printf("pattern found at index: %zi\n", offset);
    else
        puts("pattern not found");
}
```
