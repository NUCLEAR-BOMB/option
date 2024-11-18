#include <opt/option.hpp>
#include <optional>
#include <array>
#include <cstdint>

//$ @option_int_assign:
//$ mov dword ptr [rdi], 0x2
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret

//$ @option_int_assign {msvc,clang-cl}:
//$ mov dword ptr [rcx], 0x2
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret
void option_int_assign(opt::option<int>* a) {
    *a = 2;
}

//$ @option_int_assign_option:
//$ movzx eax, byte ptr [rsi + 0x4]
//$ mov byte ptr [rdi + 0x4], al
//$ mov eax, dword ptr [rsi]
//$ mov dword ptr [rdi], eax
//$ ret

//$ @option_int_assign_option {clang <15}:
//$ mov al, byte ptr [rsi + 0x4]
//$ mov byte ptr [rdi + 0x4], al
//$ mov eax, dword ptr [rsi]
//$ mov dword ptr [rdi], eax
//$ ret

//$ @option_int_assign_option {gcc}:
//$ mov eax, dword ptr [rsi]
//$ mov dword ptr [rdi], eax
//$ movzx eax, byte ptr [rsi + 0x4]
//$ mov byte ptr [rdi + 0x4], al
//$ ret

//$ @option_int_assign_option {msvc,clang-cl}:
//$ mov rax, qword ptr [rdx]
//$ mov qword ptr [rcx], rax
//$ ret
void option_int_assign_option(opt::option<int>* a, opt::option<int>* b) {
    *a = *b;
}

//$ @option_int_return:
//$ movabs rax, 0x100000002
//$ ret

// ???
//$ @option_int_return {gcc}:
//$ mov dword ptr [rsp - 0x8], 0x2
//$ mov byte ptr [rsp - 0x4], 0x1
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

// rcx = pointer to a caller allocated storage for returned `opt::option`
//$ @option_int_return {msvc}:
//$ mov dword ptr [rcx], 0x2
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret

//$ @option_int_return {clang-cl}:
//$ mov rax, rcx
//$ mov dword ptr [rcx], 0x2
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret
opt::option<int> option_int_return() {
    return 2;
}

//$ @option_float_assign:
//$ mov dword ptr [rdi], 0x3f800000
//$ ret

//$ @option_float_assign {msvc,clang-cl}:
//$ mov dword ptr [rcx], 0x3f800000
//$ ret
void option_float_assign(opt::option<float>* a) {
    *a = 1.f;
}

//$ @optional_float_assign:
//$ mov dword ptr [rdi], 0x3f800000
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret

//$ @optional_float_assign {clang & old-libstdcpp <12.0}:
//$ movabs rax, 0x13f800000
//$ mov qword ptr [rdi], rax
//$ ret

//$ @optional_float_assign {clang-cl}:
//$ mov dword ptr [rcx], 0x3f800000
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret

//$ @optional_float_assign {msvc >19.29.30157.0}:
//$ mov dword ptr [rsp + 0x8], 0x3f800000
//$ mov byte ptr [rsp + 0xc], 0x1
//$ mov rax, qword ptr [rsp + 0x8]
//$ mov qword ptr [rcx], rax
//$ ret

//$ @optional_float_assign {msvc <=19.29.30157.0}:
//$ movss xmm0, dword ptr <void __cdecl optional_float_assign(class std::optional<float> *)+0x8>
//$ movss dword ptr [rsp + 0x8], xmm0
//$ mov byte ptr [rsp + 0xc], 0x1
//$ mov rax, qword ptr [rsp + 0x8]
//$ mov qword ptr [rcx], rax
//$ ret
void optional_float_assign(std::optional<float>* a) {
    *a = 1.f;
}

//$ @option_float_return:
//$ mov eax, 0x3f800000
//$ ret

//$ @option_float_return {clang-cl}:
//$ mov rax, rcx
//$ mov dword ptr [rcx], 0x3f800000
//$ ret

