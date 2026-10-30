#!/usr/bin/env bash
set -euo pipefail

read_project_value() {
    sed -n "s/^$1[[:space:]]*=[[:space:]]*//p" Arbolsemantico.pro | tail -n 1
}

target_name="$(read_project_value TARGET).exe"
build_dir="$(read_project_value DESTDIR)"
distribution_dir="$(read_project_value PORTABLE_DIR)"

export PATH="/c/msys64/ucrt64/bin:/c/msys64/mingw64/bin:$PATH"

project_dir="$(pwd -P)"
source_exe="$project_dir/$build_dir/$target_name"
distribution_root="$project_dir/$distribution_dir"
package_name="${target_name%.*}"
package_dir="$distribution_root/$package_name"
package_exe="$package_dir/$target_name"

if [[ ! -f "$source_exe" ]]; then
    printf 'No se encontró el ejecutable compilado: %s\n' "$source_exe" >&2
    exit 1
fi

mkdir -p "$package_dir"
cp -f "$source_exe" "$package_exe"
windeployqt6 --no-compiler-runtime "$package_exe"

for file in "$package_dir"/*.exe "$package_dir"/*.dll "$package_dir"/platforms/*.dll; do
    [[ -f "$file" ]] || continue
    ldd "$file" | grep -i ucrt64 | awk '{print $3}' || true
done | sort -u | xargs -r -I{} cp -n {} "$package_dir/"

"$package_exe" >/dev/null 2>&1 &