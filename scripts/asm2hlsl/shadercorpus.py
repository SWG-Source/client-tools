# Extracts every vertex_program/*.vsh, pixel_program/*.psh, shared_program/* and effect/*
# out of the deployed client's archive stack, resolving each name exactly the way the
# running engine resolves it, and classifies each program by the language marker on its
# first line.
#
# Precedence matters: the same name appears in many archives (543 .vsh records for 286
# unique names in the .tre directories alone), and getting the order backwards means
# classifying a superseded copy of a program.
#
# THE THING THIS FILE GOT WRONG BEFORE, AND WHY IT MATTERED
# --------------------------------------------------------
# The old search_order() read only "searchTree_<sku>_<priority>" keys. The live cfg chain
# also carries FOUR "searchTOC_<sku>_<priority>" keys, at priorities 0..3, which between
# them name 198 .tre archives. 137 of the 209 .tre files in a deployed client are version
# TREE0006 with a COMPLETELY ZEROED header (numberOfFiles == 0, tocOffset == 0): they carry
# no internal directory at all, so tre.py reports 0 files for every one of them and a
# SearchTree over one would find nothing. Their contents are addressable ONLY through a
# .toc, which supplies (treeFileIndex, offset, length, compressor, compressedLength)
# externally and reads the payload straight out of the .tre. Ignoring the TOCs therefore
# hid every file that only lives in a 0006 archive, and resolved everything else from
# lower-priority copies.
#
# Concretely: vertex_program/include/functions.inc resolves through sku0_client.toc to
# patch_24_client_01.tre and is 17,375 bytes / 473 lines with a HemisphericLightData
# declaration. The best any searchTree offers is patch_12_00.tre at 12,318 bytes. The
# 17,375-byte copy is what the running client compiles.
#
# SOURCES MODELLED (read, not guessed)
#   src/engine/shared/library/sharedFile/src/shared/TreeFile.cpp
#       install(), addSearchNode(), searchNodePriorityOrder(), find(), getFileSize(),
#       fixUpFileName()
#   src/engine/shared/library/sharedFile/src/shared/TreeFile_SearchNode.cpp
#       SearchPath, SearchAbsolute, SearchTree, SearchTOC, SearchCache
#   src/engine/shared/library/sharedFoundation/src/shared/ConfigFile.cpp
#       loadFile/loadFromBuffer/processLine/processKeys, Key::addValue
#   src/engine/shared/library/sharedFoundation/src/win32/SetupSharedFoundation.cpp
#       config file is loaded, THEN the command line
#   src/game/client/application/SwgClient/src/win32/ClientMain.cpp
#       skuBits derivation from [Station] gameFeatures

import collections
import io
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = r"E:\SWG\64bit-server\_client"
sys.path.insert(0, HERE)
import tre as tremod
import toc as tocmod

OUT = os.path.join(HERE, "corpus")
MANIFEST = os.path.join(HERE, "corpus-manifest.tsv")

# Prefixes the shader corpus needs. Everything else in the 211,496 archive entries is
# irrelevant here.
WANT = ("vertex_program/", "pixel_program/", "shared_program/", "effect/")

# The engine's config chain starts at client.cfg; the rest arrive through `.include`.
ENTRY_CFG = "client.cfg"
INCLUDE_DELIM = ".include "

# ClientMain.cpp -> PlatformFeatureBits.h
FEATURE_BASE = 1 << 0
FEATURE_SPACE_BETA = 1 << 2
FEATURE_SPACE_PREORDER = 1 << 3
FEATURE_SPACE_RETAIL = 1 << 4
FEATURE_EP3_RETAIL = 1 << 9
FEATURE_EP3_PREORDER = 1 << 11
FEATURE_TOOW_PREORDER = 1 << 14
FEATURE_TOOW_RETAIL = 1 << 15


# ======================================================================
# TreeFile::fixUpFileName -- every lookup goes through this first.

