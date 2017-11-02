%%{
    machine synergy_proto_v1;
    alphtype unsigned char;

    action stc              { flow == Flow::STC }
    action mdb              { data = fpc; }
    action dlen_init        { dlen = fc; }
    action dlen_shift       { dlen <<= 8; }
    action dlen_or          { dlen |= fc; }
    action dlen_is_zero     { dlen == 0 }
    action dlen_is_safe     { (dlen > 0) && ((dlen * dsize) <= (pe - fpc)) }
    action skip_data_bytes  { fhold; fpc += (dlen * dsize); }

    i1 = any;
    i2 = any{2};
    i4 = any{4};

    dlen        = i4 >dlen_init <>*dlen_shift <>*dlen_or;
    data_bytes  = ((any when dlen_is_safe) >skip_data_bytes)
                  | ('' when dlen_is_zero);

    string = dlen >{ dsize = 1; } data_bytes;
    i4vec  = dlen >{ dsize = 4; } data_bytes;

    message := |*
        ("Synergy" when stc)    %mdb i2 i2                      => { PROC(Hello) };
        ("Synergy" when !stc)   %mdb i2 i2 string               => { PROC(HelloBack) };
        "CNOP"                  %mdb                            => { PROC(Noop) };
        "CBYE"                  %mdb                            => { PROC(Close) };
        "CINN"                  %mdb i2 i2 i4 i2                => { PROC(Enter) };
        "COUT"                  %mdb                            => { PROC(Leave) };
        "CCLP"                  %mdb i1 i4                      => { PROC(ClipboardGrab) };
        "CSEC"                  %mdb i1                         => { PROC(Screensaver) };
        "CROP"                  %mdb                            => { PROC(ResetOptions) };
        "CIAK"                  %mdb                            => { PROC(InfoAck) };
        "CALV"                  %mdb                            => { PROC(KeepAlive) };
        "DKDN"                  %mdb i2 i2 i2                   => { PROC(KeyDown) };
        "DKRP"                  %mdb i2 i2 i2 i2                => { PROC(KeyRepeat) };
        "DKUP"                  %mdb i2 i2 i2                   => { PROC(KeyUp) };
        "DMDN"                  %mdb i1                         => { PROC(MouseDown) };
        "DMUP"                  %mdb i1                         => { PROC(MouseUp) };
        "DMMV"                  %mdb i2 i2                      => { PROC(MouseMove) };
        "DMRM"                  %mdb i2 i2                      => { PROC(MouseRelMove) };
        "DMWM"                  %mdb i2 i2                      => { PROC(MouseWheelMove) };
        "DCLP"                  %mdb i1 i4 i1 string            => { PROC(ClipboardData) };
        "DINF"                  %mdb i2 i2 i2 i2 i2 i2 i2       => { PROC(Info) };
        "DSOP"                  %mdb i4vec                      => { PROC(SetOptions) };
        "DFTR"                  %mdb i1 string                  => { PROC(FileTransfer) };
        "DDRG"                  %mdb i2 string                  => { PROC(DragInfo) };
        "QINF"                  %mdb                            => { PROC(InfoQuery) };
        "EICV"                  %mdb i2 i2                      => { PROC(Incompatible) };
        "EBSY"                  %mdb                            => { PROC(Busy) };
        "EUNK"                  %mdb                            => { PROC(Unknown) };
        "EBAD"                  %mdb                            => { PROC(Bad) };
    *|;
}%%


#include "Scanner.hpp"
#include "MessageTypes.hpp"
#include <fmt/ostream.h>
#include <synergy/service/ServiceLogs.h>

#define XX_CHAR_PTR(X) reinterpret_cast<char const*>(X)

#define PROC(TYPE)                  \
{                                   \
    TYPE##Message msg;              \
    msg.read_from (XX_CHAR_PTR(ts), XX_CHAR_PTR(data), XX_CHAR_PTR(te));   \
    if (!handle (msg)) {            \
        routerLog ()->debug ("Handler failed to process core {} message: {}", \
                             ((flow == Flow::STC) ? "server" : "client"), msg);  \
        return false;               \
    }                               \
}

namespace synergy {
namespace protocol {
namespace v1 {

%% write data;

bool
process (
    Flow const flow,
    Handler& handle,
    unsigned char const* const msg,
    std::size_t const size
){
    unsigned char const* p = msg;
    unsigned char const* const pe = msg + size;
    unsigned char const* const eof = pe;
    int cs;
    int act;
    unsigned char const* ts;
    unsigned char const* te;

%% write init;

    std::size_t dlen = 0;
    std::size_t dsize = 0;
    unsigned char const* data = nullptr;

%% write exec;

    if ((cs < %%{ write first_final; }%%) || (p != pe)) {
        return false;
    }

    return true;
}

} // namespace v1
} // namespace protocol
} // namespace synergy
