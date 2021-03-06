#define SYS_close 3
#define SYS_rt_sigaction 13
#define SYS_rt_sigprocmask 14
#define SYS_ioctl 16
#define SYS_pread64 17
#define SYS_pwrite64 18
#define SYS_msync 26
#define SYS_mincore 27
#define SYS_madvise 28
#define SYS_shmget 29
#define SYS_shmat 30
#define SYS_shmctl 31
#define SYS_getitimer 36
#define SYS_setitimer 38
#define SYS_clone 56
#define SYS_execve 59
#define SYS_wait4 61
#define SYS_kill 62
#define SYS_uname 63
#define SYS_semget 64
#define SYS_semop 65
#define SYS_semctl 66
#define SYS_shmdt 67
#define SYS_msgget 68
#define SYS_msgsnd 69
#define SYS_msgrcv 70
#define SYS_msgctl 71
#define SYS_flock 73
#define SYS_chdir 80
#define SYS_fchdir 81
#define SYS_getrlimit 97
#define SYS_getrusage 98
#define SYS_times 100
#define SYS_ptrace 101
#define SYS_rt_sigpending 127
#define SYS_rt_sigtimedwait 128
#define SYS_rt_sigqueueinfo 129
#define SYS_rt_sigsuspend 130
#define SYS_sigaltstack 131
#define SYS_statfs 137
#define SYS_fstatfs 138
#define SYS_getpriority 140
#define SYS_setpriority 141
#define SYS_sched_setparam 142
#define SYS_sched_getparam 143
#define SYS_sched_setscheduler 144
#define SYS_sched_getscheduler 145
#define SYS_sched_get_priority_max 146
#define SYS_sched_get_priority_min 147
#define SYS_sched_rr_get_interval 148
#define SYS_mlock 149
#define SYS_munlock 150
#define SYS_mlockall 151
#define SYS_munlockall 152
#define SYS_adjtimex 159
#define SYS_chroot 161
#define SYS_acct 163
#define SYS_settimeofday 164
#define SYS_sethostname 170
#define SYS_setdomainname 171
#define SYS_quotactl 179
#define SYS_gettid 186
#define SYS_tkill 200
#define SYS_sched_setaffinity 203
#define SYS_sched_getaffinity 204
#define SYS_set_tid_address 218
#define SYS_semtimedop 220
#define SYS_fadvise64 221
#define SYS_timer_create 222
#define SYS_timer_settime 223
#define SYS_timer_gettime 224
#define SYS_timer_getoverrun 225
#define SYS_timer_delete 226
#define SYS_clock_settime 227
#define SYS_clock_getres 229
#define SYS_clock_nanosleep 230
#define SYS_mq_open 240
#define SYS_mq_unlink 241
#define SYS_mq_timedsend 242
#define SYS_mq_timedreceive 243
#define SYS_mq_notify 244
#define SYS_mq_getsetattr 245
#define SYS_waitid 247
#define SYS_openat 257
#define SYS_mkdirat 258
#define SYS_mknodat 259
#define SYS_fchownat 260
#define SYS_renameat 264
#define SYS_linkat 265
#define SYS_symlinkat 266
#define SYS_readlinkat 267
#define SYS_fchmodat 268
#define SYS_faccessat 269
#define SYS_pselect6 270
#define SYS_ppoll 271
#define SYS_utimensat 280
#define SYS_fallocate 285
#define SYS_preadv 295
#define SYS_pwritev 296
#define SYS_prlimit64 302
#define SYS_getcpu 309
