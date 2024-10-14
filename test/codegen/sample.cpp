#include <opt/option.hpp>
#include <optional>
#include <array>
#include <cstdint>

//$ @option_int_assign:
//$ mov dword ptr [rdi], 0x2
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret

//$ @option_int_assign {msvc}:
//$ mov dword ptr [rsp + 0x8], 0x2
//$ mov byte ptr [rsp + 0xc], 0x1
//$ mov rax, qword ptr [rsp + 0x8]
//$ mov qword ptr [rcx], rax
//$ ret

//$ @option_int_assign {clang-cl}:
//$ movabs rax, 0x100000002
//$ mov qword ptr [rcx], rax
//$ ret
void option_int_assign(opt::option<int>* a) {
    *a = 2;
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

//$ @optional_float_assign {msvc}:
//$ mov dword ptr [rsp + 0x8], 0x3f800000
//$ mov byte ptr [rsp + 0xc], 0x1
//$ mov rax, qword ptr [rsp + 0x8]
//$ mov qword ptr [rcx], rax
//$ ret
void optional_float_assign(std::optional<float>* a) {
    *a = 1.f;
}

//$ @option_float_return:
//$ movss xmm0, dword ptr <option_float_return()+0x8>
//$ ret

//$ @option_float_return {gcc}:
//$ mov eax, 0x3f800000
//$ movd xmm0, eax
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
//$ movsd xmm0, qword ptr <option_double_return()+0x8>
//$ ret

//$ @option_double_return {gcc}:
//$ movabs rax, 0x3ff0000000000000
//$ movq xmm0, rax
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

//$ @optional_double_return {gcc}:
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

//$ @option_enum_uint16_return {msvc}:
//$ mov word ptr [rcx], 0x2
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

//$ @optional_enum_uint16_return {msvc}:
//$ mov word ptr [rcx], 0x2
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x2], 0x1
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

//$ @option_big_return_none:
//$ mov rax, rdi
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @option_big_return_none {gcc}:
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ mov rax, rdi
//$ ret

//$ @option_big_return_none {msvc}:
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ mov rax, rcx
//$ ret

//$ @option_big_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ ret
opt::option<std::array<int, 1024>> option_big_return_none() {
    return opt::none;
}

//$ @optional_big_return_none:
//$ mov rax, rdi
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @optional_big_return_none {clang & libcpp}:
//$ mov rax, rdi
//$ mov byte ptr [rdi], 0x0
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ ret

//$ @optional_big_return_none {gcc}:
//$ mov byte ptr [rdi + 0x1000], 0x0
//$ mov rax, rdi
//$ ret

//$ @optional_big_return_none {msvc}:
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ mov rax, rcx
//$ ret

//$ @optional_big_return_none {clang-cl}:
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x1000], 0x0
//$ ret
std::optional<std::array<int, 1024>> optional_big_return_none() {
    return std::nullopt;
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

// ???
//$ @option_float_return_none:
//$ movss xmm0, dword ptr <option_float_return_none()+0x8>
//$ ret

//$ @option_float_return_none {gcc}:
//$ mov eax, 0xffbf69af
//$ movd xmm0, eax
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
//$ movsd xmm0, qword ptr <option_double_return_none()+0x8>
//$ ret

//$ @option_double_return_none {gcc}:
//$ movabs rax, -0x93860aa4f7671
//$ movq xmm0, rax
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
//$ setb al
//$ ret

//$ @option_bool_has_value {clang 10..<13}:
//$ mov al, byte ptr [rdi]
//$ lea ecx, [rax - 0x2]
//$ movzx ecx, cl
//$ cmp al, 0x1
//$ mov rax, -0x1
//$ cmova rax, rcx
//$ cmp rax, -0x1
//$ sete al
//$ ret

//$ @option_bool_has_value {clang <10}:
//$ mov al, byte ptr [rdi]
//$ add al, -0x2
//$ movzx eax, al
//$ mov rcx, -0x1
//$ cmovb rcx, rax
//$ cmp rcx, -0x1
//$ sete al
//$ ret

//$ @option_bool_has_value {gcc}:
//$ cmp byte ptr [rdi], 0x1
//$ setbe al
//$ ret

//$ @option_bool_has_value {msvc}:
//$ movzx eax, byte ptr [rcx]
//$ sub al, 0x2
//$ cmp al, -0x2
//$ setae al
//$ ret

//$ @option_bool_has_value {clang-cl}:
//$ cmp byte ptr [rcx], 0x2
//$ setb al
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
//$ mov eax, 0x409551
//$ add eax, dword ptr [rdi]
//$ cmp eax, 0xffffff00
//$ setb al
//$ ret

//$ @option_float_has_value {gcc}:
//$ mov eax, dword ptr [rdi]
//$ add eax, 0x409651
//$ cmp eax, 0xff
//$ seta al
//$ ret

//$ @option_float_has_value {msvc}:
//$ mov eax, dword ptr [rcx]
//$ add eax, 0x409651
//$ cmp eax, 0x100
//$ setae al
//$ ret

//$ @option_float_has_value {clang-cl}:
//$ mov eax, 0x409551
//$ add eax, dword ptr [rcx]
//$ cmp eax, 0xffffff00
//$ setb al
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
//$ movabs rax, 0x93860aa4f7571
//$ add rax, qword ptr [rdi]
//$ cmp rax, -0x100
//$ setb al
//$ ret

//$ @option_double_has_value {clang 13..<14}:
//$ movabs rax, 0x93860aa4f7671
//$ add rax, qword ptr [rdi]
//$ cmp rax, 0x100
//$ setae al
//$ ret

//$ @option_double_has_value {clang 10..<13}:
//$ movabs rax, 0x93860aa4f7671
//$ add rax, qword ptr [rdi]
//$ cmp rax, 0x100
//$ mov rcx, -0x1
//$ cmovb rcx, rax
//$ cmp rcx, -0x1
//$ sete al
//$ ret

//$ @option_double_has_value {clang <10}:
//$ movabs rax, 0x93860aa4f7671
//$ add rax, qword ptr [rdi]
//$ cmp rax, 0xff
//$ seta al
//$ ret

//$ @option_double_has_value {gcc}:
//$ movabs rax, 0x93860aa4f7671
//$ add rax, qword ptr [rdi]
//$ cmp rax, 0xff
//$ seta al
//$ ret

//$ @option_double_has_value {msvc}:
//$ mov rax, qword ptr [rcx]
//$ movabs rcx, 0x93860aa4f7671
//$ add rax, rcx
//$ cmp rax, 0x100
//$ setae al
//$ ret

//$ @option_double_has_value {clang-cl}:
//$ movabs rax, 0x93860aa4f7571
//$ add rax, qword ptr [rcx]
//$ cmp rax, -0x100
//$ setb al
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
