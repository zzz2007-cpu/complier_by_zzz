#!/usr/bin/env python3
"""Export selected Codex conversations as submission-friendly Markdown."""

from __future__ import annotations

import base64
import json
import re
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_ROOT / "output" / "词法分析实验对话记录"
ASSET_DIR = OUTPUT_DIR / "assets"

SESSIONS = [
    (
        "01-词法分析入门与骨架",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/20/rollout-2026-09-20T16-31-30-01a0bdf1-1d01-77e3-868b-f48aa7b5babb.jsonl"),
    ),
    (
        "02-Token与运算符设计",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/22/rollout-2026-09-22T23-31-59-01a0c9be-cc8b-7620-b3e7-a1bff0cb5252.jsonl"),
    ),
    (
        "03-项目编译运行检查",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/24/rollout-2026-09-24T10-35-27-01a0d144-94d9-7fc3-ab73-e69a8c5afff4.jsonl"),
    ),
    (
        "04-项目运行方法",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/29/rollout-2026-09-29T13-49-30-01a0ebb6-080a-7641-a08b-7275a9ec7dee.jsonl"),
    ),
    (
        "05-src提交文件压缩命令",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/29/rollout-2026-09-29T14-43-57-01a0ebe7-e2d0-7223-9ded-7e87446b12e9.jsonl"),
    ),
    (
        "06-词法分析实验代码打包与测试",
        Path("/home/zzz2007-cpu/.codex/sessions/2026/09/29/rollout-2026-09-29T15-01-08-01a0ebf7-9b97-75f0-9b17-3c5ce662e84e.jsonl"),
    ),
]

HIDDEN_USER_PREFIXES = ("<environment_context>", "<recommended_plugins>")


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

            if record.get("type") != "response_item":
                continue
            if payload.get("type") != "message":
                continue
            if payload.get("role") not in {"user", "assistant"}:
                continue

            contents = payload.get("content", [])
            texts = [
                item.get("text", "")
                for item in contents
                if item.get("type") in {"input_text", "output_text"}
                and item.get("text")
            ]
            combined_text = "\n\n".join(texts).strip()
            if payload.get("role") == "user" and combined_text.startswith(HIDDEN_USER_PREFIXES):
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
        f"# {title}",
        "",
        f"- 会话时间：{local_time(metadata.get('timestamp', ''))}",
        f"- 会话编号：`{metadata.get('id', metadata.get('session_id', '未知'))}`",
        "- 导出范围：仅保留用户与助手可见消息，已排除系统提示、内部推理和工具调用。",
        "",
    ]

    image_count = 0
    assistant_count = 0
    for index, message in enumerate(messages, start=1):
        role_name = "用户" if message["role"] == "user" else "助手"
        assistant_count += message["role"] == "assistant"
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

        for content in message["contents"]:
            if content.get("type") != "input_image":
                continue
            image_count += 1
            image_url = content.get("image_url", "")
            match = re.fullmatch(r"data:([^;]+);base64,(.+)", image_url, flags=re.DOTALL)
            if match:
                mime_type, encoded = match.groups()
                image_name = f"{title[:2]}-image-{image_count}{image_suffix(mime_type)}"
                image_path = ASSET_DIR / image_name
                image_path.write_bytes(base64.b64decode(encoded))
                markdown.extend([f"![用户附图 {image_count}](assets/{image_name})", ""])
            elif image_url:
                markdown.extend([f"![用户附图 {image_count}]({image_url})", ""])
            else:
                markdown.extend([f"[用户附图 {image_count}：原图数据不可用]", ""])

        markdown.extend(["---", ""])

    if assistant_count == 0:
        markdown.extend(["> 注：此会话记录中仅保存了用户消息，没有助手回复。", ""])

    output_path = OUTPUT_DIR / f"{title}.md"
    output_path.write_text("\n".join(markdown).rstrip() + "\n", encoding="utf-8")
    return len(messages), image_count


def main() -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    ASSET_DIR.mkdir(parents=True, exist_ok=True)

    summary_rows = []
    for title, session_path in SESSIONS:
        if not session_path.is_file():
            raise FileNotFoundError(f"会话文件不存在：{session_path}")
        message_count, image_count = export_session(title, session_path)
        summary_rows.append((title, message_count, image_count))

    readme = [
        "# 词法分析实验 AI 对话记录",
        "",
        "本目录收录编译实验相关的 6 个 Codex 会话。内容按原始会话顺序导出，只保留用户和助手可见消息。",
        "",
        "| 文件 | 消息数 | 附图数 |",
        "|---|---:|---:|",
    ]
    for title, message_count, image_count in summary_rows:
        readme.append(f"| [{title}.md]({title}.md) | {message_count} | {image_count} |")
    readme.extend(
        [
            "",
            "说明：系统提示、开发者指令、内部推理、工具调用及工具输出均未包含在提交记录中。",
            "",
        ]
    )
    (OUTPUT_DIR / "README.md").write_text("\n".join(readme), encoding="utf-8")

    print(f"exported={len(summary_rows)}")
    for title, message_count, image_count in summary_rows:
        print(f"{title}\tmessages={message_count}\timages={image_count}")


if __name__ == "__main__":
    main()
