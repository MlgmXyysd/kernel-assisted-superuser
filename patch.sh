#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

if ! [[ -d .git && -f drivers/base/Makefile && -f drivers/base/Kconfig ]]; then
	echo "Please run this from the top level of your kernel tree." >&2
	exit 1
fi

FILES="${0%/*}"

echo "[+] Patching"
cp "$FILES"/rootme.c drivers/base/rootme.c
grep -q ANDROID_ROOTME drivers/base/Makefile || cat "$FILES"/Kbuild.addon >> drivers/base/Makefile
grep -q ANDROID_ROOTME drivers/base/Kconfig || cat "$FILES"/Kconfig.addon >> drivers/base/Kconfig

echo "[+] Committing"
git add drivers/base/rootme.c drivers/base/Makefile drivers/base/Kconfig
git commit -s -F "$FILES"/commit-message.txt drivers/base/rootme.c drivers/base/Makefile drivers/base/Kconfig

echo "[+] Done!"

echo "[*] Remember to enable CONFIG_ANDROID_ROOTME=y for this to work. Then simply use \`kill -42 \$\$\` for root."
