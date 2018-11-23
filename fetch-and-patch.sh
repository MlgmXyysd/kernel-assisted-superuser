#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.

set -eo pipefail
trap 'rm -rf kernel-assisted-superuser-master' INT TERM EXIT
echo "[+] Downloading"
curl -LsS "https://git.zx2c4.com/kernel-assisted-superuser/snapshot/kernel-assisted-superuser-master.tar.xz" | tar -xJf -
kernel-assisted-superuser-master/patch.sh
