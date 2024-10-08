#include <opt/option.hpp>
#include <optional>

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
//$ movss xmm0, dword ptr [rip]
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
