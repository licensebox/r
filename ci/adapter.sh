#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source ci/env.sh
export R_LIBS_USER="$PWD/build/library:${R_LIBS_USER:-}"
exec Rscript test/adapter.R
