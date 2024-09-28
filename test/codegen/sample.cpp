#include <opt/option.hpp>

//$ @opt_int_assign {clang,gcc,icx}:
//$ mov dword ptr [rdi], 0x2
//$ mov byte ptr [rdi + 0x4], 0x1
//$ ret
//$ *

//$ @opt_int_assign {msvc}:
//$ mov dword ptr [rsp + 0x8], 0x2
//$ mov byte ptr [rsp + 0xc], 0x1
//$ mov rax, qword ptr [rsp + 0x8]
//$ mov qword ptr [rcx], rax
//$ ret
void opt_int_assign(opt::option<int>* a) {
    *a = 2;
}

//$ @opt_int_return {clang,icx}:
//$ movabs rax, 0x100000002
//$ ret

// ???
//$ @opt_int_return {gcc}:
//$ mov dword ptr [rsp - 0x8], 0x2
//$ mov byte ptr [rsp - 0x4], 0x1
//$ mov rax, qword ptr [rsp - 0x8]
//$ ret

// rcx = pointer to a caller allocated storage for returned `opt::option`
//$ @opt_int_return {msvc}:
//$ mov dword ptr [rcx], 0x2
//$ mov rax, rcx
//$ mov byte ptr [rcx + 0x4], 0x1
//$ ret
opt::option<int> opt_int_return() {
    return 2;
}
