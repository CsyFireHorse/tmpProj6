// Date: 2026-10-22. Drafted by: GPT-6 (Codex).
#include <stdint.h>
#include <cstring>
#include <limits>
#include <tuple>
#include <type_traits>

#if defined(_MSC_VER) && defined(_DEBUG)
// Load the MSVC C++/CRT headers before selecting the release Python ABI.
#undef _DEBUG
#include <Python.h>
#define _DEBUG
#else
#include <Python.h>
#endif

#include "zml_motion_api_ex.h"

namespace {

struct PyZmlHandle {
    PyObject_HEAD
    ZML_HANDLE handle;
};
PyTypeObject handle_type = {PyVarObject_HEAD_INIT(nullptr, 0)};

PyZmlHandle* handle_object(PyObject* obj) {
    if (!PyObject_TypeCheck(obj, &handle_type)) {
        PyErr_SetString(PyExc_TypeError, "expected ZML_HANDLE");
        return nullptr;
    }
    return reinterpret_cast<PyZmlHandle*>(obj);
}

// Preserve integer widths; conversion errors never become motion error codes.
template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type read(PyObject* obj, T& value) {
    PyObject* index = PyNumber_Index(obj);
    if (!index) return false;
    bool fits = false;
    if (std::is_signed<T>::value) {
        const long long n = PyLong_AsLongLong(index);
        fits = !PyErr_Occurred() && n >= (std::numeric_limits<T>::min)() &&
               n <= (std::numeric_limits<T>::max)();
        if (fits) value = static_cast<T>(n);
    } else {
        const unsigned long long n = PyLong_AsUnsignedLongLong(index);
        fits = !PyErr_Occurred() && n <= (std::numeric_limits<T>::max)();
        if (fits) value = static_cast<T>(n);
    }
    Py_DECREF(index);
    if (!fits && !PyErr_Occurred()) PyErr_SetString(PyExc_OverflowError, "integer out of range");
    return fits;
}
bool read(PyObject* obj, bool& value) {
    if (!PyBool_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "expected bool");
        return false;
    }
    value = obj == Py_True;
    return true;
}
bool read(PyObject* obj, double& value) {
    value = PyFloat_AsDouble(obj);
    return !PyErr_Occurred();
}
bool read(PyObject* obj, const char*& value) {
    Py_ssize_t size = 0;
    value = PyUnicode_AsUTF8AndSize(obj, &size);
    if (!value) return false;
    if (std::strlen(value) != static_cast<size_t>(size)) {
        PyErr_SetString(PyExc_ValueError, "embedded null character");
        return false;
    }
    return true;
}
bool read(PyObject* obj, ZML_HANDLE& value) {
    PyZmlHandle* holder = handle_object(obj);
    if (!holder) return false;
    value = holder->handle;
    return true;
}
bool read(PyObject* obj, PyObject*& value) { value = obj; return true; }

