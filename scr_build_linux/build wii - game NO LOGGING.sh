#!/usr/bin/env bash
set -Eeuo pipefail
exec bash "$(dirname "${BASH_SOURCE[0]}")/build wii.sh" game -DMC_LOG_LEVEL=0 -DWII_OOM_ERROR_SCREEN=ON -DWII_ENABLE_NETWORK=ON "$@"
