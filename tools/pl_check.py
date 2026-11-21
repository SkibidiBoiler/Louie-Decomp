#!/usr/bin/env python3
"""Cheap checks for the PL2 port: several jobs in one run, short output (local only, reads nothing from the net).

  rooms LO HI [--where e:23,30 o:15]   types placed in rooms LO..HI (room ids); '*' = no factory case yet;
                                       --where lists the placements (type,x,y,params) of those types
  run [--build] -s SPEC [-s SPEC ...]  optionally build Debug (prints only errors/warnings), then run each
                                       scenario with --trace --mute (PL_SEED=1) and summarise it

SPEC = "label|game args|types", e.g. "bites|--level 7 --room 2 --at 20,60 --customer 8 --input D3-5 --frames 80|23,24"
The summary gives: sounds (count), the player's first/last line and hp changes, for each enemy/object id of the
listed types its first and last position and its state sequence (new ids = spawned), bullets and effects seen.
Add --capture-frame N --capture f.png to the game args for a picture (the run then stops at that frame).
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter, OrderedDict

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DATA = os.environ.get('PL_DATA', 'C:/Users/alfre/Proyectos/datos-papas/build-cur')
EXE = os.path.join(ROOT, 'build', 'windows', 'papalouie.exe')
ITEM = re.compile(r' (\d+):(\w+) (-?\d+),(-?\d+) ?(\S*)')


def factory_types():
    got = {}
    for kind, f in (('e', 'EnemyManager.cpp'), ('o', 'ObjectManager.cpp')):
        s = open(os.path.join(ROOT, 'src/game/papaGame/managers', f), encoding='utf-8').read()
        got[kind] = set(re.findall(r'case (\d+): return', s))
    return got


def cmd_rooms(a):
    s = open(os.path.join(DATA, 'pl2/levels.xml'), encoding='utf-8').read()
    have = factory_types()
    want = {'e': set(), 'o': set()}
    for w in a.where or []:
        k, v = w.split(':')
        want[k] |= set(v.split(','))
    for lv in re.findall(r'<level>(.*?)</level>', s, re.S):
        rid = int(re.search(r'<roomID>(\d+)', lv).group(1))
        if not a.lo <= rid <= a.hi:
            continue
        title = re.search(r'<roomTitle>(.*?)<', lv).group(1)
        out, places = [], []
        for k, tag in (('e', 'enemyArray'), ('o', 'objectArray')):
            m = re.search(r'<%s>(.*?)</%s>' % (tag, tag), lv, re.S)
            items = m.group(1).split('|') if m and m.group(1) else []
            c = Counter(i.split(',')[0] for i in items)
            out.append(k + ': ' + ' '.join('%s%sx%d' % (t, '' if t in have[k] else '*', n)
                                          for t, n in sorted(c.items(), key=lambda x: int(x[0]))))
            places += ['  %s %s' % (k, i) for i in items if i.split(',')[0] in want[k]]
        print(rid, title, ' | '.join(out))
        print('\n'.join(places)) if places else None


def build():
    r = subprocess.run(['cmd', '/c', os.path.join('tools', 'build_windows.bat'), 'Debug'], cwd=ROOT,
                       capture_output=True, text=True, errors='replace')
    bad = [l for l in (r.stdout + r.stderr).splitlines() if re.search(r'error|warning C|FAILED', l)]
    print('build: %s' % ('OK' if r.returncode == 0 and not any('error' in l for l in bad) else 'FAILED'))
    for l in bad[:30]:
        print('  ' + l.strip()[:220])
    return r.returncode == 0


def dedup(seq):
    out = []
    for x in seq:
        if not out or out[-1][0] != x:
            out.append([x, 1])
        else:
            out[-1][1] += 1
    return ' '.join('%sx%d' % (x, n) if n > 1 else x for x, n in out)


def summarise(label, text, types):
    print('== %s' % label)
    sounds = Counter(re.findall(r'[\w-]+\.wav|\bjump\w*\b', ' '.join(l for l in text if l.startswith('S '))))
    print('  sounds: ' + (' '.join('%sx%d' % kv if kv[1] > 1 else kv[0] for kv in sounds.items()) or '-'))
    tl = [l for l in text if l.startswith('T ')]
    if tl:
        print('  first: ' + tl[0][:100])
        print('  last:  ' + tl[-1][:100])
        hp, prev = [], None
        for l in tl:
            m = re.search(r'^T (\d+) .* hp (\d+)', l)
            if m and m.group(2) != prev:
                hp.append('f%s=%s' % m.groups())
                prev = m.group(2)
        print('  hp: ' + ' '.join(hp))
    for kind in ('E', 'O'):
        tracks, first_frame = OrderedDict(), None
        for n, l in enumerate(x for x in text if x.startswith(kind + ' ')):
            for i, t, x, y, st in ITEM.findall(l):
                if t not in types:
                    continue
                d = tracks.setdefault(i, {'t': t, 'from': n, 'first': (x, y), 'states': []})
                d['last'] = (x, y)
                d['states'].append(st or '-')
        idle = []
        for i, d in tracks.items():
            if d['first'] == d['last'] and len(set(d['states'])) == 1 and d['from'] <= 2:
                idle.append('%s%s:%s' % (kind, i, d['t']))
                continue
            st = d['states']
            if kind == 'O' and len(set(st)) > 4:  # objects: animation frames, too many to list
                seq = 'frames %s, %d changes' % (' '.join(sorted(set(st), key=lambda f: int(f[1:]) if f[1:].isdigit() else 0)),
                                                 sum(1 for a, b in zip(st, st[1:]) if a != b))
            else:
                seq = dedup(st)
            print('  %s%s:%s %s,%s -> %s,%s%s  %s' % (kind, i, d['t'], *d['first'], *d['last'],
                                                     ' (spawned)' if d['from'] > 2 else '', seq[:160]))
        if idle:
            print('  idle: ' + ' '.join(idle))
    for kind in ('B', 'FX'):
        c = Counter()
        for l in text:
            if l.startswith(kind + ' '):
                c.update(set(t for _, t, *_ in ITEM.findall(l)))
        if c:
            print('  %s: %s' % (kind, ' '.join('%s(%d frames)' % kv for kv in c.items())))
    for l in text:
        if l.startswith('Capture'):
            print('  ' + l)


def cmd_run(a):
    if a.build and not build():
        return 1
    env = dict(os.environ, PL_SEED=os.environ.get('PL_SEED', '1'))
    for spec in a.s:
        label, args, types = (spec.split('|') + ['', ''])[:3]
        cmd = [EXE, '--data', DATA, '--mute', '--trace'] + args.split()
        r = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True, errors='replace')
        summarise(label, (r.stdout + r.stderr).splitlines(), set(t for t in types.split(',') if t))
    return 0


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest='cmd', required=True)
    r = sub.add_parser('rooms')
    r.add_argument('lo', type=int)
    r.add_argument('hi', type=int)
    r.add_argument('--where', nargs='*')
    r.set_defaults(f=cmd_rooms)
    u = sub.add_parser('run')
    u.add_argument('--build', action='store_true')
    u.add_argument('-s', action='append', default=[])
    u.set_defaults(f=cmd_run)
    a = p.parse_args()
    sys.exit(a.f(a) or 0)


if __name__ == '__main__':
    main()