template <typename... T>
bool parse(PyObject* args, T&... values) {
    if (PyTuple_GET_SIZE(args) != sizeof...(T)) {
        PyErr_Format(PyExc_TypeError, "expected %u arguments", unsigned(sizeof...(T)));
        return false;
    }
    Py_ssize_t next = 0;
    bool ok = true;
    int order[] = {0, (ok = ok && read(PyTuple_GET_ITEM(args, next++), values), 0)...};
    (void)order;
    return ok;
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, PyObject*>::type encode(const T& value) {
    return std::is_signed<T>::value ? PyLong_FromLongLong(value)
                                   : PyLong_FromUnsignedLongLong(value);
}
PyObject* encode(const bool& value) { return PyBool_FromLong(value); }
PyObject* encode(const double& value) { return PyFloat_FromDouble(value); }

template <typename T, size_t N>
bool read(PyObject* obj, T (&values)[N]) {
    PyObject* seq = PySequence_Fast(obj, "expected a fixed-size sequence");
    if (!seq) return false;
    bool ok = PySequence_Fast_GET_SIZE(seq) == N;
    if (!ok) PyErr_SetString(PyExc_ValueError, "incorrect fixed array length");
    for (size_t i = 0; ok && i < N; ++i) ok = read(PySequence_Fast_GET_ITEM(seq, i), values[i]);
    Py_DECREF(seq);
    return ok;
}
template <typename T, size_t N>
PyObject* encode(const T (&values)[N]) {
    PyObject* list = PyList_New(N);
    if (!list) return nullptr;
    for (size_t i = 0; i < N; ++i) {
        PyObject* item = encode(values[i]);
        if (!item) { Py_DECREF(list); return nullptr; }
        PyList_SET_ITEM(list, i, item);
    }
    return list;
}

// The same public-field list handles dictionary input and output without aliases.
template <typename T> struct Record;
#define FIELD(name) f(#name, value.name)
#define RECORD_BEGIN(type) template <> struct Record<type> { \
    template <typename V, typename F> static bool visit(V& value, F& f) { return
#define RECORD_END ; } };
RECORD_BEGIN(ZML_AxisState)
    FIELD(axis_id) && FIELD(axis_state) && FIELD(error_id) && FIELD(tick) &&
    FIELD(act_pos) && FIELD(set_pos)
RECORD_END
RECORD_BEGIN(ZML_AxisStateEx)
    FIELD(axis_id) && FIELD(axis_state) && FIELD(error_id) && FIELD(tick) &&
    FIELD(act_pos) && FIELD(set_pos) && FIELD(set_vel) && FIELD(set_acc) &&
    FIELD(set_position) && FIELD(act_position)
RECORD_END
RECORD_BEGIN(ZML_CmdResultInfo)
    FIELD(id) && FIELD(command_type) && FIELD(target_index) && FIELD(target_kind) &&
    FIELD(status) && FIELD(io_count) && FIELD(error_code) && FIELD(tick) &&
    FIELD(data) && FIELD(data_ext)
RECORD_END
RECORD_BEGIN(ZML_IoMetadataEntry)
    FIELD(mask) && FIELD(io_number) && FIELD(index) && FIELD(bit_offset) &&
    FIELD(io_type) && FIELD(source_kind) && FIELD(slave_idx) && FIELD(subindex) &&
    FIELD(bit_size) && FIELD(is_input)
RECORD_END
RECORD_BEGIN(ZML_FlowCtrlConfig)
    FIELD(period) && FIELD(batch_size) && FIELD(verbose_flag) && FIELD(reserved)
RECORD_END
RECORD_BEGIN(ZML_FlowCtrlStatus)
    FIELD(period) && FIELD(batch_size) && FIELD(verbose_flag) && FIELD(reserved) &&
    FIELD(latency) && FIELD(axis_num) && FIELD(cycle_time)
RECORD_END
RECORD_BEGIN(ZML_HomeSdoConfig)
    FIELD(mode) && FIELD(offset) && FIELD(velocity_fast) && FIELD(velocity_slow) &&
    FIELD(acceleration) && FIELD(timeout_ms)
RECORD_END
#undef RECORD_BEGIN
#undef RECORD_END
#undef FIELD

struct DictReader {
    PyObject* dict;
    template <typename T> bool operator()(const char* key, T& value) {
        PyObject* item = PyDict_GetItemString(dict, key);
        if (!item) { PyErr_SetString(PyExc_KeyError, key); return false; }
        return read(item, value);
    }
};
struct DictWriter {
    PyObject* dict;
    template <typename T> bool operator()(const char* key, const T& value) {
        PyObject* item = encode(value);
        if (!item) return false;
        const int ret = PyDict_SetItemString(dict, key, item);
        Py_DECREF(item);
        return ret == 0;
    }
};
template <typename T>
bool read_record(PyObject* obj, T& value) {
    if (!PyDict_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "expected a dictionary with public RPC fields");
        return false;
    }
    DictReader reader = {obj};
    return Record<T>::visit(value, reader);
}
template <typename T>
PyObject* record_to_dict(const T& value) {
    PyObject* dict = PyDict_New();
    if (!dict) return nullptr;
    DictWriter writer = {dict};
    if (!Record<T>::visit(value, writer)) { Py_DECREF(dict); return nullptr; }
    return dict;
}
#define RECORD_ENCODER(type) PyObject* encode(const type& value) { return record_to_dict(value); }
RECORD_ENCODER(ZML_AxisState)
RECORD_ENCODER(ZML_AxisStateEx)
RECORD_ENCODER(ZML_CmdResultInfo)
RECORD_ENCODER(ZML_IoMetadataEntry)
RECORD_ENCODER(ZML_FlowCtrlConfig)
RECORD_ENCODER(ZML_FlowCtrlStatus)
#undef RECORD_ENCODER

PyObject* ret_data(ZML_RESULT ret, PyObject* data) {
    return data ? Py_BuildValue("(iN)", ret, data) : nullptr;
}
PyObject* none() { Py_INCREF(Py_None); return Py_None; }

// Scalar and record arguments follow C signature order; pure outputs consume no Python input.
template <typename T> struct Argument {
    static const bool output = false;
    T value = {};
    bool load(PyObject* obj) { return read(obj, value); }
    T get() { return value; }
    PyObject* python() { return none(); }
};
template <typename T> struct Argument<T*> {
    static const bool output = true;
    T value = {};
    bool load(PyObject*) { return true; }
    T* get() { return &value; }
    PyObject* python() { return encode(value); }
};
// The opaque pointer is an input value, unlike ordinary C output pointers.
template <> struct Argument<ZML_HANDLE> {
    static const bool output = false;
    ZML_HANDLE value = nullptr;
    bool load(PyObject* obj) { return read(obj, value); }
    ZML_HANDLE get() { return value; }
    PyObject* python() { return none(); }
};
template <typename T> struct Argument<const T*> {
    static const bool output = false;
    T value = {};
    bool load(PyObject* obj) { return read_record(obj, value); }
    const T* get() { return &value; }
    PyObject* python() { return none(); }
};
template <> struct Argument<const char*> {
    static const bool output = false;
    const char* value = nullptr;
    bool load(PyObject* obj) { return read(obj, value); }
    const char* get() { return value; }
    PyObject* python() { return none(); }
};
template <> struct Argument<char*> {
    static const bool output = true;
    // All identity buffers meet the capacities required by the public C API.
    char value[ZML_ABOUT_INFO_MAX] = {};
    bool load(PyObject*) { return true; }
    char* get() { return value; }
    PyObject* python() { return PyUnicode_FromString(value); }
};
static_assert(ZML_ABOUT_INFO_MAX >= ZML_VERSION_INFO_MAX &&
              ZML_ABOUT_INFO_MAX >= ZML_CONTROLLER_INFO_MAX, "identity buffer capacity");

template <size_t... I> struct Indices {};
template <size_t N, size_t... I> struct MakeIndices : MakeIndices<N - 1, N - 1, I...> {};
template <size_t... I> struct MakeIndices<0, I...> { typedef Indices<I...> type; };

template <size_t I = 0, typename... A>
typename std::enable_if<I == sizeof...(A), bool>::type
load_arguments(PyObject*, Py_ssize_t&, std::tuple<Argument<A>...>&) { return true; }
template <size_t I = 0, typename... A>
typename std::enable_if<I < sizeof...(A), bool>::type
load_arguments(PyObject* args, Py_ssize_t& next, std::tuple<Argument<A>...>& values) {
    auto& value = std::get<I>(values);
    if (!value.output) {
        if (next == PyTuple_GET_SIZE(args)) {
            PyErr_SetString(PyExc_TypeError, "missing RPC input argument");
            return false;
        }
        if (!value.load(PyTuple_GET_ITEM(args, next++))) return false;
    }
    return load_arguments<I + 1>(args, next, values);
}
template <size_t I = 0, typename... A>
typename std::enable_if<I == sizeof...(A), bool>::type
write_outputs(PyObject*, Py_ssize_t&, ZML_RESULT, std::tuple<Argument<A>...>&) { return true; }
template <size_t I = 0, typename... A>
typename std::enable_if<I < sizeof...(A), bool>::type
write_outputs(PyObject* tuple, Py_ssize_t& next, ZML_RESULT ret,
              std::tuple<Argument<A>...>& values) {
    auto& value = std::get<I>(values);
    if (value.output) {
        PyObject* obj = ret == 0 ? value.python() : none();
        if (!obj) return false;
        PyTuple_SET_ITEM(tuple, next++, obj);
    }
    return write_outputs<I + 1>(tuple, next, ret, values);
}
template <typename Signature> struct Binding;
template <typename... A> struct Binding<ZML_RESULT (*)(A...)> {
    template <ZML_RESULT (*Fn)(A...), size_t... I>
    static ZML_RESULT invoke(std::tuple<Argument<A>...>& values, Indices<I...>) {
        return Fn(std::get<I>(values).get()...);
    }
    template <ZML_RESULT (*Fn)(A...)>
    static PyObject* call(PyObject*, PyObject* args) {
        std::tuple<Argument<A>...> values;
        Py_ssize_t next = 0;
        if (!load_arguments(args, next, values)) return nullptr;
        if (next != PyTuple_GET_SIZE(args)) {
            PyErr_SetString(PyExc_TypeError, "too many RPC input arguments");
            return nullptr;
        }
        const ZML_RESULT ret = invoke<Fn>(values, typename MakeIndices<sizeof...(A)>::type());
        const int outputs[] = {0, Argument<A>::output...};
        Py_ssize_t count = 0;
        for (size_t i = 0; i < sizeof...(A) + 1; ++i) count += outputs[i];
        if (!count) return PyLong_FromLong(ret);
        PyObject* tuple = PyTuple_New(count + 1);
        if (!tuple) return nullptr;
        PyObject* result = PyLong_FromLong(ret);
        if (!result) { Py_DECREF(tuple); return nullptr; }
        PyTuple_SET_ITEM(tuple, 0, result);
        next = 1;
        if (!write_outputs(tuple, next, ret, values)) { Py_DECREF(tuple); return nullptr; }
        return tuple;
    }
};

// Opening creates exactly one owner; explicit close and GC share its native pointer.
template <typename T, ZML_RESULT (*Fn)(ZML_HANDLE*, T)>
PyObject* open_handle(PyObject*, PyObject* args) {
    T input = {};
    if (!parse(args, input)) return nullptr;
    PyZmlHandle* holder = PyObject_New(PyZmlHandle, &handle_type);
    if (!holder) return nullptr;
    holder->handle = nullptr;
    const ZML_RESULT ret = Fn(&holder->handle, input);
    if (ret != 0) { Py_DECREF(holder); return ret_data(ret, none()); }
    return ret_data(ret, reinterpret_cast<PyObject*>(holder));
}
PyObject* close_handle(PyObject*, PyObject* args) {
    PyObject* obj = nullptr;
    if (!parse(args, obj)) return nullptr;
    PyZmlHandle* holder = handle_object(obj);
    return holder ? PyLong_FromLong(zml_close(&holder->handle)) : nullptr;
}
void handle_dealloc(PyZmlHandle* self) {
    if (self->handle) (void)zml_close(&self->handle);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}
PyObject* handle_new(PyTypeObject*, PyObject*, PyObject*) {
    PyErr_SetString(PyExc_TypeError, "ZML_HANDLE is created by zml_open*");
    return nullptr;
}

// Temporary numeric buffers only adapt sequences; axis count and order remain explicit.
template <typename T> struct Sequence {
    T* data = nullptr;
    ~Sequence() { PyMem_Free(data); }
    bool load(PyObject* obj, int32_t count) {
        PyObject* seq = PySequence_Fast(obj, "expected a numeric sequence");
        if (!seq) return false;
        bool ok = PySequence_Fast_GET_SIZE(seq) == count;
        if (!ok) PyErr_SetString(PyExc_ValueError, "sequence length must equal ax_num");
        if (ok && count > 0) {
            data = PyMem_New(T, count);
            if (!data) { PyErr_NoMemory(); ok = false; }
        }
        for (int32_t i = 0; ok && i < count; ++i)
            ok = read(PySequence_Fast_GET_ITEM(seq, i), data[i]);
        Py_DECREF(seq);
        return ok;
    }
};
template <ZML_RESULT (*Fn)(ZML_HANDLE, int32_t, const int32_t*, const double*)>
PyObject* move_axes(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    int32_t count = 0;
    PyObject *axes = nullptr, *positions = nullptr;
    if (!parse(args, handle, count, axes, positions)) return nullptr;
    Sequence<int32_t> axis;
    Sequence<double> pos;
    if (!axis.load(axes, count) || !pos.load(positions, count)) return nullptr;
    return PyLong_FromLong(Fn(handle, count, axis.data, pos.data));
}
template <ZML_RESULT (*Fn)(ZML_HANDLE, int32_t, const int32_t*)>
PyObject* group_axes(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    int32_t count = 0;
    PyObject* axes = nullptr;
    if (!parse(args, handle, count, axes)) return nullptr;
    Sequence<int32_t> axis;
    if (!axis.load(axes, count)) return nullptr;
    return PyLong_FromLong(Fn(handle, count, axis.data));
}
template <ZML_RESULT (*Fn)(ZML_HANDLE, int32_t, const int32_t*, double, double, double,
                          double, int32_t)>
PyObject* move_circle(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    int32_t count = 0, direction = 0;
    PyObject* axes = nullptr;
    double end1 = 0, end2 = 0, centre1 = 0, centre2 = 0;
    if (!parse(args, handle, count, axes, end1, end2, centre1, centre2, direction)) return nullptr;
    Sequence<int32_t> axis;
    if (!axis.load(axes, count)) return nullptr;
    return PyLong_FromLong(Fn(handle, count, axis.data, end1, end2, centre1, centre2, direction));
}
template <ZML_RESULT (*Fn)(ZML_HANDLE, int32_t, const int32_t*, double, double, double, double)>
PyObject* move_circle2(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    int32_t count = 0;
    PyObject* axes = nullptr;
    double mid1 = 0, mid2 = 0, end1 = 0, end2 = 0;
    if (!parse(args, handle, count, axes, mid1, mid2, end1, end2)) return nullptr;
    Sequence<int32_t> axis;
    if (!axis.load(axes, count)) return nullptr;
    return PyLong_FromLong(Fn(handle, count, axis.data, mid1, mid2, end1, end2));
}
PyObject* sdo_download(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    uint16_t slave = 0, index = 0;
    uint8_t sub_index = 0;
    uint32_t length = 0;
    PyObject* data = nullptr;
    if (!parse(args, handle, slave, index, sub_index, length, data)) return nullptr;
    Py_buffer buffer = {};
    if (PyObject_GetBuffer(data, &buffer, PyBUF_CONTIG_RO) != 0) return nullptr;
    if (buffer.len != length) {
        PyBuffer_Release(&buffer);
        PyErr_SetString(PyExc_ValueError, "buffer length must equal data_len");
        return nullptr;
    }
    const ZML_RESULT ret = zml_ecat_coe_sdo_download(handle, slave, index, sub_index,
                                                   length, buffer.buf);
    PyBuffer_Release(&buffer);
    return PyLong_FromLong(ret);
}
PyObject* sdo_upload(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    uint16_t slave = 0, index = 0;
    uint8_t sub_index = 0;
    uint32_t length = 0, word = 0;
    if (!parse(args, handle, slave, index, sub_index, length)) return nullptr;
    // The existing C API accepts only 1, 2 or 4 bytes and validates before copying.
    const ZML_RESULT ret = zml_ecat_coe_sdo_upload(handle, slave, index, sub_index, &length, &word);
    PyObject* data = ret == 0 ? PyBytes_FromStringAndSize(reinterpret_cast<char*>(&word), length)
                             : none();
    return data ? Py_BuildValue("(iIN)", ret, length, data) : nullptr;
}
PyObject* command_results(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    uint32_t length = 0;
    if (!parse(args, handle, length)) return nullptr;
    ZML_CmdResultInfo* buffer = length ? PyMem_New(ZML_CmdResultInfo, length) : nullptr;
    if (length && !buffer) return PyErr_NoMemory();
    // Preserve one-call capacity semantics and the C API's returned inout_len on every result.
    const ZML_RESULT ret = zml_get_cmd_results(handle, buffer, &length);
    PyObject* results = ret == 0 ? PyList_New(length) : none();
    if (results && ret == 0) {
        for (uint32_t i = 0; i < length; ++i) {
            PyObject* item = encode(buffer[i]);
            if (!item) { Py_CLEAR(results); break; }
            PyList_SET_ITEM(results, i, item);
        }
    }
    PyMem_Free(buffer);
    return results ? Py_BuildValue("(iNI)", ret, results, length) : nullptr;
}
PyObject* io_metadata(PyObject*, PyObject* args) {
    ZML_HANDLE handle = nullptr;
    uint32_t capacity = 0, count = 0;
    if (!parse(args, handle, capacity)) return nullptr;
    ZML_IoMetadataEntry* buffer = capacity ? PyMem_New(ZML_IoMetadataEntry, capacity) : nullptr;
    if (capacity && !buffer) return PyErr_NoMemory();
    const ZML_RESULT ret = zml_get_io_metadata(handle, buffer, capacity, &count);
    PyObject* entries = ret == 0 ? PyList_New(count) : none();
    if (entries && ret == 0) {
        for (uint32_t i = 0; i < count; ++i) {
            PyObject* item = encode(buffer[i]);
            if (!item) { Py_CLEAR(entries); break; }
            PyList_SET_ITEM(entries, i, item);
        }
    }
    PyMem_Free(buffer);
    return entries ? Py_BuildValue("(iNI)", ret, entries, count) : nullptr;
}

#define BIND(fn) {#fn, Binding<decltype(&fn)>::call<fn>, METH_VARARGS, #fn " (public RPC API)."}
#define CUSTOM(fn, impl) {#fn, impl, METH_VARARGS, #fn " (public RPC API)."}
PyMethodDef methods[] = {
    CUSTOM(zml_open_eth, (open_handle<const char*, zml_open_eth>)),
    CUSTOM(zml_open_pci, (open_handle<uint32_t, zml_open_pci>)),
    CUSTOM(zml_open, (open_handle<const char*, zml_open>)),
    CUSTOM(zml_close, close_handle),
    BIND(zml_ping),
    BIND(zml_mailbox_ping),
    BIND(zml_download_file),
    BIND(zml_upload_file),
    BIND(zml_download_eni),
    BIND(zml_download_syscfg),
    BIND(zml_upload_eni),
    BIND(zml_upload_syscfg),
    BIND(zml_ecat_start),
    BIND(zml_ecat_stop),
    BIND(zml_get_version_info),
    BIND(zml_get_about),
    BIND(zml_get_controller_info),
    BIND(zml_read_di),
    BIND(zml_read_do),
    BIND(zml_write_do),
    BIND(zml_read_ai),
    BIND(zml_read_ao),
    BIND(zml_write_ao),
    BIND(zml_write_virtual_di),
    BIND(zml_write_virtual_ai),
    BIND(zml_ecat_bus_is_running),
    BIND(zml_ecat_request_master_state),
    BIND(zml_ecat_get_master_req_state),
    BIND(zml_ecat_get_master_state),
    CUSTOM(zml_ecat_coe_sdo_download, sdo_download),
    CUSTOM(zml_ecat_coe_sdo_upload, sdo_upload),
    BIND(zml_ecat_get_slave_count),
    BIND(zml_ecat_get_active_slave_count),
    BIND(zml_ecat_get_slave_state),
    BIND(zml_ecat_clear_slave_state_error),
    BIND(zml_ecat_pi_enable_set),
    BIND(zml_ecat_pi_enable_get),
    BIND(zml_ax_power_on),
    BIND(zml_ax_power_off),
    BIND(zml_ax_set_enable),
    BIND(zml_ax_get_enable),
    BIND(zml_ax_set_units),
    BIND(zml_ax_get_units),
    BIND(zml_ax_read_state),
    BIND(zml_ax_read_set_position),
    BIND(zml_ax_read_act_position),
    BIND(zml_ax_read_sw_limit_negative),
    BIND(zml_ax_read_sw_limit_enable),
    BIND(zml_ax_read_sw_limit_positive),
    BIND(zml_ax_read_units_num),
    BIND(zml_ax_read_units_den),
    BIND(zml_ax_set_velocity),
    BIND(zml_ax_get_speed),
    BIND(zml_ax_set_accel),
    BIND(zml_ax_get_accel),
    BIND(zml_ax_get_decel),
    BIND(zml_ax_get_jerk),
    BIND(zml_ax_set_decel),
    BIND(zml_ax_set_jerk),
    BIND(zml_ax_set_position),
    BIND(zml_ax_stop),
    BIND(zml_ax_halt),
    BIND(zml_ax_reset),
    BIND(zml_ax_move_rel),
    BIND(zml_ax_moveadd),
    BIND(zml_ax_move_abs),
    BIND(zml_ax_move_velocity),
    BIND(zml_ax_forward),
    BIND(zml_ax_reverse),
    BIND(zml_ax_home_start),
    BIND(zml_ax_home_get_status),
    BIND(zml_ax_home_stop),
    BIND(zml_set_master_axis),
    BIND(zml_gear_in),
    BIND(zml_gear_out),
    CUSTOM(zml_moveabs, move_axes<zml_moveabs>),
    CUSTOM(zml_moverel, move_axes<zml_moverel>),
    BIND(zml_group_select),
    CUSTOM(zml_group_setaxes, group_axes<zml_group_setaxes>),
    CUSTOM(zml_group_enable, group_axes<zml_group_enable>),
    CUSTOM(zml_group_disable, group_axes<zml_group_disable>),
    CUSTOM(zml_group_reset, group_axes<zml_group_reset>),
    BIND(zml_execute_command),
    // Typed read-parameter and sleep sync, added by GLM-5.3 (opencode), 2026-10-23.
    BIND(zml_ax_read_parameter),
    BIND(zml_ax_write_parameter),
    BIND(zml_set_flow_config),
    BIND(zml_get_flow_status),
    BIND(zml_get_flow_config),
    BIND(zml_ax_get_state),
    BIND(zml_ax_get_state_ex),
    CUSTOM(zml_get_cmd_results, command_results),
    CUSTOM(zml_get_io_metadata, io_metadata),
    BIND(zml_ax_set_fastdecel),
    BIND(zml_ax_set_direction),
    BIND(zml_ax_set_buffermode),
    BIND(zml_ax_set_speed_ratio),
    BIND(zml_get_connection_state),
    BIND(zml_wait_until_connection_ready),
    BIND(zml_ax_wait_ticks),
    BIND(zml_ax_wait_seconds),
    BIND(zml_ax_wait_idle),
    BIND(zml_sleep_ms),
    BIND(zml_ax_home),
    BIND(zml_ax_homing),
    BIND(zml_ax_datum),
    CUSTOM(zml_movecirc, move_circle<zml_movecirc>),
    CUSTOM(zml_movecircabs, move_circle<zml_movecircabs>),
    CUSTOM(zml_movecirc2, move_circle2<zml_movecirc2>),
    CUSTOM(zml_movecirc2abs, move_circle2<zml_movecirc2abs>),
    {nullptr, nullptr, 0, nullptr}
};
#undef BIND
#undef CUSTOM

PyModuleDef module = {PyModuleDef_HEAD_INIT, "zmc_rpc",
                     "Python bindings for the public zml_* RPC C API.", -1, methods,
                     nullptr, nullptr, nullptr, nullptr};
}  // namespace