//$ @option_float_return {msvc}:
//$ mov dword ptr [rcx], 0x3f800000
//$ mov rax, rcx
//$ ret
opt::option<float> option_float_return() {
    return 1.f;
}

//$ @optional_float_return:
//$ movabs rax, 0x13f800000
//$ ret

//$ @optional_float_return {gcc}:
//$ mov dword ptr [rsp - 0x8], 0x3f800000
//$ mov byte ptr [rsp - 0x4], 0x1
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @optional_float_return {clang-cl}:
//$ mov rax, rcx
//$ mov dword ptr [rcx], 0x3f800000
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret

//$ @optional_float_return {msvc}:
//$ mov dword ptr [rcx], 0x3f800000
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret
std::optional<float> optional_float_return() {
    return 1.f;
}

//$ @option_double_return:
//$ movabs rax, 0x3ff0000000000000
//$ ret

//$ @option_double_return {msvc}:
//$ movabs rax, 0x3ff0000000000000
//$ mov qword ptr [rcx], rax
//$ mov rax, rcx
//$ ret

//$ @option_double_return {clang-cl}:
//$ mov rax, rcx
//$ movabs rcx, 0x3ff0000000000000
//$ mov qword ptr [rax], rcx
//$ ret
opt::option<double> option_double_return() {
    return 1.;
}

//$ @optional_double_return:
//$ movsd xmm0, qword ptr <optional_double_return()+0x8>
//$ mov al, 0x1
//$ ret

//$ @optional_double_return {clang & libcpp}:
//$ movabs rax, 0x3ff0000000000000
//$ mov dl, 0x1
//$ ret

//$ @optional_double_return {gcc 14, gcc 11, gcc 12}:
//$ mov byte ptr [rsp - 0x10], 0x1
//$ movsd xmm0, qword ptr <optional_double_return()+0x11>
//$ mov rax, qword ptr [rsp - 0x10]
//$ ret

//$ @optional_double_return {gcc 13}:
//$ mov byte ptr [rsp - 0x10], 0x1
//$ movsd xmm0, qword ptr <optional_double_return()+0xd>
//$ mov rax, qword ptr [rsp - 0x10]
//$ ret

//$ @optional_double_return {msvc}:
//$ movabs rax, 0x3ff0000000000000
//$ mov byte ptr [rcx + 0x8], 0x1
//$ mov qword ptr [rcx], rax
//$ mov rax, rcx
//$ ret

//$ @optional_double_return {clang-cl}:
//$ mov rax, rcx
//$ movabs rcx, 0x3ff0000000000000
//$ mov qword ptr [rax], rcx
//$ mov byte ptr [rax + 0x8], 0x1
//$ ret
std::optional<double> optional_double_return() {
    return 1.;
}

enum class enum_uint16 : std::uint16_t { a = 2 };

//$ @option_enum_uint16_return:
//$ mov ax, 0x2
//$ ret

//$ @option_enum_uint16_return {gcc}:
//$ mov eax, 0x2
//$ ret

//$ @option_enum_uint16_return {msvc >19.29.30157.0}:
//$ mov word ptr [rcx], 0x2
//$ mov rax, rcx
//$ ret

//$ @option_enum_uint16_return {msvc <=19.29.30157.0}:
//$ mov eax, 0x2
//$ mov word ptr [rcx], ax
//$ mov rax, rcx
//$ ret

//$ @option_enum_uint16_return {clang-cl}:
//$ mov rax, rcx
//$ mov word ptr [rcx], 0x2
//$ ret
opt::option<enum_uint16> option_enum_uint16_return() {
    return enum_uint16::a;
}

//$ @optional_enum_uint16_return:
//$ mov eax, 0x10002
//$ ret

//$ @optional_enum_uint16_return {gcc}:
//$ mov eax, 0x2
//$ mov byte ptr [rsp - 0x2], 0x1
//$ mov word ptr [rsp - 0x4], ax
//$ mov eax, dword ptr [rsp - 0x4]
//$ ret

