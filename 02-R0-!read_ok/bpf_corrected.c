#include <vmlinux.h>

#include <bpf/bpf_helpers.h>


SEC("uprobe/test_uprobe_func")
void BPF_UPROBE(test_uprobe_func) {

    // These func calls will initialize R0
    bpf_printk("Add these log lines"); // 4: R0_w=scalar()
    bpf_printk("So that");
    bpf_printk("R0 gets initialized"); // R0=scalar()
}


// SEC("uprobe/test_uprobe_func1")
// int BPF_UPROBE(test_uprobe_func1) {
//    return 0;
// }

char __license[] SEC("license") = "Dual MIT/GPL";



