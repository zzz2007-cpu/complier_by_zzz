#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import subprocess
import tempfile
from pathlib import Path


COMPILER = Path(
    os.environ.get("COMPILER_PATH", "/tmp/compiler-semantic-build.7SE8o6/Compiler")
)


CASES = [
    {
        "name": "official_correct_sample",
        "source": """const int year = 2026, month = 9;
int day;

int getDay(){
    static int day;
    day = get_int();
    return day;
}

int fun(){
    return 12345;
}

int main(){
    day = getDay();
    printf(\"Tody is %d-%d-%d\\n\", year, month, day);
    {
        int length = fun();
        if(length > 4){
            printf(\"%d\\n\", length);
        }
    }
    return 0;
}
""",
        "error": "",
        "symbol": """1 year ConstInt
1 month ConstInt
1 day Int
1 getDay IntFunc
1 fun IntFunc
2 day StaticInt
5 length Int
""",
    },
    {
        "name": "all_semantic_error_codes",
        "source": """int duplicated;
char duplicated;
const int fixed = 1;
void badVoid(){
    return 1;
}
int missingReturn(){
}
int takesInt(int x){ return x; }
int main(){
    fixed = 2;
    unknown = 1;
    takesInt();
    char c;
    int x;
    x = c;
    printf(\"%d %c\", x);
    continue;
    switch (x) {
        case 1: break;
        case 1: break;
    }
    return 0;
}
""",
        "error": """2 b
5 f
8 g
11 h
12 c
13 d
16 e
17 l
18 m
21 n
""",
    },
    {
        "name": "lexer_and_parser_errors",
        "source": """int main(){
    int a = 1
    if (a & 1) {
        a = 1;
    }
    if (a > 0 {
        a = 1;
    }
    int b[2;
    return 0;
}
""",
        "error": """2 i
3 a
6 j
9 k
""",
    },
    {
        "name": "types_arrays_calls_printf",
        "source": """int use(int x, char cs[]){ return x; }
int main(){
    int ia[2];
    char ca[2] = \"x\";
    char c;
    int x;
    x = ia[c];
    x = ca[1];
    x = use(c, ca);
    x = use(x, ia);
    printf(\"%s %c %d\", ca, x, c);
    ia = ia;
    x();
    return 0;
}
""",
        "error": """7 e
8 e
9 e
10 e
11 e
12 e
13 e
""",
    },
    {
        "name": "switch_and_loop_control",
        "source": """int main(){
    char c;
    int x;
    switch (c) {
        case 'a': break;
        case 99: break;
        case 'b': continue;
        case '\\n': break;
        case '\\n': break;
        default: break;
        default: break;
    }
    while (x) {
        switch (x) {
            case 0: continue;
            default: break;
        }
    }
    return 0;
}
""",
        "error": """6 e
7 m
9 n
11 n
""",
    },
    {
        "name": "scope_and_symbol_kinds",
        "source": """const char cc = 'a';
const int cia[2] = {1, 2};
char ga[3];
char f(int p, char a[]){
    static char local[2];
    {
        int p;
    }
    return cc;
}
int main(){
    return 0;
}
""",
        "error": "",
        "symbol": """1 cc ConstChar
1 cia ConstIntArray
1 ga CharArray
1 f CharFunc
2 p Int
2 a CharArray
2 local StaticCharArray
3 p Int
""",
    },
    {
        "name": "array_length_and_returns",
        "source": """char c;
int a[c];
char returnsChar(){
    return 1;
}
int main(){
    return c;
}
""",
        "error": """2 e
4 e
7 e
""",
    },
]


def run_case(case: dict[str, str]) -> dict[str, object]:
    with tempfile.TemporaryDirectory(prefix="compiler-semantic-case-") as directory:
        root = Path(directory)
        (root / "testfile.txt").write_text(case["source"], encoding="utf-8")
        result = subprocess.run(
            [str(COMPILER)], cwd=root, capture_output=True, text=True, timeout=10
        )
        error_path = root / "error.txt"
        symbol_path = root / "symbol.txt"
        error = error_path.read_text(encoding="utf-8") if error_path.exists() else "<missing>"
        symbol = symbol_path.read_text(encoding="utf-8") if symbol_path.exists() else "<missing>"
        expected_symbol = case.get("symbol")
        passed = (
            result.returncode == 0
            and error == case["error"]
            and (expected_symbol is None or symbol == expected_symbol)
        )
        return {
            "name": case["name"],
            "passed": passed,
            "returncode": result.returncode,
            "expected_error": case["error"],
            "actual_error": error,
            "expected_symbol": expected_symbol,
            "actual_symbol": symbol if expected_symbol is not None else None,
            "stdout": result.stdout,
            "stderr": result.stderr,
        }


results = [run_case(case) for case in CASES]
print(json.dumps(results, ensure_ascii=False, indent=2))
raise SystemExit(0 if all(item["passed"] for item in results) else 1)