def fixup(fileName):
    """TreeFile::fixUpFileName: strip leading '/', './' and '../', convert backslashes to
    forward slashes, lowercase, collapse repeated slashes."""
    f = fileName.replace("\\", "/")
    while f.startswith("/"):
        f = f[1:]
    while f.startswith("./"):
        f = f[2:]
    while f.startswith("../"):
        f = f[3:]
    f = f.lower()
    out = []
    prev_slash = False
    for c in f:
        cur_slash = (c == "/")
        if not cur_slash or not prev_slash:
            out.append(c)
        prev_slash = cur_slash
    return "".join(out)


# ======================================================================
# ConfigFile

class ConfigFile(object):
    """A model of sharedFoundation/ConfigFile.cpp, only as far as this tool needs.

    The parts that matter and are easy to get wrong:
      * `.include "name"` recurses immediately, so an included file's values land AFTER
        the values above the include line and BEFORE the values below it.
      * loadFile() resets the current section to NULL, so an included file must open its
        own [Section] header; the includer's section is restored afterwards.
      * Key::addValue APPENDS. A repeated key therefore has several values, and
        getKeyString(section, key, index) walks them in listed order -- which is how
        `searchTree_00_0` names 65 archives. The 3-argument getKeyInt returns the LAST
        value, which is why maxSearchPriority=12 appearing in both client.cfg and live.cfg
        is still 12.
      * A single line can carry several values separated by '&'; '#' and ';' start
        comments.
    """

    def __init__(self, root):
        self.root = root
        self.sections = {}      # section -> key -> [values]
        self.current = None
        self.provenance = []    # (file, line, section, key, value)
        self.missing = []

    def load(self, name):
        self.current = None     # ConfigFile::loadFile resets ms_currentSection
        path = name if os.path.isabs(name) else os.path.join(self.root, name)
        if not os.path.exists(path):
            self.missing.append(name)
            return False
        with open(path, "rb") as f:
            data = f.read()
        for lineno, raw in enumerate(data.split(b"\n"), 1):
            self._line(raw.decode("latin-1").lstrip(), name, lineno)
        return True

    def _line(self, line, path, lineno):
        if not line or line[0] in "#;":
            return
        if line.startswith(INCLUDE_DELIM):
            rest = line[len(INCLUDE_DELIM):]
            if rest.startswith('"'):
                end = rest.find('"', 1)
                saved = self.current
                self.load(rest[1:end])
                self.current = saved
            return                      # an unquoted .include is silently ignored
        if line[0] == "[":
            end = line.find("]")
            self.current = line[1:end]
            self.sections.setdefault(self.current, {})
            return
        self._keys(line, path, lineno)

    def _keys(self, line, path, lineno):
        line = line.lstrip()
        eq = line.find("=")
        if eq < 0:
            return
        key = line[:eq].rstrip()
        rest = line[eq + 1:]
        i = 0
        while i < len(rest):
            while i < len(rest) and rest[i].isspace():
                i += 1
            if i >= len(rest):
                break
            if rest[i] != '"':
                j = i
                while j < len(rest) and not rest[j].isspace() and rest[j] not in '#&;':
                    j += 1
                value = rest[i:j]
                i = j
            else:
                j = i + 1
                while j < len(rest) and rest[j] != '"':
                    j += 1
                value = rest[i + 1:j]
                i = j + 1 if j < len(rest) else j
            self.sections.setdefault(self.current, {}).setdefault(key, []).append(value)
            self.provenance.append((path, lineno, self.current, key, value))
            while i < len(rest) and rest[i].isspace():
                i += 1
            if i >= len(rest) or rest[i] in '&;#':
                if i < len(rest) and rest[i] == '&':
                    self._keys(rest[i + 1:], path, lineno)
                break

    def get_string(self, section, key, index, default=None):
        vals = self.sections.get(section, {}).get(key)
        if vals is None or index >= len(vals):
            return default
        return vals[index]

    def get_int(self, section, key, default):
        """The 3-argument ConfigFile::getKeyInt -- returns the LAST value."""
        vals = self.sections.get(section, {}).get(key)
        if not vals:
            return default
        v = vals[-1]
        try:
            return int(v, 16) if v.lower().startswith("0x") else int(v)
        except ValueError:
            return default


