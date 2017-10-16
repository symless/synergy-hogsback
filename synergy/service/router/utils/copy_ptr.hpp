#pragma once
#include <memory>

/* A super dumb smart pointer that just copies when it's copied. This is to
 * work around flatbuffers inability to inline tables when generating object
 * API code.
 *
 * Ref: A Proposal for the World’s Dumbest Smart Pointer
 *      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3840.pdf
 */
template <typename Deleter>
struct copy_ptr_deleter_base;

template <typename T>
struct copy_ptr_deleter_base<std::default_delete<T>> {
    template <typename... Args>
    copy_ptr_deleter_base (Args&&...) noexcept {
    }

    void
    destroy (T* const ptr) noexcept {
        std::default_delete<T> () (ptr);
    }
};

template <typename T, typename Deleter = std::default_delete<T>>
class copy_ptr final : copy_ptr_deleter_base<Deleter> {
public:
    copy_ptr () noexcept : ptr_ (nullptr) {
    }

    template <typename U>
    explicit copy_ptr (std::unique_ptr<U, Deleter> ptr) noexcept
        : ptr_ (ptr.release ()) {
    }

    template <typename U>
    explicit copy_ptr (U* const ptr, Deleter deleter = Deleter ()) noexcept
        : copy_ptr_deleter_base<Deleter> (std::move (deleter)), ptr_ (ptr) {
    }

    copy_ptr (copy_ptr<T, Deleter>&& src) noexcept : ptr_ (src.ptr_) {
        src.ptr_ = nullptr;
    }

    copy_ptr (copy_ptr<T, Deleter> const& src) : ptr_ (nullptr) {
        if (src.ptr_) {
            ptr_ = new T (*src.ptr_);
        }
    }

    copy_ptr<T, Deleter>&
    operator= (copy_ptr<T, Deleter>&& src) noexcept {
        if (src.ptr_) {
            if (ptr_) {
                copy_ptr_deleter_base<Deleter>::destroy (ptr_);
            }
            ptr_ = src.release ();
        } else {
            if (ptr_) {
                copy_ptr_deleter_base<Deleter>::destroy (ptr_);
                ptr_ = nullptr;
            }
        }
        return *this;
    }

    copy_ptr<T, Deleter>&
    operator= (copy_ptr<T, Deleter> const& src) {
        if (src.ptr_) {
            if (ptr_) {
                // Copy assign instead of allocating a copy.
                *ptr_ = *src.ptr_;
            } else {
                ptr_ = new T (*src.ptr_);
            }
        } else {
            if (ptr_) {
                copy_ptr_deleter_base<Deleter>::destroy (ptr_);
                ptr_ = nullptr;
            }
        }
        return *this;
    }

    ~copy_ptr () noexcept {
        if (ptr_) {
            copy_ptr_deleter_base<Deleter>::destroy (ptr_);
        }
    }

    explicit operator bool () const noexcept {
        return ptr_;
    }

    T*
    get () const noexcept {
        return ptr_;
    }

    T*
    release () noexcept {
        T* const ptr = ptr_;
        ptr_         = nullptr;
        return ptr;
    }

    T* operator-> () const noexcept {
        return ptr_;
    }

    T& operator* () const noexcept {
        return *ptr_;
    }

private:
    T* ptr_;
};
