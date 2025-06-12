REPLACE_DICT = {
    '"': '\\"',
    "\n": "\\n",
    "\t": "\\t",
    "\r": "\\r",
}


def escape_string(s: str) -> str:
    """Escape a string for use in a C code"""
    for k, v in REPLACE_DICT.items():
        s = s.replace(k, v)
    return s