def sku_bits(cfg):
    """ClientMain.cpp: [Station] gameFeatures, minus [ClientGame] gameBitsToClear, mapped
    onto the four sku bits. Game::setGameFeatureBits stores the value unmasked."""
    features = cfg.get_int("Station", "gameFeatures", 0)
    features &= ~cfg.get_int("ClientGame", "gameBitsToClear", 0)
    if cfg.get_int("ClientGame", "setJtlRetailIfBetaIsSet", 0):
        if features & (FEATURE_SPACE_BETA | FEATURE_SPACE_PREORDER):
            features |= FEATURE_SPACE_RETAIL
    if features & FEATURE_EP3_PREORDER:
        features |= FEATURE_EP3_RETAIL
    if features & FEATURE_TOOW_PREORDER:
        features |= FEATURE_TOOW_RETAIL
    bits = 0
    if features & FEATURE_BASE:
        bits |= 1 << 0
    if features & FEATURE_SPACE_RETAIL:
        bits |= 1 << 1
    if features & FEATURE_EP3_RETAIL:
        bits |= 1 << 2
    if features & FEATURE_TOOW_RETAIL:
        bits |= 1 << 3
    return features, bits


# ======================================================================
# Search nodes

class SearchNode(object):
    """One entry in TreeFile's ms_searchNodes list."""

    kind = "?"

    def __init__(self, name, priority, sku, insertion_index, cfg_key, cfg_index):
        self.name = name
        self.priority = priority
        self.sku = sku
        self.insertion_index = insertion_index
        self.cfg_key = cfg_key
        self.cfg_index = cfg_index
        self.rank = None

    def __repr__(self):
        return "<%s rank=%s pri=%d ins=%d %s>" % (
            self.kind, self.rank, self.priority, self.insertion_index, self.name)

    def candidates(self, accept):
        """Yield (fixedName, handle) for every name in this node that `accept` likes.

        A name is only yielded when fixup(storedName) == storedName. The engine looks a
        file up by CRC of the ALREADY fixed-up (lowercased) name, so a stored name that is
        not already lowercase/forward-slashed can never be found by any caller and must
        not be treated as a candidate.
        """
        return iter(())

    def deleted(self, handle):
        """True when this node's entry is a tombstone that ABORTS the whole search.

        Only SearchTree does this: localExists() sets `deleted = true` for a matched entry
        with length == 0, and TreeFile::find/getFileSize loop on `!deleted`, so the file is
        reported ABSENT even though lower-priority nodes hold a copy.
        """
        return False

    def size(self, handle):
        raise NotImplementedError

    def read(self, handle):
        raise NotImplementedError

    def container(self, handle):
        """Where the bytes physically live, for reporting."""
        return self.name


class SearchPath(SearchNode):
    """SearchPath: a directory prefix. exists() is FileStreamer::exists(path + '/' + name)."""

    kind = "path"

    def __init__(self, *a, **kw):
        SearchNode.__init__(self, *a, **kw)
        self.base = self.name if os.path.isabs(self.name) else os.path.join(ROOT, self.name)

    def candidates(self, accept):
        for dirpath, _dirs, files in os.walk(self.base):
            for f in files:
                full = os.path.join(dirpath, f)
                rel = os.path.relpath(full, self.base).replace(os.sep, "/")
                if rel != fixup(rel):
                    continue
                if accept(rel):
                    yield rel, full

    def size(self, handle):
        return os.path.getsize(handle)

    def read(self, handle):
        with open(handle, "rb") as f:
            return f.read()

    def container(self, handle):
        return handle


