#!/usr/bin/env python3
"""Export the selected parser-stage Codex conversations as Markdown."""

from __future__ import annotations

import base64
import json
import re
import shutil
from datetime import datetime
from pathlib import Path
from zoneinfo import ZoneInfo


PROJECT_ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = PROJECT_ROOT / "output" / "语法分析实验对话记录"
ASSET_DIR = OUTPUT_DIR / "assets"

SESSIONS = [
    ("01-解释语法分析任务本质", Path("/home/zzz2007-cpu/.codex/sessions/2026/09/30/rollout-2026-09-30T12-22-25-01a0f08c-aa3c-79f3-9985-555c780fa103.jsonl")),
    ("02-规划语法分析实现", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/02/rollout-2026-10-02T15-37-48-01a0fb8c-2fd5-7ce1-a156-9e997955778e.jsonl")),
    ("03-规划语法分析实现", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/02/rollout-2026-10-02T15-51-20-01a0fb98-a7f0-7c30-a0eb-1cc58825c394.jsonl")),
    ("04-继续实现编译器下一阶段", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/03/rollout-2026-10-03T14-25-36-01a10070-86bd-70c2-bd44-104060a97452.jsonl")),
    ("05-规划编译器下一阶段", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/03/rollout-2026-10-03T14-51-49-01a10088-84f2-74f2-9adb-9e10663705b2.jsonl")),
    ("06-规划编译器下一阶段", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/03/rollout-2026-10-03T18-11-30-01a1013f-5569-7bb1-ba98-acce3e48ead6.jsonl")),
    ("07-规划语法分析实现", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/04/rollout-2026-10-04T13-05-30-01a1054d-8b20-76e3-b8bf-2f6a0af4a3cb.jsonl")),
    ("08-规划语法阶段下一步", Path("/home/zzz2007-cpu/.codex/sessions/2026/10/04/rollout-2026-10-04T14-44-19-01a105a8-021b-7690-86f5-458f98fde4ba.jsonl")),
]

HIDDEN_USER_PREFIXES = ("<environment_context>", "<recommended_plugins>")


def local_time(timestamp: str) -> str:
    if not timestamp:
        return "时间未记录"
    moment = datetime.fromisoformat(timestamp.replace("Z", "+00:00"))
    return moment.astimezone(ZoneInfo("Asia/Shanghai")).strftime("%Y-%m-%d %H:%M:%S")


def image_suffix(mime_type: str) -> str:
    return {"image/png": ".png", "image/jpeg": ".jpg", "image/webp": ".webp", "image/gif": ".gif"}.get(mime_type, ".bin")


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
            texts = [item.get("text", "") for item in contents if item.get("type") in {"input_text", "output_text"} and item.get("text")]
            combined_text = "\n\n".join(texts).strip()
            if payload.get("role") == "user" and combined_text.startswith(HIDDEN_USER_PREFIXES):
                continue
            messages.append({"role": payload["role"], "text": combined_text, "contents": contents, "timestamp": record.get("timestamp", "")})

    markdown = [
        f"# {title[3:]}", "",
        f"- 会话时间：{local_time(metadata.get('timestamp', ''))}",
        f"- 会话编号：`{metadata.get('id', metadata.get('session_id', '未知'))}`",
        "- 导出范围：仅保留用户与助手可见消息，已排除系统提示、开发者指令、内部推理和工具调用。", "",
    ]
    image_count = 0
    for index, message in enumerate(messages, start=1):
        role_name = "用户" if message["role"] == "user" else "助手"
        markdown.extend([f"## {index}. {role_name}", "", f"时间：{local_time(message['timestamp'])}", ""])
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
                (ASSET_DIR / image_name).write_bytes(base64.b64decode(encoded))
                markdown.extend([f"![用户附图 {image_count}](assets/{image_name})", ""])
            elif image_url:
                markdown.extend([f"![用户附图 {image_count}]({image_url})", ""])
            else:
                markdown.extend([f"[用户附图 {image_count}：原图数据不可用]", ""])
        markdown.extend(["---", ""])
    (OUTPUT_DIR / f"{title}.md").write_text("\n".join(markdown).rstrip() + "\n", encoding="utf-8")
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
        "# 语法分析实验 AI 对话记录", "",
        "本目录收录截图所列的 8 个 Codex 会话，按会话时间从早到晚排列。内容按原始会话顺序导出，只保留用户和助手可见消息。", "",
        "| 文件 | 消息数 | 附图数 |", "|---|---:|---:|",
    ]
    readme.extend(f"| [{title}.md]({title}.md) | {count} | {images} |" for title, count, images in rows)
    readme.extend(["", "说明：系统提示、开发者指令、内部推理、工具调用及工具输出均未包含在提交记录中。", ""])
    (OUTPUT_DIR / "README.md").write_text("\n".join(readme), encoding="utf-8")
    print(f"exported={len(rows)}")
    for title, count, images in rows:
        print(f"{title}\tmessages={count}\timages={images}")


if __name__ == "__main__":
    main()
