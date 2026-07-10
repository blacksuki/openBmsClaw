import sys
import urllib.request
from html.parser import HTMLParser

# Substrings that indicate WeChat served an anti-crawler / verification page
# instead of the real article.
VERIFICATION_MARKERS = ("环境异常", "去验证", "访问过于频繁", "请长按识别图中二维码")


class WeChatHTMLParser(HTMLParser):
    def __init__(self):
        super().__init__()
        self.in_content = False
        self.depth = 0
        # Depth of nested <script>/<style> while inside the content block.
        # Their text is never article body, so it is suppressed outright — this
        # guards against JS leaking into the output even if the div-depth counter
        # desyncs (e.g. WeChat emits an optional-close <p>/<section>).
        self.skip_depth = 0
        self.text_list = []
        # Void elements have no closing tag in HTML.
        self.void_elements = {
            'area', 'base', 'br', 'col', 'embed', 'hr', 'img', 'input',
            'link', 'meta', 'param', 'source', 'track', 'wbr'
        }

    def handle_starttag(self, tag, attrs):
        tag_lower = tag.lower()
        attrs_dict = dict(attrs)

        if attrs_dict.get('id') == 'js_content':
            self.in_content = True
            self.depth = 1
            return

        if not self.in_content:
            return

        if tag_lower in ('script', 'style'):
            self.skip_depth += 1

        # Only track depth for non-void tags (void tags have no end tag).
        if tag_lower not in self.void_elements:
            self.depth += 1

    def handle_endtag(self, tag):
        tag_lower = tag.lower()
        if not self.in_content:
            return

        if tag_lower not in self.void_elements:
            self.depth -= 1
            if self.depth <= 0:
                self.in_content = False
                self.skip_depth = 0
                return

        if tag_lower in ('script', 'style') and self.skip_depth > 0:
            self.skip_depth -= 1

    def handle_data(self, data):
        if self.in_content and self.skip_depth == 0:
            text = data.strip()
            if text:
                self.text_list.append(text)


def fetch_and_parse_wechat(url, timeout=15):
    headers = {
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36',
        'Accept-Language': 'zh-CN,zh;q=0.9,en;q=0.8',
        'Accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8'
    }

    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as response:
            html_content = response.read().decode('utf-8', errors='replace')
    except Exception as e:
        print(f"Error fetching article: {e}", file=sys.stderr)
        return None

    parser = WeChatHTMLParser()
    parser.feed(html_content)
    text = '\n'.join(parser.text_list)

    if not text:
        # Header emulation reduces, but does not guarantee, bypassing the wall.
        for marker in VERIFICATION_MARKERS:
            if marker in html_content:
                print(
                    f"WeChat verification wall detected (marker: {marker!r}). "
                    "Header emulation was not enough — retry later or fetch via a real browser session.",
                    file=sys.stderr,
                )
                return None
        print("No article content found (#js_content empty or missing).",
              file=sys.stderr)
        return None

    return text


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python3 wechat_parser.py <wechat_url>", file=sys.stderr)
        sys.exit(1)

    url = sys.argv[1]
    content = fetch_and_parse_wechat(url)
    if content:
        print(content)
    else:
        sys.exit(1)