class SearchAbsolute(SearchPath):
    """SearchAbsolute: opens the path literally, relative to the PROCESS WORKING
    DIRECTORY. That is how a loose file on disk overrides every archive. install() adds
    one unconditionally at (highest configured priority + 1), so it sits ahead of
    everything except the cache.

    Modelled here against ROOT, i.e. on the assumption the client is launched with its
    deployment directory as the working directory. There is no way to tell from the files
    on disk whether a shortcut sets a different 'Start in'.
    """

    kind = "absolute"

    def __init__(self, *a, **kw):
        SearchNode.__init__(self, *a, **kw)
        self.base = ROOT


class SearchCache(SearchNode):
    """SearchCache: install() adds it last, at (new front priority + 1), so it is the very
    first node consulted. It is EMPTY at install time; CachedFileManager fills it later
    from the file named by [SharedUtility] cache. It can never supply a copy this order
    does not already pick, because SearchCache::addCachedFile obtains its bytes through
    TreeFile::open -- i.e. from the rest of this same node list. So it contributes no
    candidates here, and is listed only so the ranks match the engine's.
    """

    kind = "cache"


class SearchTree(SearchNode):
    """SearchTree: one .tre archive read through its own internal directory.

    Only 72 of the 209 .tre files in a deployed client have one; the 137 TREE0006 archives
    have an all-zero header and contribute nothing through this node type.
    """

    kind = "tree"

    def __init__(self, *a, **kw):
        SearchNode.__init__(self, *a, **kw)
        self.path = os.path.join(ROOT, self.name)
        self.version = None
        self.entries = None         # fixedName -> tre entry tuple
        self.duplicate_names = 0
        self.unreachable_names = 0

    def _load(self):
        if self.entries is not None:
            return
        self.entries = {}
        opened = tremod.read_tre(self.path)
        if not opened:
            return
        f, self.version, entries = opened
        f.close()
        for e in entries:
            nm = e[0].replace("\\", "/")
            if nm != fixup(nm):
                self.unreachable_names += 1
                continue
            if nm in self.entries:
                self.duplicate_names += 1
                continue
            self.entries[nm] = e

    def candidates(self, accept):
        self._load()
        for nm, e in self.entries.items():
            if accept(nm):
                yield nm, e

    def deleted(self, handle):
        return handle[1] == 0       # length == 0

    def size(self, handle):
        return handle[1]

    def read(self, handle):
        f = open(self.path, "rb")
        try:
            return tremod.extract(f, handle, None)
        finally:
            f.close()


class SearchTOC(SearchNode):
    """SearchTOC: a .toc index over a list of .tre archives.

    A .toc carries NO payload of its own. Each entry names one of the .tre files listed in
    the TOC's tree-file-name block, plus the byte range inside it, so SearchTOC::open reads
    the .tre directly and never consults that .tre's internal directory. That is what makes
    the 137 directory-less TREE0006 archives readable.

    The tree list is NOT a precedence list to be re-resolved: every file name in a TOC maps
    to exactly one entry naming exactly one archive, so there is never a contest inside a
    single TOC. Re-deriving precedence from the tree order would also be wrong -- the TOC
    deliberately pins texture/defaultshader.dds and texture/defaulttexture.dds to
    default_patch.tre over the higher-indexed data_texture_01.tre.

    localExists() rejects an entry whose length == 0 or whose offset == 0, and
    SearchTOC::exists always reports deleted = false, so such an entry is a plain miss:
    lower-priority nodes still get a look. That is the opposite of SearchTree's tombstone.
    """

    kind = "toc"

    def __init__(self, *a, **kw):
        SearchNode.__init__(self, *a, **kw)
        self.path = os.path.join(ROOT, self.name)
        self.header = None
        self.tre_files = None       # index == treeFileIndex
        self.entries = None         # fixedName -> TocEntry
        self.unusable = 0
        self.duplicate_names = 0
        self.unreachable_names = 0

    def _load(self):
        if self.entries is not None:
            return
        self.header, self.tre_files, entries = tocmod.read_toc(self.path)
        self.entries = {}
        for e in entries:
            nm = e.name.replace("\\", "/")
            if nm != fixup(nm):
                self.unreachable_names += 1
                continue
            if not e.usable:
                self.unusable += 1
                continue
            if nm in self.entries:
                self.duplicate_names += 1
                continue
            self.entries[nm] = e

    def tre_list(self):
        if self.tre_files is None:
            self.tre_files = tocmod.tre_list(self.path)
        return self.tre_files

    def candidates(self, accept):
        self._load()
        for nm, e in self.entries.items():
            if accept(nm):
                yield nm, e

    def size(self, handle):
        return handle.length

    def read(self, handle):
        return tocmod.open_entry(ROOT, self.tre_files, handle)

    def container(self, handle):
        return "%s!%s" % (self.name, self.tre_files[handle.treeFileIndex])


