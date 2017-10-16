#pragma once
#include <cstddef>
#include <memory>

namespace synergy {
namespace protocol {
namespace v1 {

class MessageImpl {
public:
    virtual ~MessageImpl () noexcept;
    virtual std::unique_ptr<MessageImpl> clone () const = 0;

    virtual char const* type_name () const noexcept = 0;
    virtual char const* type_tag () const noexcept  = 0;

    virtual std::size_t size () const noexcept = 0;
    virtual void write_to (char* buf) const    = 0;
    virtual void
    read_from (char const* header, char const* data, char const* end) = 0;

protected:
    MessageImpl () noexcept                   = default;
    MessageImpl (MessageImpl const&) noexcept = default;
    MessageImpl& operator= (MessageImpl const&) noexcept = default;
};

} // namespace v1
} // namespace protocol
} // namespace synergy
