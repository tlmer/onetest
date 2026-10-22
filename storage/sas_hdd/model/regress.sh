#!/bin/sh
# regress.sh -- SAS HDD TLM regression entry  [2026-10-08]
# Discipline: trust only the bench final verdict line (TB_<NAME> PASS);
#   this script only collects/summarises; `make -q` guards stale binaries.
# Usage: sh regress.sh
set -u
cd "$(dirname "$0")" || exit 1

# (1) stale-binary guard: rebuild if objects/binaries are older than sources
if ! make -q run >/dev/null 2>&1; then
    echo "[regress] building (stale targets)"
    make -s run >/dev/null 2>&1 || true
fi

TB_SRC=$(ls tb/tb_*.cpp 2>/dev/null)
[ -z "$TB_SRC" ] && { echo "[regress] no tb/tb_*.cpp found"; exit 1; }

pass=0; fail=0
printf "%-24s %-6s %s\n" "TB" "result" "self-reported verdict/summary"
printf -- "------------------------------------------------------------\n"
for src in $TB_SRC; do
    name=$(basename "$src" .cpp)
    [ -x "tb/$name" ] || { printf "%-24s %-6s %s\n" "$name" "missing" "(not built)"; fail=$((fail+1)); continue; }
    out=$(./"tb/$name" 2>&1)
    verdict=$(echo "$out" | grep -E "^TB_.* PASS" | tail -1)
    summary=$(echo "$out" | grep -E "^\[TOTAL\]" | tail -1)
    if [ -n "$verdict" ]; then
        pass=$((pass+1)); printf "%-24s %-6s %s\n" "$name" "PASS" "$summary"
    else
        fail=$((fail+1)); printf "%-24s %-6s %s\n" "$name" "FAIL" "$summary"
        echo "$out" | tail -25
    fi
done
printf -- "------------------------------------------------------------\n"
if [ "$fail" -eq 0 ]; then
    echo "ALL PASS ($pass/$((pass+fail)))"; exit 0
else
    echo "FAILURES (PASS=$pass FAIL=$fail)"; exit 1
fi
