
#line 1 "Scanner.cpp.rl"

#line 56 "Scanner.cpp.rl"


#include "Scanner.hpp"
#include "MessageTypes.hpp"
#include <fmt/ostream.h>
#include <synergy/service/ServiceLogs.h>

#define PROC(TYPE)                                                             \
    {                                                                          \
        TYPE##Message msg;                                                     \
        msg.read_from (ts, data, te);                                          \
        if (!handle (msg)) {                                                   \
            routerLog ()->debug (                                              \
                "Handler failed to process core {} message: {}",               \
                ((flow == Flow::STC) ? "server" : "client"),                   \
                msg);                                                          \
            return false;                                                      \
        }                                                                      \
    }

namespace synergy {
namespace protocol {
namespace v1 {


#line 30 "Scanner.cpp"
static const int synergy_proto_v1_start       = 168;
static const int synergy_proto_v1_first_final = 168;
static const int synergy_proto_v1_error       = 0;

static const int synergy_proto_v1_en_message = 168;


#line 80 "Scanner.cpp.rl"

bool
process (Flow const flow, Handler& handle, char const* const msg,
         std::size_t const size) {
    char const* p         = msg;
    char const* const pe  = msg + size;
    char const* const eof = pe;
    int cs;
    int act;
    char const* ts;
    char const* te;


#line 56 "Scanner.cpp"
    {
        cs  = synergy_proto_v1_start;
        ts  = 0;
        te  = 0;
        act = 0;
    }

#line 97 "Scanner.cpp.rl"

    std::size_t dlen  = 0;
    std::size_t dsize = 0;
    char const* data  = nullptr;


#line 71 "Scanner.cpp"
    {
        short _widec;
        if (p == pe)
            goto _test_eof;
        switch (cs) {
        tr19 :
#line 32 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (ClipboardGrab) }
        }
            goto st168;
        tr33 :
#line 30 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (Enter) }
        }
            goto st168;
        tr42 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 33 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (Screensaver) }
            }
            goto st168;
        tr92 :
#line 46 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (Info) }
        }
            goto st168;
        tr102 :
#line 37 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (KeyDown) }
        }
            goto st168;
        tr111 :
#line 38 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (KeyRepeat) }
        }
            goto st168;
        tr118 :
#line 39 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (KeyUp) }
        }
            goto st168;
        tr125 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 40 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (MouseDown) }
            }
            goto st168;
        tr130 :
#line 42 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (MouseMove) }
        }
            goto st168;
        tr135 :
#line 43 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (MouseRelMove) }
        }
            goto st168;
        tr137 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 41 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (MouseUp) }
            }
            goto st168;
        tr142 :
#line 44 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (MouseWheelMove) }
        }
            goto st168;
        tr161 :
#line 51 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (Incompatible) }
        }
            goto st168;
        tr190 :
#line 26 "Scanner.cpp.rl"
        {
            te = p + 1;
            { PROC (Hello) }
        }
            goto st168;
        tr197 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 36 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (KeepAlive) }
            }
            goto st168;
        tr198 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 29 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Close) }
            }
            goto st168;
        tr199 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 35 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (InfoAck) }
            }
            goto st168;
        tr200 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 28 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Noop) }
            }
            goto st168;
        tr201 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 31 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Leave) }
            }
            goto st168;
        tr202 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 34 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (ResetOptions) }
            }
            goto st168;
        tr203 :
#line 45 "Scanner.cpp.rl"
        {
            te = p;
            p--;
            { PROC (ClipboardData) }
        }
            goto st168;
        tr204 :
#line 12 "Scanner.cpp.rl"
        {
            p--;
            p += (dlen * dsize);
        }
#line 45 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (ClipboardData) }
            }
            goto st168;
        tr205 :
#line 49 "Scanner.cpp.rl"
        {
            te = p;
            p--;
            { PROC (DragInfo) }
        }
            goto st168;
        tr206 :
#line 12 "Scanner.cpp.rl"
        {
            p--;
            p += (dlen * dsize);
        }
#line 49 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (DragInfo) }
            }
            goto st168;
        tr207 :
#line 48 "Scanner.cpp.rl"
        {
            te = p;
            p--;
            { PROC (FileTransfer) }
        }
            goto st168;
        tr208 :
#line 12 "Scanner.cpp.rl"
        {
            p--;
            p += (dlen * dsize);
        }
#line 48 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (FileTransfer) }
            }
            goto st168;
        tr209 :
#line 47 "Scanner.cpp.rl"
        {
            te = p;
            p--;
            { PROC (SetOptions) }
        }
            goto st168;
        tr210 :
#line 12 "Scanner.cpp.rl"
        {
            p--;
            p += (dlen * dsize);
        }
#line 47 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (SetOptions) }
            }
            goto st168;
        tr211 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 54 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Bad) }
            }
            goto st168;
        tr212 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 52 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Busy) }
            }
            goto st168;
        tr213 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 53 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (Unknown) }
            }
            goto st168;
        tr214 :
#line 6 "Scanner.cpp.rl"
        {
            data = p;
        }
#line 50 "Scanner.cpp.rl"
            {
                te = p;
                p--;
                { PROC (InfoQuery) }
            }
            goto st168;
        tr215 :
#line 27 "Scanner.cpp.rl"
        {
            te = p;
            p--;
            { PROC (HelloBack) }
        }
            goto st168;
        tr216 :
#line 12 "Scanner.cpp.rl"
        {
            p--;
            p += (dlen * dsize);
        }
#line 27 "Scanner.cpp.rl"
            {
                te = p + 1;
                { PROC (HelloBack) }
            }
            goto st168;
        st168 :
#line 1 "NONE"
        {
            ts = 0;
        }
            if (++p == pe)
                goto _test_eof168;
            case 168:
#line 1 "NONE"
            {
                ts = p;
            }
