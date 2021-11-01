#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
# Copyright (C) 2002-2022 Jim Wu <mlgmxyysd@meowcat.org>. All Rights Reserved.

if ! [[ -d .git && -f drivers/base/Makefile && -f drivers/base/Kconfig ]]; then
	echo "Please run this from the top level of your kernel tree." >&2
	exit 1
fi

FILES="${0%/*}"

echo "[+] Patching"
cp "$FILES"/superuser.c drivers/base/superuser.c
grep -q ASSISTED_SUPERUSER drivers/base/Makefile || cat "$FILES"/Kbuild.addon >> drivers/base/Makefile
grep -q ASSISTED_SUPERUSER drivers/base/Kconfig || cat "$FILES"/Kconfig.addon >> drivers/base/Kconfig

echo "[+] Committing"
git add drivers/base/superuser.c drivers/base/Makefile drivers/base/Kconfig
git commit -s -F "$FILES"/commit-message.txt drivers/base/superuser.c drivers/base/Makefile drivers/base/Kconfig

echo "[+] Deleting temporary files"
rm -rf "$FILES"

echo "[+] Done!"

echo "[*] Remember to enable CONFIG_ASSISTED_SUPERUSER=y for this to work. Then simply use \`su\` for root."
