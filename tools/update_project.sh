#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if (( BASH_VERSINFO[0] < 4 )); then
    echo "Error: this script requires bash >= 4.0 (associative arrays used)." >&2
    exit 1
fi

for cmd in perl realpath find sort cp mkdir grep cat dirname basename; do
done

if ! realpath --help 2>&1 | grep -q -- '--relative-to'; then
    echo "Error: realpath must support --relative-to (GNU coreutils realpath)." >&2
    exit 1
fi

if [ $# -ne 1 ]; then
    echo "Usage: $0 /path/to/content_folder" >&2
    exit 1
fi

CONTENT_DIR="$1"

# Extract all local file links from a markdown file, resolved to repo-root-relative paths.
# Handles both [text](path) and ![alt](path) links, including multi-line forms.
extract_links() {
    local abs_file="$1"
    local file_dir="$2"   # directory of the file, relative to repo root

    perl -0777 -ne '
        while (/!?\[(?:[^\]]*)\]\(\s*([^)]+?)\s*\)/g) {
            my $link = $1;
            $link =~ s/\s+//g;        # strip internal whitespace from multi-line links
            $link =~ s/#.*$//;        # strip fragment
            next unless length $link;
            next if $link =~ m{^[a-zA-Z][a-zA-Z0-9+.-]*://};  # skip URLs
            print "$link\n";
        }
    ' "$abs_file" | while IFS= read -r link; do
        if [[ "$link" == /* ]]; then
            echo "${link#/}"
        else
            realpath -m --relative-to="$REPO_ROOT" "$REPO_ROOT/$file_dir/$link"
        fi
    done | grep -v '^\.\.' || true   # drop anything that escapes the repo root
}

declare -A visited=()
# Queue entries are "file|linkedfrom" pairs (| as separator)
declare -a queue=("README.md|")

while [ ${#queue[@]} -gt 0 ]; do
    entry="${queue[0]}"
    queue=("${queue[@]:1}")

    current="${entry%%|*}"
    parent="${entry#*|}"

    [[ -v visited["$current"] ]] && continue
    visited["$current"]=1

    src="$REPO_ROOT/$current"
    dst="$CONTENT_DIR/$current"

    if [ ! -f "$src" ]; then
        if [ -n "$parent" ]; then
            echo "Warning: not found: $current (linked from: $parent)" >&2
        else
            echo "Warning: not found: $current" >&2
        fi
        continue
    fi

    echo "Copying: $current"
    mkdir -p "$(dirname "$dst")"
    cp "$src" "$dst"

    # Only recurse into markdown files
    [[ "$current" != *.md ]] && continue

    file_dir="$(dirname "$current")"

    while IFS= read -r linked; do
        [[ -z "$linked" ]] && continue
        [[ -v visited["$linked"] ]] && continue
        queue+=("$linked|$current")
    done < <(extract_links "$src" "$file_dir")
done

# Copy the entire examples tree
echo "Copying examples/ tree..."
cp -r "$REPO_ROOT/examples" "$CONTENT_DIR/"

# Map a filename to a fenced-code language hint
code_lang() {
    case "$1" in
        CMakeLists.txt|*.cmake) echo "cmake"   ;;
        *.c|*.h)                echo "c"        ;;
        *.cpp|*.cc|*.cxx|*.hpp) echo "cpp"      ;;
        *.py)                   echo "python"   ;;
        *.sh|*.bash)            echo "bash"     ;;
        *.md)                   echo "markdown" ;;
        *.patch|*.diff)         echo "diff"     ;;
        *.json)                 echo "json"     ;;
        *.yaml|*.yml)           echo "yaml"     ;;
        *.js|*.ts)              echo "js"       ;;
        *)                      echo ""         ;;
    esac
}

# Generate index.md in every directory of the examples tree
generate_index() {
    local abs_dir="$1"   # absolute path inside content dir
    local rel_dir="$2"   # path relative to content dir (for display)

    local -a dirs=() files=()
    while IFS= read -r -d '' item; do
        local name
        name="$(basename "$item")"
        if [ -d "$item" ]; then
            dirs+=("$name")
        else
            [[ "$name" == "index.md" ]] && continue
            files+=("$name")
        fi
    done < <(find "$abs_dir" -maxdepth 1 -mindepth 1 -print0 | sort -z)

    {
        echo "# $rel_dir"
        echo ""

        for d in "${dirs[@]+"${dirs[@]}"}"; do
            echo "- [$d/]($d/index.md)"
        done

        for f in "${files[@]+"${files[@]}"}"; do
            local lang
            lang="$(code_lang "$f")"
            echo ""
            echo "### [$f]($f)"
            echo ""
            echo '```'"$lang"
            cat "$abs_dir/$f"
            echo '```'
        done
    } > "$abs_dir/index.md"

    echo "Generated index: $rel_dir/index.md"

    for d in "${dirs[@]+"${dirs[@]}"}"; do
        generate_index "$abs_dir/$d" "$rel_dir/$d"
    done
}

generate_index "$CONTENT_DIR/examples" "examples"

echo "Done. Copied ${#visited[@]} doc file(s) to $CONTENT_DIR"
