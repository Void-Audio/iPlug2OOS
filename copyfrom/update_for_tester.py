import sys
import os
import re

HEADER_PATH = os.path.join("source", "licenser.h")
DEFAULT_MESSAGE = "Please enter your license key."

def update_message(new_message):
    if not os.path.isfile(HEADER_PATH):
        print(f"[ERROR] File not found: {HEADER_PATH}")
        return

    with open(HEADER_PATH, "r", encoding="utf-8") as f:
        lines = f.readlines()

    updated = False
    pattern = r'updateMessage\s*\(\s*".*?"\s*\)\s*;'

    with open(HEADER_PATH, "w", encoding="utf-8") as f:
        for line in lines:
            if re.search(pattern, line):
                new_line = f'            updateMessage("{new_message}");\n'
                f.write(new_line)
                updated = True
            else:
                f.write(line)

    if updated:
        print(f"[OK] Updated license message to: \"{new_message}\"")
    else:
        print(f"[WARN] No matching updateMessage() call found in {HEADER_PATH}")

def main():
    if len(sys.argv) != 2:
        print("Usage:")
        print("  python update_for_tester.py NAME       # Sets to 'Enter NAME tester key:'")
        print("  python update_for_tester.py --release  # Resets to default message")
        sys.exit(1)

    arg = sys.argv[1].strip()

    if arg == "--release":
        update_message(DEFAULT_MESSAGE)
    else:
        safe_name = arg.replace('"', '').replace('\\', '')
        tester_message = f"Enter {safe_name} tester key:"
        update_message(tester_message)

if __name__ == "__main__":
    main()