//$ @optional_enum_uint16_return {msvc >19.29.30157.0}:
//$ mov word ptr [rcx], 0x2
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x2], 0x1
//$ ret

//$ @optional_enum_uint16_return {msvc <=19.29.30157.0}:
//$ mov eax, 0x2
//$ mov byte ptr [rcx + 0x2], 0x1
//$ mov word ptr [rcx], ax
//$ mov rax, rcx
//$ ret

//$ @optional_enum_uint16_return {clang-cl}:
//$ mov rax, rcx
//$ mov word ptr [rcx], 0x2
//$ mov byte ptr [rcx + 0x2], 0x1
//$ ret
std::optional<enum_uint16> optional_enum_uint16_return() {
    return enum_uint16::a;
}

//$ @option_float_reset:
//$ mov dword ptr [rdi], 0xffbf69af
//$ ret

//$ @option_float_reset {msvc,clang-cl}:
//$ mov dword ptr [rcx], 0xffbf69af
//$ ret
void option_float_reset(opt::option<float>* a) {
    a->reset();
}

//$ @optional_float_reset:
//$ mov byte ptr [rdi + 0x4], 0x0
//$ ret

//$ @optional_float_reset {clang & libcpp,clang & old-libstdcpp,gcc <13.0,gcc & old-libstdcpp,icx}:
//$ cmp byte ptr [rdi + 0x4], 0x0
//$ je <L0>
//$ mov byte ptr [rdi + 0x4], 0x0
//$ <L0>:
//$ ret

//$ @optional_float_reset {msvc,clang-cl}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
void optional_float_reset(std::optional<float>* a) {
    a->reset();
}

//$ @option_float_emplace:
//$ mov dword ptr [rdi], 0x3f800000
//$ ret

//$ @option_float_emplace {msvc,clang-cl}:
//$ mov dword ptr [rcx], 0x3f800000
//$ ret
void option_float_emplace(opt::option<float>* a) {
    a->emplace(1.f);
}

//$ @optional_float_emplace:
//$ mov dword ptr [rdi], 0x3f800000
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret

//$ @optional_float_emplace {clang & old-libstdcpp <12.0,clang & libcpp <12.0}:
//$ cmp byte ptr [rdi + 0x4], 0x0
//$ je <L0>
//$ mov byte ptr [rdi + 0x4], 0x0
//$ <L0>:
//$ mov dword ptr [rdi], 0x3f800000
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret

//$ @optional_float_emplace {msvc,clang-cl}:
//$ mov dword ptr [rcx], 0x3f800000
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret
void optional_float_emplace(std::optional<float>* a) {
    a->emplace(1.f);
}

//$ @option_bool_reset:
//$ mov byte ptr [rdi], 0x2
//$ ret

//$ @option_bool_reset {msvc,clang-cl}:
//$ mov byte ptr [rcx], 0x2
//$ ret
void option_bool_reset(opt::option<bool>* a) {
    a->reset();
}

//$ @optional_bool_reset:
//$ mov byte ptr [rdi + 0x1], 0x0
//$ ret

//$ @optional_bool_reset {clang & libcpp,clang & old-libstdcpp,gcc <13.0,gcc & old-libstdcpp,icx}:
//$ cmp byte ptr [rdi + 0x1], 0x0
//$ je <L0>
//$ mov byte ptr [rdi + 0x1], 0x0
//$ <L0>:
//$ ret

//$ @optional_bool_reset {msvc,clang-cl}:
//$ mov byte ptr [rcx + 0x1], 0x0
//$ ret
void optional_bool_reset(std::optional<bool>* a) {
    a->reset();
}

//$ @option_int_return_default_ctor:
//$ xor eax, eax
//$ ret

// ???
//$ @option_int_return_default_ctor {gcc}:
//$ mov byte ptr [rsp - 0x4], 0x0
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @option_int_return_default_ctor {msvc}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ mov rax, rcx
//$ ret

