#include <vmlinux.h>

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>


SEC("uprobe/test_uprobe_func")
void BPF_UPROBE(test_uprobe_func) {
    
}




