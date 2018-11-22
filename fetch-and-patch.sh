#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

set -eo pipefail
trap 'rm -rf android-kernel-development-backdoor-master' INT TERM EXIT
echo "[+] Downloading"
curl -LsS "https://git.zx2c4.com/android-kernel-development-backdoor/snapshot/android-kernel-development-backdoor-master.tar.xz" | tar -xJf -
android-kernel-development-backdoor-master/patch.sh
