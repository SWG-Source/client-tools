"""Validate the precise legacy x64 SDK archive tested with the original wrappers."""
import hashlib
from pathlib import Path
import sys

EXPECTED = 'a48539793ceb80d68df27d5e913fc2d977d142a4e8e5cfb36d80725c034d17c4'

def main():
    if len(sys.argv) != 2:
        raise ValueError('expected the path to the official LCDSDK Lib/x64/lglcd.lib')
    source = Path(sys.argv[1])
    actual = hashlib.sha256(source.read_bytes()).hexdigest()
    if actual != EXPECTED:
        raise ValueError('Logitech LCD SDK archive differs from the verified x64 provider; refusing an unverified or wrong-architecture library')
    print('Verified official legacy x64 lglcd.lib: ' + actual)

if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError) as error:
        sys.exit(str(error))
