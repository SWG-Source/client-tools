"""Synthetic PE controls: metadata normalization must never hide instructions."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('compare_dll', Path(__file__).with_name('compare_dll.py'))
compare_dll = importlib.util.module_from_spec(spec)
spec.loader.exec_module(compare_dll)


def fixture():
    b = bytearray(1024)
    b[:2] = b'MZ'
    struct.pack_into('<I', b, 60, 128)
    b[128:132] = b'PE\0\0'
    struct.pack_into('<HHIIIHH', b, 132, 0x14c, 2, 123, 0, 0, 224, 0x2102)
    struct.pack_into('<H', b, 152, 0x10b)
    struct.pack_into('<I', b, 152+92, 16)
    struct.pack_into('<II', b, 152+96+48, 0x2000, 28)
    for offset, name, va, rp, flags in ((376, b'.text', 0x1000, 512, 0x60000020),
                                      (416, b'.rdata', 0x2000, 768, 0x40000040)):
        b[offset:offset+8] = name.ljust(8, b'\0')
        struct.pack_into('<IIII', b, offset+8, 256, va, 256, rp)
        struct.pack_into('<I', b, offset+36, flags)
    b[512:768] = b'\x90'*256
    struct.pack_into('<IIII', b, 768+12, 2, 64, 0x2040, 832)
    b[832:836] = b'RSDS'
    path = b'C:\\Win32\\dpvs.pdb\0'
    b[856:856+len(path)] = path
    date = b'Sep 29 2026 12:34:56\0'
    b[928:928+len(date)] = date
    return b


class Normalization(unittest.TestCase):
    def test_valid_metadata_only(self):
        a, b = fixture(), fixture()
        b[136] ^= 1  # COFF timestamp
        b[772] ^= 1  # debug timestamp
        b[836] ^= 1  # CodeView GUID
        self.assertEqual(compare_dll.normalize(a)[0], compare_dll.normalize(b)[0])

    def test_instruction_change_is_not_normalized(self):
        a, b = fixture(), fixture()
        b[512] ^= 1
        self.assertNotEqual(compare_dll.normalize(a)[0], compare_dll.normalize(b)[0])

    def test_debug_directory_cannot_cover_instructions(self):
        for timestamp in (1, 2):
            b = fixture()
            struct.pack_into('<II', b, 152+96+48, 0x1000, 28)
            b[512:540] = b'\0'*28
            struct.pack_into('<I', b, 516, timestamp)
            with self.assertRaises(ValueError):
                compare_dll.normalize(b)

    def test_codeview_cannot_cover_instructions(self):
        b = fixture()
        struct.pack_into('<II', b, 768+20, 0x1000, 512)
        b[512:516] = b'RSDS'
        with self.assertRaises(ValueError):
            compare_dll.normalize(b)

    def test_virtual_tail_is_not_file_data(self):
        b = fixture()
        struct.pack_into('<I', b, 416+16, 28)
        with self.assertRaises(ValueError):
            compare_dll.normalize(b)

    def test_metadata_cannot_be_executable_or_writable(self):
        for flags in (0x60000040, 0xc0000040):
            b = fixture()
            struct.pack_into('<I', b, 416+36, flags)
            with self.assertRaises(ValueError):
                compare_dll.normalize(b)

    def test_date_cannot_cover_instructions(self):
        b = fixture()
        b[512:532], b[928:948] = b[928:948], b'\0'*20
        with self.assertRaises(ValueError):
            compare_dll.normalize(b)

    def test_truncated_file_rejected(self):
        with self.assertRaises(ValueError):
            compare_dll.normalize(fixture()[:-1])


if __name__ == '__main__':
    unittest.main()
