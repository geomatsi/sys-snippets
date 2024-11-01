#define _GNU_SOURCE

#include <sys/syscall.h>
#include <asm/hwprobe.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <stdio.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

long sys_riscv_hwprobe(struct riscv_hwprobe *pairs, size_t pairc,
		size_t cpuc, cpu_set_t *cpus, unsigned int flags) {
	return syscall(SYS_riscv_hwprobe, pairs, pairc, cpuc, cpus, flags);
}

void show_isa_string(int cpu, uint64_t base, uint64_t ext)
{
	printf("cpu(%d) base(0x%lx) ext(0x%lx) rv%ld%s%s%s%s%s%s%s(...)\n",
		cpu >= 0 ? cpu : -1, base, ext, sizeof(long) * 8,
		base & RISCV_HWPROBE_BASE_BEHAVIOR_IMA ? "ima" : "",
		ext  & RISCV_HWPROBE_IMA_FD ? "fd" : "",
		ext  & RISCV_HWPROBE_IMA_C ? "c" : "",
		ext  & RISCV_HWPROBE_IMA_V ? "v" : "",
		ext  & RISCV_HWPROBE_EXT_ZBA ? "_zba" : "",
		ext  & RISCV_HWPROBE_EXT_ZBB ? "_zbb" : "",
		ext  & RISCV_HWPROBE_EXT_ZBS ? "_zbs" : "");
}

int main(void)
{
	struct riscv_hwprobe req1[] = {
		{RISCV_HWPROBE_KEY_MVENDORID, 0},
		{RISCV_HWPROBE_KEY_MARCHID, 0},
		{RISCV_HWPROBE_KEY_MIMPID, 0},
	};

	struct riscv_hwprobe req2[] = {
		{RISCV_HWPROBE_KEY_BASE_BEHAVIOR, 0},
		{RISCV_HWPROBE_KEY_IMA_EXT_0, 0},
	};

	cpu_set_t cpus_all;
	cpu_set_t cpus;
	int ret;

	CPU_ZERO(&cpus_all);

	for (int cpu = 0; ; cpu++) {
		CPU_ZERO(&cpus);
		CPU_SET(cpu, &cpus);

		ret = sys_riscv_hwprobe(&req1[0], ARRAY_SIZE(req1),
				sizeof(cpus), &cpus, 0);
		if (ret != 0) {
			if (errno == EINVAL) {
				/* no more online cpus (fragile: based on kernel code) */
				break;
			} else {
				printf("%s:%d: hwprobe failure: ret(%d) errno(%d)\n",
					__func__, __LINE__, ret, errno);
				exit(-1);
			}
		}

		printf("cpu#%d mvendorid(0x%llx) marchid(0x%llx) mimpid(0x%llx)\n",
			cpu, req1[0].value, req1[1].value, req1[2].value);

		ret = sys_riscv_hwprobe(&req2[0], ARRAY_SIZE(req2),
				sizeof(cpus), &cpus, 0);
		if (ret != 0) {
			printf("%s:%d: hwprobe failure: ret(%d) errno(%d)\n",
				__func__, __LINE__, ret, errno);
			exit(-1);
		}

		show_isa_string(cpu, req2[0].value, req2[1].value);
		CPU_SET(cpu, &cpus_all);
	}

	/* get common isa subset for all the harts */
	ret = sys_riscv_hwprobe(&req2[0], ARRAY_SIZE(req2),
			sizeof(cpus_all), &cpus_all, 0);
	if (ret != 0) {
		printf("%s:%d: hwprobe failure: ret(%d) errno(%d)\n",
			__func__, __LINE__, ret, errno);
		exit(-1);
	}

	show_isa_string(-1, req2[0].value, req2[1].value);
	return 0;
}
