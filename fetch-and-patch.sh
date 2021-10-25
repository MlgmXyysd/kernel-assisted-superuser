#!/bin/bash
# SPDX-License-Identifier: GPL-2.0
# Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
# Copyright (C) 2002-2022 Jim Wu <mlgmxyysd@meowcat.org>. All Rights Reserved.

set -eo pipefail
trap 'rm -rf kernel-assisted-superuser' INT TERM EXIT
echo "[+] Downloading"
git clone "https://github.com/MlgmXyysd/kernel-assisted-superuser/"
kernel-assisted-superuser/patch.sh
