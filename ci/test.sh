#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source ci/env.sh
mkdir -p build/library
R CMD build . --no-build-vignettes --no-manual
R CMD INSTALL --library=build/library licensebox_0.2.0.tar.gz
export R_LIBS_USER="$PWD/build/library:${R_LIBS_USER:-}"
Rscript test/lifetime.R
