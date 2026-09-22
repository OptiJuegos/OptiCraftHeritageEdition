#!/usr/bin/env bash
set -Eeuo pipefail
exec bash "$(dirname "${BASH_SOURCE[0]}")/build wii.sh" game -DWII_LOG_LEVEL=0 "$@"