//$ @option_int_return_default_ctor {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
opt::option<int> option_int_return_default_ctor() {
    return {};
}

//$ @option_int_return_none:
//$ xor eax, eax
//$ ret

// ???
//$ @option_int_return_none {gcc}:
//$ mov byte ptr [rsp - 0x4], 0x0
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @option_int_return_none {msvc}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ mov rax, rcx
//$ ret

//$ @option_int_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
opt::option<int> option_int_return_none() {
    return opt::none;
}

//$ @optional_int_return_default_ctor:
//$ xor eax, eax
//$ ret

//$ @optional_int_return_default_ctor {gcc}:
//$ mov byte ptr [rsp - 0x4], 0x0
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @optional_int_return_default_ctor {msvc}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_int_return_default_ctor {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
std::optional<int> optional_int_return_default_ctor() {
    return {};
}

//$ @optional_int_return_none:
//$ xor eax, eax
//$ ret

//$ @optional_int_return_none {gcc}:
//$ mov byte ptr [rsp - 0x4], 0x0
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @optional_int_return_none {msvc}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_int_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
std::optional<int> optional_int_return_none() {
    return std::nullopt;
}

//$ @option_int64_return_none:
//$ xor edx, edx
//$ ret

// ???
//$ @option_int64_return_none {gcc}:
//$ mov byte ptr [rsp - 0x10], 0x0
//$ mov rax, qword ptr [rsp - 0x18]
//$ mov rdx, qword ptr [rsp - 0x10]
//$ ret

//$ @option_int64_return_none {msvc}:
//$ mov byte ptr [rcx + 0x8], 0x0
//$ mov rax, rcx
//$ ret

//$ @option_int64_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x8], 0x0
//$ ret
opt::option<std::int64_t> option_int64_return_none() {
    return opt::none;
}

//$ @option_int_1024_return_none:
//$ mov rax, rdi
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @option_int_1024_return_none {gcc}:
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ mov rax, rdi
//$ ret

//$ @option_int_1024_return_none {msvc}:
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ mov rax, rcx
//$ ret

//$ @option_int_1024_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ ret
opt::option<std::array<int, 1024>> option_int_1024_return_none() {
    return opt::none;
}

//$ @optional_int_1024_return_none:
//$ mov rax, rdi
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @optional_int_1024_return_none {clang & libcpp}:
//$ mov rax, rdi
//$ mov byte ptr [rdi], 0x0
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @optional_int_1024_return_none {gcc}:
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ mov rax, rdi
//$ ret

//$ @optional_int_1024_return_none {msvc}:
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_int_1024_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ ret
std::optional<std::array<int, 1024>> optional_int_1024_return_none() {
    return std::nullopt;
}

//$ @option_float_1024_return_none:
//$ mov rax, rdi
//$ mov dword ptr [rdi], 0xffbf69af
//$ ret

//$ @option_float_1024_return_none {gcc}:
//$ mov dword ptr [rdi], 0xffbf69af
//$ mov rax, rdi
//$ ret

//$ @option_float_1024_return_none {msvc}:
//$ mov dword ptr [rcx], 0xffbf69af
//$ mov rax, rcx
//$ ret

//$ @option_float_1024_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov dword ptr [rcx], 0xffbf69af
//$ ret
opt::option<std::array<float, 1024>> option_float_1024_return_none() {
    return opt::none;
}

//$ @option_bool_return_none:
//$ mov al, 0x2
//$ ret

//$ @option_bool_return_none {gcc}:
//$ mov eax, 0x2
//$ ret

//$ @option_bool_return_none {msvc}:
//$ mov byte ptr [rcx], 0x2
//$ mov rax, rcx
//$ ret

//$ @option_bool_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx], 0x2
//$ ret
opt::option<bool> option_bool_return_none() {
    return opt::none;
}

//$ @optional_bool_return_none:
//$ xor eax, eax
//$ ret

//$ @optional_bool_return_none {gcc & old-libstdcpp <12}:
//$ xor eax, eax
//$ mov ah, 0x0
//$ ret

//$ @optional_bool_return_none {msvc}:
//$ mov byte ptr [rcx + 0x1], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_bool_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x1], 0x0
//$ ret
std::optional<bool> optional_bool_return_none() {
    return std::nullopt;
}

//$ @option_float_return_none:
//$ mov eax, 0xffbf69af
//$ ret

//$ @option_float_return_none {msvc}:
//$ mov dword ptr [rcx], 0xffbf69af
//$ mov rax, rcx
//$ ret

//$ @option_float_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov dword ptr [rcx], 0xffbf69af
//$ ret
opt::option<float> option_float_return_none() {
    return opt::none;
}

//$ @optional_float_return_none:
//$ xor eax, eax
//$ ret

//$ @optional_float_return_none {gcc}:
//$ mov byte ptr [rsp - 0x4], 0x0
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

//$ @optional_float_return_none {msvc}:
//$ mov byte ptr [rcx + 0x4], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_float_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x0
//$ ret
std::optional<float> optional_float_return_none() {
    return std::nullopt;
}

//$ @option_double_return_none:
//$ movabs rax, -0x93860aa4f7671
//$ ret

//$ @option_double_return_none {msvc}:
//$ movabs rax, -0x93860aa4f7671
//$ mov qword ptr [rcx], rax
//$ mov rax, rcx
//$ ret

//$ @option_double_return_none {clang-cl}:
//$ mov rax, rcx
//$ movabs rcx, -0x93860aa4f7671
//$ mov qword ptr [rax], rcx
//$ ret
opt::option<double> option_double_return_none() {
    return opt::none;
}

//$ @optional_double_return_none:
//$ xor eax, eax
//$ ret

//$ @optional_double_return_none {clang & libcpp}:
//$ xor eax, eax
//$ xor edx, edx
//$ ret

//$ @optional_double_return_none {gcc}:
//$ mov byte ptr [rsp - 0x10], 0x0
//$ movsd xmm0, qword ptr [rsp - 0x18]
//$ mov rax, qword ptr [rsp - 0x10]
//$ ret

//$ @optional_double_return_none {msvc}:
//$ mov byte ptr [rcx + 0x8], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_double_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x8], 0x0
//$ ret
std::optional<double> optional_double_return_none() {
    return std::nullopt;
}

//$ @option_bool_has_value:
//$ cmp byte ptr [rdi], 0x2
//$ setne al
//$ ret

//$ @option_bool_has_value {msvc,clang-cl}:
//$ cmp byte ptr [rcx], 0x2
//$ setne al
//$ ret
bool option_bool_has_value(opt::option<bool>* a) {
    return a->has_value();
}

//$ @optional_bool_has_value:
//$ movzx eax, byte ptr [rdi + 0x1]
//$ ret

//$ @optional_bool_has_value {clang <15}:
//$ mov al, byte ptr [rdi + 0x1]
//$ ret

//$ @optional_bool_has_value {msvc,clang-cl}:
//$ movzx eax, byte ptr [rcx + 0x1]
//$ ret
bool optional_bool_has_value(std::optional<bool>* a) {
    return a->has_value();
}

//$ @option_float_has_value:
//$ cmp dword ptr [rdi], 0xffbf69af
//$ setne al
//$ ret

//$ @option_float_has_value {msvc}:
//$ mov eax, dword ptr [rcx]
//$ add eax, 0x409651
//$ setne al
//$ ret

//$ @option_float_has_value {clang-cl}:
//$ cmp dword ptr [rcx], 0xffbf69af
//$ setne al
//$ ret
bool option_float_has_value(opt::option<float>* a) {
    return a->has_value();
}

//$ @optional_float_has_value:
//$ movzx eax, byte ptr [rdi + 0x4]
//$ ret

//$ @optional_float_has_value {clang <15}:
//$ mov al, byte ptr [rdi + 0x4]
//$ ret

//$ @optional_float_has_value {msvc,clang-cl}:
//$ movzx eax, byte ptr [rcx + 0x4]
//$ ret
bool optional_float_has_value(std::optional<float>* a) {
    return a->has_value();
}

//$ @option_double_has_value:
//$ movabs rax, -0x93860aa4f7671
//$ cmp qword ptr [rdi], rax
//$ setne al
//$ ret

//$ @option_double_has_value {msvc}:
//$ mov rax, qword ptr [rcx]
//$ movabs rcx, 0x93860aa4f7671
//$ add rax, rcx
//$ setne al
//$ ret

//$ @option_double_has_value {clang-cl}:
//$ movabs rax, -0x93860aa4f7671
//$ cmp qword ptr [rcx], rax
//$ setne al
//$ ret
bool option_double_has_value(opt::option<double>* a) {
    return a->has_value();
}

//$ @optional_double_has_value:
//$ movzx eax, byte ptr [rdi + 0x8]
//$ ret

//$ @optional_double_has_value {clang <15}:
//$ mov al, byte ptr [rdi + 0x8]
//$ ret

//$ @optional_double_has_value {msvc,clang-cl}:
//$ movzx eax, byte ptr [rcx + 0x8]
//$ ret
bool optional_double_has_value(std::optional<double>* a) {
    return a->has_value();
}

//$ @option_option_bool_has_value:
//$ cmp byte ptr [rdi], 0x3
//$ setne al
//$ ret

// ???
//$ @option_option_bool_has_value {msvc}:
//$ movzx eax, byte ptr [rcx]
//$ sub al, 0x2
//$ cmp al, 0x1
//$ setne al
//$ ret

//$ @option_option_bool_has_value {clang-cl}:
//$ cmp byte ptr [rcx], 0x3
//$ setne al
//$ ret
bool option_option_bool_has_value(opt::option<opt::option<bool>>* a) {
    return a->has_value();
}

//$ @option_option_bool_nested_has_value:
//$ movzx eax, byte ptr [rdi]
//$ add al, -0x4
//$ cmp al, -0x2
//$ setb al
//$ ret

//$ @option_option_bool_nested_has_value {clang 11..<15}:
//$ mov al, byte ptr [rdi]
//$ and al, -0x2
//$ cmp al, 0x2
//$ setne al
//$ ret

//$ @option_option_bool_nested_has_value {clang <11}:
//$ mov al, byte ptr [rdi]
//$ or al, 0x1
//$ cmp al, 0x3
//$ setne al
//$ ret

// ???
//$ @option_option_bool_nested_has_value {gcc 13, gcc 14}:
//$ movzx edx, byte ptr [rdi]
//$ xor eax, eax
//$ cmp dl, 0x3
//$ je <L0>
//$ cmp dl, 0x2
//$ setne al
//$ <L0>:
//$ ret

//$ @option_option_bool_nested_has_value {gcc <13}:
//$ movzx eax, byte ptr [rdi]
//$ sub eax, 0x2
//$ cmp al, 0x1
//$ seta al
//$ ret

//$ @option_option_bool_nested_has_value {msvc}:
//$ movzx eax, byte ptr [rcx]
//$ sub al, 0x2
//$ cmp al, 0x1
//$ je <L0>
//$ test al, al
//$ je <L0>
//$ mov al, 0x1
//$ ret
//$ <L0>:
//$ xor al, al
//$ ret

//$ @option_option_bool_nested_has_value {clang-cl}:
//$ movzx eax, byte ptr [rcx]
//$ add al, -0x4
//$ cmp al, -0x2
//$ setb al
//$ ret
bool option_option_bool_nested_has_value(opt::option<opt::option<bool>>* a) {
    return a->has_value() && (*a)->has_value();
}