#line 258 "Scanner.cpp"
                _widec = (*p);
                if (83 <= (*p) && (*p) <= 83) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                switch (_widec) {
                    case 67:
                        goto st1;
                    case 68:
                        goto st35;
                    case 69:
                        goto st129;
                    case 81:
                        goto st141;
                    case 339:
                        goto st144;
                    case 595:
                        goto st158;
                }
                goto st0;
            st0:
                cs = 0;
                goto _out;
            st1:
                if (++p == pe)
                    goto _test_eof1;
            case 1:
                switch ((*p)) {
                    case 65:
                        goto st2;
                    case 66:
                        goto st4;
                    case 67:
                        goto st6;
                    case 73:
                        goto st13;
                    case 78:
                        goto st26;
                    case 79:
                        goto st28;
                    case 82:
                        goto st30;
                    case 83:
                        goto st32;
                }
                goto st0;
            st2:
                if (++p == pe)
                    goto _test_eof2;
            case 2:
                if ((*p) == 76)
                    goto st3;
                goto st0;
            st3:
                if (++p == pe)
                    goto _test_eof3;
            case 3:
                if ((*p) == 86)
                    goto st169;
                goto st0;
            st169:
                if (++p == pe)
                    goto _test_eof169;
            case 169:
                goto tr197;
            st4:
                if (++p == pe)
                    goto _test_eof4;
            case 4:
                if ((*p) == 89)
                    goto st5;
                goto st0;
            st5:
                if (++p == pe)
                    goto _test_eof5;
            case 5:
                if ((*p) == 69)
                    goto st170;
                goto st0;
            st170:
                if (++p == pe)
                    goto _test_eof170;
            case 170:
                goto tr198;
            st6:
                if (++p == pe)
                    goto _test_eof6;
            case 6:
                if ((*p) == 76)
                    goto st7;
                goto st0;
            st7:
                if (++p == pe)
                    goto _test_eof7;
            case 7:
                if ((*p) == 80)
                    goto st8;
                goto st0;
            st8:
                if (++p == pe)
                    goto _test_eof8;
            case 8:
                goto tr15;
            tr15 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st9;
            st9:
                if (++p == pe)
                    goto _test_eof9;
            case 9:
#line 358 "Scanner.cpp"
                goto st10;
            st10:
                if (++p == pe)
                    goto _test_eof10;
            case 10:
                goto st11;
            st11:
                if (++p == pe)
                    goto _test_eof11;
            case 11:
                goto st12;
            st12:
                if (++p == pe)
                    goto _test_eof12;
            case 12:
                goto tr19;
            st13:
                if (++p == pe)
                    goto _test_eof13;
            case 13:
                switch ((*p)) {
                    case 65:
                        goto st14;
                    case 78:
                        goto st15;
                }
                goto st0;
            st14:
                if (++p == pe)
                    goto _test_eof14;
            case 14:
                if ((*p) == 75)
                    goto st171;
                goto st0;
            st171:
                if (++p == pe)
                    goto _test_eof171;
            case 171:
                goto tr199;
            st15:
                if (++p == pe)
                    goto _test_eof15;
            case 15:
                if ((*p) == 78)
                    goto st16;
                goto st0;
            st16:
                if (++p == pe)
                    goto _test_eof16;
            case 16:
                goto tr24;
            tr24 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st17;
            st17:
                if (++p == pe)
                    goto _test_eof17;
            case 17:
#line 416 "Scanner.cpp"
                goto st18;
            st18:
                if (++p == pe)
                    goto _test_eof18;
            case 18:
                goto st19;
            st19:
                if (++p == pe)
                    goto _test_eof19;
            case 19:
                goto st20;
            st20:
                if (++p == pe)
                    goto _test_eof20;
            case 20:
                goto st21;
            st21:
                if (++p == pe)
                    goto _test_eof21;
            case 21:
                goto st22;
            st22:
                if (++p == pe)
                    goto _test_eof22;
            case 22:
                goto st23;
            st23:
                if (++p == pe)
                    goto _test_eof23;
            case 23:
                goto st24;
            st24:
                if (++p == pe)
                    goto _test_eof24;
            case 24:
                goto st25;
            st25:
                if (++p == pe)
                    goto _test_eof25;
            case 25:
                goto tr33;
            st26:
                if (++p == pe)
                    goto _test_eof26;
            case 26:
                if ((*p) == 79)
                    goto st27;
                goto st0;
            st27:
                if (++p == pe)
                    goto _test_eof27;
            case 27:
                if ((*p) == 80)
                    goto st172;
                goto st0;
            st172:
                if (++p == pe)
                    goto _test_eof172;
            case 172:
                goto tr200;
            st28:
                if (++p == pe)
                    goto _test_eof28;
            case 28:
                if ((*p) == 85)
                    goto st29;
                goto st0;
            st29:
                if (++p == pe)
                    goto _test_eof29;
            case 29:
                if ((*p) == 84)
                    goto st173;
                goto st0;
            st173:
                if (++p == pe)
                    goto _test_eof173;
            case 173:
                goto tr201;
            st30:
                if (++p == pe)
                    goto _test_eof30;
            case 30:
                if ((*p) == 79)
                    goto st31;
                goto st0;
            st31:
                if (++p == pe)
                    goto _test_eof31;
            case 31:
                if ((*p) == 80)
                    goto st174;
                goto st0;
            st174:
                if (++p == pe)
                    goto _test_eof174;
            case 174:
                goto tr202;
            st32:
                if (++p == pe)
                    goto _test_eof32;
            case 32:
                if ((*p) == 69)
                    goto st33;
                goto st0;
            st33:
                if (++p == pe)
                    goto _test_eof33;
            case 33:
                if ((*p) == 67)
                    goto st34;
                goto st0;
            st34:
                if (++p == pe)
                    goto _test_eof34;
            case 34:
                goto tr42;
            st35:
                if (++p == pe)
                    goto _test_eof35;
            case 35:
                switch ((*p)) {
                    case 67:
                        goto st36;
                    case 68:
                        goto st48;
                    case 70:
                        goto st56;
                    case 73:
                        goto st63;
                    case 75:
                        goto st79;
                    case 77:
                        goto st103;
                    case 83:
                        goto st123;
                }
                goto st0;
            st36:
                if (++p == pe)
                    goto _test_eof36;
            case 36:
                if ((*p) == 76)
                    goto st37;
                goto st0;
            st37:
                if (++p == pe)
                    goto _test_eof37;
            case 37:
                if ((*p) == 80)
                    goto st38;
                goto st0;
            st38:
                if (++p == pe)
                    goto _test_eof38;
            case 38:
                goto tr52;
            tr52 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st39;
            st39:
                if (++p == pe)
                    goto _test_eof39;
            case 39:
