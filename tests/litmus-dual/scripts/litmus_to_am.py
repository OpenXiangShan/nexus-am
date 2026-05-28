#!/usr/bin/env python3
"""Convert 2-thread RISC-V litmus tests into nexus-am bare-metal C tests.

This copy lives in nexus-am under tests/litmus-dual.  By default it uses
tests/litmus-dual/.cache/litmus-tests-riscv as the litmus source repository
and writes generated AM sources to tests/litmus-dual/generated.
"""

from __future__ import annotations

import argparse
import curses
import json
import re
import shutil
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, List, Optional, Sequence, Set, Tuple


REG_ALIASES: Dict[str, str] = {
    "zero": "x0",
    "ra": "x1",
    "sp": "x2",
    "gp": "x3",
    "tp": "x4",
    "t0": "x5",
    "t1": "x6",
    "t2": "x7",
    "s0": "x8",
    "fp": "x8",
    "s1": "x9",
    "a0": "x10",
    "a1": "x11",
    "a2": "x12",
    "a3": "x13",
    "a4": "x14",
    "a5": "x15",
    "a6": "x16",
    "a7": "x17",
    "s2": "x18",
    "s3": "x19",
    "s4": "x20",
    "s5": "x21",
    "s6": "x22",
    "s7": "x23",
    "s8": "x24",
    "s9": "x25",
    "s10": "x26",
    "s11": "x27",
    "t3": "x28",
    "t4": "x29",
    "t5": "x30",
    "t6": "x31",
}
for i in range(32):
    REG_ALIASES[f"x{i}"] = f"x{i}"


class UnsupportedFeatureError(Exception):
    pass


class UserCancelledError(Exception):
    pass


SCRIPT_DIR = Path(__file__).resolve().parent
LITMUS_DUAL_DIR = SCRIPT_DIR.parent
DEFAULT_REPO_ROOT = LITMUS_DUAL_DIR / ".cache" / "litmus-tests-riscv"
DEFAULT_OUTPUT_DIR = LITMUS_DUAL_DIR / "generated"
EXCLUDED_CATEGORY_PREFIXES = ("to-AArch64",)


def canonical_reg(name: str) -> str:
    key = name.strip().lower()
    if key not in REG_ALIASES:
        raise UnsupportedFeatureError(f"unknown register: {name}")
    return REG_ALIASES[key]


def reg_index(reg: str) -> int:
    return int(reg[1:])


def sanitize_ident(text: str) -> str:
    ident = re.sub(r"[^A-Za-z0-9_]", "_", text)
    if not ident or ident[0].isdigit():
        ident = f"_{ident}"
    return ident


def is_int_literal(text: str) -> bool:
    return re.fullmatch(r"-?(?:0x[0-9A-Fa-f]+|[0-9]+)", text.strip()) is not None


def int_literal_to_c(text: str) -> str:
    value = text.strip()
    if value.startswith("-"):
        return f"((uint64_t)({value}))"
    return f"((uint64_t)({value}ULL))"


def strip_litmus_comments(text: str) -> str:
    # litmus comments use (* ... *)
    return re.sub(r"\(\*.*?\*\)", "", text, flags=re.DOTALL)


def c_string_literal(line: str) -> str:
    escaped = line.replace("\\", "\\\\").replace('"', '\\"')
    return f"\"{escaped}\""


@dataclass
class LitmusExpr:
    kind: str
    value: Optional[str] = None
    left: Optional["LitmusExpr"] = None
    right: Optional["LitmusExpr"] = None


@dataclass
class ParsedTest:
    source: Path
    rel_source: str
    test_id: str
    test_name: str
    mem_symbols: List[str]
    mem_inits: Dict[str, str]
    reg_inits: Dict[int, Dict[str, str]]
    thread_instrs: Dict[int, List[str]]
    cond_kind: str
    cond_text: str
    cond_expr: LitmusExpr
    cond_regs: Set[Tuple[int, str]]
    cond_mems: Set[str]


@dataclass
class ConversionResult:
    converted: List[ParsedTest]
    skipped: List[Dict[str, str]]


@dataclass
class AsmOperandUse:
    instrs: List[str]
    used_regs: Set[str]
    written_regs: Set[str]
    read_before_write_regs: Set[str]


def tokenize_expr(expr: str) -> List[Tuple[str, str]]:
    tokens: List[Tuple[str, str]] = []
    i = 0
    n = len(expr)
    while i < n:
        ch = expr[i]
        if ch.isspace():
            i += 1
            continue
        if expr.startswith("/\\", i):
            tokens.append(("AND", "/\\"))
            i += 2
            continue
        if expr.startswith("\\/", i):
            tokens.append(("OR", "\\/"))
            i += 2
            continue
        if ch == "(":
            tokens.append(("LP", ch))
            i += 1
            continue
        if ch == ")":
            tokens.append(("RP", ch))
            i += 1
            continue
        if ch == "=":
            tokens.append(("EQ", ch))
            i += 1
            continue
        if expr.startswith("not", i) and (i + 3 == n or not expr[i + 3].isalnum()):
            tokens.append(("NOT", "not"))
            i += 3
            continue
        m = re.match(r"[01]:[A-Za-z_][A-Za-z0-9_]*", expr[i:])
        if m:
            tokens.append(("REGREF", m.group(0)))
            i += len(m.group(0))
            continue
        m = re.match(r"-?(?:0x[0-9A-Fa-f]+|[0-9]+)", expr[i:])
        if m:
            tokens.append(("NUM", m.group(0)))
            i += len(m.group(0))
            continue
        m = re.match(r"[A-Za-z_][A-Za-z0-9_]*", expr[i:])
        if m:
            tokens.append(("IDENT", m.group(0)))
            i += len(m.group(0))
            continue
        raise UnsupportedFeatureError(f"cannot tokenize condition near: {expr[i:i+24]!r}")
    return tokens


class ExprParser:
    def __init__(self, tokens: Sequence[Tuple[str, str]]) -> None:
        self.tokens = list(tokens)
        self.pos = 0

    def at_end(self) -> bool:
        return self.pos >= len(self.tokens)

    def peek(self) -> Optional[Tuple[str, str]]:
        return None if self.at_end() else self.tokens[self.pos]

    def accept(self, token_type: str) -> Optional[str]:
        t = self.peek()
        if t is None or t[0] != token_type:
            return None
        self.pos += 1
        return t[1]

    def expect(self, token_type: str) -> str:
        value = self.accept(token_type)
        if value is None:
            got = self.peek()
            raise UnsupportedFeatureError(f"expected {token_type}, got {got}")
        return value

    def parse(self) -> LitmusExpr:
        expr = self.parse_or()
        if not self.at_end():
            raise UnsupportedFeatureError(f"unexpected token in condition: {self.peek()}")
        return expr

    def parse_or(self) -> LitmusExpr:
        node = self.parse_and()
        while self.accept("OR") is not None:
            node = LitmusExpr(kind="or", left=node, right=self.parse_and())
        return node

    def parse_and(self) -> LitmusExpr:
        node = self.parse_unary()
        while self.accept("AND") is not None:
            node = LitmusExpr(kind="and", left=node, right=self.parse_unary())
        return node

    def parse_unary(self) -> LitmusExpr:
        if self.accept("NOT") is not None:
            return LitmusExpr(kind="not", left=self.parse_unary())
        if self.accept("LP") is not None:
            node = self.parse_or()
            self.expect("RP")
            return node
        return self.parse_atom()

    def parse_atom(self) -> LitmusExpr:
        left = self.parse_value()
        if self.accept("EQ") is not None:
            right = self.parse_value()
            return LitmusExpr(kind="eq", left=left, right=right)
        return left

    def parse_value(self) -> LitmusExpr:
        t = self.peek()
        if t is None:
            raise UnsupportedFeatureError("unexpected end of expression")
        if t[0] == "REGREF":
            self.pos += 1
            return LitmusExpr(kind="reg", value=t[1])
        if t[0] == "NUM":
            self.pos += 1
            return LitmusExpr(kind="num", value=t[1])
        if t[0] == "IDENT":
            self.pos += 1
            ident = t[1]
            if ident == "true":
                return LitmusExpr(kind="num", value="1")
            if ident == "false":
                return LitmusExpr(kind="num", value="0")
            return LitmusExpr(kind="mem", value=ident)
        if t[0] == "LP":
            self.pos += 1
            node = self.parse_or()
            self.expect("RP")
            return node
        raise UnsupportedFeatureError(f"unsupported value token in condition: {t}")


