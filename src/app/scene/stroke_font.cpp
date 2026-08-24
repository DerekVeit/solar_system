#include "app/scene/stroke_font.hpp"

#include <cstdint>

namespace solar::app {

namespace {

constexpr int8_t kPenUp = 126;
constexpr int8_t kEnd = 127;

// Stick glyphs on a 4×6 cap (x 0..4, y 0..6; descenders to -2). 126 = pen up.
// clang-format off
const int8_t kSpace[] = {kEnd};
const int8_t kBox[] = {0,0, 4,0, 4,6, 0,6, 0,0, kEnd};

const int8_t kA[] = {0,0, 0,4, 2,6, 4,4, 4,0, kPenUp, 0,3, 4,3, kEnd};
const int8_t kB[] = {0,0, 0,6, 3,6, 4,5, 4,4, 3,3, 0,3, kPenUp, 3,3, 4,2, 4,1, 3,0, 0,0, kEnd};
const int8_t kC[] = {4,5, 3,6, 1,6, 0,5, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t kD[] = {0,0, 0,6, 3,6, 4,5, 4,1, 3,0, 0,0, kEnd};
const int8_t kE[] = {4,6, 0,6, 0,0, 4,0, kPenUp, 0,3, 3,3, kEnd};
const int8_t kF[] = {4,6, 0,6, 0,0, kPenUp, 0,3, 3,3, kEnd};
const int8_t kG[] = {4,5, 3,6, 1,6, 0,5, 0,1, 1,0, 3,0, 4,1, 4,3, 2,3, kEnd};
const int8_t kH[] = {0,0, 0,6, kPenUp, 4,0, 4,6, kPenUp, 0,3, 4,3, kEnd};
const int8_t kI[] = {1,6, 3,6, kPenUp, 2,6, 2,0, kPenUp, 1,0, 3,0, kEnd};
const int8_t kJ[] = {0,1, 1,0, 3,0, 4,1, 4,6, kEnd};
const int8_t kK[] = {0,0, 0,6, kPenUp, 4,6, 0,3, 4,0, kEnd};
const int8_t kL[] = {0,6, 0,0, 4,0, kEnd};
const int8_t kM[] = {0,0, 0,6, 2,3, 4,6, 4,0, kEnd};
const int8_t kN[] = {0,0, 0,6, 4,0, 4,6, kEnd};
const int8_t kO[] = {1,0, 3,0, 4,1, 4,5, 3,6, 1,6, 0,5, 0,1, 1,0, kEnd};
const int8_t kP[] = {0,0, 0,6, 3,6, 4,5, 4,4, 3,3, 0,3, kEnd};
const int8_t kQ[] = {1,0, 3,0, 4,1, 4,5, 3,6, 1,6, 0,5, 0,1, 1,0, kPenUp, 2,2, 4,-1, kEnd};
const int8_t kR[] = {0,0, 0,6, 3,6, 4,5, 4,4, 3,3, 0,3, kPenUp, 2,3, 4,0, kEnd};
const int8_t kS[] = {4,5, 3,6, 1,6, 0,5, 0,4, 1,3, 3,3, 4,2, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t kT[] = {0,6, 4,6, kPenUp, 2,6, 2,0, kEnd};
const int8_t kU[] = {0,6, 0,1, 1,0, 3,0, 4,1, 4,6, kEnd};
const int8_t kV[] = {0,6, 2,0, 4,6, kEnd};
const int8_t kW[] = {0,6, 0,0, 2,3, 4,0, 4,6, kEnd};
const int8_t kX[] = {0,6, 4,0, kPenUp, 0,0, 4,6, kEnd};
const int8_t kY[] = {0,6, 2,3, 4,6, kPenUp, 2,3, 2,0, kEnd};
const int8_t kZ[] = {0,6, 4,6, 0,0, 4,0, kEnd};

const int8_t ka[] = {4,0, 4,4, kPenUp, 4,3, 3,4, 1,4, 0,3, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t kb[] = {0,0, 0,6, kPenUp, 0,3, 2,4, 3,4, 4,3, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t kc[] = {4,3, 3,4, 1,4, 0,3, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t kd[] = {4,0, 4,6, kPenUp, 4,3, 2,4, 1,4, 0,3, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t ke[] = {0,2, 4,2, 4,3, 3,4, 1,4, 0,3, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t kf[] = {1,0, 1,5, 2,6, 3,6, kPenUp, 0,3, 3,3, kEnd};
const int8_t kg[] = {4,1, 3,0, 1,0, 0,1, 0,3, 1,4, 3,4, 4,3, 4,1, 4,-1, 3,-2, 1,-2, 0,-1, kEnd};
const int8_t kh[] = {0,0, 0,6, kPenUp, 0,3, 2,4, 3,4, 4,3, 4,0, kEnd};
const int8_t ki[] = {2,0, 2,4, kPenUp, 2,5, 2,6, kEnd};
const int8_t kj[] = {1,-1, 2,-2, 3,-2, 3,4, kPenUp, 3,5, 3,6, kEnd};
const int8_t kk[] = {0,0, 0,6, kPenUp, 3,4, 0,2, kPenUp, 1,2, 4,0, kEnd};
const int8_t kl[] = {1,6, 2,6, 2,0, 3,0, kEnd};
const int8_t km[] = {0,0, 0,4, kPenUp, 0,3, 1,4, 2,3, 2,0, kPenUp, 2,3, 3,4, 4,3, 4,0, kEnd};
const int8_t kn[] = {0,0, 0,4, kPenUp, 0,3, 2,4, 3,4, 4,3, 4,0, kEnd};
const int8_t ko[] = {1,0, 3,0, 4,1, 4,3, 3,4, 1,4, 0,3, 0,1, 1,0, kEnd};
const int8_t kp[] = {0,-2, 0,4, kPenUp, 0,3, 2,4, 3,4, 4,3, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t kq[] = {4,-2, 4,4, kPenUp, 4,3, 2,4, 1,4, 0,3, 0,1, 1,0, 3,0, 4,1, kEnd};
const int8_t kr[] = {0,0, 0,4, kPenUp, 0,3, 2,4, 3,4, 4,3, kEnd};
const int8_t ks[] = {4,3, 3,4, 1,4, 0,3, 1,2, 3,2, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t kt[] = {1,6, 1,1, 2,0, 3,0, kPenUp, 0,4, 3,4, kEnd};
const int8_t ku[] = {0,4, 0,1, 1,0, 3,0, 4,1, 4,4, kEnd};
const int8_t kv[] = {0,4, 2,0, 4,4, kEnd};
const int8_t kw[] = {0,4, 1,0, 2,2, 3,0, 4,4, kEnd};
const int8_t kx[] = {0,4, 4,0, kPenUp, 0,0, 4,4, kEnd};
const int8_t ky[] = {0,4, 2,1, 4,4, kPenUp, 2,1, 1,-2, kEnd};
const int8_t kz[] = {0,4, 4,4, 0,0, 4,0, kEnd};

const int8_t k0[] = {1,0, 3,0, 4,1, 4,5, 3,6, 1,6, 0,5, 0,1, 1,0, kPenUp, 0,1, 4,5, kEnd};
const int8_t k1[] = {1,5, 2,6, 2,0, kPenUp, 1,0, 3,0, kEnd};
const int8_t k2[] = {0,5, 1,6, 3,6, 4,5, 4,4, 0,0, 4,0, kEnd};
const int8_t k3[] = {0,5, 1,6, 3,6, 4,5, 4,4, 3,3, 1,3, kPenUp, 3,3, 4,2, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t k4[] = {3,0, 3,6, kPenUp, 0,2, 4,2, kPenUp, 0,2, 3,6, kEnd};
const int8_t k5[] = {4,6, 0,6, 0,3, 3,3, 4,2, 4,1, 3,0, 1,0, 0,1, kEnd};
const int8_t k6[] = {3,6, 1,6, 0,5, 0,1, 1,0, 3,0, 4,1, 4,2, 3,3, 0,3, kEnd};
const int8_t k7[] = {0,6, 4,6, 1,0, kEnd};
const int8_t k8[] = {1,3, 0,4, 0,5, 1,6, 3,6, 4,5, 4,4, 3,3, 1,3, 0,2, 0,1, 1,0, 3,0, 4,1, 4,2, 3,3, kEnd};
const int8_t k9[] = {1,0, 3,0, 4,1, 4,5, 3,6, 1,6, 0,5, 0,4, 1,3, 4,3, kEnd};

const int8_t kHyphen[] = {1,3, 3,3, kEnd};
const int8_t kPeriod[] = {2,0, 2,1, kEnd};
const int8_t kComma[] = {2,1, 1,-1, kEnd};
const int8_t kApostrophe[] = {2,5, 2,6, kEnd};
const int8_t kColon[] = {2,1, 2,2, kPenUp, 2,4, 2,5, kEnd};
// clang-format on

const int8_t* glyph_strokes(char ch) {
    switch (ch) {
        case ' ':
            return kSpace;
        case 'A':
            return kA;
        case 'B':
            return kB;
        case 'C':
            return kC;
        case 'D':
            return kD;
        case 'E':
            return kE;
        case 'F':
            return kF;
        case 'G':
            return kG;
        case 'H':
            return kH;
        case 'I':
            return kI;
        case 'J':
            return kJ;
        case 'K':
            return kK;
        case 'L':
            return kL;
        case 'M':
            return kM;
        case 'N':
            return kN;
        case 'O':
            return kO;
        case 'P':
            return kP;
        case 'Q':
            return kQ;
        case 'R':
            return kR;
        case 'S':
            return kS;
        case 'T':
            return kT;
        case 'U':
            return kU;
        case 'V':
            return kV;
        case 'W':
            return kW;
        case 'X':
            return kX;
        case 'Y':
            return kY;
        case 'Z':
            return kZ;
        case 'a':
            return ka;
        case 'b':
            return kb;
        case 'c':
            return kc;
        case 'd':
            return kd;
        case 'e':
            return ke;
        case 'f':
            return kf;
        case 'g':
            return kg;
        case 'h':
            return kh;
        case 'i':
            return ki;
        case 'j':
            return kj;
        case 'k':
            return kk;
        case 'l':
            return kl;
        case 'm':
            return km;
        case 'n':
            return kn;
        case 'o':
            return ko;
        case 'p':
            return kp;
        case 'q':
            return kq;
        case 'r':
            return kr;
        case 's':
            return ks;
        case 't':
            return kt;
        case 'u':
            return ku;
        case 'v':
            return kv;
        case 'w':
            return kw;
        case 'x':
            return kx;
        case 'y':
            return ky;
        case 'z':
            return kz;
        case '0':
            return k0;
        case '1':
            return k1;
        case '2':
            return k2;
        case '3':
            return k3;
        case '4':
            return k4;
        case '5':
            return k5;
        case '6':
            return k6;
        case '7':
            return k7;
        case '8':
            return k8;
        case '9':
            return k9;
        case '-':
            return kHyphen;
        case '.':
            return kPeriod;
        case ',':
            return kComma;
        case '\'':
            return kApostrophe;
        case ':':
            return kColon;
        default:
            return kBox;
    }
}

template <typename Fn>
void for_each_segment(char ch, Fn&& fn) {
    const int8_t* p = glyph_strokes(ch);
    bool has = false;
    double last_x = 0.0;
    double last_y = 0.0;
    while (*p != kEnd) {
        if (*p == kPenUp) {
            has = false;
            ++p;
            continue;
        }
        const double x = *p++;
        const double y = *p++;
        if (has) {
            fn(last_x, last_y, x, y);
        }
        last_x = x;
        last_y = y;
        has = true;
    }
}

} // namespace

std::size_t stroke_text_line_vertex_count(std::string_view text) {
    std::size_t count = 0;
    for (const char ch : text) {
        for_each_segment(ch, [&](double, double, double, double) { count += 2; });
    }
    return count;
}

void append_stroke_text(std::string_view text, const glm::dvec3& origin, const glm::dvec3& axis_x,
                        const glm::dvec3& axis_y, std::vector<glm::dvec3>& endpoints) {
    double cursor = 0.0;
    for (const char ch : text) {
        for_each_segment(ch, [&](double x0, double y0, double x1, double y1) {
            endpoints.push_back(origin + (cursor + x0) * axis_x + y0 * axis_y);
            endpoints.push_back(origin + (cursor + x1) * axis_x + y1 * axis_y);
        });
        cursor += kStrokeFontAdvance;
    }
}

} // namespace solar::app
