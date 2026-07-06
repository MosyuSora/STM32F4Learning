# -*- coding: utf-8 -*-
"""
Chapter 6 readability-rewrite validator.

Mechanical acceptance checks for the section-by-section rewrite, so quality
does not silently degrade while editing one large document.

Usage:
    python tools/check_ch6.py                # check whole chapter
    python tools/check_ch6.py --section 3    # check only "## 3 ..." section

Exit code 0 = no FAIL (warnings allowed), 1 = at least one FAIL.
Checks are intentionally mechanical; semantic correctness of source-code
cross-references is verified by the human gate (rubric in the checklist).
"""
import argparse
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
CH_DIR = os.path.dirname(HERE)                       # Chapter6 dir
MD = os.path.join(CH_DIR, "Chapter6_手撕FreeRTOS_底层核心机制.md")
IMG_DIR = os.path.join(CH_DIR, "img")
CODE_DIR = os.path.join(CH_DIR, "code")
ALLOW = os.path.join(HERE, "ch6_allowed_symbols.txt")

# ---- config -----------------------------------------------------------------
TOTAL_LINE_FAIL = 3200          # whole chapter hard cap
SECTION_LINE_WARN = 300         # per "## " section soft cap

# heading substrings that mark the old template / second pass -> must be gone
BANNED_HEADING = ["细读", "图 0", "图 00", "代码：", "现象 ->", "是什么", "为什么存在"]
# inline template scaffolding phrases that should not survive the rewrite
BANNED_INLINE = [
    r"现象\s*->\s*是什么",
    r"这一节先看现象",
    r"图后要留下一个判断",
    r"读这张图时先",
]

GREEN, RED, YEL, RST = "\033[32m", "\033[31m", "\033[33m", "\033[0m"
if os.name == "nt" and not os.environ.get("WT_SESSION"):
    GREEN = RED = YEL = RST = ""   # plain output in legacy console

fails, warns = [], []
def fail(msg): fails.append(msg)
def warn(msg): warns.append(msg)


def load_symbols():
    if not os.path.exists(ALLOW):
        return set()
    with open(ALLOW, encoding="utf-8") as f:
        return {ln.strip() for ln in f if ln.strip() and not ln.startswith("#")}


def split_sections(lines):
    """Return list of (heading, start_idx, end_idx) for every '## ' section."""
    idx = [i for i, ln in enumerate(lines) if ln.startswith("## ")]
    out = []
    for k, i in enumerate(idx):
        end = idx[k + 1] if k + 1 < len(idx) else len(lines)
        out.append((lines[i].rstrip(), i, end))
    return out


def check(text, lines, only_section=None, is_full=True):
    # 1. total budget
    if only_section is None and len(lines) > TOTAL_LINE_FAIL:
        fail(f"whole chapter {len(lines)} lines > cap {TOTAL_LINE_FAIL}")

    sections = split_sections(lines)

    # 2. per-section budget + banned headings + duplicate titles
    seen = {}
    for head, s, e in sections:
        if only_section is not None and f"## {only_section} " not in head + " ":
            continue
        span = e - s
        if span > SECTION_LINE_WARN:
            warn(f"{head.strip()!r}: {span} lines > {SECTION_LINE_WARN}")
        key = re.sub(r"^##\s*\d+(\.\d+)?\s*", "", head).strip()
        if key in seen:
            fail(f"duplicate section title {key!r} (also line {seen[key]})")
        seen[key] = s + 1

    for i, ln in enumerate(lines, 1):
        if ln.startswith("#"):
            for b in BANNED_HEADING:
                if b in ln:
                    fail(f"L{i}: banned heading marker {b!r}: {ln.strip()}")

    # 3. banned inline template phrases
    for pat in BANNED_INLINE:
        for m in re.finditer(pat, text):
            ln = text[: m.start()].count("\n") + 1
            fail(f"L{ln}: template residue matches /{pat}/")

    # 4. figure references: exist + not archived + collect usage
    used_figs = set()
    for m in re.finditer(r"img/((?:_archive/)?fig-[\w.-]+\.png)", text):
        rel = m.group(1)
        ln = text[: m.start()].count("\n") + 1
        if rel.startswith("_archive/"):
            fail(f"L{ln}: references archived figure {rel}")
        p = os.path.join(IMG_DIR, rel)
        if not os.path.exists(p):
            fail(f"L{ln}: figure not found: img/{rel}")
        used_figs.add(os.path.basename(rel))

    # unreferenced (kept) figures -> warn (only meaningful on the full canonical file)
    if os.path.isdir(IMG_DIR) and only_section is None and is_full:
        for fn in sorted(os.listdir(IMG_DIR)):
            if fn.endswith(".png") and fn not in used_figs:
                warn(f"figure never referenced: img/{fn}")

    # 5. demo dir references exist
    for m in re.finditer(r"code/(v\d+_[\w-]+)", text):
        d = m.group(1)
        if not os.path.isdir(os.path.join(CODE_DIR, d)):
            ln = text[: m.start()].count("\n") + 1
            fail(f"L{ln}: demo dir not found: code/{d}")

    # 6. advisory: FreeRTOS-looking symbols not on the allowlist
    allow = load_symbols()
    if allow:
        cand = set(re.findall(r"`((?:px|ux|pv|pc|pd|x|v|prv)[A-Za-z0-9_]{3,})`", text))
        cand |= set(re.findall(r"`([A-Za-z_]+_t)`", text))
        cand = {c for c in cand if not re.match(r"v\d+_", c)}  # drop demo dir names
        for sym in sorted(cand - allow):
            warn(f"symbol not on allowlist (verify vs source): {sym}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--section", default=None, help="only check '## N ...'")
    ap.add_argument("--file", default=MD, help="md file to check (default: canonical)")
    args = ap.parse_args()

    if not os.path.exists(args.file):
        print(f"{RED}FAIL{RST} chapter md not found: {args.file}")
        sys.exit(1)
    with open(args.file, encoding="utf-8") as f:
        text = f.read()
    lines = text.splitlines()

    check(text, lines, args.section, is_full=(os.path.abspath(args.file) == os.path.abspath(MD)))

    scope = f"section {args.section}" if args.section else f"whole chapter ({len(lines)} lines)"
    print(f"== check_ch6: {scope} ==")
    for w in warns:
        print(f"{YEL}WARN{RST} {w}")
    for f_ in fails:
        print(f"{RED}FAIL{RST} {f_}")
    print(f"-- {len(fails)} fail, {len(warns)} warn --")
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