#line 575 "Scanner.cpp"
                goto st40;
            st40:
                if (++p == pe)
                    goto _test_eof40;
            case 40:
                goto st41;
            st41:
                if (++p == pe)
                    goto _test_eof41;
            case 41:
                goto st42;
            st42:
                if (++p == pe)
                    goto _test_eof42;
            case 42:
                goto st43;
            st43:
                if (++p == pe)
                    goto _test_eof43;
            case 43:
                goto st44;
            st44:
                if (++p == pe)
                    goto _test_eof44;
            case 44:
                goto tr58;
            tr58 :
#line 22 "Scanner.cpp.rl"
            {
                dsize = 1;
            }
#line 7 "Scanner.cpp.rl"
                { dlen = (*p); }
                goto st45;
            st45:
                if (++p == pe)
                    goto _test_eof45;
            case 45:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 616 "Scanner.cpp"
                goto st46;
            st46:
                if (++p == pe)
                    goto _test_eof46;
            case 46:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 626 "Scanner.cpp"
                goto st47;
            st47:
                if (++p == pe)
                    goto _test_eof47;
            case 47:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 636 "Scanner.cpp"
                goto st175;
            st175:
                if (++p == pe)
                    goto _test_eof175;
            case 175:
                _widec = (*p);
                _widec = (short) (640 + ((*p) - -128));
                if (
#line 11 "Scanner.cpp.rl"
                    (dlen > 0) && ((dlen * dsize) <= (pe - p)))
                    _widec += 256;
                if (896 <= _widec && _widec <= 1151)
                    goto tr204;
                goto tr203;
            st48:
                if (++p == pe)
                    goto _test_eof48;
            case 48:
                if ((*p) == 82)
                    goto st49;
                goto st0;
            st49:
                if (++p == pe)
                    goto _test_eof49;
            case 49:
                if ((*p) == 71)
                    goto st50;
                goto st0;
            st50:
                if (++p == pe)
                    goto _test_eof50;
            case 50:
                goto tr64;
            tr64 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st51;
            st51:
                if (++p == pe)
                    goto _test_eof51;
            case 51:
#line 677 "Scanner.cpp"
                goto st52;
            st52:
                if (++p == pe)
                    goto _test_eof52;
            case 52:
                goto tr66;
            tr66 :
#line 22 "Scanner.cpp.rl"
            {
                dsize = 1;
            }
#line 7 "Scanner.cpp.rl"
                { dlen = (*p); }
                goto st53;
            st53:
                if (++p == pe)
                    goto _test_eof53;
            case 53:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 698 "Scanner.cpp"
                goto st54;
            st54:
                if (++p == pe)
                    goto _test_eof54;
            case 54:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 708 "Scanner.cpp"
                goto st55;
            st55:
                if (++p == pe)
                    goto _test_eof55;
            case 55:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 718 "Scanner.cpp"
                goto st176;
            st176:
                if (++p == pe)
                    goto _test_eof176;
            case 176:
                _widec = (*p);
                _widec = (short) (640 + ((*p) - -128));
                if (
#line 11 "Scanner.cpp.rl"
                    (dlen > 0) && ((dlen * dsize) <= (pe - p)))
                    _widec += 256;
                if (896 <= _widec && _widec <= 1151)
                    goto tr206;
                goto tr205;
            st56:
                if (++p == pe)
                    goto _test_eof56;
            case 56:
                if ((*p) == 84)
                    goto st57;
                goto st0;
            st57:
                if (++p == pe)
                    goto _test_eof57;
            case 57:
                if ((*p) == 82)
                    goto st58;
                goto st0;
            st58:
                if (++p == pe)
                    goto _test_eof58;
            case 58:
                goto tr72;
            tr72 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st59;
            st59:
                if (++p == pe)
                    goto _test_eof59;
            case 59:
#line 759 "Scanner.cpp"
                goto tr73;
            tr73 :
#line 22 "Scanner.cpp.rl"
            {
                dsize = 1;
            }
#line 7 "Scanner.cpp.rl"
                { dlen = (*p); }
                goto st60;
            st60:
                if (++p == pe)
                    goto _test_eof60;
            case 60:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 775 "Scanner.cpp"
                goto st61;
            st61:
                if (++p == pe)
                    goto _test_eof61;
            case 61:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 785 "Scanner.cpp"
                goto st62;
            st62:
                if (++p == pe)
                    goto _test_eof62;
            case 62:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 795 "Scanner.cpp"
                goto st177;
            st177:
                if (++p == pe)
                    goto _test_eof177;
            case 177:
                _widec = (*p);
                _widec = (short) (640 + ((*p) - -128));
                if (
#line 11 "Scanner.cpp.rl"
                    (dlen > 0) && ((dlen * dsize) <= (pe - p)))
                    _widec += 256;
                if (896 <= _widec && _widec <= 1151)
                    goto tr208;
                goto tr207;
            st63:
                if (++p == pe)
                    goto _test_eof63;
            case 63:
                if ((*p) == 78)
                    goto st64;
                goto st0;
            st64:
                if (++p == pe)
                    goto _test_eof64;
            case 64:
                if ((*p) == 70)
                    goto st65;
                goto st0;
            st65:
                if (++p == pe)
                    goto _test_eof65;
            case 65:
                goto tr79;
            tr79 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st66;
            st66:
                if (++p == pe)
                    goto _test_eof66;
            case 66:
#line 836 "Scanner.cpp"
                goto st67;
            st67:
                if (++p == pe)
                    goto _test_eof67;
            case 67:
                goto st68;
            st68:
                if (++p == pe)
                    goto _test_eof68;
            case 68:
                goto st69;
            st69:
                if (++p == pe)
                    goto _test_eof69;
            case 69:
                goto st70;
            st70:
                if (++p == pe)
                    goto _test_eof70;
            case 70:
                goto st71;
            st71:
                if (++p == pe)
                    goto _test_eof71;
            case 71:
                goto st72;
            st72:
                if (++p == pe)
                    goto _test_eof72;
            case 72:
                goto st73;
            st73:
                if (++p == pe)
                    goto _test_eof73;
            case 73:
                goto st74;
            st74:
                if (++p == pe)
                    goto _test_eof74;
            case 74:
                goto st75;
            st75:
                if (++p == pe)
                    goto _test_eof75;
            case 75:
                goto st76;
            st76:
                if (++p == pe)
                    goto _test_eof76;
            case 76:
                goto st77;
            st77:
                if (++p == pe)
                    goto _test_eof77;
            case 77:
                goto st78;
            st78:
                if (++p == pe)
                    goto _test_eof78;
            case 78:
                goto tr92;
            st79:
                if (++p == pe)
                    goto _test_eof79;
            case 79:
                switch ((*p)) {
                    case 68:
                        goto st80;
                    case 82:
                        goto st87;
                    case 85:
                        goto st96;
                }
                goto st0;
            st80:
                if (++p == pe)
                    goto _test_eof80;
            case 80:
                if ((*p) == 78)
                    goto st81;
                goto st0;
            st81:
                if (++p == pe)
                    goto _test_eof81;
            case 81:
                goto tr97;
            tr97 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st82;
            st82:
                if (++p == pe)
                    goto _test_eof82;
            case 82:
#line 928 "Scanner.cpp"
                goto st83;
            st83:
                if (++p == pe)
                    goto _test_eof83;
            case 83:
                goto st84;
            st84:
                if (++p == pe)
                    goto _test_eof84;
            case 84:
                goto st85;
            st85:
                if (++p == pe)
                    goto _test_eof85;
            case 85:
                goto st86;
            st86:
                if (++p == pe)
                    goto _test_eof86;
            case 86:
                goto tr102;
            st87:
                if (++p == pe)
                    goto _test_eof87;
            case 87:
                if ((*p) == 80)
                    goto st88;
                goto st0;
            st88:
                if (++p == pe)
                    goto _test_eof88;
            case 88:
                goto tr104;
            tr104 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st89;
            st89:
                if (++p == pe)
                    goto _test_eof89;
            case 89:
#line 970 "Scanner.cpp"
                goto st90;
            st90:
                if (++p == pe)
                    goto _test_eof90;
            case 90:
                goto st91;
            st91:
                if (++p == pe)
                    goto _test_eof91;
            case 91:
                goto st92;
            st92:
                if (++p == pe)
                    goto _test_eof92;
            case 92:
                goto st93;
            st93:
                if (++p == pe)
                    goto _test_eof93;
            case 93:
                goto st94;
            st94:
                if (++p == pe)
                    goto _test_eof94;
            case 94:
                goto st95;
            st95:
                if (++p == pe)
                    goto _test_eof95;
            case 95:
                goto tr111;
            st96:
                if (++p == pe)
                    goto _test_eof96;
            case 96:
                if ((*p) == 80)
                    goto st97;
                goto st0;
            st97:
                if (++p == pe)
                    goto _test_eof97;
            case 97:
                goto tr113;
            tr113 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st98;
            st98:
                if (++p == pe)
                    goto _test_eof98;
            case 98:
#line 1022 "Scanner.cpp"
                goto st99;
            st99:
                if (++p == pe)
                    goto _test_eof99;
            case 99:
                goto st100;
            st100:
                if (++p == pe)
                    goto _test_eof100;
            case 100:
                goto st101;
            st101:
                if (++p == pe)
                    goto _test_eof101;
            case 101:
                goto st102;
            st102:
                if (++p == pe)
                    goto _test_eof102;
            case 102:
                goto tr118;
            st103:
                if (++p == pe)
                    goto _test_eof103;
            case 103:
                switch ((*p)) {
                    case 68:
                        goto st104;
                    case 77:
                        goto st106;
                    case 82:
                        goto st111;
                    case 85:
                        goto st116;
                    case 87:
                        goto st118;
                }
                goto st0;
            st104:
                if (++p == pe)
                    goto _test_eof104;
            case 104:
                if ((*p) == 78)
                    goto st105;
                goto st0;
            st105:
                if (++p == pe)
                    goto _test_eof105;
            case 105:
                goto tr125;
            st106:
                if (++p == pe)
                    goto _test_eof106;
            case 106:
                if ((*p) == 86)
                    goto st107;
                goto st0;
            st107:
                if (++p == pe)
                    goto _test_eof107;
            case 107:
                goto tr127;
            tr127 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st108;
            st108:
                if (++p == pe)
                    goto _test_eof108;
            case 108:
#line 1088 "Scanner.cpp"
                goto st109;
            st109:
                if (++p == pe)
                    goto _test_eof109;
            case 109:
                goto st110;
            st110:
                if (++p == pe)
                    goto _test_eof110;
            case 110:
                goto tr130;
            st111:
                if (++p == pe)
                    goto _test_eof111;
            case 111:
                if ((*p) == 77)
                    goto st112;
                goto st0;
            st112:
                if (++p == pe)
                    goto _test_eof112;
            case 112:
                goto tr132;
            tr132 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st113;
            st113:
                if (++p == pe)
                    goto _test_eof113;
            case 113:
#line 1120 "Scanner.cpp"
                goto st114;
            st114:
                if (++p == pe)
                    goto _test_eof114;
            case 114:
                goto st115;
            st115:
                if (++p == pe)
                    goto _test_eof115;
            case 115:
                goto tr135;
            st116:
                if (++p == pe)
                    goto _test_eof116;
            case 116:
                if ((*p) == 80)
                    goto st117;
                goto st0;
            st117:
                if (++p == pe)
                    goto _test_eof117;
            case 117:
                goto tr137;
            st118:
                if (++p == pe)
                    goto _test_eof118;
            case 118:
                if ((*p) == 77)
                    goto st119;
                goto st0;
            st119:
                if (++p == pe)
                    goto _test_eof119;
            case 119:
                goto tr139;
            tr139 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st120;
            st120:
                if (++p == pe)
                    goto _test_eof120;
            case 120:
#line 1164 "Scanner.cpp"
                goto st121;
            st121:
                if (++p == pe)
                    goto _test_eof121;
            case 121:
                goto st122;
            st122:
                if (++p == pe)
                    goto _test_eof122;
            case 122:
                goto tr142;
            st123:
                if (++p == pe)
                    goto _test_eof123;
            case 123:
                if ((*p) == 79)
                    goto st124;
                goto st0;
            st124:
                if (++p == pe)
                    goto _test_eof124;
            case 124:
                if ((*p) == 80)
                    goto st125;
                goto st0;
            st125:
                if (++p == pe)
                    goto _test_eof125;
            case 125:
                goto tr145;
            tr145 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
#line 23 "Scanner.cpp.rl"
                { dsize = 4; }
#line 7 "Scanner.cpp.rl"
                { dlen = (*p); }
                goto st126;
            st126:
                if (++p == pe)
                    goto _test_eof126;
            case 126:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1211 "Scanner.cpp"
                goto st127;
            st127:
                if (++p == pe)
                    goto _test_eof127;
            case 127:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1221 "Scanner.cpp"
                goto st128;
            st128:
                if (++p == pe)
                    goto _test_eof128;
            case 128:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1231 "Scanner.cpp"
                goto st178;
            st178:
                if (++p == pe)
                    goto _test_eof178;
            case 178:
                _widec = (*p);
                _widec = (short) (640 + ((*p) - -128));
                if (
#line 11 "Scanner.cpp.rl"
                    (dlen > 0) && ((dlen * dsize) <= (pe - p)))
                    _widec += 256;
                if (896 <= _widec && _widec <= 1151)
                    goto tr210;
                goto tr209;
            st129:
                if (++p == pe)
                    goto _test_eof129;
            case 129:
                switch ((*p)) {
                    case 66:
                        goto st130;
                    case 73:
                        goto st133;
                    case 85:
                        goto st139;
                }
                goto st0;
            st130:
                if (++p == pe)
                    goto _test_eof130;
            case 130:
                switch ((*p)) {
                    case 65:
                        goto st131;
                    case 83:
                        goto st132;
                }
                goto st0;
            st131:
                if (++p == pe)
                    goto _test_eof131;
            case 131:
                if ((*p) == 68)
                    goto st179;
                goto st0;
            st179:
                if (++p == pe)
                    goto _test_eof179;
            case 179:
                goto tr211;
            st132:
                if (++p == pe)
                    goto _test_eof132;
            case 132:
                if ((*p) == 89)
                    goto st180;
                goto st0;
            st180:
                if (++p == pe)
                    goto _test_eof180;
            case 180:
                goto tr212;
            st133:
                if (++p == pe)
                    goto _test_eof133;
            case 133:
                if ((*p) == 67)
                    goto st134;
                goto st0;
            st134:
                if (++p == pe)
                    goto _test_eof134;
            case 134:
                if ((*p) == 86)
                    goto st135;
                goto st0;
            st135:
                if (++p == pe)
                    goto _test_eof135;
            case 135:
                goto tr158;
            tr158 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st136;
            st136:
                if (++p == pe)
                    goto _test_eof136;
            case 136:
#line 1315 "Scanner.cpp"
                goto st137;
            st137:
                if (++p == pe)
                    goto _test_eof137;
            case 137:
                goto st138;
            st138:
                if (++p == pe)
                    goto _test_eof138;
            case 138:
                goto tr161;
            st139:
                if (++p == pe)
                    goto _test_eof139;
            case 139:
                if ((*p) == 78)
                    goto st140;
                goto st0;
            st140:
                if (++p == pe)
                    goto _test_eof140;
            case 140:
                if ((*p) == 75)
                    goto st181;
                goto st0;
            st181:
                if (++p == pe)
                    goto _test_eof181;
            case 181:
                goto tr213;
            st141:
                if (++p == pe)
                    goto _test_eof141;
            case 141:
                if ((*p) == 73)
                    goto st142;
                goto st0;
            st142:
                if (++p == pe)
                    goto _test_eof142;
            case 142:
                if ((*p) == 78)
                    goto st143;
                goto st0;
            st143:
                if (++p == pe)
                    goto _test_eof143;
            case 143:
                if ((*p) == 70)
                    goto st182;
                goto st0;
            st182:
                if (++p == pe)
                    goto _test_eof182;
            case 182:
                goto tr214;
            st144:
                if (++p == pe)
                    goto _test_eof144;
            case 144:
                _widec = (*p);
                if (121 <= (*p) && (*p) <= 121) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 377)
                    goto st145;
                goto st0;
            st145:
                if (++p == pe)
                    goto _test_eof145;
            case 145:
                _widec = (*p);
                if (110 <= (*p) && (*p) <= 110) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 366)
                    goto st146;
                goto st0;
            st146:
                if (++p == pe)
                    goto _test_eof146;
            case 146:
                _widec = (*p);
                if (101 <= (*p) && (*p) <= 101) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 357)
                    goto st147;
                goto st0;
            st147:
                if (++p == pe)
                    goto _test_eof147;
            case 147:
                _widec = (*p);
                if (114 <= (*p) && (*p) <= 114) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 370)
                    goto st148;
                goto st0;
            st148:
                if (++p == pe)
                    goto _test_eof148;
            case 148:
                _widec = (*p);
                if (103 <= (*p) && (*p) <= 103) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 359)
                    goto st149;
                goto st0;
            st149:
                if (++p == pe)
                    goto _test_eof149;
            case 149:
                _widec = (*p);
                if (121 <= (*p) && (*p) <= 121) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 377)
                    goto st150;
                goto st0;
            st150:
                if (++p == pe)
                    goto _test_eof150;
            case 150:
                goto tr173;
            tr173 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st151;
            st151:
                if (++p == pe)
                    goto _test_eof151;
            case 151:
#line 1469 "Scanner.cpp"
                goto st152;
            st152:
                if (++p == pe)
                    goto _test_eof152;
            case 152:
                goto st153;
            st153:
                if (++p == pe)
                    goto _test_eof153;
            case 153:
                goto st154;
            st154:
                if (++p == pe)
                    goto _test_eof154;
            case 154:
                goto tr177;
            tr177 :
#line 22 "Scanner.cpp.rl"
            {
                dsize = 1;
            }
#line 7 "Scanner.cpp.rl"
                { dlen = (*p); }
                goto st155;
            st155:
                if (++p == pe)
                    goto _test_eof155;
            case 155:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1500 "Scanner.cpp"
                goto st156;
            st156:
                if (++p == pe)
                    goto _test_eof156;
            case 156:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1510 "Scanner.cpp"
                goto st157;
            st157:
                if (++p == pe)
                    goto _test_eof157;
            case 157:
#line 8 "Scanner.cpp.rl"
            {
                dlen <<= 8;
            }
#line 9 "Scanner.cpp.rl"
                { dlen |= (*p); }
#line 1520 "Scanner.cpp"
                goto st183;
            st183:
                if (++p == pe)
                    goto _test_eof183;
            case 183:
                _widec = (*p);
                _widec = (short) (640 + ((*p) - -128));
                if (
#line 11 "Scanner.cpp.rl"
                    (dlen > 0) && ((dlen * dsize) <= (pe - p)))
                    _widec += 256;
                if (896 <= _widec && _widec <= 1151)
                    goto tr216;
                goto tr215;
            st158:
                if (++p == pe)
                    goto _test_eof158;
            case 158:
                _widec = (*p);
                if (121 <= (*p) && (*p) <= 121) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 633)
                    goto st159;
                goto st0;
            st159:
                if (++p == pe)
                    goto _test_eof159;
            case 159:
                _widec = (*p);
                if (110 <= (*p) && (*p) <= 110) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 622)
                    goto st160;
                goto st0;
            st160:
                if (++p == pe)
                    goto _test_eof160;
            case 160:
                _widec = (*p);
                if (101 <= (*p) && (*p) <= 101) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 613)
                    goto st161;
                goto st0;
            st161:
                if (++p == pe)
                    goto _test_eof161;
            case 161:
                _widec = (*p);
                if (114 <= (*p) && (*p) <= 114) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 626)
                    goto st162;
                goto st0;
            st162:
                if (++p == pe)
                    goto _test_eof162;
            case 162:
                _widec = (*p);
                if (103 <= (*p) && (*p) <= 103) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 615)
                    goto st163;
                goto st0;
            st163:
                if (++p == pe)
                    goto _test_eof163;
            case 163:
                _widec = (*p);
                if (121 <= (*p) && (*p) <= 121) {
                    _widec = (short) (128 + ((*p) - -128));
                    if (
#line 5 "Scanner.cpp.rl"
                        flow == Flow::STC)
                        _widec += 256;
                }
                if (_widec == 633)
                    goto st164;
                goto st0;
            st164:
                if (++p == pe)
                    goto _test_eof164;
            case 164:
                goto tr187;
            tr187 :
#line 6 "Scanner.cpp.rl"
            {
                data = p;
            }
                goto st165;
            st165:
                if (++p == pe)
                    goto _test_eof165;
            case 165:
#line 1631 "Scanner.cpp"
                goto st166;
            st166:
                if (++p == pe)
                    goto _test_eof166;
            case 166:
                goto st167;
            st167:
                if (++p == pe)
                    goto _test_eof167;
            case 167:
                goto tr190;
        }
    _test_eof168:
        cs = 168;
        goto _test_eof;
    _test_eof1:
        cs = 1;
        goto _test_eof;
    _test_eof2:
        cs = 2;
        goto _test_eof;
    _test_eof3:
        cs = 3;
        goto _test_eof;
    _test_eof169:
        cs = 169;
        goto _test_eof;
    _test_eof4:
        cs = 4;
        goto _test_eof;
    _test_eof5:
        cs = 5;
        goto _test_eof;
    _test_eof170:
        cs = 170;
        goto _test_eof;
    _test_eof6:
        cs = 6;
        goto _test_eof;
    _test_eof7:
        cs = 7;
        goto _test_eof;
    _test_eof8:
        cs = 8;
        goto _test_eof;
    _test_eof9:
        cs = 9;
        goto _test_eof;
    _test_eof10:
        cs = 10;
        goto _test_eof;
    _test_eof11:
        cs = 11;
        goto _test_eof;
    _test_eof12:
        cs = 12;
        goto _test_eof;
    _test_eof13:
        cs = 13;
        goto _test_eof;
    _test_eof14:
        cs = 14;
        goto _test_eof;
    _test_eof171:
        cs = 171;
        goto _test_eof;
    _test_eof15:
        cs = 15;
        goto _test_eof;
    _test_eof16:
        cs = 16;
        goto _test_eof;
    _test_eof17:
        cs = 17;
        goto _test_eof;
    _test_eof18:
        cs = 18;
        goto _test_eof;
    _test_eof19:
        cs = 19;
        goto _test_eof;
    _test_eof20:
        cs = 20;
        goto _test_eof;
    _test_eof21:
        cs = 21;
        goto _test_eof;
    _test_eof22:
        cs = 22;
        goto _test_eof;
    _test_eof23:
        cs = 23;
        goto _test_eof;
    _test_eof24:
        cs = 24;
        goto _test_eof;
    _test_eof25:
        cs = 25;
        goto _test_eof;
    _test_eof26:
        cs = 26;
        goto _test_eof;
    _test_eof27:
        cs = 27;
        goto _test_eof;
    _test_eof172:
        cs = 172;
        goto _test_eof;
    _test_eof28:
        cs = 28;
        goto _test_eof;
    _test_eof29:
        cs = 29;
        goto _test_eof;
    _test_eof173:
        cs = 173;
        goto _test_eof;
    _test_eof30:
        cs = 30;
        goto _test_eof;
    _test_eof31:
        cs = 31;
        goto _test_eof;
    _test_eof174:
        cs = 174;
        goto _test_eof;
    _test_eof32:
        cs = 32;
        goto _test_eof;
    _test_eof33:
        cs = 33;
        goto _test_eof;
    _test_eof34:
        cs = 34;
        goto _test_eof;
    _test_eof35:
        cs = 35;
        goto _test_eof;
    _test_eof36:
        cs = 36;
        goto _test_eof;
    _test_eof37:
        cs = 37;
        goto _test_eof;
    _test_eof38:
        cs = 38;
        goto _test_eof;
    _test_eof39:
        cs = 39;
        goto _test_eof;
    _test_eof40:
        cs = 40;
        goto _test_eof;
    _test_eof41:
        cs = 41;
        goto _test_eof;
    _test_eof42:
        cs = 42;
        goto _test_eof;
    _test_eof43:
        cs = 43;
        goto _test_eof;
    _test_eof44:
        cs = 44;
        goto _test_eof;
    _test_eof45:
        cs = 45;
        goto _test_eof;
    _test_eof46:
        cs = 46;
        goto _test_eof;
    _test_eof47:
        cs = 47;
        goto _test_eof;
    _test_eof175:
        cs = 175;
        goto _test_eof;
    _test_eof48:
        cs = 48;
        goto _test_eof;
    _test_eof49:
        cs = 49;
        goto _test_eof;
    _test_eof50:
        cs = 50;
        goto _test_eof;
    _test_eof51:
        cs = 51;
        goto _test_eof;
    _test_eof52:
        cs = 52;
        goto _test_eof;
    _test_eof53:
        cs = 53;
        goto _test_eof;
    _test_eof54:
        cs = 54;
        goto _test_eof;
    _test_eof55:
        cs = 55;
        goto _test_eof;
    _test_eof176:
        cs = 176;
        goto _test_eof;
    _test_eof56:
        cs = 56;
        goto _test_eof;
    _test_eof57:
        cs = 57;
        goto _test_eof;
    _test_eof58:
        cs = 58;
        goto _test_eof;
    _test_eof59:
        cs = 59;
        goto _test_eof;
    _test_eof60:
        cs = 60;
        goto _test_eof;
    _test_eof61:
        cs = 61;
        goto _test_eof;
    _test_eof62:
        cs = 62;
        goto _test_eof;
    _test_eof177:
        cs = 177;
        goto _test_eof;
    _test_eof63:
        cs = 63;
        goto _test_eof;
    _test_eof64:
        cs = 64;
        goto _test_eof;
    _test_eof65:
        cs = 65;
        goto _test_eof;
    _test_eof66:
        cs = 66;
        goto _test_eof;
    _test_eof67:
        cs = 67;
        goto _test_eof;
    _test_eof68:
        cs = 68;
        goto _test_eof;
    _test_eof69:
        cs = 69;
        goto _test_eof;
    _test_eof70:
        cs = 70;
        goto _test_eof;
    _test_eof71:
        cs = 71;
        goto _test_eof;
    _test_eof72:
        cs = 72;
        goto _test_eof;
    _test_eof73:
        cs = 73;
        goto _test_eof;
    _test_eof74:
        cs = 74;
        goto _test_eof;
    _test_eof75:
        cs = 75;
        goto _test_eof;
    _test_eof76:
        cs = 76;
        goto _test_eof;
    _test_eof77:
        cs = 77;
        goto _test_eof;
    _test_eof78:
        cs = 78;
        goto _test_eof;
    _test_eof79:
        cs = 79;
        goto _test_eof;
    _test_eof80:
        cs = 80;
        goto _test_eof;
    _test_eof81:
        cs = 81;
        goto _test_eof;
    _test_eof82:
        cs = 82;
        goto _test_eof;
    _test_eof83:
        cs = 83;
        goto _test_eof;
    _test_eof84:
        cs = 84;
        goto _test_eof;
    _test_eof85:
        cs = 85;
        goto _test_eof;
    _test_eof86:
        cs = 86;
        goto _test_eof;
    _test_eof87:
        cs = 87;
        goto _test_eof;
    _test_eof88:
        cs = 88;
        goto _test_eof;
    _test_eof89:
        cs = 89;
        goto _test_eof;
    _test_eof90:
        cs = 90;
        goto _test_eof;
    _test_eof91:
        cs = 91;
        goto _test_eof;
    _test_eof92:
        cs = 92;
        goto _test_eof;
    _test_eof93:
        cs = 93;
        goto _test_eof;
    _test_eof94:
        cs = 94;
        goto _test_eof;
    _test_eof95:
        cs = 95;
        goto _test_eof;
    _test_eof96:
        cs = 96;
        goto _test_eof;
    _test_eof97:
        cs = 97;
        goto _test_eof;
    _test_eof98:
        cs = 98;
        goto _test_eof;
    _test_eof99:
        cs = 99;
        goto _test_eof;
    _test_eof100:
        cs = 100;
        goto _test_eof;
    _test_eof101:
        cs = 101;
        goto _test_eof;
    _test_eof102:
        cs = 102;
        goto _test_eof;
    _test_eof103:
        cs = 103;
        goto _test_eof;
    _test_eof104:
        cs = 104;
        goto _test_eof;
    _test_eof105:
        cs = 105;
        goto _test_eof;
    _test_eof106:
        cs = 106;
        goto _test_eof;
    _test_eof107:
        cs = 107;
        goto _test_eof;
    _test_eof108:
        cs = 108;
        goto _test_eof;
    _test_eof109:
        cs = 109;
        goto _test_eof;
    _test_eof110:
        cs = 110;
        goto _test_eof;
    _test_eof111:
        cs = 111;
        goto _test_eof;
    _test_eof112:
        cs = 112;
        goto _test_eof;
    _test_eof113:
        cs = 113;
        goto _test_eof;
    _test_eof114:
        cs = 114;
        goto _test_eof;
    _test_eof115:
        cs = 115;
        goto _test_eof;
    _test_eof116:
        cs = 116;
        goto _test_eof;
    _test_eof117:
        cs = 117;
        goto _test_eof;
    _test_eof118:
        cs = 118;
        goto _test_eof;
    _test_eof119:
        cs = 119;
        goto _test_eof;
    _test_eof120:
        cs = 120;
        goto _test_eof;
    _test_eof121:
        cs = 121;
        goto _test_eof;
    _test_eof122:
        cs = 122;
        goto _test_eof;
    _test_eof123:
        cs = 123;
        goto _test_eof;
    _test_eof124:
        cs = 124;
        goto _test_eof;
    _test_eof125:
        cs = 125;
        goto _test_eof;
    _test_eof126:
        cs = 126;
        goto _test_eof;
    _test_eof127:
        cs = 127;
        goto _test_eof;
    _test_eof128:
        cs = 128;
        goto _test_eof;
    _test_eof178:
        cs = 178;
        goto _test_eof;
    _test_eof129:
        cs = 129;
        goto _test_eof;
    _test_eof130:
        cs = 130;
        goto _test_eof;
    _test_eof131:
        cs = 131;
        goto _test_eof;
    _test_eof179:
        cs = 179;
        goto _test_eof;
    _test_eof132:
        cs = 132;
        goto _test_eof;
    _test_eof180:
        cs = 180;
        goto _test_eof;
    _test_eof133:
        cs = 133;
        goto _test_eof;
    _test_eof134:
        cs = 134;
        goto _test_eof;
    _test_eof135:
        cs = 135;
        goto _test_eof;
    _test_eof136:
        cs = 136;
        goto _test_eof;
    _test_eof137:
        cs = 137;
        goto _test_eof;
    _test_eof138:
        cs = 138;
        goto _test_eof;
    _test_eof139:
        cs = 139;
        goto _test_eof;
    _test_eof140:
        cs = 140;
        goto _test_eof;
    _test_eof181:
        cs = 181;
        goto _test_eof;
    _test_eof141:
        cs = 141;
        goto _test_eof;
    _test_eof142:
        cs = 142;
        goto _test_eof;
    _test_eof143:
        cs = 143;
        goto _test_eof;
    _test_eof182:
        cs = 182;
        goto _test_eof;
    _test_eof144:
        cs = 144;
        goto _test_eof;
    _test_eof145:
        cs = 145;
        goto _test_eof;
    _test_eof146:
        cs = 146;
        goto _test_eof;
    _test_eof147:
        cs = 147;
        goto _test_eof;
    _test_eof148:
        cs = 148;
        goto _test_eof;
    _test_eof149:
        cs = 149;
        goto _test_eof;
    _test_eof150:
        cs = 150;
        goto _test_eof;
    _test_eof151:
        cs = 151;
        goto _test_eof;
    _test_eof152:
        cs = 152;
        goto _test_eof;
    _test_eof153:
        cs = 153;
        goto _test_eof;
    _test_eof154:
        cs = 154;
        goto _test_eof;
    _test_eof155:
        cs = 155;
        goto _test_eof;
    _test_eof156:
        cs = 156;
        goto _test_eof;
    _test_eof157:
        cs = 157;
        goto _test_eof;
    _test_eof183:
        cs = 183;
        goto _test_eof;
    _test_eof158:
        cs = 158;
        goto _test_eof;
    _test_eof159:
        cs = 159;
        goto _test_eof;
    _test_eof160:
        cs = 160;
        goto _test_eof;
    _test_eof161:
        cs = 161;
        goto _test_eof;
    _test_eof162:
        cs = 162;
        goto _test_eof;
    _test_eof163:
        cs = 163;
        goto _test_eof;
    _test_eof164:
        cs = 164;
        goto _test_eof;
    _test_eof165:
        cs = 165;
        goto _test_eof;
    _test_eof166:
        cs = 166;
        goto _test_eof;
    _test_eof167:
        cs = 167;
        goto _test_eof;

    _test_eof : {}
        if (p == eof) {
            switch (cs) {
                case 169:
                    goto tr197;
                case 170:
                    goto tr198;
                case 171:
                    goto tr199;
                case 172:
                    goto tr200;
                case 173:
                    goto tr201;
                case 174:
                    goto tr202;
                case 175:
                    goto tr203;
                case 176:
                    goto tr205;
                case 177:
                    goto tr207;
                case 178:
                    goto tr209;
                case 179:
                    goto tr211;
                case 180:
                    goto tr212;
                case 181:
                    goto tr213;
                case 182:
                    goto tr214;
                case 183:
                    goto tr215;
            }
        }

    _out : {}
    }

#line 103 "Scanner.cpp.rl"

    if ((cs < 168) || (p != pe)) {
        return false;
    }

    return true;
}

} // namespace v1
} // namespace protocol
} // namespace synergy
