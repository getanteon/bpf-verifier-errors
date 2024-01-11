
#include <vmlinux.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>

struct msg_t {
   char message[12];
};

struct {
    __uint(type, BPF_MAP_TYPE_HASH);
    __uint(max_entries, 10240);
    __type(key, u64);
    __type(value, struct msg_t);
} id_name_map SEC(".maps");

SEC("ksyscall/execve")
int kprobe_exec(void *ctx)
{
   struct msg_t *p;
   
   u64 id = bpf_get_current_pid_tgid();

   p = bpf_map_lookup_elem(&id_name_map, &id);
   // NULL check
   if(p){
    char a = p->message[0];
    bpf_printk("%d", a);
   }

   return 0;
}


char __license[] SEC("license") = "Dual MIT/GPL";

