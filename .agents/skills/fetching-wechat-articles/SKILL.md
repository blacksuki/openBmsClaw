---
name: fetching-wechat-articles
description: Use when needing to retrieve and parse the main text content of WeChat Official Account (mp.weixin.qq.com) articles, especially when encountering environment verification pages or script leakage issues during tag depth parsing.
---

# Fetching WeChat Articles

## Overview
WeChat applies aggressive crawler checks and structures its pages with inline script blocks appended at the bottom. This skill outlines how to reduce the chance of tripping WeChat's verification walls using header emulation, and how to parse the article body without leaking into surrounding JavaScript code.

## When to Use
- When trying to extract the text content of a WeChat Official Account article URL (`https://mp.weixin.qq.com/s/...`).
- When standard HTTP library calls fail or return an empty page titled "环境异常" or "去验证".
- When custom tag-depth parsing leaks or extracts Javascript code instead of clean article text.

### When NOT to Use
- For general web scraping of other domains.
- When an official RSS feed or WeChat API is available.

## Core Pattern
To fetch WeChat articles reliably:
1. **Emulate Headers**: Include `User-Agent` and `Accept-Language` simulating a real desktop browser to reduce the chance of tripping Tencent's captcha. This lowers the odds but does **not** guarantee a bypass.
2. **Use a request timeout**: Never call `urlopen` without a `timeout` — a verification page or stalled connection can otherwise hang indefinitely.
3. **Suppress `<script>`/`<style>` content**: Never emit text collected inside script/style tags. This is the real guard against JavaScript leaking into the output.
4. **Handle HTML void tags**: When tracking `#js_content` nesting by depth, skip void tags (`<img>`, `<br>`, …) so the depth counter stays aligned.
5. **Detect the verification wall**: If no content is extracted, scan the HTML for markers like `环境异常` / `去验证` and report it explicitly rather than returning an empty result.

## Quick Reference
| Target Element | HTML Attribute / Selector | Description |
|---|---|---|
| **Article Container** | `<div id="js_content">` | Holds the main body text |
| **Article Title** | `<meta property="og:title">` | Open Graph title metadata |
| **Cover Image** | `<meta property="og:image">` | Open Graph image metadata |

## Implementation
The parsing helper script is available as a reusable Python command:
- [wechat_parser.py](wechat_parser.py)

### Example Usage
Run from this skill's directory (or pass the full path to the script):
```bash
python3 wechat_parser.py "https://mp.weixin.qq.com/s/bf44aiW9oO138TqOw5NTTQ"
```

## Common Mistakes
### 1. Tag-depth parsing leaks into inline scripts
- **Symptom**: The extracted text contains thousands of lines of JavaScript library code.
- **Cause**: Depth-based extraction assumes every tag closes. Two things break that: (a) void elements (`<img>`, `<br>`) have no end tag; (b) WeChat also emits **optional-close** tags (`<p>`, `<section>`, `<li>`) that `HTMLParser` does not auto-close. Either can leave the depth counter above `0` past the real end of `#js_content`, so trailing page scripts leak in.
- **Fix (primary)**: Suppress all text collected inside `<script>` / `<style>` (a `skip_depth` flag). This makes JS leakage structurally impossible regardless of depth accuracy.
- **Fix (secondary)**: Skip depth changes for void elements so the counter stays aligned for well-formed content.

### 2. No timeout / silent verification failure
- **Symptom**: The command hangs, or exits non-zero with no explanation.
- **Cause**: `urlopen` without `timeout` can block forever; and when the verification wall is served, `#js_content` is absent so the result is silently empty.
- **Fix**: Pass `timeout=` to `urlopen`, and when extraction yields nothing, scan for verification markers (`环境异常`, `去验证`) and report explicitly.