# ======================================================================
# TreeFile::install

def search_order(root=None, cfg=None):
    """The engine's full resolution order, most-preferred node first.

    HOW install() BUILDS THE LIST (TreeFile.cpp:84-191)
      * maxPriority = ConfigFile [SharedFile] maxSearchPriority, default 20 in the code,
        set to 12 by these cfg files.
      * It loops sku: 0 first, UNCONDITIONALLY (the loop's `first` flag short-circuits the
        bit test on the first pass), then each set bit of skuBits. The config key text is
        sprintf("searchTree%s%d", skuText, priority) where skuText is "" when skuBits == 0
        and "_%02d_" otherwise -- which is why the real keys read "searchTree_00_0" and
        "searchTOC_03_3": FIRST number is the SKU, SECOND is the PRIORITY.
      * Within one sku it loops priority 0 UP TO maxPriority INCLUSIVE, and within one
        (sku, priority) it inserts searchPath entries, then searchTree entries, then
        searchTOC entries, each in the order the cfg lists them.
      * Then it adds a SearchAbsolute at (current front priority + 1), and then a
        SearchCache at (the NEW front priority + 1). So the head of the list is
        cache, absolute, and only then the highest-priority archive.

    HOW THE ORDER FALLS OUT (TreeFile.cpp:284-308)
      searchNodePriorityOrder is `a->getPriority() > b->getPriority()`, so ms_searchNodes
      is kept sorted by priority DESCENDING. addSearchNode inserts with std::lower_bound
      against that comparator. lower_bound returns the first position whose priority is
      NOT greater than the new node's -- i.e. the first element of equal priority -- and
      insert() puts the new node BEFORE it. Therefore:

          AT EQUAL PRIORITY, THE LAST NODE ADDED IS SEARCHED FIRST.

      (The doc comment above addSearchNode claims "inserted after the last priority
      match", which is the opposite of what the code does. The code is what runs, and
      last-added-wins is also the only rule under which a patch tree can patch anything.)

      Two consequences worth spelling out, because both are easy to re-break:
        1. Within one priority, the resolution order is the REVERSE of the cfg listing
           order.
        2. Within one sku, a searchTOC beats a searchTree of equal priority, because
           install() inserts TOCs after trees. This is purely an insertion-order artifact,
           NOT a type precedence -- if a higher-numbered sku declared a searchTree at the
           same priority as a lower-numbered sku's searchTOC, that TREE would be inserted
           later and would win. In this client every tree is sku 00 and every TOC is added
           after them, so TOC beats tree at each shared priority.

      Note also that the searchTree entries are NOT all at priority 0: seven of them sit
      at priorities 2..8 and five of those outrank all four TOCs.

    Returns the ordered list of SearchNode objects, with .rank filled in.
    """
    global ROOT
    if root is not None:
        ROOT = root
    if cfg is None:
        cfg = ConfigFile(ROOT)
        cfg.load(ENTRY_CFG)

    _features, bits = sku_bits(cfg)
    max_priority = cfg.get_int("SharedFile", "maxSearchPriority", 20)

    nodes = []
    skipped = []
    counter = [0]

    def add(node):
        # std::lower_bound over a priority-DESCENDING list, then insert before.
        lo, hi = 0, len(nodes)
        while lo < hi:
            mid = (lo + hi) // 2
            if nodes[mid].priority > node.priority:
                lo = mid + 1
            else:
                hi = mid
        nodes.insert(lo, node)

    def make(cls, name, priority, sku, key, index):
        n = cls(name, priority, sku, counter[0], key, index)
        counter[0] += 1
        return n

    KINDS = (("searchPath", SearchPath), ("searchTree", SearchTree), ("searchTOC", SearchTOC))

    first = True
    sku = 0
    remaining = bits
    while first or remaining:
        if first or (remaining & (1 << sku)):
            sku_text = "_%02d_" % sku if remaining else ""
            for priority in range(0, max_priority + 1):
                for prefix, cls in KINDS:
                    key = "%s%s%d" % (prefix, sku_text, priority)
                    index = 0
                    while True:
                        value = cfg.get_string("SharedFile", key, index, None)
                        if value is None:
                            break
                        if cls is SearchPath or os.path.exists(os.path.join(ROOT, value)):
                            add(make(cls, value, priority, sku, key, index))
                        else:
                            # addSearchTree/addSearchTOC only construct the node when
                            # FileStreamer::exists() is true; otherwise they WARNING and
                            # add nothing.
                            skipped.append((prefix, value, sku, priority))
                        index += 1
        first = False
        remaining &= ~(1 << sku)
        sku += 1

    front = nodes[0].priority if nodes else -1
    abs_priority = cfg.get_int("SharedFile", "searchAbsolute", front + 1)
    add(make(SearchAbsolute, "<process working directory>", abs_priority, None,
             "searchAbsolute", 0))
    front = nodes[0].priority
    cache_priority = cfg.get_int("SharedFile", "searchCache", front + 1)
    add(make(SearchCache, "<SearchCache, empty at install>", cache_priority, None,
             "searchCache", 0))

    for rank, node in enumerate(nodes):
        node.rank = rank

    search_order.cfg = cfg
    search_order.sku_bits = bits
    search_order.max_priority = max_priority
    search_order.skipped = skipped
    return nodes