PyMODINIT_FUNC PyInit_zmc_rpc(void) {
    handle_type.tp_name = "zmc_rpc.ZML_HANDLE";
    handle_type.tp_basicsize = sizeof(PyZmlHandle);
    handle_type.tp_flags = Py_TPFLAGS_DEFAULT;
    handle_type.tp_doc = "Opaque RPC handle; created only by zml_open*.";
    handle_type.tp_new = handle_new;
    handle_type.tp_dealloc = reinterpret_cast<destructor>(handle_dealloc);
    if (PyType_Ready(&handle_type) < 0) return nullptr;
    PyObject* result = PyModule_Create(&module);
    if (!result) return nullptr;
    Py_INCREF(&handle_type);
    if (PyModule_AddObject(result, "ZML_HANDLE", reinterpret_cast<PyObject*>(&handle_type)) < 0) {
        Py_DECREF(&handle_type);
        Py_DECREF(result);
        return nullptr;
    }
#define CONSTANT(name) if (PyModule_AddIntConstant(result, #name, name) < 0) { \
    Py_DECREF(result); return nullptr; }
    CONSTANT(ZML_HOME_STATUS_IDLE)
    CONSTANT(ZML_HOME_STATUS_BUSY)
    CONSTANT(ZML_HOME_STATUS_DONE)
    CONSTANT(ZML_HOME_STATUS_ERROR)
    CONSTANT(ZML_HOME_STATUS_ABORTED)
    CONSTANT(ZML_VERSION_INFO_MAX)
    CONSTANT(ZML_ABOUT_INFO_MAX)
    CONSTANT(ZML_CONTROLLER_INFO_MAX)
#undef CONSTANT
    return result;
}