def collect_expr_refs(expr: LitmusExpr, regs: Set[Tuple[int, str]], mems: Set[str]) -> None:
    if expr.kind == "reg":
        hart_s, reg_s = expr.value.split(":", 1)  # type: ignore[union-attr]
        hart = int(hart_s)
        regs.add((hart, canonical_reg(reg_s)))
        return
    if expr.kind == "mem":
        mems.add(expr.value)  # type: ignore[arg-type]
        return
    if expr.left is not None:
        collect_expr_refs(expr.left, regs, mems)
    if expr.right is not None:
        collect_expr_refs(expr.right, regs, mems)


def expr_to_c(
    expr: LitmusExpr,
    mem_field: Dict[str, str],
    reg_field: Optional[Dict[Tuple[int, str], str]] = None,
) -> str:
    if expr.kind == "num":
        return int_literal_to_c(expr.value)
    if expr.kind == "mem":
        sym = expr.value
        if sym not in mem_field:
            raise UnsupportedFeatureError(f"condition references unknown memory symbol: {sym}")
        return f"((uint64_t)(ctx->{mem_field[sym]}))"
    if expr.kind == "reg":
        hart_s, reg_s = expr.value.split(":", 1)  # type: ignore[union-attr]
        hart = int(hart_s)
        reg = canonical_reg(reg_s)
        if reg == "x0":
            return "((uint64_t)(0ULL))"
        if reg_field is not None:
            key = (hart, reg)
            if key not in reg_field:
                raise UnsupportedFeatureError(
                    f"condition references uncaptured register: P{hart}:{reg}"
                )
            return f"((uint64_t)(ctx->{reg_field[key]}))"
        return f"((uint64_t)(ctx->regs[{hart}][{reg_index(reg)}]))"
    if expr.kind == "eq":
        return (
            f"({expr_to_c(expr.left, mem_field, reg_field)} == "
            f"{expr_to_c(expr.right, mem_field, reg_field)})"
        )
    if expr.kind == "and":
        return (
            f"({expr_to_c(expr.left, mem_field, reg_field)} && "
            f"{expr_to_c(expr.right, mem_field, reg_field)})"
        )
    if expr.kind == "or":
        return (
            f"({expr_to_c(expr.left, mem_field, reg_field)} || "
            f"{expr_to_c(expr.right, mem_field, reg_field)})"
        )
    if expr.kind == "not":
        return f"(!{expr_to_c(expr.left, mem_field, reg_field)})"
    raise UnsupportedFeatureError(f"unknown expression node: {expr.kind}")


def parse_init_block(init_text: str) -> Tuple[Dict[str, str], Dict[int, Dict[str, str]], Set[str]]:
    mem_inits: Dict[str, str] = {}
    reg_inits: Dict[int, Dict[str, str]] = {0: {}, 1: {}}
    mem_symbols: Set[str] = set()

    stmts = [s.strip() for s in init_text.replace("\n", " ").split(";")]
    for stmt in stmts:
        if not stmt:
            continue
        m = re.fullmatch(r"([0-9]+)\s*:\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)", stmt)
        if m:
            hart = int(m.group(1))
            if hart not in (0, 1):
                raise UnsupportedFeatureError(f"not a 2-thread test (init references P{hart})")
            reg = canonical_reg(m.group(2))
            value = m.group(3).strip()
            if is_int_literal(value):
                reg_inits[hart][reg] = value
                continue
            if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", value):
                mem_symbols.add(value)
                reg_inits[hart][reg] = value
                continue
            raise UnsupportedFeatureError(f"unsupported register init value: {stmt}")
        m = re.fullmatch(r"([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.+)", stmt)
        if m:
            sym = m.group(1)
            val = m.group(2).strip()
            if not is_int_literal(val):
                raise UnsupportedFeatureError(f"unsupported memory init value: {stmt}")
            mem_symbols.add(sym)
            mem_inits[sym] = val
            continue
        raise UnsupportedFeatureError(f"unsupported init statement: {stmt}")

    return mem_inits, reg_inits, mem_symbols


def normalize_instr_cell(cell: str) -> str:
    text = cell.strip()
    text = re.sub(r";\s*$", "", text)
    return text.strip()


