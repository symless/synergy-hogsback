#include "Connection.hpp"
#include "KeepAlive.hpp"
#include "Message.hpp"
#include "MessageReader.hpp"
#include "MessageWriter.hpp"
#include <fmt/format.h>
#include <iostream>
#include <synergy/service/ServiceLogs.h>

uint32_t Connection::next_connection_id_ = 0;

Connection::Connection (tcp::socket socket)
    : id_ (++next_connection_id_),
      socket_ (std::move (socket)),
      endpoint_ (socket_.remote_endpoint ()),
      context_(ssl::context::tlsv12),
      stream_(socket, context_),
      reader_ (stream_),
      writer_ (stream_) {
    routerLog ()->info ("Connection {} created", id ());

    boost::system::error_code ec;
    socket_.set_option (tcp::no_delay (true), ec);

    if (ec) {
        routerLog ()->info (
            "Failed to disable Nagles algorithm on connection {}", id ());
    }

    if (!set_tcp_keep_alive_options (socket_, 5, 2, 10)) {
        throw std::runtime_error (fmt::format (
            "Failed to enable TCP keepalive on connection {}", id ()));
    }
}

Connection::~Connection () noexcept {
    routerLog ()->info ("Connection {} destroyed", id ());
}

void
Connection::start (bool fromServer) {
    if (fromServer) {
        loadRawCertificate();
    }

    asio::spawn (
        socket_.get_io_service (),
        [ this, self = shared_from_this (), fromServer ](auto ctx) {
        boost::system::error_code ec;
        stream_.async_handshake(fromServer ? ssl::stream_base::server : ssl::stream_base::client, ctx[ec]);

        if (ec) {
            on_disconnect (self);
        }
        else {
            enabled_ = true;
            on_connected (self);

            while (true) {
                MessageHeader header;
                Message message;

                if (reader_.read_header (header, ctx)) {
                    if (reader_.read_body (header, message, ctx)) {
                        on_message (header, message, self);
                    }
                }

                auto ec = reader_.error ();
                if (!enabled_ || (ec == asio::error::operation_aborted)) {
                    break;
                } else if ((ec == asio::error::connection_reset) ||
                           (ec == asio::error::eof) ||
                           (ec == asio::error::timed_out)) {
                    on_disconnect (self);
                    break;
                } else if (ec) {
                    throw boost::system::system_error (ec, ec.message ());
                }
            }

            routerLog ()->info ("Connection {} terminated receive loop",
                                this->id ());
        }
    });
}

void
Connection::stop () {
    enabled_ = false;
    boost::system::error_code ec;
    socket_.cancel (ec);
}

bool
Connection::send (Message const& message, asio::yield_context ctx) {
    auto header = message.make_header ();
    return send (header, message, ctx);
}

bool
Connection::send (MessageHeader const& header, Message const& message,
                  asio::yield_context ctx) {
    return writer_.write (header, message, ctx);
}

tcp::endpoint
Connection::endpoint () const {
    return tcp::endpoint (endpoint_.address (), 24802);
}

void
Connection::loadRawCertificate()
{
    const std::string cert = sslCertificate();
    const std::string key = sslKey();
    const std::string dh = sslDH();

    context_.set_options(
                boost::asio::ssl::context::default_workarounds |
                boost::asio::ssl::context::no_sslv3 |
                boost::asio::ssl::context::single_dh_use);

    context_.use_certificate_chain(
            boost::asio::buffer(cert.data(), cert.size()));

    context_.use_private_key(
        boost::asio::buffer(key.data(), key.size()),
        boost::asio::ssl::context::file_format::pem);

    context_.use_tmp_dh(
        boost::asio::buffer(dh.data(), dh.size()));
}

const std::string
Connection::sslCertificate()
{
    return  "-----BEGIN CERTIFICATE-----\n"
                 "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
                 "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
                 "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
                 "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
                 "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
                 "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
                 "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
                 "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
                 "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
                 "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
                 "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
                 "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
                 "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
                 "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
                 "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
                 "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
                 "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
                 "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
                 "UEVbkhd5qstF6qWK\n"
                 "-----END CERTIFICATE-----\n";
}

const std::string
Connection::sslKey()
{
    return "-----BEGIN PRIVATE KEY-----\n"
           "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
           "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
           "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
           "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
           "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
           "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
           "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
           "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
           "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
           "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
           "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
           "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
          "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
           "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
           "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
           "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
           "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
           "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
           "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
           "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
          "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
           "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
           "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
           "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
           "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
           "pVOUFq5mW8p0zbtfHbjkgxyF\n"
           "-----END PRIVATE KEY-----\n";
}

const std::string
Connection::sslDH()
{
    return "-----BEGIN DH PARAMETERS-----\n"
           "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
           "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
           "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
           "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
           "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
           "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
           "-----END DH PARAMETERS-----\n";
}