# ======================================================================
# Resolution

Winner = collections.namedtuple("Winner", "name node handle size container")


def resolve(nodes, accept):
    """Resolve every name `accept` likes, the way TreeFile::find does.

    find() walks ms_searchNodes front to back and returns the first node whose exists()
    says yes, so per name the winner is simply the highest-ranked node holding it. The one
    wrinkle is the SearchTree tombstone: a matched entry with length == 0 sets `deleted`,
    and find()'s loop condition is `!deleted`, so the search STOPS and the file is reported
    absent even though lower-priority nodes have a copy. Walking the nodes in rank order
    and deciding each name at its first hit reproduces both behaviours.

    Returns (winners, tombstoned) where winners is {name: Winner} and tombstoned is
    {name: node} for names a SearchTree tombstone killed outright.
    """
    winners = {}
    tombstoned = {}
    for node in nodes:
        for name, handle in node.candidates(accept):
            if name in winners or name in tombstoned:
                continue            # a higher-ranked node already decided this name
            if node.deleted(handle):
                tombstoned[name] = node
                continue
            winners[name] = Winner(name, node, handle, node.size(handle),
                                   node.container(handle))
    return winners, tombstoned


def read(winner):
    return winner.node.read(winner.handle)


# ======================================================================

def wanted(name):
    return name.startswith(WANT)