def split_litmus_sections(text: str) -> Tuple[str, str]:
    start = text.find("{")
    if start < 0:
        raise UnsupportedFeatureError("missing init block start '{'")
    depth = 0
    end = -1
    for i in range(start, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                end = i
                break
    if end < 0:
        raise UnsupportedFeatureError("missing init block end '}'")
    return text[start + 1 : end], text[end + 1 :]


def parse_threads_and_condition(tail_text: str) -> Tuple[Dict[int, List[str]], str, str, LitmusExpr]:
    lines = [ln.rstrip() for ln in tail_text.splitlines()]
    header_idx = -1
    for i, line in enumerate(lines):
        if re.fullmatch(r"\s*P0\s*\|\s*P1\s*;\s*", line):
            header_idx = i
            break
    if header_idx < 0:
        raise UnsupportedFeatureError("not a 2-thread test (missing 'P0 | P1 ;' header)")

    thread_instrs: Dict[int, List[str]] = {0: [], 1: []}
    cond_idx = -1
    for i in range(header_idx + 1, len(lines)):
        s = lines[i].strip()
        if not s:
            continue
        if re.match(r"^(exists|forall|~exists)\b", s):
            cond_idx = i
            break
        if re.match(r"^(filter|locations)\b", s):
            raise UnsupportedFeatureError(f"unsupported clause before condition: {s}")
        parts = lines[i].split("|")
        if len(parts) < 2:
            continue
        c0 = normalize_instr_cell(parts[0])
        c1 = normalize_instr_cell(parts[1])
        if c0:
            thread_instrs[0].append(c0)
        if c1:
            thread_instrs[1].append(c1)

    if cond_idx < 0:
        raise UnsupportedFeatureError("missing condition clause")

    cond_lines = [ln.strip() for ln in lines[cond_idx:] if ln.strip()]
    if any(re.match(r"^(filter|locations)\b", ln) for ln in cond_lines):
        raise UnsupportedFeatureError("filter/locations conditions are unsupported")
    head = cond_lines[0]
    m = re.match(r"^(exists|~exists|forall)\b(.*)$", head)
    if m is None:
        raise UnsupportedFeatureError("only 'exists', '~exists', and 'forall' conditions are supported")

    cond_kw = m.group(1)
    rest = m.group(2).strip()
    if rest:
        expr_text = rest
        if len(cond_lines) > 1:
            expr_text = " ".join([expr_text] + cond_lines[1:]).strip()
    else:
        expr_text = " ".join(cond_lines[1:]).strip()
    if not expr_text:
        raise UnsupportedFeatureError(f"empty condition expression for '{cond_kw}'")

    tokens = tokenize_expr(expr_text)
    cond_expr = ExprParser(tokens).parse()
    cond_kind = {
        "exists": "exists",
        "~exists": "not_exists",
        "forall": "forall",
    }[cond_kw]
    cond_text = f"{cond_kw} {expr_text}".strip()
    return thread_instrs, cond_kind, cond_text, cond_expr


def extract_registers(instr: str) -> Set[str]:
    regs: Set[str] = set()
    for tok in re.findall(r"[A-Za-z_][A-Za-z0-9_]*", instr):
        lower = tok.lower()
        if lower in REG_ALIASES:
            regs.add(canonical_reg(lower))
        elif re.fullmatch(r"f[0-9]+", lower) is not None:
            raise UnsupportedFeatureError("floating-point registers are unsupported")
    return regs


def rewrite_labels(instrs: List[str], prefix: str) -> List[str]:
    label_map: Dict[str, str] = {}
    label_def = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$")
    for ins in instrs:
        m = label_def.match(ins)
        if m:
            old = m.group(1)
            label_map[old] = f"{prefix}_{sanitize_ident(old)}"

    rewritten: List[str] = []
    for ins in instrs:
        line = ins
        m = label_def.match(line)
        if m:
            old = m.group(1)
            rest = m.group(2).strip()
            line = f"{label_map[old]}:"
            if rest:
                line += f" {rest}"
        for old, new in label_map.items():
            line = re.sub(rf"\b{re.escape(old)}\b", new, line)
        rewritten.append(line.strip())
    return rewritten


def rewrite_operand_registers(operands: str) -> str:
    """Canonicalize register aliases in one operand list (e.g. a0 -> x10)."""
    token_re = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")

    def repl(m: re.Match[str]) -> str:
        tok = m.group(0)
        low = tok.lower()
        if low in REG_ALIASES:
            return canonical_reg(low)
        return tok

    return token_re.sub(repl, operands)


def rewrite_thread_registers(instrs: List[str]) -> List[str]:
    out: List[str] = []
    label_def = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$")
    for ins in instrs:
        line = ins.strip()
        if not line:
            continue
        label: Optional[str] = None
        body = line
        m = label_def.match(line)
        if m:
            label = m.group(1)
            body = m.group(2).strip()
            if not body:
                out.append(f"{label}:")
                continue

        parts = body.split(None, 1)
        opcode = parts[0]
        operands = parts[1].strip() if len(parts) > 1 else ""
        if operands:
            operands = rewrite_operand_registers(operands)
            body = f"{opcode} {operands}".rstrip()
        else:
            body = opcode

        if label is not None:
            out.append(f"{label}: {body}")
        else:
            out.append(body)

    return out


def lower_memord_pseudo(instr: str) -> List[str]:
    """Lower litmus pseudo memory-order suffixes to baseline GNU asm forms.

    Examples:
      lw.aq  -> lw + fence r,rw
      sw.rl  -> fence rw,w + sw
      fence.tso -> fence rw,rw
    """
    text = instr.strip()
    if not text:
        return []

    m = re.match(r"^([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$", text)
    label: Optional[str] = None
    body = text
    if m:
        label = m.group(1)
        body = m.group(2).strip()
        if not body:
            return [f"{label}:"]

    parts = body.split(None, 1)
    opcode = parts[0]
    operands = parts[1].strip() if len(parts) > 1 else ""

    lowered: List[str]
    op = opcode.lower()

    def normalize_atomic_opcode(opcode_text: str) -> str:
        op_parts = opcode_text.split(".")
        low_parts = [p.lower() for p in op_parts]
        if not op_parts:
            return opcode_text
        family = low_parts[0]
        if not (family == "lr" or family == "sc" or family.startswith("amo")):
            return opcode_text
        tail: List[str] = []
        while low_parts and low_parts[-1] in {"aq", "rl"}:
            tail.append(low_parts.pop())
            op_parts.pop()
        if not tail:
            return opcode_text
        has_aq = "aq" in tail
        has_rl = "rl" in tail
        if has_aq and has_rl:
            op_parts.append("aqrl")
        elif has_aq:
            op_parts.append("aq")
        elif has_rl:
            op_parts.append("rl")
        return ".".join(op_parts)

    opcode = normalize_atomic_opcode(opcode)
    op = opcode.lower()
    if op == "fence.tso":
        lowered = ["fence rw,rw"]
    else:
        lowered = [f"{opcode} {operands}".rstrip()]
        if "." in op:
            base, *mods = op.split(".")
            if base in {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld", "sb", "sh", "sw", "sd"}:
                if all(m in {"aq", "rl"} for m in mods):
                    before: List[str] = []
                    after: List[str] = []
                    if "rl" in mods:
                        before.append("fence rw,w")
                    if "aq" in mods:
                        after.append("fence r,rw")
                    lowered = before + [f"{base} {operands}".rstrip()] + after

    if label is not None and lowered:
        lowered[0] = f"{label}: {lowered[0]}"
    return lowered


def lower_instruction_list(instrs: List[str]) -> List[str]:
    out: List[str] = []
    for ins in instrs:
        out.extend(lower_memord_pseudo(ins))
    return out


def validate_riscv_instructions(thread_instrs: Dict[int, List[str]]) -> None:
    aarch64_ops = {
        "add",
        "adr",
        "adrp",
        "and",
        "b",
        "bl",
        "blr",
        "br",
        "cbnz",
        "cbz",
        "clrex",
        "dmb",
        "dsb",
        "eor",
        "isb",
        "ldar",
        "ldarb",
        "ldarh",
        "ldaxr",
        "ldr",
        "ldrb",
        "ldrh",
        "ldxr",
        "mov",
        "orr",
        "stlr",
        "stlrb",
        "stlrh",
        "stlxr",
        "str",
        "strb",
        "strh",
        "stxr",
        "sub",
    }
    aarch64_reg = re.compile(r"\b(?:[wx][0-9]+|sp|xzr|wzr)\b", re.IGNORECASE)
    for instrs in thread_instrs.values():
        for instr in instrs:
            text = instr.strip()
            m = re.match(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$", text)
            if m:
                text = m.group(2).strip()
            if not text:
                continue
            parts = text.split(None, 1)
            opcode = parts[0].lower()
            operands = parts[1] if len(parts) > 1 else ""
            if opcode in aarch64_ops and aarch64_reg.search(operands):
                raise UnsupportedFeatureError("AArch64 instruction is unsupported")


def parse_litmus_file(path: Path, root: Path) -> ParsedTest:
    raw = path.read_text(encoding="utf-8")
    text = strip_litmus_comments(raw)
    init_text, tail_text = split_litmus_sections(text)

    mem_inits, reg_inits, mem_symbols_from_init = parse_init_block(init_text)
    thread_instrs, cond_kind, cond_text, cond_expr = parse_threads_and_condition(tail_text)
    validate_riscv_instructions(thread_instrs)

    cond_regs: Set[Tuple[int, str]] = set()
    cond_mems: Set[str] = set()
    collect_expr_refs(cond_expr, cond_regs, cond_mems)

    mem_symbols = set(mem_symbols_from_init) | set(cond_mems)
    for hart in (0, 1):
        for _, value in reg_inits[hart].items():
            if re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", value):
                mem_symbols.add(value)

    rel_source = str(path.relative_to(root))
    test_id = sanitize_ident(rel_source.replace("/", "__").replace(".litmus", ""))
    test_name = path.stem

    thread_instrs[0] = lower_instruction_list(
        rewrite_thread_registers(rewrite_labels(thread_instrs[0], f"{test_id}_h0"))
    )
    thread_instrs[1] = lower_instruction_list(
        rewrite_thread_registers(rewrite_labels(thread_instrs[1], f"{test_id}_h1"))
    )

    return ParsedTest(
        source=path,
        rel_source=rel_source,
        test_id=test_id,
        test_name=test_name,
        mem_symbols=sorted(mem_symbols),
        mem_inits=mem_inits,
        reg_inits=reg_inits,
        thread_instrs=thread_instrs,
        cond_kind=cond_kind,
        cond_text=cond_text,
        cond_expr=cond_expr,
        cond_regs=cond_regs,
        cond_mems=cond_mems,
    )


def expand_at_file(path: Path, visited: Set[Path]) -> List[Path]:
    path = path.resolve()
    if path in visited:
        return []
    visited.add(path)
    results: List[Path] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        target = (path.parent / line).resolve()
        if target.name.startswith("@"):
            results.extend(expand_at_file(target, visited))
        else:
            if target.suffix != ".litmus":
                raise UnsupportedFeatureError(f"@file references non-litmus file: {target}")
            results.append(target)
    return results


def expand_inputs(input_path: Path) -> List[Path]:
    resolved = input_path.expanduser().resolve()
    if resolved.is_dir():
        return sorted(resolved.rglob("*.litmus"))
    if resolved.name.startswith("@"):
        return expand_at_file(resolved, set())
    if resolved.suffix == ".litmus":
        return [resolved]
    raise UnsupportedFeatureError(f"unsupported input path: {input_path}")


def relpath_for_report(path: Path, root: Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError:
        return str(path)


@dataclass(frozen=True)
class Category:
    key: str
    path: Path
    count: int


@dataclass
class CategoryNode:
    key: str
    label: str
    path: Optional[Path]
    count: int = 0
    parent: Optional["CategoryNode"] = None
    children: List["CategoryNode"] = field(default_factory=list)
    index: int = -1

def litmus_test_root(repo_root: Path) -> Path:
    root = repo_root / "tests"
    if not root.is_dir():
        raise UnsupportedFeatureError(f"missing litmus tests directory: {root}")
    return root


def category_depth(category: Category) -> int:
    if category.key == "all":
        return 0
    return category.key.count("/") + 1


def category_parent_key(key: str) -> Optional[str]:
    if key == "all":
        return None
    if "/" not in key:
        return "all"
    return key.rsplit("/", 1)[0]


def category_label(key: str) -> str:
    if key == "all":
        return "all"
    return key.rsplit("/", 1)[-1]


def is_excluded_category(key: str) -> bool:
    return any(key == prefix or key.startswith(prefix + "/") for prefix in EXCLUDED_CATEGORY_PREFIXES)


def discover_categories(repo_root: Path) -> List[Category]:
    tests_root = litmus_test_root(repo_root)
    categories: List[Category] = []
    for path in sorted(tests_root.rglob("@all")):
        rel = path.parent.relative_to(tests_root).as_posix()
        key = rel if rel != "." else "all"
        if is_excluded_category(key):
            continue
        count = len(expand_at_file(path, set()))
        categories.append(Category(key=key, path=path, count=count))

    if not categories:
        categories.append(
            Category("all", tests_root, len(sorted(tests_root.rglob("*.litmus"))))
        )
    categories.sort(key=lambda c: (category_depth(c), c.key))
    return categories


def build_category_tree(categories: Sequence[Category]) -> Tuple[CategoryNode, Dict[str, CategoryNode]]:
    by_key: Dict[str, Category] = {c.key: c for c in categories}
    if "all" not in by_key:
        total = sum(c.count for c in categories if "/" not in c.key)
        by_key["all"] = Category("all", Path("."), total)

    nodes: Dict[str, CategoryNode] = {}

    def ensure_node(key: str) -> CategoryNode:
        if key in nodes:
            return nodes[key]
        cat = by_key.get(key)
        node = CategoryNode(
            key=key,
            label=category_label(key),
            path=cat.path if cat is not None else None,
            count=cat.count if cat is not None else 0,
        )
        nodes[key] = node
        parent_key = category_parent_key(key)
        if parent_key is not None:
            parent = ensure_node(parent_key)
            node.parent = parent
            parent.children.append(node)
        return node

    for key in sorted(by_key, key=lambda k: (category_depth(Category(k, Path("."), 0)), k)):
        ensure_node(key)

    for node in nodes.values():
        node.children.sort(key=lambda child: child.label)

    ordered: List[CategoryNode] = []

    def assign_indices(node: CategoryNode) -> None:
        node.index = len(ordered)
        ordered.append(node)
        for child in node.children:
            assign_indices(child)

    root = ensure_node("all")
    assign_indices(root)
    return root, nodes


def ordered_category_nodes(categories: Sequence[Category]) -> List[CategoryNode]:
    _, nodes = build_category_tree(categories)
    return sorted(nodes.values(), key=lambda node: node.index)


def category_index_by_key(categories: Sequence[Category]) -> Dict[str, int]:
    return {category.key: i for i, category in enumerate(categories)}


def category_by_key(categories: Sequence[Category]) -> Dict[str, Category]:
    return {category.key: category for category in categories}


def subtree_category_keys(node: CategoryNode, index_by_key: Dict[str, int]) -> Set[str]:
    keys: Set[str] = set()

    def visit(cur: CategoryNode) -> None:
        if cur.key in index_by_key:
            keys.add(cur.key)
        for child in cur.children:
            visit(child)

    visit(node)
    return keys


def ancestor_category_keys(node: CategoryNode, index_by_key: Dict[str, int]) -> Set[str]:
    keys: Set[str] = set()
    cur = node.parent
    while cur is not None:
        if cur.key in index_by_key:
            keys.add(cur.key)
        cur = cur.parent
    return keys


def select_subtree(
    selected: Set[str], node: CategoryNode, index_by_key: Dict[str, int]
) -> None:
    selected.update(subtree_category_keys(node, index_by_key))


def deselect_subtree(
    selected: Set[str], node: CategoryNode, index_by_key: Dict[str, int]
) -> None:
    selected.difference_update(subtree_category_keys(node, index_by_key))
    selected.difference_update(ancestor_category_keys(node, index_by_key))


def node_selection_state(
    node: CategoryNode, selected: Set[str], index_by_key: Dict[str, int]
) -> str:
    keys = subtree_category_keys(node, index_by_key)
    if not keys:
        return "none"
    selected_count = sum(1 for key in keys if key in selected)
    if selected_count == 0:
        return "none"
    if selected_count == len(keys):
        return "checked"
    return "partial"


def subtree_unique_test_counts(
    categories: Sequence[Category], root: CategoryNode, index_by_key: Dict[str, int]
) -> Dict[str, int]:
    categories_by_key = category_by_key(categories)
    counts: Dict[str, int] = {}

    def visit(node: CategoryNode) -> Set[Path]:
        seen: Set[Path] = set()
        if node.key in index_by_key:
            for path in expand_inputs(categories_by_key[node.key].path):
                seen.add(path.resolve())
        for child in node.children:
            seen.update(visit(child))
        counts[node.key] = len(seen)
        return seen

    visit(root)
    return counts


def selected_indices_from_keys_uncompacted(
    selected_keys: Set[str], categories: Sequence[Category]
) -> List[int]:
    index_by_key = category_index_by_key(categories)
    return sorted(index_by_key[key] for key in selected_keys if key in index_by_key)


def selected_indices_from_keys(
    selected_keys: Set[str], categories: Sequence[Category]
) -> List[int]:
    return selected_indices_from_keys_uncompacted(selected_keys, categories)


def print_categories(categories: Sequence[Category]) -> None:
    ordered = ordered_category_nodes(categories)
    index_by_key = category_index_by_key(categories)
    root, _ = build_category_tree(categories)
    counts = subtree_unique_test_counts(categories, root, index_by_key)
    width = max((len(str(node.index + 1)) for node in ordered), default=1)
    for node in ordered:
        indent = "  " * category_depth(Category(node.key, Path("."), 0))
        marker = "-" if node.children else " "
        count = counts.get(node.key, node.count)
        print(f"{node.index + 1:>{width}}. {indent}{marker} {node.label:<32} {count:>6} tests")


def parse_selection(text: str, categories: Sequence[Category]) -> List[int]:
    ordered = ordered_category_nodes(categories)
    _, nodes = build_category_tree(categories)
    index_by_key = category_index_by_key(categories)
    selected_keys: Set[str] = set()
    for raw in text.split(","):
        item = raw.strip()
        if not item:
            continue
        if item.lower() in {"all", "*"}:
            select_subtree(selected_keys, nodes["all"], index_by_key)
            continue
        if re.fullmatch(r"\d+\s*-\s*\d+", item):
            left, right = item.split("-", 1)
            start = int(left) - 1
            end = int(right) - 1
            if start > end:
                start, end = end, start
            if start < 0 or end >= len(ordered):
                raise UnsupportedFeatureError(f"selection range out of bounds: {item}")
            for index in range(start, end + 1):
                select_subtree(selected_keys, ordered[index], index_by_key)
            continue
        if item.isdigit():
            index = int(item) - 1
            if index < 0 or index >= len(ordered):
                raise UnsupportedFeatureError(f"selection out of bounds: {item}")
            select_subtree(selected_keys, ordered[index], index_by_key)
            continue

        matches = [node.key for node in ordered if node.key == item]
        if not matches:
            matches = [node.key for node in ordered if node.key.endswith("/" + item)]
        if not matches:
            raise UnsupportedFeatureError(f"unknown category: {item}")
        for key in matches:
            select_subtree(selected_keys, nodes[key], index_by_key)

    return selected_indices_from_keys(selected_keys, categories)


def default_category_indices(categories: Sequence[Category]) -> List[int]:
    indices = [
        i for i, category in enumerate(categories) if category.key == "non-mixed-size"
    ]
    if not indices and categories:
        indices = [i for i, category in enumerate(categories) if category.key == "all"]
    if not indices and categories:
        indices = [0]
    return indices


def default_selected_categories(categories: Sequence[Category]) -> List[Category]:
    root, nodes = build_category_tree(categories)
    index_by_key = category_index_by_key(categories)
    selected_keys: Set[str] = set()
    for index in default_category_indices(categories):
        select_subtree(selected_keys, nodes[categories[index].key], index_by_key)
    if not selected_keys:
        select_subtree(selected_keys, root, index_by_key)
    return [categories[i] for i in selected_indices_from_keys(selected_keys, categories)]


def choose_categories_prompt(categories: Sequence[Category]) -> List[Category]:
    print("Available litmus categories:")
    print_categories(categories)
    print("")
    print("Select categories by number/name/range, for example: 2,4-6,RELAX")
    print("Selecting a parent includes all categories below it.")
    print("Press Enter for non-mixed-size, or type all for everything.")
    default_selected = default_selected_categories(categories)

    while True:
        try:
            answer = input("litmus categories> ").strip()
        except EOFError:
            answer = ""
        if not answer:
            return default_selected
        try:
            indices = parse_selection(answer, categories)
        except UnsupportedFeatureError as exc:
            print(f"Invalid selection: {exc}", file=sys.stderr)
            continue
        if indices:
            return [categories[i] for i in indices]
        print("Select at least one category.", file=sys.stderr)


def choose_categories_curses(categories: Sequence[Category]) -> List[Category]:
    if not categories:
        return []

    root, nodes = build_category_tree(categories)
    index_by_key = category_index_by_key(categories)
    counts = subtree_unique_test_counts(categories, root, index_by_key)
    selected_keys: Set[str] = set()
    for index in default_category_indices(categories):
        select_subtree(selected_keys, nodes[categories[index].key], index_by_key)
    expanded: Set[str] = {"all"}
    for key in selected_keys:
        node = nodes.get(key)
        while node is not None and node.parent is not None:
            expanded.add(node.parent.key)
            node = node.parent
    initial_visible: List[CategoryNode] = []

    def collect_initial_visible(node: CategoryNode) -> None:
        initial_visible.append(node)
        if node.children and node.key in expanded:
            for child in node.children:
                collect_initial_visible(child)

    collect_initial_visible(root)
    cursor = next(
        (i for i, node in enumerate(initial_visible) if node.key in selected_keys),
        0,
    )
    top = 0

    def visible_nodes() -> List[CategoryNode]:
        visible: List[CategoryNode] = []

        def visit(node: CategoryNode) -> None:
            visible.append(node)
            if node.children and node.key in expanded:
                for child in node.children:
                    visit(child)

        visit(root)
        return visible

    def draw(stdscr: "curses._CursesWindow") -> None:
        stdscr.erase()
        height, width = stdscr.getmaxyx()
        list_height = max(1, height - 4)
        width = max(width, 20)

        title = "Litmus categories"
        help_line = "Up/Down move  Space select  Left/Right collapse/expand  a all  n none  Enter build  q cancel"
        stdscr.addnstr(0, 0, title, width - 1, curses.A_BOLD)
        stdscr.addnstr(1, 0, help_line, width - 1)

        current_visible = visible_nodes()
        for row in range(list_height):
            idx = top + row
            if idx >= len(current_visible):
                break
            node = current_visible[idx]
            depth = category_depth(Category(node.key, Path("."), 0))
            indent = "  " * depth
            if node.children:
                fold = "-" if node.key in expanded else "+"
            else:
                fold = " "
            state = node_selection_state(node, selected_keys, index_by_key)
            marker = "x" if state == "checked" else ("-" if state == "partial" else " ")
            count = counts.get(node.key, node.count)
            line = f"[{marker}] {fold} {indent}{node.label:<44} {count:>6} tests"
            attr = curses.A_REVERSE if idx == cursor else curses.A_NORMAL
            stdscr.addnstr(row + 3, 0, line, width - 1, attr)

        selected_indices = selected_indices_from_keys(selected_keys, categories)
        selected_names_full = [categories[i].key for i in selected_indices]
        sample_names = selected_names_full[:3]
        suffix = "" if len(selected_names_full) <= 3 else ", ..."
        selected_names = ", ".join(sample_names) + suffix if sample_names else "none"
        footer = f"selected {len(selected_names_full)} categories: {selected_names}"
        stdscr.addnstr(height - 1, 0, footer, width - 1)
        stdscr.refresh()

    def run(stdscr: "curses._CursesWindow") -> List[int]:
        nonlocal cursor, top, selected_keys
        try:
            curses.curs_set(0)
        except curses.error:
            pass
        stdscr.keypad(True)

        while True:
            height, _ = stdscr.getmaxyx()
            list_height = max(1, height - 4)
            current_visible = visible_nodes()
            cursor = min(cursor, max(0, len(current_visible) - 1))
            if cursor < top:
                top = cursor
            elif cursor >= top + list_height:
                top = cursor - list_height + 1
            draw(stdscr)

            key = stdscr.getch()
            if key in (curses.KEY_UP, ord("k")):
                cursor = max(0, cursor - 1)
            elif key in (curses.KEY_DOWN, ord("j")):
                cursor = min(len(current_visible) - 1, cursor + 1)
            elif key in (curses.KEY_PPAGE,):
                cursor = max(0, cursor - list_height)
            elif key in (curses.KEY_NPAGE,):
                cursor = min(len(current_visible) - 1, cursor + list_height)
            elif key in (curses.KEY_HOME,):
                cursor = 0
            elif key in (curses.KEY_END,):
                cursor = len(current_visible) - 1
            elif key == ord(" "):
                node = current_visible[cursor]
                state = node_selection_state(node, selected_keys, index_by_key)
                if state == "checked":
                    deselect_subtree(selected_keys, node, index_by_key)
                else:
                    select_subtree(selected_keys, node, index_by_key)
            elif key in (curses.KEY_RIGHT, ord("l")):
                node = current_visible[cursor]
                if node.children:
                    expanded.add(node.key)
            elif key in (curses.KEY_LEFT, ord("h")):
                node = current_visible[cursor]
                if node.children and node.key in expanded:
                    expanded.remove(node.key)
                elif node.parent is not None:
                    parent_pos = next(
                        (i for i, visible in enumerate(current_visible) if visible.key == node.parent.key),
                        cursor,
                    )
                    cursor = parent_pos
            elif key in (ord("a"), ord("A")):
                selected_keys.clear()
                select_subtree(selected_keys, root, index_by_key)
            elif key in (ord("n"), ord("N")):
                selected_keys.clear()
            elif key in (ord("\n"), curses.KEY_ENTER, 10, 13):
                indices = selected_indices_from_keys(selected_keys, categories)
                if indices:
                    return indices
            elif key in (ord("q"), ord("Q"), 27):
                raise UserCancelledError

    indices = curses.wrapper(run)
    return [categories[i] for i in indices]


def choose_categories_interactive(categories: Sequence[Category]) -> List[Category]:
    if sys.stdin.isatty() and sys.stdout.isatty():
        try:
            return choose_categories_curses(categories)
        except curses.error as exc:
            print(f"curses UI unavailable ({exc}); falling back to prompt.", file=sys.stderr)
    return choose_categories_prompt(categories)


def resolve_selected_inputs(
    repo_root: Path,
    input_path: Optional[Path],
    category_names: Optional[Sequence[str]],
    interactive: bool,
) -> Tuple[List[Path], List[str]]:
    if input_path is not None:
        full_input = (
            (repo_root / input_path).resolve()
            if not input_path.is_absolute()
            else input_path.expanduser().resolve()
        )
        return expand_inputs(full_input), [str(full_input)]

    categories = discover_categories(repo_root)
    if category_names:
        selected_keys: Set[str] = set()
        for category_name in category_names:
            for index in parse_selection(category_name, categories):
                selected_keys.add(categories[index].key)
        selected = [categories[i] for i in selected_indices_from_keys(selected_keys, categories)]
    elif interactive:
        selected = choose_categories_interactive(categories)
    else:
        selected = default_selected_categories(categories)

    paths: List[Path] = []
    seen: Set[Path] = set()
    for category in selected:
        for path in expand_inputs(category.path):
            resolved = path.resolve()
            if resolved not in seen:
                seen.add(resolved)
                paths.append(resolved)
    return paths, [category.key for category in selected]


def write_runtime_header(path: Path) -> None:
    path.write_text(
        """#ifndef LITMUS_RUNTIME_H
#define LITMUS_RUNTIME_H

#include <am.h>
#include <xsextra.h>
#include <klib.h>
#include <stdint.h>

#define LITMUS_COND_EXISTS 0
#define LITMUS_COND_NOT_EXISTS 1
#define LITMUS_COND_FORALL 2

typedef struct litmus_test {
  const char *name;
  const char *condition;
  int condition_kind;
  void *ctx;
  void (*init)(void *ctx);
  void (*run_p0)(void *ctx);
  void (*run_p1)(void *ctx);
  int (*check)(void *ctx);
  uint64_t matches;
} litmus_test_t;

#endif
""",
        encoding="utf-8",
    )


def write_registry_header(path: Path) -> None:
    path.write_text(
        """#ifndef LITMUS_REGISTRY_H
#define LITMUS_REGISTRY_H

#include "litmus_runtime.h"

extern litmus_test_t *litmus_tests[];
extern const int litmus_test_count;

#endif
""",
        encoding="utf-8",
    )


def write_main_c(path: Path) -> None:
    path.write_text(
        """#include "litmus_registry.h"

#ifndef LITMUS_ITERS
#define LITMUS_ITERS 10
#endif

#ifndef LITMUS_PERTURB
#define LITMUS_PERTURB 0
#endif

#ifndef LITMUS_PERTURB_MASK
#define LITMUS_PERTURB_MASK 0x3ffu
#endif

static inline uint32_t litmus_lcg_next(uint32_t x) {
  return x * 1664525u + 1013904223u;
}

static inline uint32_t litmus_perturb(uint32_t seed) {
#if LITMUS_PERTURB
  seed = litmus_lcg_next(seed);
  uint32_t delay = (seed & LITMUS_PERTURB_MASK);
  for (uint32_t k = 0; k < delay; k++) {
    asm volatile("" ::: "memory");
  }
#endif
  return seed;
}

static void litmus_report(litmus_test_t *t) {
  uint64_t check_true = t->matches;
  uint64_t check_false = ((uint64_t)LITMUS_ITERS) - check_true;
  uint64_t cond_true = check_true;
  uint64_t cond_false = check_false;
  const char *class_name = "Allowed";
  int validated = 0;

  if (t->condition_kind == LITMUS_COND_EXISTS) {
    class_name = "Allowed";
    validated = (cond_true > 0);
  } else if (t->condition_kind == LITMUS_COND_NOT_EXISTS) {
    class_name = "Forbidden";
    validated = (cond_true == 0);
  } else {
    class_name = "Required";
    // check() counts counterexamples for forall (final_ok = !final_cond).
    cond_true = check_false;
    cond_false = check_true;
    validated = (cond_false == 0);
  }

  const char *obs = (!cond_true) ? "Never" : (!cond_false ? "Always" : "Sometimes");
  printf("[litmus] Test %s %s\\n"
         "[litmus] Condition %s is %svalidated\\n"
         "[litmus] Observation %s %s %lu %lu\\n\\n",
         t->name, class_name, t->condition, validated ? "" : "NOT ",
         t->name, obs, cond_true, cond_false);
}

static void litmus_entry_p0(uint32_t seed) {
  for (int i = 0; i < litmus_test_count; i++) {
    litmus_test_t *t = litmus_tests[i];
    void *ctx = t->ctx;
    void (*init)(void *) = t->init;
    void (*run)(void *) = t->run_p0;
    int (*check)(void *) = t->check;
    uint32_t test_salt = ((uint32_t)(i + 1) * 0x27d4eb2du);
    uint32_t iter_salt = 0;

    t->matches = 0;
    _barrier();

    for (int it = 0; it < LITMUS_ITERS; it++) {
      init(ctx);
      _barrier();

      iter_salt += 0x165667b1u;
      seed ^= test_salt;
      seed ^= iter_salt;
      seed = litmus_perturb(seed);
      run(ctx);
      _barrier();

      if (check(ctx)) {
        t->matches++;
      }
      _barrier();
    }

    litmus_report(t);
    _barrier();
  }

  printf("[litmus] done. tests=%d\\n", litmus_test_count);
  _halt(0);
}

static void litmus_entry_p1(uint32_t seed) {
  for (int i = 0; i < litmus_test_count; i++) {
    litmus_test_t *t = litmus_tests[i];
    void *ctx = t->ctx;
    void (*run)(void *) = t->run_p1;
    uint32_t test_salt = ((uint32_t)(i + 1) * 0x27d4eb2du);
    uint32_t iter_salt = 0;

    _barrier();

    for (int it = 0; it < LITMUS_ITERS; it++) {
      _barrier();

      iter_salt += 0x165667b1u;
      seed ^= test_salt;
      seed ^= iter_salt;
      seed = litmus_perturb(seed);
      run(ctx);
      _barrier();

      _barrier();
    }

    _barrier();
  }

  while (1) {
  }
}

static void litmus_entry(void) {
  int cpu = _cpu();
  uint32_t seed = 0x9e3779b9u ^ ((uint32_t)cpu * 0x85ebca6bu);

  if (cpu == 0) {
    litmus_entry_p0(seed);
  } else if (cpu == 1) {
    litmus_entry_p1(seed);
  }

  while (1) {
  }
}

int main(void) {
  _mpe_setncpu('2');
  _mpe_init(litmus_entry);
  return 0;
}
""",
        encoding="utf-8",
    )


def emit_reg_load_expr(value: str, mem_field: Dict[str, str]) -> str:
    if is_int_literal(value):
        return int_literal_to_c(value)
    if value not in mem_field:
        raise UnsupportedFeatureError(f"register init references unknown symbol: {value}")
    return f"((uintptr_t)&ctx->{mem_field[value]})"


def asm_operand_name(reg: str) -> str:
    return f"{reg}"


def rewrite_inline_asm_operands(operands: str) -> Tuple[str, Set[str]]:
    token_re = re.compile(r"\b[A-Za-z_][A-Za-z0-9_]*\b")
    used_regs: Set[str] = set()

    def repl(m: re.Match[str]) -> str:
        tok = m.group(0)
        low = tok.lower()
        if low in REG_ALIASES:
            reg = canonical_reg(low)
            if reg == "x0":
                return "x0"
            used_regs.add(reg)
            return f"%[{asm_operand_name(reg)}]"
        return tok

    return token_re.sub(repl, operands), used_regs


def split_asm_operands(operands: str) -> List[str]:
    if not operands:
        return []
    return [part.strip() for part in operands.split(",") if part.strip()]


def regs_in_operand(text: str) -> Set[str]:
    regs: Set[str] = set()
    for tok in re.findall(r"\b[A-Za-z_][A-Za-z0-9_]*\b", text):
        low = tok.lower()
        if low in REG_ALIASES:
            reg = canonical_reg(low)
            if reg != "x0":
                regs.add(reg)
    return regs


def first_operand_reg(operand: str) -> Optional[str]:
    regs = sorted(regs_in_operand(operand), key=lambda r: operand.find(r))
    return regs[0] if regs else None


def classify_asm_operands(opcode: str, operands: str) -> Tuple[Set[str], Set[str]]:
    """Return (read_regs, written_regs) for the supported RISC-V subset.

    Unknown opcodes fall back to old conservative behavior by treating every
    operand register as read/write.
    """
    op = opcode.lower()
    base = op.split(".", 1)[0]
    parts = split_asm_operands(operands)
    all_regs = regs_in_operand(operands)
    read_regs: Set[str] = set()
    written_regs: Set[str] = set()

    def write_first() -> None:
        if parts:
            reg = first_operand_reg(parts[0])
            if reg is not None:
                written_regs.add(reg)

    def read_operands(start: int = 0, stop: Optional[int] = None) -> None:
        for part in parts[start:stop]:
            read_regs.update(regs_in_operand(part))

    load_ops = {"lb", "lbu", "lh", "lhu", "lw", "lwu", "ld"}
    store_ops = {"sb", "sh", "sw", "sd"}
    branch_ops = {
        "beq",
        "bne",
        "blt",
        "bge",
        "bltu",
        "bgeu",
        "beqz",
        "bnez",
        "blez",
        "bgez",
        "bltz",
        "bgtz",
    }
    r_type_ops = {
        "add",
        "sub",
        "sll",
        "slt",
        "sltu",
        "xor",
        "srl",
        "sra",
        "or",
        "and",
        "addw",
        "subw",
        "sllw",
        "srlw",
        "sraw",
    }
    i_type_ops = {
        "addi",
        "slti",
        "sltiu",
        "xori",
        "ori",
        "andi",
        "slli",
        "srli",
        "srai",
        "addiw",
        "slliw",
        "srliw",
        "sraiw",
    }

    if base in load_ops:
        write_first()
        read_operands(1)
    elif base in store_ops:
        read_operands(0)
    elif base in branch_ops:
        read_operands(0, 2)
    elif base in r_type_ops:
        write_first()
        read_operands(1)
    elif base in i_type_ops:
        write_first()
        read_operands(1, 2)
    elif base in {"lui", "auipc", "li"}:
        write_first()
    elif base == "mv":
        write_first()
        read_operands(1, 2)
    elif base == "not":
        write_first()
        read_operands(1, 2)
    elif base == "neg":
        write_first()
        read_operands(1, 2)
    elif base in {"seqz", "snez", "sltz", "sgtz"}:
        write_first()
        read_operands(1, 2)
    elif base == "lr":
        write_first()
        read_operands(1)
    elif base == "sc":
        write_first()
        read_operands(1)
    elif base.startswith("amo"):
        write_first()
        read_operands(1)
    elif base in {"fence", "fence.i", "nop"}:
        pass
    elif base == "jal":
        write_first()
    elif base == "jalr":
        write_first()
        read_operands(1)
    else:
        read_regs = set(all_regs)
        written_regs = set(all_regs)

    read_regs.discard("x0")
    written_regs.discard("x0")
    return read_regs, written_regs


def rewrite_thread_inline_asm(instrs: List[str]) -> AsmOperandUse:
    out: List[str] = []
    used_regs: Set[str] = set()
    written_regs: Set[str] = set()
    read_before_write_regs: Set[str] = set()
    label_def = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$")

    for ins in instrs:
        line = ins.strip()
        if not line:
            continue
        label: Optional[str] = None
        body = line
        m = label_def.match(line)
        if m:
            label = m.group(1)
            body = m.group(2).strip()
            if not body:
                out.append(f"{label}:")
                continue

        parts = body.split(None, 1)
        opcode = parts[0]
        operands = parts[1].strip() if len(parts) > 1 else ""
        if operands:
            read_regs, instr_written_regs = classify_asm_operands(opcode, operands)
            read_before_write_regs |= read_regs - written_regs
            written_regs |= instr_written_regs
            operands, regs = rewrite_inline_asm_operands(operands)
            used_regs |= regs
            body = f"{opcode} {operands}".rstrip()
        else:
            body = opcode

        if label is not None:
            out.append(f"{label}: {body}")
        else:
            out.append(body)

    return AsmOperandUse(
        instrs=out,
        used_regs=used_regs,
        written_regs=written_regs & used_regs,
        read_before_write_regs=read_before_write_regs & used_regs,
    )


def reg_capture_field(hart: int, reg: str) -> str:
    return f"reg_p{hart}_{reg}"


def asm_opcode_from_line(line: str) -> Optional[str]:
    text = line.strip()
    if not text:
        return None
    label_def = re.compile(r"^\s*([A-Za-z_][A-Za-z0-9_.]*)\s*:\s*(.*)$")
    m = label_def.match(text)
    if m:
        text = m.group(2).strip()
    if not text:
        return None
    return text.split(None, 1)[0].lower()


def thread_has_control_flow(instrs: List[str]) -> bool:
    branch_ops = {
        "beq",
        "bne",
        "blt",
        "bge",
        "bltu",
        "bgeu",
        "beqz",
        "bnez",
        "blez",
        "bgez",
        "bltz",
        "bgtz",
        "jal",
        "jalr",
    }
    for ins in instrs:
        op = asm_opcode_from_line(ins)
        if op in branch_ops:
            return True
    return False


def build_test_c(test: ParsedTest) -> str:
    mem_field = {sym: f"mem_{sanitize_ident(sym)}" for sym in test.mem_symbols}
    captured_regs = sorted(
        ((hart, reg) for hart, reg in test.cond_regs if reg != "x0"),
        key=lambda item: (item[0], reg_index(item[1])),
    )
    reg_field = {
        (hart, reg): reg_capture_field(hart, reg) for hart, reg in captured_regs
    }
    cond_c = expr_to_c(test.cond_expr, mem_field, reg_field)

    lines: List[str] = []
    lines.append('#include "litmus_runtime.h"')
    lines.append("")
    ctx_type = f"ctx_{test.test_id}_t"
    lines.append(f"typedef struct {ctx_type} {{")
    for sym in test.mem_symbols:
        lines.append(f"  volatile uint64_t {mem_field[sym]};")
    for hart, reg in captured_regs:
        lines.append(f"  uint64_t {reg_field[(hart, reg)]};")
    if not test.mem_symbols and not captured_regs:
        lines.append("  uint8_t unused;")
    lines.append(f"}} {ctx_type};")
    lines.append("")
    lines.append(f"static {ctx_type} g_ctx;")
    lines.append("")

    lines.append("static void init(void *opaque) {")
    lines.append(f"  {ctx_type} *ctx = ({ctx_type} *)opaque;")
    lines.append("  (void)ctx;")
    for sym in test.mem_symbols:
        init_val = test.mem_inits.get(sym, "0")
        lines.append(f"  ctx->{mem_field[sym]} = {int_literal_to_c(init_val)};")
    lines.append("}")
    lines.append("")

    for hart in (0, 1):
        fn = f"run_p{hart}"
        lines.append(f"static void {fn}(void *opaque) {{")
        lines.append(f"  {ctx_type} *ctx = ({ctx_type} *)opaque;")

        thread_regs: Set[str] = set()
        for ins in test.thread_instrs[hart]:
            thread_regs |= extract_registers(ins)
        thread_regs |= set(test.reg_inits[hart].keys())
        thread_regs |= {r for h, r in test.cond_regs if h == hart}
        thread_regs.discard("x0")
        ordered_regs = sorted(thread_regs, key=reg_index)

        asm_use = rewrite_thread_inline_asm(test.thread_instrs[hart])
        if asm_use.used_regs - set(ordered_regs):
            missing = ", ".join(sorted(asm_use.used_regs - set(ordered_regs), key=reg_index))
            raise UnsupportedFeatureError(f"missing asm operands for thread registers: {missing}")

        needs_initial_value = set(test.reg_inits[hart].keys())
        if thread_has_control_flow(test.thread_instrs[hart]):
            needs_initial_value |= asm_use.written_regs | asm_use.read_before_write_regs
        else:
            needs_initial_value |= asm_use.read_before_write_regs
        needs_initial_value |= ({r for h, r in test.cond_regs if h == hart} - asm_use.written_regs)
        needs_initial_value.discard("x0")

        for reg in ordered_regs:
            if reg in needs_initial_value:
                if reg in test.reg_inits[hart]:
                    init_expr = emit_reg_load_expr(test.reg_inits[hart][reg], mem_field)
                else:
                    init_expr = int_literal_to_c("0")
                lines.append(f"  uint64_t {asm_operand_name(reg)} = {init_expr};")
            else:
                lines.append(f"  uint64_t {asm_operand_name(reg)};")

        input_value_regs = set(asm_use.read_before_write_regs)
        if thread_has_control_flow(test.thread_instrs[hart]):
            input_value_regs |= asm_use.written_regs
        readwrite_regs = sorted(asm_use.written_regs & input_value_regs, key=reg_index)
        writeonly_regs = sorted(asm_use.written_regs - input_value_regs, key=reg_index)
        inputonly_regs = sorted(input_value_regs - asm_use.written_regs, key=reg_index)

        asm_outputs = [
            f'[{asm_operand_name(reg)}] "+&r"({asm_operand_name(reg)})'
            for reg in readwrite_regs
        ]
        asm_outputs.extend(
            f'[{asm_operand_name(reg)}] "=&r"({asm_operand_name(reg)})'
            for reg in writeonly_regs
        )
        asm_inputs = [
            f'[{asm_operand_name(reg)}] "r"({asm_operand_name(reg)})'
            for reg in inputonly_regs
        ]

        lines.append("  asm volatile(")
        if asm_use.instrs:
            for ins in asm_use.instrs:
                escaped = ins.replace('"', '\\"')
                lines.append(f'      "{escaped}\\n\\t"')
        else:
            lines.append('      ""')
        lines.append(f"      : {', '.join(asm_outputs)}" if asm_outputs else "      :")
        lines.append(f"      : {', '.join(asm_inputs)}" if asm_inputs else "      :")
        lines.append('      : "memory", "cc");')

        cond_regs = sorted((r for h, r in test.cond_regs if h == hart), key=reg_index)
        for reg in cond_regs:
            if reg == "x0":
                continue
            else:
                lines.append(f"  ctx->{reg_field[(hart, reg)]} = {asm_operand_name(reg)};")
        lines.append("}")
        lines.append("")

    lines.append("static int check(void *opaque) {")
    lines.append(f"  {ctx_type} *ctx = ({ctx_type} *)opaque;")
    lines.append("  (void)ctx;")
    if test.cond_kind == "forall":
        lines.append(f"  return !({cond_c});")
    else:
        lines.append(f"  return ({cond_c});")
    lines.append("}")
    lines.append("")

    cond_kind_macro = {
        "exists": "LITMUS_COND_EXISTS",
        "not_exists": "LITMUS_COND_NOT_EXISTS",
        "forall": "LITMUS_COND_FORALL",
    }[test.cond_kind]
    lines.append(f"litmus_test_t litmus_{test.test_id} = {{")
    lines.append(f'  .name = "{test.rel_source}",')
    lines.append(f"  .condition = {c_string_literal(test.cond_text)},")
    lines.append(f"  .condition_kind = {cond_kind_macro},")
    lines.append("  .ctx = &g_ctx,")
    lines.append("  .init = init,")
    lines.append("  .run_p0 = run_p0,")
    lines.append("  .run_p1 = run_p1,")
    lines.append("  .check = check,")
    lines.append("  .matches = 0,")
    lines.append("};")
    lines.append("")
    return "\n".join(lines)


def write_registry_c(path: Path, tests: Sequence[ParsedTest]) -> None:
    lines: List[str] = ['#include "litmus_registry.h"', ""]
    for t in tests:
        lines.append(f"extern litmus_test_t litmus_{t.test_id};")
    lines.append("")
    lines.append("litmus_test_t *litmus_tests[] = {")
    for t in tests:
        lines.append(f"  &litmus_{t.test_id},")
    lines.append("};")
    lines.append("")
    lines.append("const int litmus_test_count = sizeof(litmus_tests) / sizeof(litmus_tests[0]);")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def generate_to_output(tests: Sequence[ParsedTest], output_dir: Path, clean: bool) -> None:
    include_dir = output_dir / "include"
    src_dir = output_dir / "src"
    test_dir = src_dir / "tests"

    if clean and output_dir.exists():
        for p in [
            include_dir,
            src_dir,
            output_dir / "Makefile",
            output_dir / "conversion-report.json",
            output_dir / ".litmus-generated.stamp",
        ]:
            if p.is_dir():
                shutil.rmtree(p)
            elif p.exists():
                p.unlink()

    include_dir.mkdir(parents=True, exist_ok=True)
    test_dir.mkdir(parents=True, exist_ok=True)

    write_runtime_header(include_dir / "litmus_runtime.h")
    write_registry_header(include_dir / "litmus_registry.h")
    write_main_c(src_dir / "main.c")
    write_registry_c(src_dir / "registry.c", tests)

    for t in tests:
        test_file = test_dir / f"{t.test_id}.c"
        test_file.write_text(build_test_c(t), encoding="utf-8")


def convert_tests(
    repo_root: Path, litmus_paths: Sequence[Path], max_tests: Optional[int]
) -> ConversionResult:
    converted: List[ParsedTest] = []
    skipped: List[Dict[str, str]] = []

    for path in litmus_paths:
        try:
            if not path.is_file():
                raise UnsupportedFeatureError(f"missing litmus file: {path}")
            parsed = parse_litmus_file(path, repo_root)
            converted.append(parsed)
        except UnsupportedFeatureError as exc:
            skipped.append({"path": relpath_for_report(path, repo_root), "reason": str(exc)})
        if max_tests is not None and len(converted) >= max_tests:
            break

    return ConversionResult(converted=converted, skipped=skipped)


def build_arg_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Convert 2-thread RISC-V litmus tests into nexus-am dual-core bare-metal tests."
    )
    p.add_argument(
        "--repo-root",
        type=Path,
        default=DEFAULT_REPO_ROOT,
        help=f"Root of litmus-tests-riscv repository (default: {DEFAULT_REPO_ROOT}).",
    )
    p.add_argument(
        "--input",
        type=Path,
        default=None,
        help="Input litmus file, directory, or @set file. Overrides category selection.",
    )
    p.add_argument(
        "--category",
        action="append",
        default=None,
        help=(
            "Generate one category by name/number/range. Can be repeated. "
            "Use --list-categories to inspect names."
        ),
    )
    p.add_argument(
        "--interactive",
        action="store_true",
        help="Show an interactive category selector before generating.",
    )
    p.add_argument(
        "--list-categories",
        action="store_true",
        help="List available litmus categories and exit.",
    )
    p.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=f"Output AM test directory (default: {DEFAULT_OUTPUT_DIR}).",
    )
    p.add_argument(
        "--max-tests",
        type=int,
        default=None,
        help="Limit number of converted tests (after filtering unsupported cases).",
    )
    p.add_argument(
        "--clean",
        action="store_true",
        help="Clean generated output files in output directory before writing.",
    )
    p.add_argument(
        "--strict",
        action="store_true",
        help="Exit non-zero if any test is skipped.",
    )
    return p


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_arg_parser().parse_args(argv)
    repo_root = args.repo_root.expanduser().resolve()
    output_dir = args.output_dir.expanduser().resolve()

    if args.list_categories:
        print_categories(discover_categories(repo_root))
        return 0

    try:
        litmus_paths, selected_inputs = resolve_selected_inputs(
            repo_root=repo_root,
            input_path=args.input,
            category_names=args.category,
            interactive=args.interactive,
        )
    except UserCancelledError:
        print("Cancelled.")
        return 0
    result = convert_tests(repo_root, litmus_paths, args.max_tests)

    if not result.converted:
        print("No tests converted. Check conversion-report for skip reasons.", file=sys.stderr)

    generate_to_output(result.converted, output_dir, args.clean)

    report = {
        "repo_root": str(repo_root),
        "input": selected_inputs,
        "output_dir": str(output_dir),
        "candidate_count": len(litmus_paths),
        "converted_count": len(result.converted),
        "skipped_count": len(result.skipped),
        "converted": [t.rel_source for t in result.converted],
        "skipped": result.skipped,
    }
    (output_dir / "conversion-report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True), encoding="utf-8"
    )
    (output_dir / ".litmus-generated.stamp").write_text("ok\n", encoding="utf-8")

    print(f"Converted: {len(result.converted)}")
    print(f"Skipped:   {len(result.skipped)}")
    print(f"Selected:  {', '.join(selected_inputs)}")
    print(f"Output:    {output_dir}")
    print(f"Report:    {output_dir / 'conversion-report.json'}")

    if args.strict and result.skipped:
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
