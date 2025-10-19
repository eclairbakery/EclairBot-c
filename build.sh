cd "$(dirname "$0")" || exit 1
mkdir -p ./build || exit 1

CC="${CC:-gcc}"
$CC -O0 -g ./nob.c -o build/nob

export __ECLAIRBOTC_PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build/nob $@
