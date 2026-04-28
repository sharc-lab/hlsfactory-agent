/*
 * Minimal SystemC syntax stub
 * For compilation testing with clang++ only - NOT for simulation or synthesis
 *
 * This header provides just enough of the SystemC surface area used by the
 * extracted benchmarks to satisfy syntax-only compilation. Numeric types are
 * delegated to the existing ap_int/ap_fixed stubs.
 */

#ifndef __SYSTEMC_STUB_H__
#define __SYSTEMC_STUB_H__

#include <cstddef>
#include <cstdint>
#include <iostream>

#include "ap_fixed.h"
#include "ap_int.h"

using std::cerr;
using std::cout;
using std::endl;

typedef const char* sc_module_name;

enum sc_time_unit {
    SC_FS = 0,
    SC_PS = 1,
    SC_NS = 2,
    SC_US = 3,
    SC_MS = 4,
    SC_SEC = 5
};

class sc_event_finder {
};

class sc_sensitive {
public:
    template <typename T>
    sc_sensitive& operator<<(const T&) {
        return *this;
    }
};

template <typename T>
class sc_signal_rw_if {
public:
    virtual ~sc_signal_rw_if() = default;
    virtual T read() const = 0;
    virtual void write(const T& value) = 0;
};

template <typename T>
class sc_signal : public sc_signal_rw_if<T> {
public:
    sc_signal() : value_() {}
    explicit sc_signal(const char*) : value_() {}
    sc_signal(const char*, const T& value) : value_(value) {}

    T read() const override {
        return value_;
    }

    void write(const T& value) override {
        value_ = value;
    }

    sc_signal& operator=(const T& value) {
        write(value);
        return *this;
    }

    operator T() const {
        return read();
    }

    sc_signal_rw_if<T>* operator->() {
        return this;
    }

    const sc_signal_rw_if<T>* operator->() const {
        return this;
    }

    sc_event_finder pos() const {
        return sc_event_finder();
    }

    sc_event_finder neg() const {
        return sc_event_finder();
    }

private:
    T value_;
};

template <typename T>
class sc_port_base : public sc_signal_rw_if<T> {
public:
    sc_port_base() : iface_(nullptr), value_() {}
    explicit sc_port_base(const char*) : iface_(nullptr), value_() {}

    void bind(sc_signal_rw_if<T>& iface) {
        iface_ = &iface;
    }

    void operator()(sc_signal_rw_if<T>& iface) {
        bind(iface);
    }

    T read() const override {
        return iface_ ? iface_->read() : value_;
    }

    void write(const T& value) override {
        if (iface_) {
            iface_->write(value);
        } else {
            value_ = value;
        }
    }

    sc_port_base& operator=(const T& value) {
        write(value);
        return *this;
    }

    operator T() const {
        return read();
    }

    sc_signal_rw_if<T>* operator->() {
        return iface_ ? iface_ : this;
    }

    const sc_signal_rw_if<T>* operator->() const {
        return iface_ ? iface_ : this;
    }

    sc_event_finder pos() const {
        return sc_event_finder();
    }

    sc_event_finder neg() const {
        return sc_event_finder();
    }

protected:
    sc_signal_rw_if<T>* iface_;
    T value_;
};

template <typename T>
class sc_in : public sc_port_base<T> {
public:
    sc_in() = default;
    explicit sc_in(const char* name) : sc_port_base<T>(name) {}
};

template <typename T>
class sc_out : public sc_port_base<T> {
public:
    sc_out() = default;
    explicit sc_out(const char* name) : sc_port_base<T>(name) {}
};

template <typename T>
class sc_inout : public sc_port_base<T> {
public:
    sc_inout() = default;
    explicit sc_inout(const char* name) : sc_port_base<T>(name) {}
};

typedef sc_in<bool> sc_in_clk;

class sc_module {
public:
    sc_module() : name_(""), sensitive() {}
    explicit sc_module(sc_module_name name) : name_(name), sensitive() {}
    virtual ~sc_module() = default;

    sc_module_name name() const {
        return name_;
    }

    sc_sensitive sensitive;

private:
    sc_module_name name_;
};

class sc_clock : public sc_signal<bool> {
public:
    sc_clock()
        : sc_signal<bool>() {}

    explicit sc_clock(const char* name)
        : sc_signal<bool>(name) {}

    sc_clock(
        const char* name,
        double,
        sc_time_unit = SC_NS,
        double = 0.5,
        double = 0.0,
        sc_time_unit = SC_NS,
        bool = true)
        : sc_signal<bool>(name) {}
};

class sc_trace_file {
};

inline sc_trace_file* sc_create_vcd_trace_file(const char*) {
    return new sc_trace_file();
}

inline void sc_close_vcd_trace_file(sc_trace_file* trace_file) {
    delete trace_file;
}

template <typename T>
inline void sc_trace(sc_trace_file*, const T&, const char*) {
}

inline void sc_start(double = 0.0, sc_time_unit = SC_NS) {
}

inline void sc_stop() {
}

inline void wait() {
}

inline const char* sc_time_stamp() {
    return "0 s";
}

template <int W>
using sc_int = ap_int<W>;

template <int W>
using sc_uint = ap_uint<W>;

template <int W>
using sc_bigint = ap_int<W>;

template <int W>
using sc_biguint = ap_uint<W>;

template <int W, int I, int Q = AP_TRN, int O = AP_WRAP>
using sc_fixed = ap_fixed<W, I, Q, O>;

template <int W, int I, int Q = AP_TRN, int O = AP_WRAP>
using sc_ufixed = ap_ufixed<W, I, Q, O>;

#ifndef SC_TRN
#define SC_TRN AP_TRN
#endif

#ifndef SC_RND
#define SC_RND AP_RND
#endif

#ifndef SC_WRAP
#define SC_WRAP AP_WRAP
#endif

#ifndef SC_SAT
#define SC_SAT AP_SAT
#endif

#define SC_MODULE(name) struct name : public sc_module
#define SC_CTOR(name) name(sc_module_name module_name = #name)
#define SC_HAS_PROCESS(name)
#define SC_METHOD(func) ((void)0)
#define SC_THREAD(func) ((void)0)
#define SC_CTHREAD(func, edge) ((void)0)
#define reset_signal_is(sig, value) ((void)0)
#define dont_initialize() ((void)0)

#endif // __SYSTEMC_STUB_H__
