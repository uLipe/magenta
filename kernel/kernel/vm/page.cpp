// Copyright 2016 The Fuchsia Authors
// Copyright (c) 2014 Travis Geiselbrecht
//
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file or at
// https://opensource.org/licenses/MIT

#include <kernel/vm/page.h>

#include <stdio.h>
#include <kernel/vm.h>

const char *page_state_to_string(unsigned int state) {
    switch (state) {
        case VM_PAGE_STATE_FREE: return "free";
        case VM_PAGE_STATE_ALLOC: return "alloc";
        case VM_PAGE_STATE_WIRED: return "wired";
        case VM_PAGE_STATE_HEAP: return "heap";
        case VM_PAGE_STATE_OBJECT: return "object";
        case VM_PAGE_STATE_MMU: return "mmu";
        default: return "unknown";
    }
}

void dump_page(const vm_page_t* page) {
    printf("page %p: address 0x%lx state %s flags 0x%x\n", page, vm_page_to_paddr(page),
           page_state_to_string(page->state), page->flags);
}


