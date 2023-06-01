/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2019 Linus Walleij <linus.walleij@linaro.org>
 */

#ifndef LTP_IOPRIO_H
#define LTP_IOPRIO_H

/*
 * Gives us 8 prio classes with 13-bits of data for each class
 */
#define IOPRIO_CLASS_SHIFT	13
#define IOPRIO_CLASS_MASK	0x07
#define IOPRIO_PRIO_MASK	((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(ioprio)	\
	(((ioprio) >> IOPRIO_CLASS_SHIFT) & IOPRIO_CLASS_MASK)
#define IOPRIO_PRIO_DATA(ioprio)	((ioprio) & IOPRIO_PRIO_MASK)
#define IOPRIO_PRIO_VALUE(class, data)	\
	((((class) & IOPRIO_CLASS_MASK) << IOPRIO_CLASS_SHIFT) | \
	 ((data) & IOPRIO_PRIO_MASK))

/*
 * These are the io priority classes as implemented by the BFQ and mq-deadline
 * schedulers. RT is the realtime class, it always gets premium service. For
 * ATA disks supporting NCQ IO priority, RT class IOs will be processed using
 * high priority NCQ commands. BE is the best-effort scheduling class, the
 * default for any process. IDLE is the idle scheduling class, it is only
 * served when no one else is using the disk.
 */
enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE,
};

/*
 * The RT and BE priority classes both support up to 8 priority levels that
 * can be specified using the lower 3-bits of the priority data.
 */
#define IOPRIO_LEVEL_NR_BITS		3
#define IOPRIO_NR_LEVELS		(1 << IOPRIO_LEVEL_NR_BITS)
#define IOPRIO_LEVEL_MASK		(IOPRIO_NR_LEVELS - 1)
#define IOPRIO_PRIO_LEVEL(ioprio)	((ioprio) & IOPRIO_LEVEL_MASK)

#define IOPRIO_BE_NR			IOPRIO_NR_LEVELS

/*
 * Possible values for the "which" argument of the ioprio_get() and
 * ioprio_set() system calls (see "man ioprio_set").
 */
enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER,
};

static const char * const to_class_str[] = {
	[IOPRIO_CLASS_NONE] = "NONE",
	[IOPRIO_CLASS_RT]   = "REALTIME",
	[IOPRIO_CLASS_BE]   = "BEST-EFFORT",
	[IOPRIO_CLASS_IDLE] = "IDLE"
};

static inline int sys_ioprio_get(int which, int who)
{
	return tst_syscall(__NR_ioprio_get, which, who);
}

static inline int sys_ioprio_set(int which, int who, int ioprio)
{
	return tst_syscall(__NR_ioprio_set, which, who, ioprio);
}

/* Priority range from 0 (highest) to 7 (lowest) */
static inline int prio_in_range(int prio)
{
	if ((prio < 0) || (prio > 7))
		return 0;
	return 1;
}

/* Priority range from 0 to 3 using the enum */
static inline int class_in_range(int class)
{
	if ((class < IOPRIO_CLASS_NONE) || (class > IOPRIO_CLASS_IDLE))
		return 0;
	return 1;
}

static inline void ioprio_check_setting(int class, int prio, int report)
{
	int res;
	int newclass, newprio;

	res = sys_ioprio_get(IOPRIO_WHO_PROCESS, 0);
	if (res == -1) {
		tst_res(TFAIL | TTERRNO,
			 "reading back prio failed");
		return;
	}

	newclass = IOPRIO_PRIO_CLASS(res);
	newprio = IOPRIO_PRIO_LEVEL(res);
	if (newclass != class)
		tst_res(TFAIL,
			"wrong class after setting, expected %s got %s",
			to_class_str[class],
			to_class_str[newclass]);
	else if (newprio != prio)
		tst_res(TFAIL,
			"wrong prio after setting, expected %d got %d",
			prio, newprio);
	else if (report)
		tst_res(TPASS, "ioprio_set new class %s, new prio %d",
			to_class_str[newclass],
			newprio);
}

#endif
