#!/usr/bin/env python3
"""Export the selected semantic-analysis Codex chats as private Markdown."""

from __future__ import annotations

import base64
import json
import re
import shutil
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_ROOT / "output" / "语义分析实验对话记录"
ASSET_DIR = OUTPUT_DIR / "assets"

SESSIONS = [
    (
        "01-检查语义分析的课程文档，规划下面的实现方案",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/04/rollout-2026-10-04T23-04-52-01a10772-45c9-7431-a4ad-33eed98e4add.jsonl"),
    ),
    (
        "02-完善 visitVarDecl 实现",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/05/rollout-2026-10-05T16-09-37-01a10b1c-78cc-7553-9b05-f00081b04d5d.jsonl"),
    ),
    (
        "03-查询语义分析阶段",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/07/rollout-2026-10-07T11-16-22-01a1145c-b47e-7ea2-91fc-6f42c00f0f7c.jsonl"),
    ),
    (
        "04-Clarify visitMulExp operand types",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/07/rollout-2026-10-07T11-39-05-01a11471-82d8-7453-b999-8a7baff048f5.jsonl"),
    ),
    (
        "05-检查并规划语义分析实现",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/07/rollout-2026-10-07T14-24-46-01a11509-3143-7f93-9068-85a8cb06ecee.jsonl"),
    ),
    (
        "06-解释 ConstExp 与初始化值",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/07/rollout-2026-10-07T16-24-56-01a11577-360e-7861-bd77-5874f1f2d713.jsonl"),
    ),
    (
        "07-检查语义分析进度",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/10/07/rollout-2026-10-07T18-11-56-01a115d9-2c0e-7442-bf99-944c9cad711d.jsonl"),
    ),
]

HIDDEN_USER_PREFIXES = (
    "<environment_context>",
    "<recommended_plugins>",
    "<permissions instructions>",
)


def local_time(timestamp: str) -> str:
    if not timestamp:
        return "时间未记录"
    moment = datetime.fromisoformat(timestamp.replace("Z", "+00:00"))
    return moment.astimezone(ZoneInfo("Asia/Shanghai")).strftime("%Y-%m-%d %H:%M:%S")


def image_suffix(mime_type: str) -> str:
    return {
        "image/png": ".png",
        "image/jpeg": ".jpg",
        "image/webp": ".webp",
        "image/gif": ".gif",
    }.get(mime_type, ".bin")


def export_session(title: str, session_path: Path) -> tuple[int, int]:
    metadata: dict = {}
    messages: list[dict] = []

    with session_path.open(encoding="utf-8") as source:
        for raw_line in source:
            try:
                record = json.loads(raw_line)
            except json.JSONDecodeError:
                continue

            payload = record.get("payload", {})
            if record.get("type") == "session_meta":
                metadata = payload
                continue
            if record.get("type") != "response_item" or payload.get("type") != "message":
                continue
            if payload.get("role") not in {"user", "assistant"}:
                continue

            contents = payload.get("content", [])
            texts = [
                item.get("text", "")
                for item in contents
                if item.get("type") in {"input_text", "output_text"} and item.get("text")
            ]
            combined_text = "\n\n".join(texts).strip()
            if payload.get("role") == "user" and combined_text.startswith(HIDDEN_USER_PREFIXES):
                continue

            has_user_image = payload.get("role") == "user" and any(
                item.get("type") == "input_image" for item in contents
            )
            if not combined_text and not has_user_image:
                continue

            messages.append(
                {
                    "role": payload["role"],
                    "text": combined_text,
                    "contents": contents,
                    "timestamp": record.get("timestamp", ""),
                }
            )

    markdown = [
        f"# {title[3:]}",
        "",
        f"- 会话时间：{local_time(metadata.get('timestamp', ''))}",
        f"- 会话编号：`{metadata.get('id', metadata.get('session_id', '未知'))}`",
        "- 导出范围：仅保留用户与助手可见消息以及用户附图，已排除系统提示、开发者指令、内部推理、工具调用及工具输出。",
        "",
    ]

    image_count = 0
    for index, message in enumerate(messages, start=1):
        role_name = "用户" if message["role"] == "user" else "助手"
        markdown.extend(
            [
                f"## {index}. {role_name}",
                "",
                f"时间：{local_time(message['timestamp'])}",
                "",
            ]
        )
        if message["text"]:
            markdown.extend([message["text"], ""])

        if message["role"] == "user":
            for content in message["contents"]:
                if content.get("type") != "input_image":
                    continue
                image_count += 1
                image_url = content.get("image_url", "")
                match = re.fullmatch(
                    r"data:([^;]+);base64,(.+)", image_url, flags=re.DOTALL
                )
                if match:
                    mime_type, encoded = match.groups()
                    image_name = f"{title[:2]}-image-{image_count}{image_suffix(mime_type)}"
                    (ASSET_DIR / image_name).write_bytes(base64.b64decode(encoded))
                    markdown.extend([f"![用户附图 {image_count}](assets/{image_name})", ""])
                elif image_url:
                    markdown.extend([f"![用户附图 {image_count}]({image_url})", ""])
                else:
                    markdown.extend([f"[用户附图 {image_count}：原图数据不可用]", ""])

        markdown.extend(["---", ""])

    (OUTPUT_DIR / f"{title}.md").write_text(
        "\n".join(markdown).rstrip() + "\n", encoding="utf-8"
    )
    return len(messages), image_count


def main() -> None:
    if OUTPUT_DIR.exists():
        shutil.rmtree(OUTPUT_DIR)
    ASSET_DIR.mkdir(parents=True)

    rows = []
    for title, session_path in SESSIONS:
        if not session_path.is_file():
            raise FileNotFoundError(f"会话文件不存在：{session_path}")
        rows.append((title, *export_session(title, session_path)))

    readme = [
        "# 语义分析实验 AI 对话记录",
        "",
        "本目录收录截图所列的 7 个 Codex 会话，按会话时间从早到晚排列。内容按原始会话顺序导出，只保留用户和助手可见消息及用户附图。",
        "",
        "| 文件 | 消息数 | 附图数 |",
        "|---|---:|---:|",
    ]
    readme.extend(
        f"| [{title}.md]({title}.md) | {count} | {images} |"
        for title, count, images in rows
    )
    readme.extend(
        [
            "",
            "说明：系统提示、开发者指令、内部推理、工具调用及工具输出均未包含在提交记录中。",
            "",
        ]
    )
    (OUTPUT_DIR / "README.md").write_text("\n".join(readme), encoding="utf-8")

    print(f"exported={len(rows)}")
    for title, count, images in rows:
        print(f"{title}\tmessages={count}\timages={images}")


if __name__ == "__main__":
    main()