def main():
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

    nodes = search_order()
    cfg = search_order.cfg
    print("config chain rooted at %s (missing: %s)"
          % (ENTRY_CFG, search_order.cfg.missing or "none"))
    print("skuBits = %d (%s)   maxSearchPriority = %d"
          % (search_order.sku_bits, bin(search_order.sku_bits), search_order.max_priority))
    by_kind = collections.Counter(n.kind for n in nodes)
    print("search order: %d nodes  %s" % (len(nodes), dict(by_kind)))
    if search_order.skipped:
        print("skipped (named in cfg, absent on disk): %s" % (search_order.skipped,))
    print()
    print("first 16 nodes, most preferred first:")
    print("   %-4s %-9s %-4s %-4s %s" % ("rank", "kind", "pri", "ins", "name"))
    for n in nodes[:16]:
        print("   %-4d %-9s %-4d %-4d %s" % (n.rank, n.kind, n.priority, n.insertion_index, n.name))

    winners, tombstoned = resolve(nodes, wanted)

    print()
    print("resolved %d names (%d killed by a SearchTree tombstone)"
          % (len(winners), len(tombstoned)))
    kinds = collections.Counter(os.path.splitext(n)[1] for n in winners)
    print("by extension:", dict(sorted(kinds.items())))
    src = collections.Counter("%s %s" % (w.node.kind, w.node.name) for w in winners.values())
    print("winning copies by node:")
    for key, count in src.most_common():
        print("   %-40s %d" % (key, count))

    os.makedirs(OUT, exist_ok=True)
    rows = []
    for name in sorted(winners):
        w = winners[name]
        data = read(w)
        if len(data) != w.size:
            raise SystemExit("%s: read %d bytes, node says %d" % (name, len(data), w.size))
        target = os.path.join(OUT, name.replace("/", os.sep))
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, "wb") as f:
            f.write(data)
        rows.append("%s\t%d\t%d\t%s\t%s" % (name, w.size, w.node.rank, w.node.kind, w.container))
    with open(MANIFEST, "w", encoding="utf-8", newline="\n") as f:
        f.write("# name\tsize\tnode_rank\tnode_kind\tcontainer\n")
        f.write("\n".join(rows) + "\n")

    # Classify programs by their first-line marker.
    marker = collections.Counter()
    targets = collections.Counter()
    unmarked = []
    for name in sorted(winners):
        if not name.endswith((".vsh", ".psh")):
            continue
        data = read(winners[name])
        head = data[:400]
        if head[:4] == b"FORM":
            i = data.find(b"PSRC")
            if i >= 0:
                head = data[i + 8:i + 408]
        first = head.decode("latin1", "replace").split("\n", 1)[0].strip()
        tok = first.split()
        kind = os.path.splitext(name)[1]
        lang = tok[0] if tok else ""
        if lang == "//hlsl":
            marker[(kind, "hlsl")] += 1
            targets[(kind, "hlsl", tok[1] if len(tok) > 1 else "?")] += 1
        elif lang == "//asm":
            marker[(kind, "asm")] += 1
            targets[(kind, "asm", tok[1] if len(tok) > 1 else "?")] += 1
        elif lang.startswith(("vs.", "ps.")):
            marker[(kind, "asm")] += 1
            targets[(kind, "asm", lang)] += 1
        else:
            marker[(kind, "unmarked")] += 1
            unmarked.append((name, first[:70]))

    print()
    print("language marker by program type:")
    for key in sorted(marker):
        print("   %-6s %-9s %d" % (key[0], key[1], marker[key]))
    print()
    print("declared profiles:")
    for key in sorted(targets):
        print("   %-6s %-5s %-8s %d" % (key[0], key[1], key[2], targets[key]))
    if unmarked:
        print()
        print("unmarked (%d), first 10:" % len(unmarked))
        for name, first in unmarked[:10]:
            print("   %-46s %s" % (name, first))

    print()
    print("extracted to", OUT)
    print("manifest    ", MANIFEST)


if __name__ == "__main__":
    main()
