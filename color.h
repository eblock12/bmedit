#ifndef H_COLOR
#define H_COLOR

class Color
{
public:
    Color() : r(255), g(255), b(255), a(255) {}
    Color(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a = 255) : r(_r), g(_g), b(_b), a(_a) {}
    unsigned char r, g, b, a;
};

#endif