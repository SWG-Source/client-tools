"""Bounded filesystem/entry tests; no native compiler or renderer is run."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import unittest
from unittest import mock

SPEC = importlib.util.spec_from_file_location('deps_builder', Path(__file__).resolve().parents[1] / 'build.py')
builder = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(builder)


class OwnerTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.output = self.root / 'output'
        self.output.mkdir()
        self.owner = dict(checkout=str(builder.ROOT.resolve()), platform='x64', configuration='Release')

    def snapshot(self):
        return {str(p.relative_to(self.output)): p.read_bytes() if p.is_file() else None
                for p in self.output.rglob('*')}

    def establish(self):
        builder.ensure_owner(self.output, self.owner)

    def test_empty_and_lock_only_adopt(self):
        for locked in (False, True):
            with self.subTest(locked=locked):
                if locked:
                    (self.output / '.build-lock').mkdir()
                self.establish()
                self.assertEqual(json.loads((self.output / 'owner.json').read_text()), self.owner)
                self.assertFalse((self.output / 'owner.pending').exists())
                (self.output / 'owner.json').unlink()

    def test_each_unowned_entry_blocks_without_changes(self):
        for name in ('jpeg.lib', 'stlport.lib', 'manifest.json', 'owner.pending', '.hidden', 'empty-directory'):
            with self.subTest(name=name):
                path = self.output / name
                if name == 'empty-directory':
                    path.mkdir()
                else:
                    path.write_bytes(b'preserve this fixture')
                before = self.snapshot()
                with self.assertRaisesRegex(RuntimeError, 'unowned'):
                    self.establish()
                self.assertEqual(self.snapshot(), before)
                path.rmdir() if path.is_dir() else path.unlink()

    def test_dangling_owner_is_not_absent(self):
        path = self.output / 'owner.json'
        try:
            path.symlink_to(self.output / 'missing')
        except OSError as error:
            self.skipTest('Symlinks unavailable: ' + str(error))
        with self.assertRaisesRegex(RuntimeError, 'unowned'):
            self.establish()
        self.assertTrue(path.is_symlink())
        self.assertFalse((self.output / 'owner.pending').exists())

    def test_invalid_owner_fails_unchanged(self):
        for content in ('', '{', '{"checkout":', 'null', '{}', '[]'):
            with self.subTest(content=content):
                (self.output / 'owner.json').write_text(content)
                before = self.snapshot()
                with self.assertRaises((ValueError, RuntimeError)):
                    self.establish()
                self.assertEqual(self.snapshot(), before)

    def test_each_owner_dimension_must_match(self):
        for key in self.owner:
            with self.subTest(key=key):
                (self.output / 'owner.json').write_text(json.dumps(dict(self.owner, **{key: 'different'})))
                before = self.snapshot()
                with self.assertRaisesRegex(RuntimeError, 'another checkout/platform/configuration'):
                    self.establish()
                self.assertEqual(self.snapshot(), before)

    def test_matching_owner_preserves_interrupted_output_and_owner_bytes(self):
        (self.output / 'owner.json').write_text(json.dumps(self.owner, separators=(',', ':')))
        for name in ('manifest.pending', 'owner.pending', 'jpeg.lib', 'stlport.lib'):
            (self.output / name).write_bytes(b'interrupted fixture')
        (self.output / 'deps-interrupted').mkdir()
        before = self.snapshot()
        with mock.patch.object(builder.os, 'replace', side_effect=AssertionError('must not rewrite owner')):
            self.establish()
        self.assertEqual(self.snapshot(), before)

    def test_atomic_publication_exposes_complete_owner(self):
        replace = os.replace

        def inspect_then_replace(source, destination):
            self.assertFalse(destination.exists())
            self.assertEqual(json.loads(source.read_text()), self.owner)
            replace(source, destination)

        with mock.patch.object(builder.os, 'replace', side_effect=inspect_then_replace) as publish:
            self.establish()
        publish.assert_called_once()
        self.assertEqual(json.loads((self.output / 'owner.json').read_text()), self.owner)

    def test_failed_flush_or_publication_blocks_retry(self):
        for operation in ('fsync', 'replace'):
            with self.subTest(operation=operation):
                with mock.patch.object(builder.os, operation, side_effect=OSError('injected filesystem failure')):
                    with self.assertRaisesRegex(OSError, 'injected'):
                        self.establish()
                self.assertFalse((self.output / 'owner.json').exists())
                before = self.snapshot()
                with self.assertRaisesRegex(RuntimeError, 'unowned'):
                    self.establish()
                self.assertEqual(self.snapshot(), before)
                (self.output / 'owner.pending').unlink()

    def test_partial_staged_write_fails_closed(self):
        real_open = Path.open

        class PartialWriter:
            def __enter__(inner):
                inner.stream = real_open(self.output / 'owner.pending', 'x')
                return inner

            def write(inner, content):
                inner.stream.write('{"checkout":')
                raise OSError('injected partial write')

            def __exit__(inner, *args):
                inner.stream.close()

        def open_with_partial_write(path, *args, **kwargs):
            if path == self.output / 'owner.pending':
                return PartialWriter()
            return real_open(path, *args, **kwargs)

        with mock.patch.object(Path, 'open', autospec=True, side_effect=open_with_partial_write):
            with self.assertRaisesRegex(OSError, 'partial write'):
                self.establish()
        self.assertFalse((self.output / 'owner.json').exists())
        self.assertEqual((self.output / 'owner.pending').read_text(), '{"checkout":')
        with self.assertRaisesRegex(RuntimeError, 'unowned'):
            self.establish()

    def invoke_main(self, callback):
        # Real parser, file checks, lock and owner handling; fixture hash only.
        # Replace the native-only boundary with a control-flow observer.
        archive = self.root / 'archive-fixture'
        archive.write_bytes(b'not a JPEG archive; never extracted')
        vcvars = self.root / 'vcvars-fixture'
        vcvars.write_bytes(b'never executed')
        argv = ['build.py', '--output', str(self.output), '--vcvars', str(vcvars),
                '--platform', 'x64', '--configuration', 'Release', '--jpeg-archive', str(archive)]
        os_proxy = mock.Mock(wraps=os)
        os_proxy.name = 'nt'
        with mock.patch.object(builder, 'os', os_proxy), \
                mock.patch.object(builder, 'JPEG_SHA256', hashlib.sha256(archive.read_bytes()).hexdigest()), \
                mock.patch.object(builder.sys, 'argv', argv), \
                mock.patch.object(builder, 'build', side_effect=callback) as build:
            try:
                builder.main()
            finally:
                self.assertFalse((self.output / '.build-lock').exists())
        return build

    def test_main_rejects_unowned_before_build_and_releases_lock(self):
        (self.output / 'jpeg.lib').write_bytes(b'preserve')
        before = self.snapshot()
        with self.assertRaisesRegex(RuntimeError, 'unowned'):
            self.invoke_main(lambda *args: self.fail('build reached for unowned output'))
        self.assertEqual(self.snapshot(), before)

    def test_main_publishes_owner_before_build_and_recovers_after_failure(self):
        def interrupted(args, archive, output):
            self.assertTrue((output / '.build-lock').is_dir())
            self.assertEqual(json.loads((output / 'owner.json').read_text()), self.owner)
            (output / 'manifest.pending').write_bytes(b'interrupted fixture')
            raise RuntimeError('injected build boundary failure')

        with self.assertRaisesRegex(RuntimeError, 'build boundary failure'):
            self.invoke_main(interrupted)
        before = self.snapshot()
        build = self.invoke_main(lambda *args: None)
        build.assert_called_once()
        self.assertEqual(self.snapshot(), before)

    def test_main_owner_publication_failure_never_reaches_build(self):
        with mock.patch.object(os, 'replace', side_effect=OSError('injected publication failure')):
            with self.assertRaisesRegex(OSError, 'publication failure'):
                self.invoke_main(lambda *args: self.fail('build reached before publication'))
        self.assertFalse((self.output / 'owner.json').exists())
        self.assertTrue((self.output / 'owner.pending').is_file())


if __name__ == '__main__':
    unittest.main()
