#include <algorithm>
#include <cstdint>
#include <ctime>
#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace {

struct c_classic_context;
typedef struct c_classic_context c_classic_context_t;

template <class T, std::enable_if_t<!std::is_class<T>::value, int> = 0>
auto take_prop(void* handle, const char* property_name)
{
    using f_setter = void (*)(c_classic_context_t*, T);
    using f_getter = T (*)(c_classic_context_t*);
    std::string setter_fn_name = std::string("Set_") + property_name;
    std::string getter_fn_name = std::string("Get_") + property_name;

    auto setter = reinterpret_cast<f_setter>(dlsym(handle, setter_fn_name.data()));
    auto getter = reinterpret_cast<f_getter>(dlsym(handle, getter_fn_name.data()));
    return std::make_tuple(setter, getter);
}

template <class T, std::enable_if_t<std::is_same<T, std::string>::value, int> = 0>
auto take_prop(void* handle, const char* property_name)
{
    typedef void (*f_setter)(c_classic_context_t*, const char*, size_t);
    typedef size_t (*f_getter)(c_classic_context_t*, char*, size_t);
    std::string setter_fn_name = std::string("Set_") + property_name;
    std::string getter_fn_name = std::string("Get_") + property_name;

    auto setter = reinterpret_cast<f_setter>(dlsym(handle, setter_fn_name.data()));
    auto getter = reinterpret_cast<f_getter>(dlsym(handle, getter_fn_name.data()));
    return std::make_tuple(setter, getter);
}

template <class T, std::enable_if_t<std::is_same<T, std::vector<uint8_t>>::value, int> = 0>
auto take_prop(void* handle, const char* property_name)
{
    typedef void (*f_setter)(c_classic_context_t*, const uint8_t*, size_t);
    typedef size_t (*f_getter)(c_classic_context_t*, uint8_t*, size_t);
    std::string setter_fn_name = std::string("Set_") + property_name;
    std::string getter_fn_name = std::string("Get_") + property_name;

    auto setter = reinterpret_cast<f_setter>(dlsym(handle, setter_fn_name.data()));
    auto getter = reinterpret_cast<f_getter>(dlsym(handle, getter_fn_name.data()));
    return std::make_tuple(setter, getter);
}

auto take_method(void* handle, const char* method_name)
{
    typedef int (*f_c_classic_method)(c_classic_context_t*);
    return reinterpret_cast<f_c_classic_method>(dlsym(handle, method_name));
}

int dl_main(int argc, char* argv[])
{
    using std::cerr;
    using std::endl;

    if (argc < 2) {
        cerr << "usage: " << argv[0] << "classic_interface_library_path.dll/so/dylib"
             << "[connection uri]";
        cerr << "  missing required argument" << endl;
        return EXIT_FAILURE;
    }
    void* handle = dlopen(argv[1], RTLD_LAZY);
    if (!handle) {
        cerr << "unable to load library: " << argv[1];
        return EXIT_FAILURE;
    }

    typedef c_classic_context_t* (*f_c_classic_init)(const char*);
    typedef void (*f_c_classic_deinit)(c_classic_context_t*);

    auto init = reinterpret_cast<f_c_classic_init>(dlsym(handle, "c_classic_init"));
    auto deinit = reinterpret_cast<f_c_classic_deinit>(dlsym(handle, "c_classic_deinit"));

    auto [Set_ConnectionURI, Get_ConnectionURI] = take_prop<std::string>(handle, "ConnectionURI");
    auto [Set_Connected, Get_Connected] = take_prop<bool>(handle, "Connected");

    auto Connect = take_method(handle, "Connect");
    auto Beep = take_method(handle, "Beep");

    auto instance = init(nullptr);
    if (argc > 2) {
        Set_ConnectionURI(instance, argv[2], SIZE_MAX);
    }

    auto ret = Connect(instance);
    if (ret) {
        auto connected = Get_Connected(instance);
        if (!connected) {
            return EXIT_FAILURE;
        }
    }
    ret = Beep(instance);

    deinit(instance);
    dlclose(handle);
    return EXIT_SUCCESS;
}
} // namespace

int main(int argc, char* argv[])
{
    try {
        return dl_main(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "unknown error" << std::endl;
        return EXIT_FAILURE;
    }
}
