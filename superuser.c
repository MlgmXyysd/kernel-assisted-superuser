// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

/* Hello. If this is enabled in your kernel for some reason, whoever is
 * distributing your kernel to you is a complete moron, and you shouldn't
 * use their kernel anymore. But it's not my fault! People: don't enable
 * this driver! (Note that the existence of this file does not imply the
 * driver is actually in use. Look in your .config to see whether this is
 * enabled.) -Jason
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/mman.h>

static bool is_su(const char __user *filename)
{
	static const char su_path[] = "/system/bin/su";
	char ufn[sizeof(su_path)];

	return likely(!copy_from_user(ufn, filename, sizeof(ufn))) && unlikely(!memcmp(ufn, su_path, sizeof(ufn)));
}

static int new_sh_user_path(char __user **filename)
{
	static const char sh_path[] = "/system/bin/sh";
	unsigned long addr;

	addr = vm_mmap(NULL, 0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0);
	if (IS_ERR_VALUE(addr))
		return (int)addr;
	if (copy_to_user((void __user *)addr, sh_path, sizeof(sh_path)))
		return -EFAULT;
	*filename = (char __user *)addr;
	return 0;
}

static void free_sh_user_path(char __user *filename)
{
	struct mm_struct *mm = current->mm;
	down_write(&mm->mmap_sem);
	do_munmap(mm, (unsigned long)filename, PAGE_SIZE);
	up_write(&mm->mmap_sem);
}

static long(*old_newfstatat)(int dfd, const char __user *filename, struct stat *statbuf, int flag);
static long new_newfstatat(int dfd, const char __user *filename, struct stat __user *statbuf, int flag)
{
	if (is_su(filename)) {
		char __user *new_filename;
		int ret = new_sh_user_path(&new_filename);

		if (!ret) {
			ret = old_newfstatat(dfd, new_filename, statbuf, flag);
			free_sh_user_path(new_filename);
			return ret;
		}

	}
	return old_newfstatat(dfd, filename, statbuf, flag);
}

static long(*old_faccessat)(int dfd, const char __user *filename, int mode);
static long new_faccessat(int dfd, const char __user *filename, int mode)
{
	if (is_su(filename)) {
		char __user *new_filename;
		int ret = new_sh_user_path(&new_filename);

		if (!ret) {
			ret = old_faccessat(dfd, new_filename, mode);
			free_sh_user_path(new_filename);
			return ret;
		}
	}
	return old_faccessat(dfd, filename, mode);
}

extern int selinux_enforcing;
static long (*old_execve)(const char __user *filename, const char __user *const __user *argv, const char __user *const __user *envp);
static long new_execve(const char __user *filename, const char __user *const __user *argv, const char __user *const __user *envp)
{
	if (is_su(filename)) {
		char __user *new_filename;
		int ret = new_sh_user_path(&new_filename);

		if (!ret) {
			static const char now_root[] = "You are now root.\n";
			struct file *stderr;
			struct cred *cred;

			/* It might be enough to just change the security ctx of the
			 * current task, but that requires slightly more thought than
			 * just axing the whole thing here.
			 */
			selinux_enforcing = 0;

			/* Rather than the usual commit_creds(prepare_kernel_cred(NULL)) idiom,
			 * we manually zero out the fields in our existing one, so that we
			 * don't have to futz with the task's key ring for disk access.
			 */
			cred = (struct cred *)__task_cred(current);
			memset(&cred->uid, 0, sizeof(cred->uid));
			memset(&cred->gid, 0, sizeof(cred->gid));
			memset(&cred->suid, 0, sizeof(cred->suid));
			memset(&cred->euid, 0, sizeof(cred->euid));
			memset(&cred->egid, 0, sizeof(cred->egid));
			memset(&cred->fsuid, 0, sizeof(cred->fsuid));
			memset(&cred->fsgid, 0, sizeof(cred->fsgid));
			memset(&cred->cap_inheritable, 0xff, sizeof(cred->cap_inheritable));
			memset(&cred->cap_permitted, 0xff, sizeof(cred->cap_permitted));
			memset(&cred->cap_effective, 0xff, sizeof(cred->cap_effective));
			memset(&cred->cap_bset, 0xff, sizeof(cred->cap_bset));
			memset(&cred->cap_ambient, 0xff, sizeof(cred->cap_ambient));

			stderr = fget(2);
			if (stderr) {
				kernel_write(stderr, now_root, sizeof(now_root) - 1, 0);
				fput(stderr);
			}

			ret = old_execve(new_filename, argv, envp);
			free_sh_user_path(new_filename);
			return ret;
		}
	}
	return old_execve(filename, argv, envp);
}

extern const unsigned long sys_call_table[];
static void read_syscall(void **ptr, unsigned int syscall)
{
	*ptr = READ_ONCE(*((void **)sys_call_table + syscall));
}
static void replace_syscall(unsigned int syscall, void *ptr)
{
	WRITE_ONCE(*((void **)sys_call_table + syscall), ptr);
}
#define read_and_replace_syscall(name) do { \
	read_syscall((void **)&old_ ## name, __NR_ ## name); \
	replace_syscall(__NR_ ## name, &new_ ## name); \
} while (0)

static int superuser_init(void)
{
	pr_err("WARNING WARNING WARNING WARNING WARNING\n");
	pr_err("This kernel has kernel-assisted superuser and contains a\n");
	pr_err("trivial way to get root. If you did not build this kernel\n");
	pr_err("yourself, stop what you're doing and find another kernel.\n");
	pr_err("This one is not safe to use.\n");
	pr_err("WARNING WARNING WARNING WARNING WARNING\n");

	read_and_replace_syscall(newfstatat);
	read_and_replace_syscall(faccessat);
	read_and_replace_syscall(execve);

	return 0;
}

module_init(superuser_init);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Kernel-assisted superuser for Android");
MODULE_AUTHOR("Jason A. Donenfeld <Jason@zx2c4.com>");
